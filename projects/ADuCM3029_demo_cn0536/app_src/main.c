/***************************************************************************//**
 *   @file   main.c
 *   @brief  Main aplication
 *   @author mihail.chindris@analog.com
********************************************************************************
 * Copyright 2021(c) Analog Devices, Inc.
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

#include <sys/platform.h>
#include "adi_initialize.h"
#include <drivers/pwr/adi_pwr.h>
#include "gpio.h"
#include "delay.h"
#include "geiger_counter.h"
#include "communication.h"
#include "rtc.h"
#include "rtc_extra.h"
#include "error.h"
#include "debug.h"
#include "util.h"

/* Event which triggers the counting interrupt */
#define CONFIG_XINT_EVENT	IRQ_RISING_EDGE

/* Size of the buffer used to send data */
#define DATA_BUFF_SIZE		256

int initPower()
{
	if (ADI_PWR_SUCCESS != adi_pwr_Init())
		return 0;

	if (ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u))
		return 0;

	if (ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u))
		return 0;

	return 1;
}

/* Called each second by the inboard Real Time Clock and used to measure time */
void rtc_callback(void *ctx, uint32_t data, void *extra) {
	static int	count = 0;
	bool		*is_read = ctx;

	count++;
	if (count == SAMPLING_PERIOD) {
		*is_read = true;
		count = 0;
	}
}

/* Check if SAMPLING_PERIOD have passed */
bool is_mesurment_time(bool *is_ready)
{
	if (*is_ready) {
		*is_ready = false;
		return true;
	}

	return false;
}

/* Initialize all strucures needed by the geiger_counter and for comunication */
int32_t setup(struct geiger_counter **counter,struct uart_desc **comm_desc,
		struct rtc_desc **rtc, struct irq_ctrl_desc **irq_ctrl,
		bool *is_ready)
{
	struct irq_init_param 			irq_init_param;
	struct gpio_init_param			gpio_counter_param;
	struct geiger_counter_init_parma 	init_param;
	struct rtc_init_param			rtc_param;
	struct rtc_irq_config			rtc_config;
	struct callback_desc			call;
	int32_t					ret;

	/* Initialize interrupt controller */
	irq_init_param = (struct irq_init_param) {
				.irq_ctrl_id = 0,
				.extra = NULL
				};
	irq_ctrl_init(irq_ctrl, &irq_init_param);

	/* Initialize communication over UART or over WIFI*/
	ret = init_communication(comm_desc, *irq_ctrl);
	ON_ERR_PRINT_AND_RET("Init communication failed\n", ret);

	/* Initialize geiger counter structure */
	gpio_counter_param = (struct gpio_init_param) {
				.number = CONFIG_COUNTER_GPIO,
				.extra = NULL
				};
	init_param = (struct geiger_counter_init_parma){
			.irq_desc = *irq_ctrl,
			.irq_id = CONFIG_COUNTER_XINT_ID,
			.irq_config = CONFIG_XINT_EVENT,
			.gpio_init_param = &gpio_counter_param
			};
	ret = init_geiger_counter(counter, &init_param);
	ON_ERR_PRINT_AND_RET("init_geiger_counter failed\n", ret);

	/* Initialize real time clock to be used as time reference.
	 * Will be set to generate a callback each second */
	rtc_param = (struct rtc_init_param){
		.id = 1,
		.load = 0,
		.freq = AUDCM_1HZ,
		.extra = NULL
		};
	ret = rtc_init(rtc, &rtc_param);
	ON_ERR_PRINT_AND_RET("rtc_init failed\n", ret);

	rtc_start(*rtc);
	rtc_config = (struct rtc_irq_config) {
			.rtc_handler = *rtc,
			.active_interrupts = RTC_COUNT_INT
	};
	call = (struct callback_desc){
			.callback = rtc_callback,
			.config = &rtc_config,
			.ctx = is_ready
	};
	ret = irq_register_callback(*irq_ctrl, ADUCM_RTC_INT_ID, &call);
	ON_ERR_PRINT_AND_RET("RTC irq_register_callback failed\n", ret);
	ret = irq_enable(*irq_ctrl, ADUCM_RTC_INT_ID);
	ON_ERR_PRINT_AND_RET("RTC irq_enable failed\n", ret);

	return SUCCESS;
}

int main(int argc, char *argv[])
{
	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();
	if (!initPower())
		return 1;

	/* Initialization */
	int			msg_len;
	char			msg_buff[DATA_BUFF_SIZE];
	bool 			is_ready;
	int32_t			ret;
	struct geiger_counter	*counter;
	struct rtc_desc		*rtc;
	struct irq_ctrl_desc	*irq_ctrl;
	struct uart_desc	*comm_desc;

	is_ready = 0;
	ret = setup(&counter, &comm_desc, &rtc, &irq_ctrl, &is_ready);
	ON_ERR_PRINT_AND_RET("Unable to perform setup\n", ret);

	/* Custom code */
	while (1) {
		/* Will become ready at each RTC 10 seconds alarm */
		if (is_mesurment_time(&is_ready)) {
			calculate_CPM(counter);
			msg_len = serialize_data(counter, msg_buff,
					DATA_BUFF_SIZE);
			pr_debug("Sending measurement\n");
			ret = send_data(comm_desc, msg_buff, msg_len);
			if (IS_ERR_VALUE(ret))
				break;
		}
	}

	pr_debug("Program ended due to an error\n");
	delete_geiger_counter(counter);

	return 0;
}

