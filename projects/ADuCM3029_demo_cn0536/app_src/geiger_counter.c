/***************************************************************************//**
 *   @file   geiger_counter.c
 *   @brief  Implementation of CN0536 logic
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

#include "geiger_counter.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "gpio.h"
#include "error.h"
#include "debug.h"

/* Called at each geiger pulse */
void counter_callback(void *ctx, uint32_t event, void *extra)
{
	struct geiger_counter *desc = ctx;

	desc->count++;
}

/* Initialize Geiger counter structure */
int32_t init_geiger_counter(struct geiger_counter **desc,
		struct geiger_counter_init_parma *param)
{
	struct callback_desc callback_desc;
	struct geiger_counter *ldesc;
	int32_t			ret;

	if (!desc || !param)
		return -EINVAL;

	ldesc =  (struct geiger_counter *)calloc(1, sizeof(*ldesc));
	if (!ldesc) {
		*desc = NULL;
		return -ENOMEM;
	}

	/* Config counter gpio */
	ret = gpio_get(&ldesc->counter_gpio, param->gpio_init_param);
	ON_ERR_PRINT_AND_RET("Unable to get gpio\n", ret);
	ret = gpio_direction_input(ldesc->counter_gpio);
	ON_ERR_PRINT_AND_RET("Unable to set gpio\n", ret);

	/* Config counter IRQ */
	callback_desc = (struct callback_desc) {
		.callback = counter_callback,
		.ctx = ldesc,
		.config = (void *)param->irq_config
	};
	ldesc->irq_id = param->irq_id;
	ldesc->irq_desc = param->irq_desc;
	ret = irq_register_callback(ldesc->irq_desc, param->irq_id,
			      &callback_desc);
	ON_ERR_PRINT_AND_RET("Counter irq_register_callback failed\n", ret);

	ret = irq_enable(ldesc->irq_desc, param->irq_id);
	ON_ERR_PRINT_AND_RET("Counter irq_enable failed\n", ret);

	/* Fill geiger_counter descriptor */
	memset(ldesc->samples, 0xFF, sizeof(ldesc->samples));
	ldesc->count = 0;
	ldesc->count_per_minute = 0;
	ldesc->ready = false;
	*desc = ldesc;

	return SUCCESS;
}

void delete_geiger_counter(struct geiger_counter *desc)
{
	gpio_remove(desc->counter_gpio);
	irq_unregister(desc->irq_desc, desc->irq_id);
	irq_disable(desc->irq_desc, desc->irq_id);
	free(desc);
}

/* This function will be called each SAMPLING_PERIOD seconds */
void calculate_CPM(struct geiger_counter *desc)
{
	uint32_t sum;
	uint32_t new_sample;
	uint32_t i;

	new_sample = desc->count - desc->last_count;
	desc->last_count = desc->count;

	sum = new_sample;
	if (desc->samples[0] == (uint64_t)(-1)) {//If it is the first read
		/* Fill samples with the first value */
		for (i = 0; i < NB_AVARGE_SAMPLES; i++) {
			desc->samples[i] = new_sample;
		}
	}
	/* Shift samples and calculate sum */
	for (i = 1; i < NB_AVARGE_SAMPLES; i++) {
		sum += desc->samples[i];
		desc->samples[i - 1] = desc->samples[i];
	}
	desc->samples[NB_AVARGE_SAMPLES - 1] = new_sample;

	desc->count_per_minute = ((float)sum / NB_AVARGE_SAMPLES) *
			(60 / SAMPLING_PERIOD);
	desc->us_per_hour = (float)desc->count_per_minute * CONVERSION_FACTOR;
}

/* Print data in buffer in a more readable format */
int serialize_data(struct geiger_counter *desc, char *buff, int32_t buff_size)
{
	return snprintf(buff, buff_size,
			"Total count:%6" PRIu64 "; CPM: %5" PRIu32"; "
			"us_per_hour: %3.2f\r\n",
			desc->count, desc->count_per_minute, desc->us_per_hour);
}
