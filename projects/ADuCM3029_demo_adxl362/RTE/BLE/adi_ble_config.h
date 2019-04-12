/*!
 *****************************************************************************
   @file:    adi_ble_config.h

   @brief:   Bluetooth Framework Configuration

   @details: In a non-RTOS application, this file is just used to specify a MAC
             address. In an RTOS application, we choose the active profiles
             and the task settings here. This file is copied to each project 
             to allow configuration.
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


#ifndef ADI_BLE_CONFIG_H_
#define ADI_BLE_CONFIG_H_


#include <drivers/tmr/adi_tmr.h>


/** @addtogroup adi_ble_config Bluetooth Software Configuration
 *  @ingroup ble
 *  @{
 *
 *  @brief   Configuration Macros
 *  @details Used to configure various elements of the Bluetooth software sources.
 *  
 */


/*********************************** NoOS Framework Configuration ***************************************/

/*! Choose the General Purpose Timer used by adi_ble_noos.c */
#define ADI_BLE_NOOS_GP_TIMER                  (ADI_TMR_DEVICE_GP0)

/************************************** OTP Configurable ************************************************/

/*! Maximum number of device connection (radio specific) */
#define ADI_BLE_MAX_CONNECTION                 (2u)   

/*! Maximum size for the data exchange data packet */
#define DATAEXCHANGE_PACKET_SIZE               (20u)

/* @} */


/* Macro validation */
#if (ADI_BLE_NOOS_GP_TIMER != ADI_TMR_DEVICE_GP0) && (ADI_BLE_NOOS_GP_TIMER != ADI_TMR_DEVICE_GP1) && (ADI_BLE_NOOS_GP_TIMER != ADI_TMR_DEVICE_GP2)
#error "Please choose a valid General Purpose Timer for adi_ble_noos.c to use"
#endif


#endif /* ADI_BLE_CONFIG_H_ */
