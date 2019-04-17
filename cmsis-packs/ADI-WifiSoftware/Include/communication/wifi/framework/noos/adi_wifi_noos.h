/*!
 *****************************************************************************
   @file:    adi_wifi_noos.h
   @brief:   Header file for adi_wifi_noos.c
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

*****************************************************************************/


/** @addtogroup adi_wifi_noos Non-RTOS Framework Layer
 *  @{
 */


#ifndef ADI_WIFI_NOOS
#define ADI_WIFI_NOOS

#include <stdint.h>
#include <wifi/radio/adi_wifi.h>

/*! Timeout value for #WIFI_EVENT_DISPATCH in #adi_wifi_Init, note that this is just an upper limit. */
#define ADI_WIFI_INIT_CORE_INIT_RSP_TIME_MS (5000u)


/*********************************************************************************
                                    PUBLIC APIs 
*********************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Public functions */
ADI_WIFI_RESULT adi_wifi_Init(ADI_CALLBACK const pCallbackFunc, void * const pCBParam);
ADI_WIFI_RESULT adi_wifi_DispatchEvents(const uint32_t nTimeoutMilliSeconds);
ADI_WIFI_RESULT adi_wifi_WaitForResponseWithTimeout(const uint32_t nTimeoutMilliSeconds, ADI_WIFI_AT_CMDCODE eCommandType);

/*********************************************************************************
                                   PRIVATE FUNCTIONS
*********************************************************************************/
/*! \cond PRIVATE */


/*! Local prototypes */
void    			       adi_wifi_TimerCallback(void * pCBParam, uint32_t Event, void * pArg);
void                 adi_wifi_PingTimerCallback(void * pCBParam, uint32_t Event, void * pArg);
uint8_t              adi_wifi_IsTimerDone(uint8_t nIsPingTimer);
ADI_WIFI_RESULT 	   adi_wifi_StartTimer(const uint32_t nMilliSeconds, uint8_t nIsPingTimer);
ADI_WIFI_RESULT 	   adi_wifi_StopTimer(uint8_t nIsPingTimer);
void    			       adi_wifi_NoosCallback(void * pCBParam, uint32_t Event, void * pArg);
uint8_t    			     adi_wifi_IsEventPending(void);
void    			       adi_wifi_ClearEventPending(void);
void 				         adi_wifi_ClearHardwareErrorPending(void);
uint32_t    		     adi_wifi_IsHardwareErrorPending(void);

/*! \endcond */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ADI_WIFI_NOOS */


/* @} */
