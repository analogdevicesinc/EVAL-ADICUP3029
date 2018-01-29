/*! *****************************************************************************
 * @file:  adi_wifi_transport.c
 * @brief: Transport Abstraction Layer (TAL)
 -----------------------------------------------------------------------------
Copyright (c) 2017 Analog Devices, Inc.

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


/** @addtogroup adi_wifi_transport Transport Abstraction Layer
 *  @ingroup WiFi
 *  @{
 *
 *  @brief   Transport Abstraction Layer (TAL) API
 *  @details The set of functions in this module provide an abstraction layer
 *           between the radio (companion) module and the underlying hardware.
 */

/*! \cond PRIVATE */

#include <adi_wifi_transport.h>

/*! Transport layer static data. */
static ADI_WIFI_TAL_DATA  gTransportData;
static ADI_WIFI_TAL_DATA *pTransportData = &gTransportData;

/*! \endcond */


/*********************************************************************************
                                    API IMPLEMENTATIONS
*********************************************************************************/

/*!
 * @brief      Transport layer initialization.
 *
 * @details    Sets up the transaction driver (SPI or UART).
 *
 * @param [in] pEventCallback : Pointer to the companion module callback function.
 *
 * @return     ADI_WIFI_TAL_RESULT
 *                - #ADI_WIFI_TAL_SUCCESS           No error occurred.
 *                - #ADI_WIFI_TAL_FAILED_OPEN       Transport driver failed to open.
 *
 */
ADI_WIFI_TAL_RESULT adi_wifi_tal_Init(ADI_CALLBACK pEventCallback)
{
    ASSERT(pEventCallback != NULL);

#if(ADI_WIFI_TAL_PROTOCOL_CFG == 1u)

    if(adi_uart_Open(ADI_UART_DEVICE_NUM_CFG, ADI_TAL_UART_DIRECTION, pTransportData->aDeviceMemory, ADI_TAL_MEMORY,  &pTransportData->hDevice) == ADI_UART_SUCCESS)
    {
    	if(adi_uart_RegisterCallback(pTransportData->hDevice, pEventCallback) == ADI_UART_SUCCESS)
    	{
    		return(ADI_WIFI_TAL_SUCCESS);
    	}
    }

#endif

    return(ADI_WIFI_TAL_FAILED_OPEN);
}

/*!
 * @brief      Transport layer un-initialization.
 *
 * @details    Frees transport layer driver memory.
 *
 * @return     ADI_WIFI_TAL_RESULT
 *                - #ADI_WIFI_TAL_SUCCESS           No error occurred.
 *                - #ADI_WIFI_TAL_FAILED_CLOSE      Transport driver failed to close.
 *
 */
ADI_WIFI_TAL_RESULT adi_wifi_tal_Uninit()
{

#if(ADI_WIFI_TAL_PROTOCOL_CFG == 1u)

    if(adi_uart_Close(pTransportData->hDevice) == ADI_UART_SUCCESS)
    {
    	return(ADI_WIFI_TAL_SUCCESS);
    }

#endif

    return(ADI_WIFI_TAL_FAILED_CLOSE);
}

/*!
 * @brief       Transport layer write.
 *
 * @details     Writes nBufSize bytes from pBuf to the device. For the UART, this is a blocking call.
 *
 * @param [in]  pBuf     : Pointer to the data to write.
 *
 * @param [in]  nBufSize : Number of valid bytes in pBuf to write.
 *
 * @return      ADI_WIFI_TAL_RESULT
 *                 - #ADI_WIFI_TAL_SUCCESS            No error occurred.
 *                 - #ADI_WIFI_TAL_FAILED_TRANSACTION	Write transaction failed.
 */
ADI_WIFI_TAL_RESULT adi_wifi_tal_Write(uint8_t * const pBuf, const uint32_t nBufSize)
{
	uint32_t nHardwareError = 0u;

#if (ADI_WIFI_TAL_PROTOCOL_CFG == 1u)

	if(adi_uart_Write(pTransportData->hDevice, pBuf, nBufSize, &nHardwareError) == ADI_UART_SUCCESS)
	{
		return(ADI_WIFI_TAL_SUCCESS);
	}

#endif

	return(ADI_WIFI_TAL_FAILED_TRANSACTION);
}

/*!
 * @brief       Transport layer blocking read.
 *
 * @details     Reads nNumBytes byte from the device to pBuf.
 *
 * @param [in]  pBuf 	    : Pointer to the destination buffer.
 * @param [in]  nNumBytes : Number of bytes to read.
 *
 *
 * @return      ADI_WIFI_TAL_RESULT
 *                 - #ADI_WIFI_TAL_SUCCESS            No error occurred.
 *                 - #ADI_WIFI_TAL_FAILED_TRANSACTION Read transaction failed.
 *
 */
ADI_WIFI_TAL_RESULT adi_wifi_tal_Read(uint8_t * const pBuf, uint32_t nNumBytes)
{
	uint32_t nHardwareError = 0u;
	volatile uint32_t nCounter;
  
#if (ADI_WIFI_TAL_PROTOCOL_CFG == 1u)

	for(nCounter = 0; nCounter < nNumBytes; nCounter++)
	{
		if(adi_uart_ReadChar(pTransportData->hDevice, &pBuf[nCounter], &nHardwareError) != ADI_UART_SUCCESS)
		{
			return(ADI_WIFI_TAL_FAILED_TRANSACTION);
		}
	}
#endif

	return(ADI_WIFI_TAL_SUCCESS);
}

#if (ADI_WIFI_TAL_PROTOCOL_CFG == 1u)
/*!
 * @brief        Transport layer function to configure the baudrate for the UART.
 *
 * @details      Baudrate is calculated as per below equation.
 *
 *               Baudrate = (UARTCLK / (nDivM + nDivN/2048)*pow(2,nOSR+2)* nDivC)).
 *
 * @param [in]  nDivC        Specify the "nDivC" in the above equation.
 * @param [in]  nDivM        Specify the "nDivM" in the above equation.
 * @param [in]  nDivN        Specify the "nDivN" in the above equation.
 * @param [in]  nOSR         Specify the "nOSR" " in the above equation.
 *
 * @return      ADI_WIFI_TAL_RESULT
 *                 - #ADI_WIFI_TAL_SUCCESS            		     No error occurred.
 *                 - #ADI_WIFI_TAL_FAILED_CONFIGURE_BAUDRATE   Failed to configure the baudrate.
 *
 */
ADI_WIFI_TAL_RESULT adi_wifi_tal_ConfigBaudRate(uint16_t const nDivC, uint8_t  const nDivM, uint16_t const nDivN, uint8_t  const nOSR)
{
	if(adi_uart_ConfigBaudRate(pTransportData->hDevice, nDivC, nDivM, nDivN, nOSR) == ADI_UART_SUCCESS)
	{
		return(ADI_WIFI_TAL_SUCCESS);
	}

	return(ADI_WIFI_TAL_FAILED_CONFIGURE_BAUDRATE);
}

/*!
 * @brief        Configuration of UART data.
 *
 * @details      Sets the configuration parameters for the specified UART device such as word length, whether to
 *               enable/disable the parity, and the number of stop bits.
 *
 * @param [in]  eParity      Specify the type of parity check for the UART device.
 * @param [in]  eStopBits    Specify the stop-bits for the UART device.
 * @param [in]  eWordLength  Specify the word size of the data for the UART device.
 *
 * @return      ADI_WIFI_TAL_RESULT
 *                 - #ADI_WIFI_TAL_SUCCESS            		     No error occurred.
 *                 - #ADI_WIFI_TAL_FAILED_CONFIGURE_DATA       Failed to configure the data.
 *
 */
ADI_WIFI_TAL_RESULT adi_wifi_tal_SetConfiguration(ADI_UART_PARITY const  eParity, ADI_UART_STOPBITS const  eStopBits, ADI_UART_WORDLEN  const  eWordLength)
{
	if(adi_uart_SetConfiguration(pTransportData->hDevice, eParity, eStopBits, eWordLength) == ADI_UART_SUCCESS)
	{
		return(ADI_WIFI_TAL_SUCCESS);
	}

	return(ADI_WIFI_TAL_FAILED_CONFIGURE_DATA);
}

/*!
 * @brief        Flush Rx channel.
 *
 * @return      ADI_WIFI_TAL_RESULT
 *                 - #ADI_WIFI_TAL_SUCCESS            		     No error occurred.
 *                 - #ADI_WIFI_TAL_FAILED_FLUSH_CHANNEL        Failed to flush the channel.
 *
 */
ADI_WIFI_TAL_RESULT adi_wifi_tal_FlushRxChannel(void)
{
	if(adi_uart_FlushRxChannel(pTransportData->hDevice) == ADI_UART_SUCCESS)
	{
		return(ADI_WIFI_TAL_SUCCESS);
	}

	return(ADI_WIFI_TAL_FAILED_FLUSH_CHANNEL);
}
#endif

/* @} */
