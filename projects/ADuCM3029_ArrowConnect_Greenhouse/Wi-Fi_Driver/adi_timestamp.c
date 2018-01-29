/*!
 *****************************************************************************
   @file:    adi_timestamp.c
   @brief:   Time Interface
   @details: Abstraction for getting timestamps (RTOS, RTC, TMR, SysTick, etc.)
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

/** 
 *  @defgroup Utilities  Utility Definitions and Functions for Communication
 */

/** @addtogroup adi_timestamp Timestamp Interface
 *  @ingroup Utilities
 *  @{
 *
 *  @brief Timestamp Interface
 *  @details  Abstraction layer for timestamp implementation.
 */

#include <adi_global_config.h>
#include <adi_timestamp.h>
#include <adi_error_handling.h>


/*! Device memory to operate the RTC device */
static uint8_t aRtcDevMem[ADI_RTC_MEMORY_SIZE];

/*! Device handle to the RTC device */
static ADI_RTC_HANDLE hDevice;

/*! RTC device number */
#define ADI_RTC_DEVICE_NUM       (1u)


/*!  The RTC prescalar can be caluculated using the equation: 1/(32768/2^Prescalar). Set prescalar to 5u for .97 ms precision */
#define ADI_RTC_PRESCALAR       (5u)


/**
 * @brief       Initialize RTC.
 *
 * @details     This API is used to initialize the RTC to .97 ms per tick.
 *
 */
void adi_RTCInit(void)
{
    ADI_RTC_RESULT eRTCResult;
    eRTCResult = adi_rtc_Open(ADI_RTC_DEVICE_NUM, aRtcDevMem, ADI_RTC_MEMORY_SIZE, &hDevice);

    if(eRTCResult == ADI_RTC_SUCCESS)
    {
    	eRTCResult = adi_rtc_SetPreScale(hDevice, ADI_RTC_PRESCALAR);
	}	
    
    if(eRTCResult == ADI_RTC_SUCCESS)
    {
    	eRTCResult =  adi_rtc_Enable(hDevice, true);
	}
}

/**
 * @brief       Get the current RTC timestamp.
 *
 * @return      The current RTC time in .97 ms precision. 
 *
 */
uint32_t adi_GetRTCTime(void)
{
    uint32_t t;
    if (adi_rtc_GetCount(hDevice, &t) != ADI_RTC_SUCCESS) {
        t = 0u;
    }
    return t;
}

/* @} */
