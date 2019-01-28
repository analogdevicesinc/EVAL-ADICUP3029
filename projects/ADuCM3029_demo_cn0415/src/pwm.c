/***************************************************************************//**
 *   @file   pwm.c
 *   @brief  ADICUP3029 PWM driver.
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
#include <drivers/pwr/adi_pwr.h>
#include <drivers/tmr/adi_tmr.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "pwm.h"

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
float dith_ampl = 0;
bool dither_period = false;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Dither timer callback function.
 *
 * @param [in] pCBParam - Application passed parameter.
 * @param [in] nEvent   - The code of the event that triggered the interrupt.
 * @param [in] pArg     - Driver passed parameter.
 *
 * @return void
 */
static void timer0_dither_callback(void * pCBParam, uint32_t nEvent,
				   void * pArg)
{
	dith_ampl = 0 - dith_ampl;
	dither_period = true;
}

/**
 * Initialize timer peripheral with PWM functionality.
 *
 * @param [out] device     - The device structure.
 * @param [in]  init_param - The structure that contains the device initial
 * 		       		         parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
  */
int32_t pwm_setup(struct pwm_desc **device, struct pwm_init_param init_param)
{
	ADI_TMR_RESULT ret;
	ADI_TMR_PWM_CONFIG pwm_config;
	ADI_TMR_CONFIG timer_config;
	struct pwm_desc *dev;
	uint16_t match;
	uint32_t pclk_freq, prescale_freq;
	uint8_t i, j, pclk_div_flag = 0;

	/* Allocate memory for peripheral descriptor */
	dev = (struct pwm_desc *)malloc(sizeof(*dev));
	if(!dev)
		return 0;

	/* Initialization */
	dev->duty_cycle = init_param.duty_cycle;
	dev->frequency = init_param.frequency;

	/* Initialize timer for use */
	ret = adi_tmr_Init(ADI_TMR_DEVICE_GP1, 0, 0, false);
	if(ret != ADI_TMR_SUCCESS)
		goto error;

	/* Calculate timer prescaler and load value based on timer clock frequency
	 * and the desired frequency of the PWM */
	for(j = 1; j < 33; j++) {
		/* Set PCLK divider */
		if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, j))
			goto error;

		/* Get timer frequency */
		if(adi_pwr_GetClockFrequency(ADI_CLOCK_PCLK, &pclk_freq) !=
		    ADI_PWR_SUCCESS)
			goto error;

		/* Calculate timer prescaler and top maximum value */
		for(i = 0; i < 4; i++) {
			if(i == 0)
				prescale_freq = pclk_freq / dev->frequency;
			else
				prescale_freq = pclk_freq / (pow(4, i + 1) * dev->frequency);
			if(prescale_freq < 0xFFFF) {
				timer_config.nLoad = (uint16_t)prescale_freq;
				timer_config.nAsyncLoad = (uint16_t)prescale_freq;
				timer_config.ePrescaler = i;
				pclk_div_flag = 1;
				break;
			}
		}
		if(pclk_div_flag == 1)
			break;
	}

	/* Remember load value to calculate match value for duty cycle */
	dev->load = (uint16_t)timer_config.nLoad;

	/* Basic configuration for timer function */
	timer_config.bCountingUp = false;
	timer_config.bPeriodic = true;
	timer_config.bReloading = false;
	timer_config.bSyncBypass = true;
	timer_config.eClockSource = ADI_TMR_CLOCK_PCLK;

	/* Calculate match value for duty cycle */
	match = (dev->load * dev->duty_cycle) / 10000;

	/* Basic configuration for PWM functionality */
	pwm_config.bIdleHigh = false;
	pwm_config.bMatch = true;
	pwm_config.nMatchValue = match;
	pwm_config.eOutput = ADI_TMR_PWM_OUTPUT_0;

	ret = adi_tmr_ConfigTimer(ADI_TMR_DEVICE_GP1, &timer_config);
	if(ret != ADI_TMR_SUCCESS)
		goto error;

	ret = adi_tmr_ConfigPwm(ADI_TMR_DEVICE_GP1, &pwm_config);
	if(ret != ADI_TMR_SUCCESS)
		goto error;

	/* Start timer */
	ret = adi_tmr_Enable(ADI_TMR_DEVICE_GP1, true);
	if(ret != ADI_TMR_SUCCESS)
		goto error;

	*device = dev;

	return ret;

error:
	free(dev);

	return ret;
}

/**
 * Free the resources allocated by pwm_setup().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t pwm_remove(struct pwm_desc *dev)
{
	ADI_TMR_RESULT ret;

	/* Disable timer */
	ret = adi_tmr_Enable(ADI_TMR_DEVICE_GP1, false);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	free(dev);

	return ret;
}

/**
 * Changes the duty cycle of the PWM.
 *
 * @param [in] dev        - The device structure.
 * @param [in] duty_cycle - New value of the duty cycle.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t pwm_set_duty_cycle(struct pwm_desc *dev, uint16_t duty_cycle)
{
	ADI_TMR_RESULT ret;
	ADI_TMR_PWM_CONFIG pwm_config;
	uint16_t match;

	/* Disable timer */
	ret = adi_tmr_Enable(ADI_TMR_DEVICE_GP1, false);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	/* Update duty cycle in device descriptor */
	dev->duty_cycle = duty_cycle;

	/* Calculate match value for the new duty cycle */
	match = (dev->load * duty_cycle) / 10000;

	/* Resume trivial configuration */
	pwm_config.bIdleHigh = false;
	pwm_config.bMatch = true;
	pwm_config.nMatchValue = match;
	pwm_config.eOutput = ADI_TMR_PWM_OUTPUT_0;

	/* Update the duty cycle */
	ret = adi_tmr_ConfigPwm(ADI_TMR_DEVICE_GP1, &pwm_config);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	/* Re-enable the timer */
	return adi_tmr_Enable(ADI_TMR_DEVICE_GP1, true);
}

/**
 * Changes the frequency of the PWM.
 *
 * @param [in] dev           - The device structure.
 * @param [in] new_frequency - New value of the frequency.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t pwm_set_frequency(struct pwm_desc *dev, uint32_t new_frequency)
{
	ADI_TMR_RESULT ret;
	ADI_TMR_CONFIG timer_config;
	ADI_TMR_PWM_CONFIG pwm_config;
	uint32_t pclk_freq, prescale_freq;
	uint8_t i, j, pclk_div_flag = 0;
	ADI_PWR_RESULT chk;

	/* Disable timer */
	ret = adi_tmr_Enable(ADI_TMR_DEVICE_GP1, false);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	/* Update frequency in device descriptor */
	dev->frequency = new_frequency;

	/* Calculate timer prescaler and load value based on timer clock frequency
	 * and the desired frequency of the PWM */
	for(j = 1; j < 33; j++) {
		/* Set PCLK divider */
		chk = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, j);
		if(ADI_PWR_SUCCESS != chk)
			return ret;

		/* Get timer frequency */
		chk = adi_pwr_GetClockFrequency(ADI_CLOCK_PCLK, &pclk_freq);
		if(chk != ADI_PWR_SUCCESS)
			return ret;

		/* Calculate timer prescaler and top maximum value */
		for(i = 0; i < 4; i++) {
			if(i == 0)
				prescale_freq = pclk_freq / dev->frequency;
			else
				prescale_freq = pclk_freq / (pow(4, i + 1) * dev->frequency);
			if(prescale_freq < 0xFFFF) {
				timer_config.nLoad = (uint16_t)prescale_freq;
				timer_config.nAsyncLoad = (uint16_t)prescale_freq;
				timer_config.ePrescaler = i;
				pclk_div_flag = 1;
				break;
			}
		}
		if(pclk_div_flag == 1)
			break;
	}

	/* Remember load value to calculate match value for duty cycle */
	dev->load = (uint16_t)timer_config.nLoad;

	/* Basic configuration for timer function */
	timer_config.bCountingUp = false;
	timer_config.bPeriodic = true;
	timer_config.bReloading = false;
	timer_config.bSyncBypass = true;
	timer_config.eClockSource = ADI_TMR_CLOCK_PCLK;
	uint16_t match;

	/* Calculate match value for duty cycle */
	match = (dev->load * dev->duty_cycle) / 10000;

	/* Basic configuration for PWM functionality */
	pwm_config.bIdleHigh = false;
	pwm_config.bMatch = true;
	pwm_config.nMatchValue = match;
	pwm_config.eOutput = ADI_TMR_PWM_OUTPUT_0;

	ret = adi_tmr_ConfigTimer(ADI_TMR_DEVICE_GP1, &timer_config);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	ret = adi_tmr_ConfigPwm(ADI_TMR_DEVICE_GP1, &pwm_config);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	/* Start timer */
	return adi_tmr_Enable(ADI_TMR_DEVICE_GP1, true);
}

/**
 * Initialize timer peripheral with dither functionality on PWM signal.
 *
 * @param [in] device     - The device structure.
 * @param [in] init_param - The structure that contains the device initial
 *                          parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t pwm_dither_setup(struct pwm_desc *device,
			 struct pwm_init_param init_param)
{
	int32_t ret;
	ADI_TMR_CONFIG timer_config;
	uint32_t pclk_freq;

	/* Enable timer that will control the dither */
	ret = adi_tmr_Init(ADI_TMR_DEVICE_GP0, timer0_dither_callback, NULL, true);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	/* Initialize device descriptor */
	device->has_dither 		 = init_param.has_dither;
	device->dither_frequency = init_param.dither_frequency;
	device->dither_amplitude = init_param.dither_amplitude;

	/* Basic timer configuration for frequencies between 2kHz and 30Hz, the
	 * scope of the dither. */
	timer_config.bCountingUp  = false;
	timer_config.bPeriodic 	  = true;
	timer_config.bReloading   = true;
	timer_config.bSyncBypass  = true;
	timer_config.eClockSource = ADI_TMR_CLOCK_PCLK;
	timer_config.ePrescaler   = ADI_TMR_PRESCALER_16;

	/* Get clock source frequency */
	ret = adi_pwr_GetClockFrequency(ADI_CLOCK_PCLK, &pclk_freq);
	if(ret != ADI_PWR_SUCCESS)
		return ret;

	/* Calculate timer load for the desired frequency */
	pclk_freq /= 16;
	timer_config.nAsyncLoad = pclk_freq / device->dither_frequency;
	timer_config.nLoad = pclk_freq / device->dither_frequency;
	device->dload = pclk_freq / device->dither_frequency;

	dith_ampl = device->dither_amplitude;

	/* Configure dither timer */
	ret = adi_tmr_ConfigTimer(ADI_TMR_DEVICE_GP0, &timer_config);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	/* Start dither timer */
	if (device->has_dither == 1)
		return adi_tmr_Enable(ADI_TMR_DEVICE_GP0, true);

	return ret;
}

/**
 * Activate dither functionality.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t pwm_dither_activate(struct pwm_desc *dev)
{
	dev->has_dither = 1;

	/* Start dither timer */
	return adi_tmr_Enable(ADI_TMR_DEVICE_GP0, true);
}

/**
 * Deactivate dither functionality.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t pwm_dither_remove(struct pwm_desc *dev)
{
	int32_t ret;

	dev->has_dither = 0;

	/* Stop dither timer */
	ret = adi_tmr_Enable(ADI_TMR_DEVICE_GP0, false);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	return pwm_set_duty_cycle(dev, dev->duty_cycle);
}

/**
 * Change dither frequency.
 *
 * @param [in] dev           - The device structure.
 * @param [in] new_frequency - New dither frequency.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t pwm_dither_change_frequency(struct pwm_desc *dev,
				    uint32_t new_frequency)
{
	int32_t ret;
	ADI_TMR_CONFIG timer_config;
	uint32_t pclk_freq;

	/* Stop dither timer */
	ret = adi_tmr_Enable(ADI_TMR_DEVICE_GP0, false);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	dev->dither_frequency = new_frequency;

	/* Resume basic configuration */
	timer_config.bCountingUp  = false;
	timer_config.bPeriodic 	  = true;
	timer_config.bReloading   = true;
	timer_config.bSyncBypass  = true;
	timer_config.eClockSource = ADI_TMR_CLOCK_PCLK;
	timer_config.ePrescaler   = ADI_TMR_PRESCALER_16;

	/* Get clock source frequency */
	ret = adi_pwr_GetClockFrequency(ADI_CLOCK_PCLK, &pclk_freq);
	if(ret != ADI_PWR_SUCCESS)
		return ret;

	/* Calculate timer load for the desired frequency */
	pclk_freq /= 16;
	timer_config.nAsyncLoad = pclk_freq / dev->dither_frequency;
	timer_config.nLoad = pclk_freq / dev->dither_frequency;

	/* Configure dither timer with new load */
	ret = adi_tmr_ConfigTimer(ADI_TMR_DEVICE_GP0, &timer_config);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	/* Start dither timer */
	return adi_tmr_Enable(ADI_TMR_DEVICE_GP0, true);
}

/**
 * Change dither amplitude.
 *
 * @param [in] dev      - The device structure.
 * @param [in] pwm_desc - New dither amplitude.
 *
 * @return void
 */
void pwm_dither_change_amplitude(struct pwm_desc *dev, float new_amplitude)
{
	if(dith_ampl < 0)
		dith_ampl = (0 - new_amplitude);
	if(dith_ampl >= 0)
		dith_ampl = new_amplitude;

	dev->dither_amplitude = dith_ampl;
}

/**
 * Internal function used to apply dither on the PWM signal.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t pwm_dither_apply(struct pwm_desc *dev)
{
	int32_t ret = 0;
	int32_t duty;
	uint32_t temp;

	/* If dither is active */
	if(dev->has_dither == 1) {
		/* If it's time to actuate the duty_cycle */
		if(dither_period) {
			if (dev->dither_amplitude >= 1) {
				/* Store the value of the duty cycle */
				temp = dev->duty_cycle;

				/* Actuate the duty cycle. If the amplitude is too big limit the
				 * output. */
				duty = dev->duty_cycle + (int16_t)dith_ampl;
				if(duty < 0)
					duty = 0;
				if(duty > 10000)
					duty = 10000;
				ret = pwm_set_duty_cycle(dev, (uint32_t)duty);
				if(ret != 0)
					return ret;

				/* Restore duty cycle value */
				dev->duty_cycle = temp;

				/* Clear flag for dither interrupt */
				dither_period = false;
			}
		}
	}

	return ret;
}
