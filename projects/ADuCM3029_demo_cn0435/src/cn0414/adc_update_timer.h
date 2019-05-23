/***************************************************************************//**
*   @file   adc_update_timer.h
*   @brief  Update timer driver header.
*   @author Andrei Drimbarean (andrei.drimbarean@analog.com)
********************************************************************************
* Copyright 2018(c) Analog Devices, Inc.
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

#ifndef ADC_UPDATE_TIMER_H_
#define ADC_UPDATE_TIMER_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#define TIMER_MAX_SW_PERSACLE 33

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

struct adc_update_init {
	float f_update; /* Update timer frequency */
	uint8_t update_timer; /* Update timer ID */
};

struct adc_update_desc {
	float f_update; /* Update timer frequency */
	uint8_t sw_prescaler; /* Update timer software prescaler compare value */
	uint8_t update_timer; /* Update timer ID */
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initializes the timer which updates the ADC input registers. */
int32_t adc_update_setup(struct adc_update_desc **device,
			 struct adc_update_init *init_param);

/* Free the resources allocated by adc_update_remove(). */
int32_t adc_update_remove(struct adc_update_desc *dev);

/* Activate/deactivate ADC channel update timer. */
int32_t adc_update_activate(struct adc_update_desc *dev, bool status);

/* Change the update data rate from the ADC. */
int32_t adc_update_set_rate(struct adc_update_desc *dev, float new_rate);

#endif /* ADC_UPDATE_TIMER_H_ */
