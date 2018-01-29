/*!
 *****************************************************************************
   @file:    adi_wifi_config.h

   @brief:   WiFi configuration 

   @details: Static configuraiton for WiFi is specified in this file
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

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF CLAIMS OF INTELLECTUAL
PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
/*****************************************************************************/


#ifndef ADI_WIFI_CONFIG_H_
#define ADI_WIFI_CONFIG_H_


#include <drivers/tmr/adi_tmr.h>
//#include <common/adi_error_handling.h>

#include <adi_error_handling.h>

/** @addtogroup adi_wifi_config Wi-Fi Software Configuration
 *  @ingroup WiFi
 *  @{
 *
 *  @brief   Configuration Macros
 *  @details Used to configure various elements of the WiFi software sources
 *  
 */


/*********************************** Wi-Fi Module Configuration ***************************************/

/*!
 *  @brief   Transport Layer Protocol.
 *
 *  @details Choose the underlying hardware implementation of the transport layer.
 *           - 0u : SPI2
 *           - 1u : UART0
 *
 */
#define ADI_WIFI_TAL_PROTOCOL_CFG       (1u)

#if (ADI_WIFI_TAL_PROTOCOL_CFG == 1u)
/*!
 *  @brief   UART device number.
 *
 *  @details Choose the UART device number that the Wi-Fi module will connect to.
 *
 */
#define ADI_UART_DEVICE_NUM_CFG			    (0u)
#elif
/*!
 *  @brief   SPI device number.
 *
 *  @details Choose the SPI device number that the Wi-Fi module will connect to.
 *
 */
#define ADI_SPI_DEVICE_NUM_CFG			    (2u)
#endif

/***************************************** NoOS Framework Configuration *********************************************/

/*! Choose the General Purpose Timer used by adi_wifi_noos.c. */
#define ADI_WIFI_NOOS_GP_TIMER_CFG                  (ADI_TMR_DEVICE_GP0)

/*! Broker Connection Timer. Noos layer will ping the broker when this timeout expires to prevent connection loss. */
#define ADI_WIFI_BROKER_CONNECTION_GP_TIMER_CFG     (ADI_TMR_DEVICE_GP1)
/* @} */

/* Macro validation */
#if (ADI_WIFI_NOOS_GP_TIMER != ADI_TMR_DEVICE_GP0) && (ADI_WIFI_NOOS_GP_TIMER != ADI_TMR_DEVICE_GP1) && (ADI_WIFI_NOOS_GP_TIMER != ADI_TMR_DEVICE_GP2)
#error "Please choose a valid General Purpose Timer for adi_wifi_noos.c to use"
#elif (ADI_WIFI_TAL_PROTOCOL_CFG != 1u)
#error "Only UART is supported at the moment."
#elif (ADI_WIFI_BROKER_CONNECTION_GP_TIMER_CFG != ADI_TMR_DEVICE_GP0) && (ADI_WIFI_BROKER_CONNECTION_GP_TIMER_CFG != ADI_TMR_DEVICE_GP1) && (ADI_WIFI_BROKER_CONNECTION_GP_TIMER_CFG != ADI_TMR_DEVICE_GP2)
#error "Please choose a valid General Purpose Timer for adi_wifi_noos.c to use"
#endif

#endif /* ADI_WIFI_CONFIG_H_ */
