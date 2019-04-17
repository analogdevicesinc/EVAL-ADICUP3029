/*!
 *****************************************************************************
   @file:    adi_ble_logevent.c

   @brief:   Log event functions are implemented here. These functions keep track
             of logged events.

   @details:
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
#if (ADI_CFG_BLE_LOGEVENT == 1u)

/* disable misra diagnostics as necessary */
#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_5_1:"Identifiers shall not rely on the significance of more than 31 characters")
#endif /* _MISRA_RULES */

/** 
 *  @defgroup  ble_logevent
 */

/** 
 *  @addtogroup  adi_ble_logevent
 *  @ingroup ble_logevent
 *  @{
 */
#include <radio/adi_ble_logevent.h>
#define ADI_CFG_BLE_NUM_LOGEVENT_ENTRIES    (20u)


typedef struct 
{
    ADI_BLE_LOG_ID   event;        /*!< log Event */
    uint32_t         data;         /*!< data associated with the event */
    uint32_t         timeStamp;    /*!< timestamp */

} ADI_BLE_LOGEVENT_DATA;

static  ADI_BLE_LOGEVENT_DATA gBleLogEvents[ADI_CFG_BLE_NUM_LOGEVENT_ENTRIES];
static  int32_t gEvtIndx;

/**
 * @brief       Logs the specified event to the event buffer
 *
 * @details     Logs the specified event to the event buffer with timestamp
 *
 * @param [in]  event :  event that needs to be logged
 * 
 * @return      none            
 *
 */
void adi_ble_LogEvent(const ADI_BLE_LOG_ID event)
{
   gBleLogEvents[gEvtIndx].event     = event;
   gBleLogEvents[gEvtIndx].timeStamp = ADI_BLE_GET_TIMESTAMP();
   gEvtIndx++;
   if (gEvtIndx >= ADI_CFG_BLE_NUM_LOGEVENT_ENTRIES) gEvtIndx = 0;
}


/**
 * @brief       Logs the specified event a along with associated event data
 *
 * @details     Logs the specified event to the event buffer, associated data and timestamp
 *
 * @param [in]  event :  event that needs to be logged
 *
 * @param [in]  event :  event that needs to be logged
 * 
 * @return      none            
 *
 */
void adi_ble_LogEventData(const ADI_BLE_LOG_ID event,const uint32_t data)
{
   gBleLogEvents[gEvtIndx].event      = event;
   gBleLogEvents[gEvtIndx].data       = data;
   gBleLogEvents[gEvtIndx].timeStamp  = ADI_BLE_GET_TIMESTAMP();

   gEvtIndx++;

   if (gEvtIndx >= ADI_CFG_BLE_NUM_LOGEVENT_ENTRIES) gEvtIndx = 0;
}

/*@}*/

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif

#endif /* ADI_CFG_BLE_LOGEVENT */

