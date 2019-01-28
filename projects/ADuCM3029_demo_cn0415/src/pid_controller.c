/***************************************************************************//**
 *   @file   pid_controller.c
 *   @brief  ADICUP3029 PID controller driver source.
 *   @author Andrei Drimbarean (Andrei.Drimbarean@analog.com)
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
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "pid_controller.h"
#include <drivers/tmr/adi_tmr.h>
#include <drivers/pwr/adi_pwr.h>

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/

uint8_t pid_flag = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * PID timer callback function.
 *
 * @param [in] pCBParam - Application passed parameter.
 * @param [in] nEvent   - The code of the event that triggered the interrupt.
 * @param [in] pArg     - Driver passed parameter.
 *
 * @return void
 */
static void timer2_pid_callback(void * pCBParam, uint32_t nEvent, void * pArg)
{
	pid_flag = 1;
}

/**
 * Initializes the PID controller device descriptor.
 *
 * @param [out] device     - The device structure.
 * @param [in]  init_param - The structure that contains the device initial
 *                           parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t pid_setup(struct pid_controller **device,
		  struct pid_controller_init init_param)
{
	int32_t ret;
	struct pid_controller *dev;
	ADI_TMR_CONFIG pid_timer_config;
	uint32_t pclk_freq, prescale_freq;
	uint8_t i, j;
	uint8_t pclk_div_flag;

	/* Allocate memory for descriptor */
	dev = (struct pid_controller *)malloc(sizeof(*dev));
	if(!dev)
		return -1;

	/* Initialize pid controller */
	dev->setpoint = init_param.setpoint;
	dev->kp = init_param.kp;
	dev->ki = init_param.ki;
	dev->kd = init_param.kd;
	dev->f_loop = init_param.f_loop;
	dev->pid_timer_id = init_param.pid_timer_id;
	dev->error_sum = 0;
	dev->last_feedback = 0;

	/* Basic timer configuration */
	pid_timer_config.bCountingUp = false;
	pid_timer_config.bPeriodic = true;
	pid_timer_config.bReloading = false;
	pid_timer_config.bSyncBypass = true;
	pid_timer_config.eClockSource = ADI_TMR_CLOCK_PCLK;

	/* Calculate timer prescaler and load value based on timer clock frequency
	 * and the desired frequency of the PWM */
	for(j = 1; j < 33; j++) {
		/* Set PCLK divider */
		if((ret = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, j)) !=
		    ADI_PWR_SUCCESS)
			goto error;

		/* Get timer frequency */
		if((ret = adi_pwr_GetClockFrequency(ADI_CLOCK_PCLK, &pclk_freq)) !=
		    ADI_PWR_SUCCESS)
			goto error;

		/* Calculate timer prescaler and top maximum value */
		for(i = 0; i < 4; i++) {
			if(i == 0)
				prescale_freq = pclk_freq / dev->f_loop;
			else
				prescale_freq = pclk_freq / (pow(4, i + 1) * dev->f_loop);
			if(prescale_freq < 0xFFFF) {
				pid_timer_config.nLoad = (uint16_t)prescale_freq;
				pid_timer_config.nAsyncLoad = (uint16_t)prescale_freq;
				pid_timer_config.ePrescaler = i;
				pclk_div_flag = 1;
				break;
			}
		}
		if(pclk_div_flag == 1)
			break;
	}

	/* Initialize PID loop timer */
	ret = adi_tmr_Init(dev->pid_timer_id, timer2_pid_callback, NULL, true);
	if(ret != ADI_TMR_SUCCESS)
		goto error;

	/* Configure timer */
	ret = adi_tmr_ConfigTimer(dev->pid_timer_id, &pid_timer_config);
	if(ret != ADI_TMR_SUCCESS)
		goto error;

	/* Return pointer to the controller descriptor */
	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free the resources allocated by pid_setup().
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
void pid_remove(struct pid_controller *dev)
{
	free(dev);
}

/**
 * Activate/deactivate PID controller.
 *
 * @param [in] dev    - The device structure.
 * @param [in] status - true to activate PID controller;
 *                      false to deactivate PID controller.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t pid_activate(struct pid_controller *dev, bool status)
{
	int32_t ret;

	ret = adi_tmr_Enable(dev->pid_timer_id, status);
	if(ret != ADI_TMR_SUCCESS)
		return ret;
	if(!status)
		pid_flag = 0;

	return ret;
}

/**
 * PID controller for PWM duty cycle.
 *
 * @param [in]  dev    - The device structure.
 * @param [in]  input  - The input current.
 * @param [out] output - The calculated output in duty cycle percentage.
 *
 * @return void
 */
void pid_controller_output_eval(struct pid_controller *dev, float input,
				int16_t *output)
{
	float error, dif_in;

	/* Compute error */
	error = dev->setpoint - input;

	/* Compute integral error and differential error */
	dev->error_sum += dev->ki * error;
	if(dev->error_sum > PWM_OUT_MAX)
		dev->error_sum = PWM_OUT_MAX;
	else if(dev->error_sum < PWM_OUT_MIN)
		dev->error_sum = PWM_OUT_MIN;
	dif_in = input - dev->last_feedback;

	/* Compute output */
	*output = (int16_t)(dev->kp * error +
			    dev->error_sum -
			    dev->kd * dif_in);
	if(*output > PWM_OUT_MAX)
		*output = PWM_OUT_MAX;
	else if(*output < PWM_OUT_MIN)
		*output = PWM_OUT_MIN;

	/* Memorize current error for the next compute */
	dev->last_feedback = input;
}

/**
 * Set constants Kp, Ki and Kd for the PID controller.
 *
 * @param [in] dev - The device structure.
 * @param [in] kp  - The constant for the proportional component.
 * @param [in] ki  - The constant for the integral component.
 * @param [in] kd  - The constant for the derivative component.
 *
 * @return void
 */
void pid_set_constants(struct pid_controller *dev, float kp, float ki, float kd)
{
	/* Change current PID constants */
	dev->kp = kp;
	dev->ki = ki / dev->f_loop;
	dev->kd = kd * dev->f_loop;
}

/**
 * Set sample frequency for the PID controller.
 *
 * @param [in] dev        - The device structure.
 * @param [in] new_period - The new sample frequency.
 *
 * @return void
 */
void pid_set_sample_frequency(struct pid_controller *dev,
			      uint16_t new_frequency)
{
	/* Update proportions with respect to new sampling period */
	dev->ki *= (float)dev->f_loop / (float)new_frequency;
	dev->kd /= (float)dev->f_loop / (float)new_frequency;
	/* Update the sampling period */
	dev->f_loop = new_frequency;
}

/**
 * Set set-point for the PID controller.
 *
 * @param [in] dev          - The device structure.
 * @param [in] new_setpoint - The new set-point.
 *
 * @return void
 */
void pid_set_setpoint(struct pid_controller *dev, float new_setpoint)
{
	/* Set current set-point */
	dev->setpoint = new_setpoint;
}
