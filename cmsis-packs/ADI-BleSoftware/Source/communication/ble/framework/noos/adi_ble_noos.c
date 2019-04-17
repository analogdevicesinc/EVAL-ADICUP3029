/*!
 *****************************************************************************
   @file:    adi_ble_noos.c

   @brief:   Application layer support functions for non-RTOS environment.

   @details: This file contains helper functions that can be used by applications
             operating in a non-RTOS system. Applications register a callback 
             function with this layer, which is called when BLE events occur
             that are asynchronous to the application. Applications transfer 
             control to this layer to perform "dispatching" into the callback.
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
 *  @ingroup ble
 *  @{
 *
 *  @brief   Non-RTOS Framework Functions
 *  @details The set of functions in this module provide an application with a
 *  a simple interface for waiting for events from the radio with a timeout.
 *  This module is for applications in a non-RTOS operating environment. This
 *  layer uses a General Purpose Timer to implement timeouts, the user can
 *  configure the timeout in the adi_ble_config.h configuration header file.
 */


 /*! \cond PRIVATE */


#include <adi_ble_config.h>
#include <drivers/tmr/adi_tmr.h>
#include <radio/adi_ble_radio.h>
#include <common/adi_error_handling.h>
#include <framework/noos/adi_ble_noos.h>


/*! Timeout value for #CORE_INITIALIZATION_RSP in #adi_ble_Init, note that this is just an upper limit. */
#define ADI_BLE_INIT_CORE_INIT_RSP_TIME_MS (5000u)


/*! Local memory */
static ADI_CALLBACK   pApplicationCallback;
static void *         pApplicationParameter;
static volatile bool  gbTimeoutHappened;
static volatile bool  gbEventHappened;
static ADI_TMR_CONFIG sTimer = { .bPeriodic    = true,
                                 .bCountingUp  = false,
                                 .eClockSource = ADI_TMR_CLOCK_LFOSC,
                                 .ePrescaler   = ADI_TMR_PRESCALER_16,
                                 .bSyncBypass  = false,
                                 .bReloading   = false};


/*! Local prototypes */
static void    TimerCallback     (void * pCBParam, uint32_t Event, void * pArg);
static bool    IsTimerDone       (void);
static uint8_t StartTimer        (const uint32_t nMilliSeconds);
static uint8_t StopTimer         (void);
static void    BleCallback       (void * pCBParam, uint32_t Event, void * pArg);
static bool    IsEventPending    (void);
static void    ClearEventPending (void);


/*! \endcond */


/*********************************************************************************
                                    API IMPLEMENTATIONS 
*********************************************************************************/


/*!
 * @brief      Initialize Framework Layer
 *
 * @details    Initialize companion module and register callback.
 *
 * @param [in] pCallbackFunc : Application callback function for Bluetooth events.
 *
 * @param [in] pCBParam : Pointer to the applicatio callback parameter.
 *
 * @return     ADI_BLER_RESULT            
 *                  - #ADI_BLER_SUCCESS is returned upon success
 *                  - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_ble_Init(ADI_CALLBACK const pCallbackFunc, void * const pCBParam)
{
    ADI_BLER_RESULT eResult;
    ADI_TMR_RESULT  eTmrResult;

    ASSERT(pCallbackFunc != NULL);

    pApplicationCallback  = pCallbackFunc;
    pApplicationParameter = pCBParam;

    eTmrResult = adi_tmr_Init(ADI_BLE_NOOS_GP_TIMER, TimerCallback, NULL, true);
    RETURN_ERROR(eTmrResult, ADI_TMR_SUCCESS, ADI_BLER_FAILURE);    

    eResult = adi_radio_Init(BleCallback);
    RETURN_ERROR(eResult, ADI_BLER_SUCCESS, eResult);

    return adi_ble_WaitForEventWithTimeout(CORE_INITIALIZATION_RSP, ADI_BLE_INIT_CORE_INIT_RSP_TIME_MS);
}


/*!
 * @brief      Event Dispatcher
 *
 * @details    Dispatch any events received for a fixed amount of time to the application callback. This
 *             function will block the caller for the specified amount of time, and pass events to the 
 *             callback. 
 *
 * @param [in] nTimeoutMilliSeconds : The time to wait (ms) if no event occurs before erroring out.
 *
 * @return     ADI_BLER_RESULT            
 *                  - #ADI_BLER_SUCCESS is returned upon success
 *                  - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_ble_DispatchEvents(const uint32_t nTimeoutMilliSeconds)
{
    return adi_ble_WaitForEventWithTimeout(BLE_RADIO_EVENT_DISPATCH, nTimeoutMilliSeconds);
}


/*!
 * @brief      Wait for Event with Timeout
 *
 * @details    This function will block until eDesiredEvent occurs. If there is a duration of nTimeoutMilliSeconds
 *             between calling this function and the desired event, the function will timeout and return failure.
 *             If a different event is received (not eDesiredEvent) it is passed to the application supplied callback.
 *
 * @param [in] eDesiredEvent : The event we are expecting to receive. 
 *
 * @param [in] nTimeoutMilliSeconds : The time to wait (ms) if no event occurs before erroring out.
 *
 * @return     ADI_BLER_RESULT            
 *                  - #ADI_BLER_SUCCESS is returned if eDesiredEvent is received prior to timeout.
 *                  - #ADI_BLER_FAILURE is returned if eDesireEvent is not recevied prior to timeout, or a timer error occurs.
 */
ADI_BLER_RESULT adi_ble_WaitForEventWithTimeout(const ADI_BLER_EVENT eDesiredEvent, const uint32_t nTimeoutMilliSeconds)
{
    ADI_BLER_RESULT eResult = ADI_BLER_SUCCESS;
    ADI_BLER_EVENT  eBleEvent;
    uint8_t         nTmrError;
    
    nTmrError = StartTimer(nTimeoutMilliSeconds);
    RETURN_ERROR(nTmrError, 0u, ADI_BLER_FAILURE);
   
    /* WHILE(Fake infinite loop, it will exit on timeout) */
    while(1u)
    {
        /* IF(Timeout) */
        if (IsTimerDone() == true)
        {
            /* Flag error and exit */
            eResult = (eDesiredEvent == BLE_RADIO_EVENT_DISPATCH) ? ADI_BLER_SUCCESS : ADI_BLER_FAILURE;
            break;
        } /* ENDIF */

        /* IF(Radio has an event) */
        if (IsEventPending() == true)
        {
            ClearEventPending();

            /* Read event */
            eBleEvent = adi_radio_GetEvent();
            /* IF (This is the event we expected, get out) */
            if (eBleEvent == eDesiredEvent)
            {
                eResult = ADI_BLER_SUCCESS;
                break;
            }
            /* ELSE(Notify application callback) */
            else
            {
                /* IF(Callback supplied) */
                if (pApplicationCallback != NULL)
                {
                    ADI_BLE_ALERT_LEVEL eAlert;
                    void *              pData;      

                    /* SWITCH(Pack "single element" data into the callback) */
                    switch (eBleEvent)
                    {
                        case IMMEDIATE_ALERT_EVENT:
                            eAlert = adi_ble_GetImmediateAlertLevel();
                            pData = &eAlert;
                            break;

                        case LINKLOSS_ALERT_EVENT: 
                            eAlert = adi_ble_GetLinkLossAlertLevel();
                            pData = &eAlert;
                            break;

                        default:
                            pData = NULL;
                            break;                       
                    } /* ENDSWITCH */

                    pApplicationCallback(pApplicationParameter, eBleEvent, pData);
                    
                } /* ENDIF */
            } /* ENDIF */
        } /* ENDIF */
    } /* ENDWHILE */

    nTmrError = StopTimer();
    RETURN_ERROR(nTmrError, 0u, ADI_BLER_FAILURE);

    return (eResult);
}



/*********************************************************************************
                                   PRIVATE FUNCTIONS
*********************************************************************************/


/*! \cond PRIVATE */


/*!
 * @brief      Wait for Response with Timeout
 *
 * @details    Wait for a response to occur
 *
 * @param [in] nEventBits : A bit field containing a combination or one element of #ADI_BLE_EVENT_FLAG. 
 *
 * @param [in] nTimeoutMilliSeconds : The time to wait (ms) if no response occurs before erroring out.
 *
 * @return     ADI_BLER_RESULT            
 *                  - #ADI_BLER_SUCCESS is returned upon success
 *                  - #ADI_BLER_FAILURE is returned upon failure
 *
 * @note       Applications should not call this function. adi_ble_radio.c calls this function to handle 
 *             responses. 
 */
ADI_BLER_RESULT adi_ble_WaitForResponseWithTimeout(const uint32_t nEventBits, const uint32_t nTimeoutMilliSeconds)
{
    /* If we are waiting on a response, we made to the NOEVENT enumeration */
    if ((nEventBits & ADI_EVENT_FLAG_RESP_SUCCESS) == ADI_EVENT_FLAG_RESP_SUCCESS)
    {
        return adi_ble_WaitForEventWithTimeout(BLE_RESPONSE_SUCCESS_NOEVENT, nTimeoutMilliSeconds);
    }
    else
    {
        return adi_ble_WaitForEventWithTimeout(BLE_RESPONSE_SUCCESS_EVENT, nTimeoutMilliSeconds);
    }
}


/*!
 * @brief      Bluetooth Layer Callback
 *
 * @details    Callback function passed to radio module to be called when a radio
 *             event occurs.
 *
 * @param [in] pCBParam : Callback parameter (unused)
 *
 * @param [in] Event : Callback event (unused)
 *
 * @param [in] pArg : Callback argument (unused)
 *
 */
static void BleCallback(void * pCBParam, uint32_t Event, void * pArg)
{
    gbEventHappened = true;
}


/*!
 * @brief      Check Event Pending
 *
 * @details    Check if a the radio issued an event. If so, the application should 
 *             call #adi_radio_GetEvent to read it from the radio.
 *
 * @return     True if an event is pending, false if it is not.
 *
 */
static bool IsEventPending(void)
{
    return (gbEventHappened);
}


/*!
 * @brief      Clear Event Pending
 *
 * @details    Clear the event flag, should be done immediately after #adi_ble_IsEventPending
 *             returns true.
 *
 */
static void ClearEventPending(void)
{
    gbEventHappened = false;
}


/*!
 * @brief      Timer Callback
 *
 * @details    Function that gets executed when a timeout event occurs.
 *
 * @param [in] pCBParam : Callback parameter (unused)
 *
 * @param [in] Event : Callback event (unused)
 *
 * @param [in] pArg : Callback argument (unused)
 *
 */
static void TimerCallback(void * pCBParam, uint32_t Event, void * pArg)
{
    gbTimeoutHappened = true;
}


/*!
 * @brief      Timer Peek
 *
 * @details    Check if timeout occurred.
 *
 * @return     True if timeout occurred, false if it did not.
 *
 */
static bool IsTimerDone(void)
{
    return (gbTimeoutHappened);
}


/*!
 * @brief      Timer Start
 *
 * @details    Configure and start GP0 with a specific timeout value.
 *
 * @param [in] nMilliSeconds : The number of milliseconds before timeout.
 *
 * @return     0u if function passed, 1u if it failed.
 *
 */
static uint8_t StartTimer(const uint32_t nMilliSeconds)
{
    ADI_TMR_RESULT eResult;
    
    /* LFOSC = 32768 Hz, divided by 16 = 2048 Hz, 2048 counts per second is
     * about 2 counts per millisecond. 16-bit counter gives us a range of 
     * 65536 counts meaning 32768 milliseconds is the max we can support using
     * this timer configuration.
     */
    ASSERT((nMilliSeconds < 32768u) && (nMilliSeconds != 0u));
        
    /* Other fields of structure are constant */
    sTimer.nLoad        = 2u*nMilliSeconds;
    sTimer.nAsyncLoad   = 2u*nMilliSeconds;

    eResult = adi_tmr_ConfigTimer(ADI_BLE_NOOS_GP_TIMER, &sTimer);
    RETURN_ERROR(eResult, ADI_TMR_SUCCESS, 1u);
    
    gbTimeoutHappened = false;

    eResult = adi_tmr_Enable(ADI_BLE_NOOS_GP_TIMER, true);
    RETURN_ERROR(eResult, ADI_TMR_SUCCESS, 1u);

    return (0u);
}


/*!
 * @brief      Timer Stop
 *
 * @details    Disable the timer.
 *
 * @return     0u if function passed, 1u if it failed.
 *
 */
static uint8_t StopTimer(void) 
{
    if (adi_tmr_Enable(ADI_BLE_NOOS_GP_TIMER, false) != ADI_TMR_SUCCESS)
    {
        return (1u);
    }
    else
    {
        return (0u);
    }
}


/*! \endcond */


/* @} */
