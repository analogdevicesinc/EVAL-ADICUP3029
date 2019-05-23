/***************************************************************************//**
*   @file   adc_update_timer.c
*   @brief  Update timer driver source.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <drivers/tmr/adi_tmr.h>
#include <drivers/pwr/adi_pwr.h>
#include <stdlib.h>
#include <math.h>
#include "adc_update_timer.h"

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/

/* Update timer interrupt flag */
volatile uint8_t adc_channel_flag = 0;
/* Software prescaler for the timer to extend the maximum time between
 * interrupts */
volatile uint8_t adc_sw_prescaler = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Update timer interrupt callback function.
 *
 * Asserts the interrupt flag and increments the software prescaler. Standard
 * format for callback functions.
 *
 * @param [in] pCBParam - Pointer to callback parameter list.
 * @param [in] nEvent   - Interrupt source identifier.
 * @param [in/out] pArg - Random argument list.(chosen by the application)
 *
 * @return none
 */
static void timer0_adc_update_callback(void *cb_param, uint32_t event,
				       void *arg)
{
	adc_channel_flag = 1;
	adc_sw_prescaler++;
}

/**
 * Calculate timer load value and clock prescaler based on the desired
 * frequency.
 *
 * If the desired frequency is too low, use a software prescaler with maximum
 * value of 32.
 *
 * @param [in/out] dev            - Update timer descriptor structure. Contains
 *        the desired frequency as input parameter and the software prescaler as
 *        output parameter.
 * @param [out] ptr_tmr_load_val  - New load value of the timer.
 * @param [out] ptr_tmr_prescaler - New timer prescaler value.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t adc_update_timer_get_config(struct adc_update_desc *dev,
		uint16_t *ptr_tmr_load_val, ADI_TMR_PRESCALER *ptr_tmr_prescaler)
{
	uint32_t pclk_val;
	float sw_divided_freq, prescaled_freq;
	uint8_t i, j;
	uint8_t done_flag;
	uint32_t ret;

	for(j = 1; j < TIMER_MAX_SW_PERSACLE; j++) {
		dev->sw_prescaler = j;

		ret = adi_pwr_GetClockFrequency(ADI_CLOCK_PCLK, &pclk_val);
		if(ret != ADI_PWR_SUCCESS)
			return ret;

		sw_divided_freq = pclk_val / dev->sw_prescaler;
		/**
		 * The timer has two prescalers. The first is the reference clock
		 * prescaler that goes from 1 to 32 and is replaced with a software
		 * prescaler of the same function. The second is a power of 4 prescaler
		 * with the values of 1, 4, 16 and 64. These are all taken into
		 * consideration here to calculate the load value for the timer and
		 * these prescaler values. the reference clock for the timer is the
		 * peripheral clock (PCLK) of the controller.
		 * Since there are 8 channels to update, the frequency stated by the
		 * program is multiplied by 8 here to be applied to all the channels.
		 */
		for(i = 0; i < 4; i++) {
			if(i == 0) {
				prescaled_freq = sw_divided_freq / (dev->f_update * 8);
			} else {
				prescaled_freq = sw_divided_freq /
						 (pow(4, i + 1) * (dev->f_update * 8));
			}
			if(prescaled_freq < 0xFFFF) {
				*ptr_tmr_load_val = (uint16_t)prescaled_freq;
				*ptr_tmr_prescaler = i;
				done_flag = 1;
				break;
			}
		}
		if(done_flag == 1) {
			break;
		}
	}

	return ret;
}

/**
 * Initializes the timer which updates the ADC input registers.
 *
 * @param [out] device    - The device structure.
 * @param [in] init_param - Pointer to initialization structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t adc_update_setup(struct adc_update_desc **device,
			 struct adc_update_init *init_param)
{
	int32_t ret;
	struct adc_update_desc *dev;
	ADI_TMR_CONFIG timer_config;

	dev = calloc(1, sizeof *dev);
	if(!dev)
		ret = -1;

	dev->f_update = init_param->f_update;
	dev->update_timer = init_param->update_timer;

	/* Basic timer configuration */
	timer_config.bCountingUp = false;
	timer_config.bPeriodic = true;
	timer_config.bReloading = false;
	timer_config.bSyncBypass = true;
	timer_config.eClockSource = ADI_TMR_CLOCK_PCLK;

	ret = adc_update_timer_get_config(dev, &timer_config.nLoad,
					  &timer_config.ePrescaler);
	if(ret < 0)
		goto error;
	timer_config.nAsyncLoad = timer_config.nLoad;

	ret = adi_tmr_Init(dev->update_timer, timer0_adc_update_callback,
			   NULL, true);
	if(ret < ADI_TMR_SUCCESS)
		goto error;

	ret = adi_tmr_ConfigTimer(dev->update_timer, &timer_config);
	if(ret < ADI_TMR_SUCCESS)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Stop timer and free the resources allocated by adc_update_remove().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t adc_update_remove(struct adc_update_desc *dev)
{
	int32_t ret;

	if(!dev)
		return -1;

	ret = adc_update_activate(dev, false);
	if(ret < 0)
		return ret;

	free(dev);

	return ret;
}

/**
 * Activate/deactivate ADC channel update timer.
 *
 * @param [in] dev    - The device structure.
 * @param [in] enable - true to activate update timer
 *                      false to deactivate update timer
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t adc_update_activate(struct adc_update_desc *dev, bool enable)
{
	int32_t ret;

	ret = adi_tmr_Enable(dev->update_timer, enable);
	if(ret < 0)
		return ret;

	if(!enable)
		adc_channel_flag = 0;

	return ret;
}

/**
 * Change the update data rate from the ADC.
 *
 * @param [in] dev      - The device structure.
 * @param [in] new_rate - New update data rate.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t adc_update_set_rate(struct adc_update_desc *dev, float new_rate)
{
	int32_t ret;
	ADI_TMR_CONFIG timer_config;

	ret = adi_tmr_Enable(dev->update_timer, false);
	if(ret < 0)
		return -1;

	/* Basic configuration for timer function */
	timer_config.bCountingUp = false;
	timer_config.bPeriodic = true;
	timer_config.bReloading = false;
	timer_config.bSyncBypass = true;
	timer_config.eClockSource = ADI_TMR_CLOCK_PCLK;

	dev->f_update = new_rate;

	ret = adc_update_timer_get_config(dev, &timer_config.nLoad,
					  &timer_config.ePrescaler);
	if(ret < 0)
		return ret;
	timer_config.nAsyncLoad = timer_config.nLoad;

	ret = adi_tmr_ConfigTimer(dev->update_timer, &timer_config);
	if(ret < 0)
		return ret;

	return adi_tmr_Enable(dev->update_timer, true);
}
