/*!
 *****************************************************************************
   @file:    adi_timestamp.h
   @brief:   Header file for adi_ble_time.c
   @details: Public function prototypes
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

/** @addtogroup adi_timestamp Timestamp Interface
 *  @ingroup Utilities
 *  @{
 *
 *  @brief Timestamp Interface
 *  @details  Abstraction layer for timestamp implementation.
 */

#ifndef ADI_TIME_H
#define ADI_TIME_H

/*! \cond PRIVATE */

#include <stdint.h>
#include <adi_global_config.h>
#include <drivers/rtc/adi_rtc.h>
 


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Public APIs called through macros */
void adi_RTCInit(void);
uint32_t adi_GetRTCTime(void);

/*! \endcond */
#ifdef __cplusplus
}
#endif /* __cplusplus */


/*! Initialize the timestamp. In the no os case this will configure the RTC for ~1ms ticks. */
#define INIT_TIME() adi_RTCInit()

/*! Get the current timestamp valu.e */
#define GET_TIME() adi_GetRTCTime()

#endif /* ADI_TIME_H */
/* @} */
