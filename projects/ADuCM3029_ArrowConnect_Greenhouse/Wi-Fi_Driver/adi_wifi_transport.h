/*! *****************************************************************************
 * @file:  adi_wifi_transport.h
 * @brief: Header file for adi_wifi_transport.c
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
 *
 *  @{
 */

#ifndef ADI_WIFI_TAL_H
#define ADI_WIFI_TAL_H

/*! @cond PRIVATE */
#include <stdint.h>
#include <stdlib.h>
#include <adi_error_handling.h>
#include <adi_wifi_config.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if (ADI_WIFI_TAL_PROTOCOL_CFG == 1u)
#include <adi_uart.h>
/*! UART device memory */
#define ADI_TAL_MEMORY 			       (ADI_UART_BIDIR_MEMORY_SIZE)
/*! UART device direction */
#define ADI_TAL_UART_DIRECTION		 (ADI_UART_DIR_BIDIRECTION)

#else
#include <drivers/spi/adi_spi.h>
/*! SPI device memory */
#define ADI_TAL_MEMORY 			       (ADI_SPI_MEMORY_SIZE)
#endif
/*! @endcond */

/*!
 *  @struct ADI_WIFI_TAL_DATA
 *
 *  @brief  Data structure for the Wi-Fi transport layer driver.
 *
 */
typedef struct
{
#if (ADI_WIFI_TAL_PROTOCOL_CFG == 1u)
    ADI_UART_HANDLE hDevice;							            /*!< Handle for the UART driver.				  */
#else
    ADI_SPI_HANDLE  hDevice;								          /*!< Handle for the SPI driver. 			  	*/
#endif
    uint8_t         aDeviceMemory[ADI_TAL_MEMORY];		/*!< Memory for the device driver. 				*/

} ADI_WIFI_TAL_DATA;


/*!
 * @enum    ADI_WIFI_TAL_RESULT
 *
 * @details The return value of all transport layer APIs returning #ADI_WIFI_TAL_RESULT 
 *          should always be tested at the application level for success or failure. 
 *
 */
typedef enum 
{ 
    ADI_WIFI_TAL_SUCCESS,						         /*!< Function call completed successfully.					         */
    ADI_WIFI_TAL_FAILED_OPEN,    				     /*!< Transport layer failed on initialization.				       */
    ADI_WIFI_TAL_FAILED_CLOSE,    			     /*!< Transport layer failed on uninitialization.			       */
    ADI_WIFI_TAL_FAILED_TRANSACTION,   		   /*!< Transport layer failed on a data transaction.			     */
    ADI_WIFI_TAL_FAILED_CONFIGURE_BAUDRATE,  /*!< Transport layer failed to configure the baudrate.		   */
	  ADI_WIFI_TAL_FAILED_CONFIGURE_DATA, 		 /*!< Transport layer failed to configure the data format.   */
	  ADI_WIFI_TAL_FAILED_FLUSH_CHANNEL,       /*!< Transport layer failed to flush rx channel.	           */

} ADI_WIFI_TAL_RESULT;


/* Public APIs */
ADI_WIFI_TAL_RESULT adi_wifi_tal_Init(ADI_CALLBACK pEventCallback);
ADI_WIFI_TAL_RESULT adi_wifi_tal_Uninit(void);
ADI_WIFI_TAL_RESULT adi_wifi_tal_Write(uint8_t * const pBuf, const uint32_t nBufSize);
ADI_WIFI_TAL_RESULT adi_wifi_tal_Read(uint8_t * const pBuf, uint32_t nNumBytes);
#if (ADI_WIFI_TAL_PROTOCOL_CFG == 1u)
ADI_WIFI_TAL_RESULT adi_wifi_tal_ConfigBaudRate(uint16_t const nDivC, uint8_t  const nDivM, uint16_t const nDivN, uint8_t  const nOSR);
ADI_WIFI_TAL_RESULT adi_wifi_tal_SetConfiguration(ADI_UART_PARITY const  eParity, ADI_UART_STOPBITS const  eStopBits, ADI_UART_WORDLEN  const  eWordLength);
ADI_WIFI_TAL_RESULT adi_wifi_tal_FlushRxChannel(void);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ADI_WIFI_TAL_H */

/* @} */
