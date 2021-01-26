/***************************************************************************//**
 *   @file   geiger_counter.h
 *   @brief  Geiger counter interface
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

#ifndef GEIGER_COUNTER_H
#define GEIGER_COUNTER_H

#include "irq.h"
#include "irq_extra.h"
#include <stdint.h>

/* Number of seconds between each measurement */
#define SAMPLING_PERIOD		5//seconds
/* Number of measurements used for average filtering */
#define NB_AVARGE_SAMPLES	5
/* Conversion factor from counts/minute -> microSieverts/minute */
#define CONVERSION_FACTOR	0.01

/*
 * Depending of the jumper soldered on the board one of these 3 options is
 * available for reading the Geiger pulses.
 * Avaliable options are:
 * 	- 2 : JP2 soldered (default on board)
 * 	- 3 : JP3 soldered
 * 	- 4 : JP4 soldered
 */
#define JUMPER_CONFIG	2

#if JUMPER_CONFIG == 2
#define CONFIG_COUNTER_XINT_ID	ADUCM_EXTERNAL_INT3_ID
#define CONFIG_COUNTER_GPIO	33 //On IO33 pin
#elif JUMPER_CONFIG == 3
#define CONFIG_COUNTER_XINT_ID	ADUCM_EXTERNAL_INT2_ID
#define CONFIG_COUNTER_GPIO	13 //On IO13 pin
#elif JUMPER_CONFIG == 4
#define CONFIG_COUNTER_XINT_ID	ADUCM_EXTERNAL_INT0_ID
#define CONFIG_COUNTER_GPIO	15 //On IO15 pin
#endif


struct geiger_counter {
	uint64_t		count;
	uint64_t		last_count;
	uint64_t		samples[NB_AVARGE_SAMPLES];
	uint32_t		count_per_minute;
	float			us_per_hour;
	bool			ready;
	struct gpio_desc	*counter_gpio;
	struct irq_ctrl_desc	*irq_desc;
	uint32_t		irq_id;
};

struct geiger_counter_init_parma {
	/* Irq controller descriptor */
	struct irq_ctrl_desc	*irq_desc;
	/* Id of the interrupts that is used for counting pulses */
	uint32_t		irq_id;
	/* Event that triggers the interrupt */
	enum irq_mode		irq_config;
	/* Gpio init parameter used for the generated interrupt */
	struct gpio_init_param	*gpio_init_param;
};

int32_t init_geiger_counter(struct geiger_counter **desc,
		struct geiger_counter_init_parma *param);
void delete_geiger_counter(struct geiger_counter *desc);
void calculate_CPM(struct geiger_counter *desc);
int serialize_data(struct geiger_counter *desc, char *buff, int32_t buff_size);

#endif /* GEIGER_COUNTER_H */
