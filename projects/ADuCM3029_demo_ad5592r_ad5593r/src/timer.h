/**
******************************************************************************
*   @file     Timer.h
*   @brief    Header file for the Timer part.
*   @version  V0.1
*   @author   ADI
*   @date     January 2017
*
*******************************************************************************
* Copyright 2017(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*  - Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  - Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
*  - Neither the name of Analog Devices, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*  - The use of this software may or may not infringe the patent rights
*    of one or more patent holders.  This license does not release you
*    from the requirement that you obtain separate licenses from these
*    patent holders to use this software.
*  - Use of the software either in source or binary form, must be run
*    on or directly connected to an Analog Devices Inc. component.
*
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef TIMER_H_
#define TIMER_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define TIMER_MAX_SW_PERSACLE 32

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

struct timer_counter_init {
	float f_update; /* Update timer frequency */
	uint8_t update_timer; /* Update timer ID */
	void *callback_func_ptr; /* Pointer to the callback */
	void *callback_param; /* Pointer to the application passed CB parameter */
};

struct timer_counter_desc {
	float f_update; /* Update timer frequency */
	uint8_t sw_prescaler; /* Update timer software prescaler compare value */
	uint8_t update_timer; /* Update timer ID */
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Power and delay timers initialization function. */
int32_t timer_start(void);

/* Delay function of 1ms or more. */
void timer_sleep(uint32_t ticks);

/* Initializes a timer with a callback. */
int32_t timer_counter_setup(struct timer_counter_desc **device,
			    struct timer_counter_init *init_param);

/* Stop timer and free the resources allocated by timer_counter_setup(). */
int32_t timer_counter_remove(struct timer_counter_desc *dev);

/* Activate/deactivate timer. */
int32_t timer_counter_activate(struct timer_counter_desc *dev, bool enable);

/* Change the rate of the timer. */
int32_t timer_counter_set_rate(struct timer_counter_desc *dev, float new_rate);

#endif /* TIMER_H_ */
