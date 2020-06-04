/**
******************************************************************************
*   @file     timer.c
*   @brief    Source file for the Timer part.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <drivers/pwr/adi_pwr.h>
#include <drivers/tmr/adi_tmr.h>
#include "timer.h"
#include "common.h"
#include "power.h"
#include <math.h>
#include "error.h"

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

static volatile  uint32_t timer_delay_count = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Power and delay timers initialization function.
 *
 * The delay timer is the SysTick timer. Setup the SysTick timer to interrupt
 * once every milisecond.
 *
 * @params void
 *
 * @return 0 in case of success or negative error code otherwise.
 */
int32_t timer_start(void)
{
	int32_t ret;

	ret = pwr_setup();
	if(ret != SUCCESS)
		return ret;

	/* SysTick initialization to interrupt once per milisecond */
	SysTick_Config(26000);

	return ret;
}

/**
 * Delay function of 1ms or more.
 *
 * Used for non-specific delays across the program. Uses the SysTick timer
 * interrupt.
 *
 * @params [in] ticks - Number of miliseconds to delay.
 *
 * @return void
 */
void timer_sleep(uint32_t ticks)
{
	timer_delay_count = ticks;

	/* Busy wait until the SysTick decrements the counter to zero. */
	while (timer_delay_count != 0u);
}

/**
 * Callback function for SysTick timer.
 *
 * @params [in] pCBParam - Application passed pointer.
 * @params [in] nEvent   - Code of the event that triggered the interrupt.
 * @params [in] pArg     - Driver passed pointer.
 *
 * @return void
 */
void SysTick_Handler(void)
{
	/* Decrement to zero the counter used by the delay routine. */
	if (timer_delay_count != 0u)
		--timer_delay_count;
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
static int32_t timer_counter_get_config(struct timer_counter_desc *dev,
					uint16_t *ptr_tmr_load_val,
					ADI_TMR_PRESCALER *ptr_tmr_prescaler)
{
	uint32_t pclk_val;
	float sw_divided_freq, prescaled_freq;
	uint8_t i, j;
	uint8_t done_flag;
	uint32_t ret;

	for(j = 1; j <= TIMER_MAX_SW_PERSACLE; j++) {
		ret = adi_pwr_GetClockFrequency(ADI_CLOCK_PCLK, &pclk_val);
		if(ret != SUCCESS)
			return ret;
		/**
		 * The timer has two prescalers. The first is the reference clock
		 * prescaler that goes from 1 to 32 and is replaced with a software
		 * prescaler of the same function. The second is a power of 4 prescaler
		 * with the values of 1, 4, 16 and 64. These are all taken into
		 * consideration here to calculate the load value for the timer and
		 * these prescaler values. The reference clock for the timer is the
		 * peripheral clock (PCLK) of the controller.
		 */
		sw_divided_freq = pclk_val / j;
		for(i = 0; i < 4; i++) {
			if(i == 0)
				prescaled_freq = sw_divided_freq / dev->f_update;
			else
				prescaled_freq = sw_divided_freq /
						 (pow(4, i + 1) * dev->f_update);
			if(prescaled_freq < 0xFFFF) {
				*ptr_tmr_load_val = (uint16_t)prescaled_freq;
				*ptr_tmr_prescaler = i;
				done_flag = 1;
				break;
			}
		}
		if(done_flag == 1)
			break;
	}

	dev->sw_prescaler = j;

	return ret;
}

/**
 * Initializes a timer with a callback.
 *
 * @param [out] device    - The device structure.
 * @param [in] init_param - Pointer to initialization structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t timer_counter_setup(struct timer_counter_desc **device,
			    struct timer_counter_init *init_param)
{
	int32_t ret;
	struct timer_counter_desc *dev;
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

	ret = timer_counter_get_config(dev, &timer_config.nLoad,
				       &timer_config.ePrescaler);
	if(ret != SUCCESS)
		goto error;
	timer_config.nAsyncLoad = timer_config.nLoad;

	ret = adi_tmr_Init(dev->update_timer, init_param->callback_func_ptr,
			   init_param->callback_param, true);
	if(ret != SUCCESS)
		goto error;

	ret = adi_tmr_ConfigTimer(dev->update_timer, &timer_config);
	if(ret != SUCCESS)
		goto error;

	*device = dev;

	return ret;

error:
	free(dev);

	return ret;
}

/**
 * Stop timer and free the resources allocated by timer_counter_setup().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t timer_counter_remove(struct timer_counter_desc *dev)
{
	int32_t ret;

	if(!dev)
		return -1;

	ret = timer_counter_activate(dev, false);
	if(ret != SUCCESS)
		return ret;

	free(dev);

	return ret;
}

/**
 * Activate/deactivate timer.
 *
 * @param [in] dev    - The device structure.
 * @param [in] enable - true to activate update timer
 *                      false to deactivate update timer
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t timer_counter_activate(struct timer_counter_desc *dev, bool enable)
{
	return adi_tmr_Enable(dev->update_timer, enable);
}

/**
 * Change the rate of the timer.
 *
 * @param [in] dev      - The device structure.
 * @param [in] new_rate - New update data rate.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t timer_counter_set_rate(struct timer_counter_desc *dev, float new_rate)
{
	int32_t ret;
	ADI_TMR_CONFIG timer_config;

	ret = adi_tmr_Enable(dev->update_timer, false);
	if(ret != SUCCESS)
		return -1;

	/* Basic configuration for timer function */
	timer_config.bCountingUp = false;
	timer_config.bPeriodic = true;
	timer_config.bReloading = false;
	timer_config.bSyncBypass = true;
	timer_config.eClockSource = ADI_TMR_CLOCK_PCLK;

	dev->f_update = new_rate;

	ret = timer_counter_get_config(dev, &timer_config.nLoad,
				       &timer_config.ePrescaler);
	if(ret != SUCCESS)
		return ret;
	timer_config.nAsyncLoad = timer_config.nLoad;

	ret = adi_tmr_ConfigTimer(dev->update_timer, &timer_config);
	if(ret != SUCCESS)
		return ret;

	return adi_tmr_Enable(dev->update_timer, true);
}
