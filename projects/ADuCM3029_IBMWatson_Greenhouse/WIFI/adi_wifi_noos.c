/*!
 *****************************************************************************
   @file:    adi_wifi_noos.c

   @brief:   Application layer support functions for non-RTOS environment.

   @details: This file contains helper functions that can be used by applications
             operating in a non-RTOS system. Applications register a callback 
             function with this layer, which is called when Wi-Fi events occur
             that are asynchronous to the application. Applications transfer 
             control to this layer to perform "dispatching" into the callback.
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
 *  @ingroup WiFi
 *  @{
 *
 *  @brief   Non-RTOS Framework Functions
 *  @details The set of functions in this module provide an application with a
 *  a simple interface for waiting for events from the radio with a timeout.
 *  This module is for applications in a non-RTOS operating environment. This
 *  layer uses a General Purpose Timer to implement timeouts, the user can
 *  configure the timeout in this file.
 */


 /*! \cond PRIVATE */
//#include <framework/noos/adi_wifi_noos.h>
#include <adi_wifi_noos.h>
#include <adi_wifi_config.h>
#include <drivers/tmr/adi_tmr.h>

/*! Local memory */
static ADI_CALLBACK   	 pApplicationCallback;
static void *        	 pApplicationParameter;
static volatile uint8_t  gbTimeoutHappened;
static volatile uint8_t  gbPingTimeoutHappened;
static volatile uint32_t gnHardwareError;
static volatile uint8_t  gbEventHappened;
static ADI_TMR_CONFIG 	 sTimer = { .bPeriodic    = true,
                                    .bCountingUp  = false,
                                    .eClockSource = ADI_TMR_CLOCK_LFOSC,
                                    .ePrescaler   = ADI_TMR_PRESCALER_16,
                                    .bSyncBypass  = false,
                                    .bReloading   = false};

/*! \endcond */
/*********************************************************************************
                                    API IMPLEMENTATIONS 
*********************************************************************************/
/*!
 * @brief      Initialize Framework Layer.
 *
 * @details    Initialize companion module and register callback.
 *
 * @param [in] pCallbackFunc : Application callback function for Wi-Fi events.
 *
 * @param [in] pCBParam : Pointer to the application callback parameter.
 *
 * @return     ADI_WIFI_RESULT
 *                  - #ADI_WIFI_SUCCESS is returned upon success
 *                  - #ADI_WIFI_FAILURE is returned upon failure
 *
 */
ADI_WIFI_RESULT adi_wifi_Init(ADI_CALLBACK const pCallbackFunc, void * const pCBParam)
{
    ASSERT(pCallbackFunc != NULL);

    pApplicationCallback  = pCallbackFunc;
    pApplicationParameter = pCBParam;

    if((adi_tmr_Init(ADI_WIFI_NOOS_GP_TIMER_CFG, adi_wifi_TimerCallback, NULL, true) == ADI_TMR_SUCCESS)  &&
       (adi_tmr_Init(ADI_WIFI_BROKER_CONNECTION_GP_TIMER_CFG, adi_wifi_PingTimerCallback, NULL, true) == ADI_TMR_SUCCESS))
    {
    	return(adi_wifi_radio_Init(adi_wifi_NoosCallback));
    }
    return(ADI_WIFI_FAILURE);
}


/*!
 * @brief      Event dispatcher.
 *
 * @details    Dispatch any events received for a fixed amount of time to the application callback. This
 *             function will block the caller for the specified amount of time, and pass events to the
 *             callback.
 *
 * @param [in] nTimeoutMilliSeconds : The time to wait (ms) if no event occurs.
 *
 * @return     ADI_WIFI_RESULT
 *                  - #ADI_WIFI_SUCCESS is returned upon success
 *                  - #ADI_WIFI_FAILURE is returned upon failure
 *
 */
ADI_WIFI_RESULT adi_wifi_DispatchEvents(const uint32_t nTimeoutMilliSeconds)
{
	adi_wifi_ClearEventPending();

    return adi_wifi_WaitForResponseWithTimeout(nTimeoutMilliSeconds, CMD_NONE);
}

/*!
 * @brief      Wait for response with a timeout.
 *
 * @details    This function will block until eCommandType occurs. If there is a duration of nTimeoutMilliSeconds
 *             between calling this function and the desired event, the function will timeout and return failure.
 *
 * @param [in] nTimeoutMilliSeconds : The time to wait (ms) if no event occurs before erroring out.
 *
 * @param [in] eCommandType : The event we are expecting to receive.
 *
 * @return     ADI_WIFI_RESULT
 *                  - #ADI_WIFI_SUCCESS is returned upon success
 *                  - #ADI_WIFI_FAILURE is returned upon failure
 */
ADI_WIFI_RESULT adi_wifi_WaitForResponseWithTimeout(const uint32_t nTimeoutMilliSeconds, ADI_WIFI_AT_CMDCODE eCommandType)
{
	ADI_WIFI_RESULT eResult = ADI_WIFI_FAILURE;
	uint32_t        eHwError;

    if(adi_wifi_StartTimer(nTimeoutMilliSeconds, 0u) == ADI_WIFI_SUCCESS)
    {
        /* WHILE(Fake infinite loop, it will exit on timeout) */
        while(1u)
        {
            /* IF(Timeout) */
            if (adi_wifi_IsTimerDone(0u) == 1u)
            {
                /* Flag error and exit */
                eResult = (eCommandType == CMD_NONE) ? ADI_WIFI_SUCCESS : ADI_WIFI_FAILURE;
            	break;
            } /* ENDIF */

            /* IF(Radio has data to read) */
            if (adi_wifi_IsEventPending() == 1u)
            {
            	if(adi_wifi_GetEvent(eCommandType) == ADI_WIFI_SUCCESS)
            	{
                    /* Notify the application about the event */
                    pApplicationCallback(pApplicationParameter, eCommandType, NULL);
                    
                    /* Clear the event pending flag as we have now read all the data expected with this event.
                     * This needs to happen after the full packet has been received because this flag gets set
                     * with every byte received, not just the first of each message.
                     */
                    adi_wifi_ClearEventPending();
                    
                    eResult = ADI_WIFI_SUCCESS;
                    break;
            	}
            	else
            	{
            		adi_wifi_ClearEventPending();
            		break;
            	}
            }

            eHwError = adi_wifi_IsHardwareErrorPending();

            if(eHwError == 1u)
            {
            	/* Notify the application about the error */
                pApplicationCallback(pApplicationParameter, CMD_HW_ERROR,(void *) eHwError);
                adi_wifi_ClearHardwareErrorPending();
                break;
            }
        }

    	if((adi_wifi_StopTimer(0u) == ADI_WIFI_SUCCESS) && (eResult == ADI_WIFI_SUCCESS))
    	{
            eResult = ADI_WIFI_SUCCESS;
    	}
    }
    return(eResult);
}

/*!
 * @brief      Wi-Fi no operating system layer callback.
 *
 * @details    Callback function passed to radio module to be called when a ESP8266
 *             event occurs.
 *
 * @param [in] pCBParam : Callback parameter. (unused)
 *
 * @param [in] Event : Callback event. (unused)
 *
 * @param [in] pArg : Callback argument. (only used in the case of a hardware error)
 *
 */
void adi_wifi_NoosCallback(void * pCBParam, uint32_t Event, void * pArg)
{
	switch(Event){
		case ADI_UART_EVENT_RX_DATA:
			gbEventHappened = 1u;
			break;
		case ADI_UART_EVENT_HW_ERROR_DETECTED:
			gnHardwareError = (uint32_t) pArg;
			break;
		default:
			break;
	}
}


/*!
 * @brief      Timer callback.
 *
 * @details    Function that gets executed when a timeout event occurs.
 *
 * @param [in] pCBParam : Callback parameter. (unused)
 *
 * @param [in] Event : Callback event. (unused)
 *
 * @param [in] pArg : Callback argument. (unused)
 *
 */
void adi_wifi_TimerCallback(void * pCBParam, uint32_t Event, void * pArg)
{
    gbTimeoutHappened = 1u;
}

/*!
 * @brief      Ping Request Timer callback.
 *
 * @details    Function that gets executed when a Ping request to MQTT broker timeout event occurs.
 *
 * @param [in] pCBParam : Callback parameter. (unused)
 *
 * @param [in] Event : Callback event. (unused)
 *
 * @param [in] pArg : Callback argument. (unused)
 *
 */
void adi_wifi_PingTimerCallback(void * pCBParam, uint32_t Event, void * pArg)
{
	gbPingTimeoutHappened = 1u;
}

/*!
 * @brief      Check event pending.
 *
 * @details    Check if a the radio issued an event. If so, the application should 
 *             call adi_wifi_GetEvent() to read it from the radio.
 *
 * @return     1u if an event is pending, 0u if it is not.
 *
 */
uint8_t adi_wifi_IsEventPending(void)
{
    return (gbEventHappened);
}


/*!
 * @brief      Clear event pending.
 *
 * @details    Clear the event flag, should be done immediately after the application has finished
 * 			   reading the full packet from the radio.
 *
 */
void adi_wifi_ClearEventPending(void)
{
    gbEventHappened = 0u;
}


/*!
 * @brief      Check hardware error pending.
 *
 * @details    Check if a the radio issued n hardware error.
 *
 * @return     0u if an hardware error is not pending, otherwise the specific error is returned.
 *  		   This is a #ADI_UART_HW_ERRORS.
 *
 */
uint32_t adi_wifi_IsHardwareErrorPending(void)
{
    return (gnHardwareError);
}


/*!
 * @brief      Clear hardware error pending.
 *
 * @details    Clear the hardware error flag.
 *
 */
void adi_wifi_ClearHardwareErrorPending(void)
{
	gnHardwareError = 0u;
}

/*!
 * @brief      Timer peek function.
 *
 * @details    Check if timeout occurred.
 *
 * @param [in] nIsPingTimer : Parameter to differentiate which timer event to check.
 *
 * @return     1u if timeout occurred, 0u if it did not.
 *
 */
uint8_t adi_wifi_IsTimerDone(uint8_t nIsPingTimer)
{
    if(!nIsPingTimer){
    	return(gbTimeoutHappened);
    }
    else{
    	return(gbPingTimeoutHappened);
    }
}


/*!
 * @brief      Timer start.
 *
 * @details    Configure and start #ADI_WIFI_NOOS_GP_TIMER with a specific timeout value.
 *
 * @param [in] nMilliSeconds : The number of milliseconds before timeout.
 * @param [in] nIsPingTimer  : Parameter to differentiate which timer to configure.
 *
 * @return     ADI_WIFI_RESULT
 *                  - #ADI_WIFI_SUCCESS is returned upon success
 *                  - #ADI_WIFI_FAILURE is returned upon failure
 *
 */
ADI_WIFI_RESULT adi_wifi_StartTimer(const uint32_t nMilliSeconds, uint8_t nIsPingTimer)
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

    if(!nIsPingTimer){
		eResult = adi_tmr_ConfigTimer(ADI_WIFI_NOOS_GP_TIMER_CFG, &sTimer);
		RETURN_ERROR(eResult, ADI_TMR_SUCCESS, ADI_WIFI_FAILURE);

		gbTimeoutHappened = 0u;

		eResult = adi_tmr_Enable(ADI_WIFI_NOOS_GP_TIMER_CFG, true);
		RETURN_ERROR(eResult, ADI_TMR_SUCCESS, ADI_WIFI_FAILURE);
    }
    else{
		eResult = adi_tmr_ConfigTimer(ADI_WIFI_BROKER_CONNECTION_GP_TIMER_CFG, &sTimer);
		RETURN_ERROR(eResult, ADI_TMR_SUCCESS, ADI_WIFI_FAILURE);

		gbPingTimeoutHappened = 0u;

		eResult = adi_tmr_Enable(ADI_WIFI_BROKER_CONNECTION_GP_TIMER_CFG, true);
		RETURN_ERROR(eResult, ADI_TMR_SUCCESS, ADI_WIFI_FAILURE);
    }

    return (ADI_WIFI_SUCCESS);
}


/*!
 * @brief      Timer stop.
 *
 * @details    Disable the timer.
 *
 * @param [in] nIsPingTimer  : Parameter to differentiate which timer to stop.
 *
 * @return     ADI_WIFI_RESULT
 *                  - #ADI_WIFI_SUCCESS is returned upon success
 *                  - #ADI_WIFI_FAILURE is returned upon failure
 *
 */
ADI_WIFI_RESULT adi_wifi_StopTimer(uint8_t nIsPingTimer)
{
    if(!nIsPingTimer){
		if (adi_tmr_Enable(ADI_WIFI_NOOS_GP_TIMER_CFG, false) != ADI_TMR_SUCCESS)
		{
			return (ADI_WIFI_FAILURE);
		}
    }
    else{
		if (adi_tmr_Enable(ADI_WIFI_BROKER_CONNECTION_GP_TIMER_CFG, false) != ADI_TMR_SUCCESS)
		{
			return (ADI_WIFI_FAILURE);
		}
    }
    return (ADI_WIFI_SUCCESS);
}


/*! \endcond */


/* @} */
