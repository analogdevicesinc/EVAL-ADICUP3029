/*! *****************************************************************************
 * @file:  adi_ble_transport.c
 * @brief: Transport Abstraction Layer (TAL)
 -----------------------------------------------------------------------------
Copyright (c) 2016 Analog Devices, Inc.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Modified versions of the software must be conspicuously marked as such.
  - This software is licensed solely and exclusively for use with processors
    manufactured by or for Analog Devices, Inc.
  - This software may not be combined or merged with other code in any manner
    that would cause the software to become subject to terms and conditions
    which differ from those listed here.
  - Neither the name of Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.
  - The use of this software may or may not infringe the patent rights of one
    or more patent holders.  This license does not release you from the
    requirement that you obtain separate licenses from these patent holders
    to use this software.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-
INFRINGEMENT, TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF
CLAIMS OF INTELLECTUAL PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/


/** @addtogroup adi_ble_transport Transport Abstraction Layer
 *  @ingroup ble
 *  @{
 *
 *  @brief   Transport Abstraction Layer (TAL) API
 *  @details The set of functions in this module provide an abstraction layer
 *           between the radio (companion) module and the underlying hardware.
 */


 /*! \cond PRIVATE */


#include <stdlib.h>
#include <stdint.h>
#include <drivers/spi/adi_spi.h>
#include <drivers/gpio/adi_gpio.h>
#include <common/adi_error_handling.h>
#include <transport/adi_ble_transport.h>


/*! Radio internal buffer length */
#define ADI_RADIO_BUFFER_BYTES (64u)


typedef struct
{
    ADI_SPI_HANDLE hDevice;
    uint8_t        aDeviceMemory  [ADI_SPI_MEMORY_SIZE];
    uint8_t        aCallbackMemory[ADI_GPIO_MEMORY_SIZE];
    uint8_t        aFlushBuffer   [ADI_RADIO_BUFFER_BYTES];
    ADI_CALLBACK   pEventCallback;
} ADI_BLE_TAL_DATA;


/*! Command bytes */
#define SPI_READ_CMD  (0x81u)
#define SPI_WRITE_CMD (0x42u)


/*! Transport layer static data */
static ADI_BLE_TAL_DATA  gTransportData;
static ADI_BLE_TAL_DATA *pTransportData = &gTransportData;
static ADI_SPI_TRANSCEIVER sTransceive = {.nTxIncrement     = 1u,
										  .nRxIncrement     = 1u,
										  .bRD_CTL          = false,
										  .bDMA             = false };


/*! Local prototypes */
void                            TransportCallback(void * pCBParam, uint32_t Event, void * pArg);
static ADI_BLE_TRANSPORT_RESULT RadioTransaction (const bool bWrite, uint8_t * pBuf, const uint32_t nBufSize, uint8_t * nValidBytes);


/*! \endcond */


/*********************************************************************************
                                    API IMPLEMENTATIONS
*********************************************************************************/


/*!
 * @brief      Transport Layer Initialization
 *
 * @details    Sets up external interrupt and transaction driver (SPI or UART)
 *
 * @param [in] pEventCallback : Pointer to the companion module callback function 
 *
 * @return     ADI_BLE_TRANSPORT_RESULT
 *                - #ADI_BLE_TRANSPORT_SUCCESS           No error occurred
 *                - #ADI_BLE_TRANSPORT_FAILED_GPIO       GPIO driver failed
 *                - #ADI_BLE_TRANSPORT_FAILED_OPEN       Transport driver failed
 *
 */
ADI_BLE_TRANSPORT_RESULT adi_tal_Init(ADI_CALLBACK pEventCallback)
{
    ADI_BLE_TRANSPORT_RESULT eTransportResult;
    ADI_GPIO_RESULT          eGpioResult;
    ADI_SPI_RESULT           eSpiResult;
    uint8_t                  nBytes;

    ASSERT(pEventCallback != NULL);

    /* Register the callback */
    pTransportData->pEventCallback = pEventCallback;

    /* Initialize GPIO driver */
    eGpioResult = adi_gpio_Init(pTransportData->aCallbackMemory, ADI_GPIO_MEMORY_SIZE);
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);

    /* Initialize SPI driver - configuration done statically in adi_spi_config.h */
    eSpiResult = adi_spi_Open(SPI_DEV_NUM, pTransportData->aDeviceMemory, ADI_SPI_MEMORY_SIZE, &pTransportData->hDevice);
    RETURN_ERROR(eSpiResult, ADI_SPI_SUCCESS, ADI_BLE_TRANSPORT_FAILED_OPEN);
    eSpiResult = adi_spi_SetChipSelect(pTransportData->hDevice, ADI_SPI_CS_NONE);
    RETURN_ERROR(eSpiResult, ADI_SPI_SUCCESS, ADI_BLE_TRANSPORT_FAILED_OPEN);

    /* Set up external RDY interrupt for a local callback function  */
    eGpioResult = adi_gpio_InputEnable(SPI_RDY_PORT, SPI_RDY_PIN, true);
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);
    eGpioResult = adi_gpio_SetGroupInterruptPolarity(SPI_RDY_PORT, SPI_RDY_PIN);
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);
    eGpioResult = adi_gpio_SetGroupInterruptPins(SPI_RDY_PORT, ADI_GPIO_INTB_IRQ, SPI_RDY_PIN);
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);
    eGpioResult = adi_gpio_RegisterCallback(ADI_GPIO_INTB_IRQ, TransportCallback, NULL);
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);

    /* Set up the chip select signal to be driven by software */
    eGpioResult = adi_gpio_OutputEnable(SPI_CS_PORT, SPI_CS_PIN, true);
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);
    eGpioResult = adi_gpio_SetHigh(SPI_CS_PORT, SPI_CS_PIN);
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);

    /* Set up the enable signal */
    eGpioResult = adi_gpio_OutputEnable(SPI_EN_PORT, SPI_EN_PIN, true);
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);

    /* We explicitly disable radio as we do not know the state and enable again */
    eGpioResult = adi_gpio_SetLow(SPI_EN_PORT, SPI_EN_PIN);
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);

    /* FIXME: Add delay to give radio time to start up after being enabled */
    for(volatile uint32_t i = 0x0ul; i < 0xFFFFul; i++);

    eGpioResult = adi_gpio_SetHigh(SPI_EN_PORT, SPI_EN_PIN);
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);

    /* FIXME: Add delay to give radio time to start up after being enabled */
    for(volatile uint32_t i = 0x0ul; i < 0xFFFFul; i++);

    /* Flush the radio transmit buffer */
    eTransportResult = adi_tal_Read(pTransportData->aFlushBuffer, ADI_RADIO_BUFFER_BYTES, &nBytes);

    return (eTransportResult);
}


/*!
 * @brief       Transport Layer Write
 *
 * @details     Writes nBufSize bytes from pBuf to the device
 *
 * @param [in]  pBuf : Pointer to the data to write
 *
 * @param [in]  nBufSize : Number of valid bytes in pBuf to write
 *
 * @return      ADI_BLE_TRANSPORT_RESULT
 *                 - #ADI_BLE_TRANSPORT_SUCCESS            No error occurred
 *                 - #ADI_BLE_TRANSPORT_RDY_TIMEOUT        Timed out waiting for RDY signal
 *                 - #ADI_BLE_TRANSPORT_INVALID_DATA_SIZE  Data either overflowed or underflowed buffer so no trasnaction occured
 *                 - #ADI_BLE_TRANSPORT_FAILED_GPIO        GPIO Failed 
 *                 - #ADI_BLE_TRANSPORT_FAILED_TRANSACTION SPI transaction failed
 
 */
ADI_BLE_TRANSPORT_RESULT adi_tal_Write(void * pBuf, const uint32_t nBufSize)
{
    ADI_BLE_TRANSPORT_RESULT eTransportResult;
    uint8_t                  nBytes;

    ASSERT(pBuf != NULL);
    ASSERT(nBufSize > 0u);
    ASSERT(nBufSize <= ADI_RADIO_BUFFER_BYTES);

    /* Blocking call */
    eTransportResult = RadioTransaction(true, pBuf, nBufSize, &nBytes);
    RETURN_ERROR(nBytes, nBufSize, ADI_BLE_TRANSPORT_FAILED_TRANSACTION);

    return (eTransportResult);
}


/*!
 * @brief       Transport Layer Read
 *
 * @details     Reads nBufSize bytes tp pBuf from the device
 *
 * @param [in]  pBuf : Pointer to the destination buffer
 *
 * @param [in]  nBufSize : Number of valid bytes in pBuf to read
 *
 * @param [out] nBytes : Number of valid bytes read into pBuf. May be less than nBufSize but not more.
 *
 * @return      ADI_BLE_TRANSPORT_RESULT
 *                 - #ADI_BLE_TRANSPORT_SUCCESS            No error occurred
 *                 - #ADI_BLE_TRANSPORT_RDY_TIMEOUT        Timed out waiting for RDY signal
 *                 - #ADI_BLE_TRANSPORT_INVALID_DATA_SIZE  Data either overflowed or underflowed buffer so no trasnaction occured
 *                 - #ADI_BLE_TRANSPORT_FAILED_GPIO        GPIO Failed 
 *                 - #ADI_BLE_TRANSPORT_FAILED_TRANSACTION SPI transaction failed
 *
 */
ADI_BLE_TRANSPORT_RESULT adi_tal_Read(void * pBuf, const uint32_t nBufSize, uint8_t * nBytes)
{
    ADI_BLE_TRANSPORT_RESULT eTransportResult;

    ASSERT(nBytes != NULL);
    ASSERT(pBuf != NULL);
    ASSERT(nBufSize > 0u);
    ASSERT(nBufSize <= ADI_RADIO_BUFFER_BYTES);

    /* Blocking call */
    eTransportResult = RadioTransaction(false, pBuf, nBufSize, nBytes);

    return (eTransportResult);
}


/*!
 * @brief      Transport Layer Uninitialization
 *
 * @details    Free static memory previously allocated to the transport layer
 *
 * @return     ADI_BLE_TRANSPORT_RESULT
 *                - #ADI_BLE_TRANSPORT_SUCCESS      No error occurred
 *                - #ADI_BLE_TRANSPORT_FAILED_CLOSE Transport driver failed
 *                - #ADI_BLE_TRANSPORT_FAILED_GPIO  GPIO driver failed
 *
 */
ADI_BLE_TRANSPORT_RESULT adi_tal_Uninit(void)
{
    ADI_GPIO_RESULT eGpioResult;
    ADI_SPI_RESULT  eSpiResult;

    eSpiResult = adi_spi_Close(pTransportData->hDevice);
    RETURN_ERROR(eSpiResult, ADI_SPI_SUCCESS, ADI_BLE_TRANSPORT_FAILED_CLOSE);

    eGpioResult = adi_gpio_UnInit();
    RETURN_ERROR(eGpioResult, ADI_GPIO_SUCCESS, ADI_BLE_TRANSPORT_FAILED_GPIO);

    return (ADI_BLE_TRANSPORT_SUCCESS);
}


/*********************************************************************************
                                   PRIVATE FUNCTIONS
*********************************************************************************/


/*! \cond PRIVATE */


/*!
 * @brief      Local Callback Function
 *
 * @details    This function is called when RDY changes from low to high, this means
 *             an event has occurred
 *
 */
void TransportCallback(void * pCBParam, uint32_t Event, void * pArg)
{
    pTransportData->pEventCallback(pCBParam, Event, pArg);
}


/*!
 * @brief       SPI Radio Transaction
 *
 * @details     The radio has a unique SPI protocol that requires CS/RDY sync
 *              and a two byte status transfer that must be done before the 
 *              normal transfer
 *
 * @param [in]  bWrite : True for write, false for read
 *
 * @param [in]  pBuf : Pointer to the destination buffer
 *
 * @param [in]  nBufSize : Number of bytes to read
 *
 * @param [out] nValidBytes : Number of valid bytes written or read
 *
 * @return      ADI_BLE_TRANSPORT_RESULT
 *                 - #ADI_BLE_TRANSPORT_SUCCESS            No error occurred
 *                 - #ADI_BLE_TRANSPORT_RDY_TIMEOUT        Timed out waiting for RDY signal
 *                 - #ADI_BLE_TRANSPORT_INVALID_DATA_SIZE  Data either overflowed or underflowed buffer so no trasnaction occured
 *                 - #ADI_BLE_TRANSPORT_FAILED_GPIO        GPIO Failed 
 *                 - #ADI_BLE_TRANSPORT_FAILED_TRANSACTION SPI transaction failed
 *
 * @note        Assumes the calling function verifies the input parameters
 *
 */
static ADI_BLE_TRANSPORT_RESULT RadioTransaction(const bool bWrite, uint8_t * pBuf, const uint32_t nBufSize, uint8_t * nValidBytes)
{
    ADI_BLE_TRANSPORT_RESULT nReturnVal =  ADI_BLE_TRANSPORT_SUCCESS;
    uint32_t                 nCounter = 0x0ul;
    uint16_t                 nData;
    uint8_t                  aTxBuffer[2u];
    uint8_t                  aRxBuffer[2u];

    /* Disable interrupt */
    NVIC_ClearPendingIRQ(SYS_GPIO_INTB_IRQn);
    NVIC_DisableIRQ(SYS_GPIO_INTB_IRQn);

    /* Lower CS */
    if (adi_gpio_SetLow(SPI_CS_PORT, SPI_CS_PIN) != ADI_GPIO_SUCCESS)
    {
        /* Perform clean-up before exiting */
        NVIC_ClearPendingIRQ(SYS_GPIO_INTB_IRQn);
        NVIC_EnableIRQ(SYS_GPIO_INTB_IRQn);

        /* Safe to return here since no need to raise GPIO if it never lowered */
        return ADI_BLE_TRANSPORT_FAILED_GPIO;
    }

    /* Fake while loop to allow breaking on error in order to raise GPIO before returning */
    do
    {
        /* Wait for RDY high */
        do
        {
            if(adi_gpio_GetData(SPI_RDY_PORT, SPI_RDY_PIN, &nData) != ADI_GPIO_SUCCESS)
            {
                nReturnVal = ADI_BLE_TRANSPORT_FAILED_GPIO;
                break;
            }

            nCounter++;

        } while((nData == 0u) && (nCounter < 100000u));

        /* Enable interrupt */
        NVIC_ClearPendingIRQ(SYS_GPIO_INTB_IRQn);
        NVIC_EnableIRQ(SYS_GPIO_INTB_IRQn);

        if(nReturnVal != ADI_BLE_TRANSPORT_SUCCESS)
        {
            break;
        }

        if(nCounter >= 100000u)
        {
            nReturnVal = ADI_BLE_TRANSPORT_RDY_TIMEOUT;
            break;
        }

        /* Pack a 2 byte SPI transfer to send the command and read status register */
        sTransceive.TransmitterBytes = 2u;
        sTransceive.ReceiverBytes    = 2u;
        sTransceive.pTransmitter     = aTxBuffer;
        sTransceive.pReceiver        = aRxBuffer;

        aTxBuffer[0u] = (bWrite == true) ? SPI_WRITE_CMD : SPI_READ_CMD;
        aTxBuffer[1u] = 0x00u;
        aRxBuffer[0u] = 0x00u;
        aRxBuffer[1u] = 0x00u;

        /* Blocking call */
        if(adi_spi_MasterReadWrite(pTransportData->hDevice, &sTransceive) != ADI_SPI_SUCCESS)
        {
            nReturnVal = ADI_BLE_TRANSPORT_FAILED_TRANSACTION;
            break;
        }

        /* See how many bytes the radio has in its buffer */
        *nValidBytes = aRxBuffer[1u];

        /* Must not overflow/underflow the radio buffers */
        if (bWrite == true)
        {
            /* Make sure there is enough room in the radio receive buffer */
            if (*nValidBytes < nBufSize)
            {
                nReturnVal = ADI_BLE_TRANSPORT_INVALID_DATA_SIZE;
                break;
            }
            else
            {
                *nValidBytes = nBufSize;
            }

            sTransceive.TransmitterBytes = *nValidBytes;
            sTransceive.ReceiverBytes    = 0u;
            sTransceive.pTransmitter     = pBuf;
            sTransceive.pReceiver        = NULL;
        }
        else
        {
            /* Ensure read will fit in provided buffer, and it isn't 0 which will hang the SPI driver */
            if (*nValidBytes > nBufSize)
            {
                nReturnVal = ADI_BLE_TRANSPORT_INVALID_DATA_SIZE;
                break;

            }

            sTransceive.TransmitterBytes = 0u;
            sTransceive.ReceiverBytes    = *nValidBytes;
            sTransceive.pTransmitter     = NULL;
            sTransceive.pReceiver        = pBuf;
        }

        /* Blocking call */
        if(*nValidBytes != 0)
        {
            if(adi_spi_MasterReadWrite(pTransportData->hDevice, &sTransceive) != ADI_SPI_SUCCESS)
            {
                nReturnVal = ADI_BLE_TRANSPORT_FAILED_TRANSACTION;
                break;
            }
        }

    } while(0);

    /* Raise CS */
    if (adi_gpio_SetHigh(SPI_CS_PORT, SPI_CS_PIN) != ADI_GPIO_SUCCESS)
    {
        nReturnVal = ADI_BLE_TRANSPORT_FAILED_GPIO;
    }

    return (nReturnVal);
}


/*! \endcond */


/* @} */
