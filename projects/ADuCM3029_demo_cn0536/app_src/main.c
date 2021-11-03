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
#include "aducm3029_gpio.h"
#include "delay.h"
#include "geiger_counter.h"
#include "communication.h"
#include "rtc.h"
#include "rtc_extra.h"
#include "error.h"
#include "gpio.h"
#include "debug.h"
#include "util.h"

/* Event which triggers the counting interrupt */
#define CONFIG_XINT_EVENT	IRQ_RISING_EDGE

/* Size of the buffer used to send data */
#define DATA_BUFF_SIZE		256

#define GREEN_LED_NB		32
#define BLUE_LED_NB		31
#define LED_ON			GPIO_HIGH
#define LED_OFF			GPIO_LOW

enum status_e {
	STATUS_SETUP,
	STATUS_OK,
	STATUS_ERR
};

struct gpio_desc *green_led;
struct gpio_desc *blue_led;

void update_led_status(enum status_e status)
{
	if (!green_led || !blue_led) {
		struct gpio_init_param par = {
			.platform_ops = &aducm_gpio_ops
		};
		par.number = GREEN_LED_NB;
		gpio_get(&green_led, &par);
		gpio_direction_output(green_led, LED_OFF);
		par.number = BLUE_LED_NB;
		gpio_get(&blue_led, &par);
		gpio_direction_output(blue_led, LED_OFF);
	}

	switch (status) {
	case STATUS_SETUP:
		gpio_set_value(green_led, LED_OFF);
		gpio_set_value(blue_led, LED_ON);
		break;
	case STATUS_OK:
		gpio_set_value(green_led, LED_ON);
		gpio_set_value(blue_led, LED_OFF);
		break;
	case STATUS_ERR:
		;
		int32_t val = LED_ON;
		while (true) {
			gpio_set_value(green_led, val);
			gpio_set_value(blue_led, !val);
			val = !val;
			mdelay(1000);
		}
		break;
	default:
		break;
	}
}


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

volatile bool is_ready;

/* Called each second by the inboard Real Time Clock and used to measure time */
void rtc_callback(void *ctx, uint32_t data, void *extra)
{
	static int	count = 0;

	count++;
	if (count == SAMPLING_PERIOD) {
		is_ready = true;
		count = 0;
	}
}

/* Check if SAMPLING_PERIOD have passed */
bool is_mesurment_time()
{
	if (is_ready) {
		is_ready = false;
		return true;
	}

	return false;
}

/* Initialize all strucures needed by the geiger_counter and for comunication */
int32_t setup(struct geiger_counter **counter,union comm_desc *comm_desc,
	      struct rtc_desc **rtc, struct irq_ctrl_desc **irq_ctrl)
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
		.platform_ops = &aducm_irq_ops,
		.extra = NULL
	};
	irq_ctrl_init(irq_ctrl, &irq_init_param);

	/* Initialize communication over UART or over WIFI*/
	ret = init_communication(comm_desc, *irq_ctrl);
	ON_ERR_PRINT_AND_RET("Init communication failed\n", ret);

	/* Initialize geiger counter structure */
	gpio_counter_param = (struct gpio_init_param) {
		.number = CONFIG_COUNTER_GPIO,
		.platform_ops = &aducm_gpio_ops,
		.extra = NULL
	};
	init_param = (struct geiger_counter_init_parma) {
		.irq_desc = *irq_ctrl,
		.irq_id = CONFIG_COUNTER_XINT_ID,
		.irq_config = CONFIG_XINT_EVENT,
		.gpio_init_param = &gpio_counter_param
	};
	ret = init_geiger_counter(counter, &init_param);
	ON_ERR_PRINT_AND_RET("init_geiger_counter failed\n", ret);

	/* Initialize real time clock to be used as time reference.
	 * Will be set to generate a callback each second */
	rtc_param = (struct rtc_init_param) {
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
	call = (struct callback_desc) {
		.callback = rtc_callback,
		.config = &rtc_config,
		.ctx = NULL
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
	int32_t			ret;
	struct geiger_counter	*counter;
	struct rtc_desc		*rtc;
	struct irq_ctrl_desc	*irq_ctrl;
	union comm_desc		comm_desc;

	is_ready = 0;
	update_led_status(STATUS_SETUP);
	ret = setup(&counter, &comm_desc, &rtc, &irq_ctrl);
	if (IS_ERR_VALUE(ret)) {
		printf("Unable to perform setup\n");
		update_led_status(STATUS_ERR);
	}
	update_led_status(STATUS_OK);
	/* Custom code */
	while (1) {
		/* Will become ready at each RTC 10 seconds alarm */
		if (is_mesurment_time()) {
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
	update_led_status(STATUS_ERR);

	return 0;
}

