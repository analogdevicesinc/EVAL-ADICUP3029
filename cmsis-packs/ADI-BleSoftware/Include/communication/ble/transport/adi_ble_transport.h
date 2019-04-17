/*! *****************************************************************************
 * @file:  adi_ble_transport.h
 * @brief: Header file for adi_ble_transport.c
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
 *  @{
 */


#ifndef ADI_BLE_TRANSPORT_H
#define ADI_BLE_TRANSPORT_H


/*! \cond PRIVATE */


#include <stdint.h>
#include <adi_callback.h>
#include <adi_processor.h>


/*! \endcond */


/*! SPI device number */
#define SPI_DEV_NUM   (2u)

/*! CS signal (active low) port */
#define SPI_CS_PORT   (ADI_GPIO_PORT1)

/*! CS signal (active low) pin */
#define SPI_CS_PIN    (ADI_GPIO_PIN_5)

/*! RDY signal (active high) port */
#define SPI_RDY_PORT  (ADI_GPIO_PORT0)

/*! RDY signal (active high) pin */
#define SPI_RDY_PIN   (ADI_GPIO_PIN_3)

/* For COG boards (EV-COG-AD3029 & EV-COG-AD4050) */
#if defined(__EVCOG__)

/*! EN signal (active high) port */
#define SPI_EN_PORT   (ADI_GPIO_PORT1)

/*! EN signal (active high) pin */
#define SPI_EN_PIN    (ADI_GPIO_PIN_11)

/* For CUP boards (ADICUP3029) */
#else  

/*! EN signal (active high) port */
#define SPI_EN_PORT   (ADI_GPIO_PORT2)

/*! EN signal (active high) pin */
#define SPI_EN_PIN    (ADI_GPIO_PIN_9)

#endif /* __EVCOG__ */


/*!
 *  @brief   Transport Layer Protocol
 *
 *  @details Choose the underlying hardware implementation of the transport layer
 *           - 0u : SPI2
 *           - 1u : UART0
 *           
 */
#define ADI_BLE_TRANSPORT_CFG_PROTOCOL (0u)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 *
 * @enum    ADI_BLE_TRANSPORT_RESULT
 *
 * @details The return value of all TAL APIs returning #ADI_BLE_TRANSPORT_RESULT should
 *          always be tested at the application level for success or failure. 
 *
 */
typedef enum 
{
    /*! Function call completed successfully */
    ADI_BLE_TRANSPORT_SUCCESS,
    /*! GPIO driver failed */
    ADI_BLE_TRANSPORT_FAILED_GPIO,
    /*! Transport layer failed on initialization */
    ADI_BLE_TRANSPORT_FAILED_OPEN,
    /*! Transport layer failed on uninitialization */
    ADI_BLE_TRANSPORT_FAILED_CLOSE,
    /*! Transport layer failed on a data transaction */
    ADI_BLE_TRANSPORT_FAILED_TRANSACTION,
    /*! Transport layer failed due to invalid data size */
    ADI_BLE_TRANSPORT_INVALID_DATA_SIZE,
    /*! Transport layer failed waiting for the RDY signal to go high */
    ADI_BLE_TRANSPORT_RDY_TIMEOUT,
} ADI_BLE_TRANSPORT_RESULT;


/* Public APIs */
ADI_BLE_TRANSPORT_RESULT adi_tal_Init  (ADI_CALLBACK pEventCallback);
ADI_BLE_TRANSPORT_RESULT adi_tal_Write (void * pBuf, const uint32_t nBufSize);
ADI_BLE_TRANSPORT_RESULT adi_tal_Read  (void * pBuf, const uint32_t nBufSize, uint8_t * nBytes);
ADI_BLE_TRANSPORT_RESULT adi_tal_Uninit(void);


/*! \cond PRIVATE */


/*! Configuration macro validation */
#if ADI_BLE_TRANSPORT_CFG_PROTOCOL != 0u
#error "Only SPI is supported at the moment."
#endif

/*! \endcond */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ADI_BLE_TRANSPORT_H */


/* @} */
