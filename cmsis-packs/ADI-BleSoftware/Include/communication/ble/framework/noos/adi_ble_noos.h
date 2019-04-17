/*!
 *****************************************************************************
   @file:    adi_ble_noos.h
   @brief:   Header file for adi_ble_noos.c
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

*****************************************************************************/


/** @addtogroup adi_ble_noos Non-RTOS Framework Layer
 *  @{
 */


#ifndef ADI_BLE_NOOS
#define ADI_BLE_NOOS


#include <stdint.h>
#include <radio/adi_ble_radio.h>


/*********************************************************************************
                                    PUBLIC APIs 
*********************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

ADI_BLER_RESULT adi_ble_Init                      (ADI_CALLBACK const pCallbackFunc, void * const pCBParam);
ADI_BLER_RESULT adi_ble_DispatchEvents            (const uint32_t nTimeoutMilliSeconds);
ADI_BLER_RESULT adi_ble_WaitForEventWithTimeout   (const ADI_BLER_EVENT eDesiredEvent, const uint32_t nTimeoutMilliSeconds);

/*********************************************************************************
                                   PRIVATE FUNCTIONS
*********************************************************************************/


/*! \cond PRIVATE */


ADI_BLER_RESULT adi_ble_WaitForResponseWithTimeout(const uint32_t nEventBits, const uint32_t nTimeoutMilliSeconds);


/*! \endcond */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ADI_BLE_NOOS */


/* @} */
