/***************************************************************************//**
 *   @file   adpd410x_app_config.h
 *   @brief  ADPD410x Application Configuration File
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
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

#ifndef ADPD410X_APP_CONFIG_H_
#define ADPD410X_APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stddef.h>
#include "adpd410x.h"
#include "app_config.h"
#ifndef ADPD4101_SUPPORT
#include "spi_extra.h"
#else
#include "i2c_extra.h"
#endif
#include "aducm3029_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
#ifndef ADPD410X_APP_DEMO_WQ
#define ADPD410X_ACTIVE_TIMESLOTS 8
#define ADPD410X_REG_DEFAULT_NR   31
#else
#define ADPD410X_ACTIVE_TIMESLOTS 4
#define ADPD410X_REG_DEFAULT_NR   13
#endif
#define ADPD410X_APP_CODE_ODR_DEFAULT	50

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
#ifndef ADPD4101_SUPPORT
struct aducm_spi_init_param aducm_spi_init = {
	.continuous_mode = true,
	.dma = false,
	.half_duplex = false,
	.master_mode = MASTER,
};
#endif

static struct adpd410x_init_param adpd4100_param = {
#ifndef ADPD4101_SUPPORT
	.dev_ops_init = {
		.spi_phy_init = {
			.device_id = 0,
			.max_speed_hz = 1000000,
			.chip_select = 1,
			.mode = NO_OS_SPI_MODE_0,
			.extra = &aducm_spi_init,
			.platform_ops = &aducm_spi_ops
		},
	},
	.dev_type = ADPD4100,
#else
	.dev_ops_init = {
		.i2c_phy_init = {
			.max_speed_hz = 400000,
			.slave_address = 0x24,
			.extra = NULL,
			.platform_ops = &aducm_i2c_ops
		},
	},
	.dev_type = ADPD4101,
#endif
	.clk_opt = ADPD410X_INTLFO_INTHFO,
	.ext_lfo_freq = 0,
	.gpio0 = {
#ifndef ADPD4101_SUPPORT
		.number = 0x08,
#else
		.number = 0x0F,
#endif
		.extra = NULL,
		.platform_ops = &aducm_gpio_ops
	},
	.gpio1 = {
		.number = 0x0D,
		.extra = NULL,
		.platform_ops = &aducm_gpio_ops
	},
	.gpio2 = {
		.number = 0x09,
		.extra = NULL,
		.platform_ops = &aducm_gpio_ops
	},
	.gpio3 = {
		.number = 0x21,
		.extra = NULL,
		.platform_ops = &aducm_gpio_ops
	},
};

#ifndef ADPD410X_APP_DEMO_WQ
/* Timeslots Configuration */
static struct adpd410x_timeslot_init ts_init_tab[] = {
	{
		.ts_inputs = {
			.option = ADPD410X_INaCH1_INbDIS,
			.pair = ADPD410X_INP12,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x40,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.enable_ch2 = false,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
	{
		.ts_inputs = {
			.option = ADPD410X_INaDIS_INbCH1,
			.pair = ADPD410X_INP12,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x40,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.enable_ch2 = false,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
	{
		.ts_inputs = {
			.option = ADPD410X_INaCH1_INbDIS,
			.pair = ADPD410X_INP34,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x40,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.enable_ch2 = false,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
	{
		.ts_inputs = {
			.option = ADPD410X_INaDIS_INbCH1,
			.pair = ADPD410X_INP34,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x40,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.enable_ch2 = false,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
	{
		.ts_inputs = {
			.option = ADPD410X_INaCH1_INbDIS,
			.pair = ADPD410X_INP56,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x40,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.enable_ch2 = false,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
	{
		.ts_inputs = {
			.option = ADPD410X_INaDIS_INbCH1,
			.pair = ADPD410X_INP56,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x40,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.enable_ch2 = false,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
	{
		.ts_inputs = {
			.option = ADPD410X_INaCH1_INbDIS,
			.pair = ADPD410X_INP78,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x40,
			},
		},
		.enable_ch2 = false,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
	{
		.ts_inputs = {
			.option = ADPD410X_INaDIS_INbCH1,
			.pair = ADPD410X_INP78,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x40,
			},
		},
		.enable_ch2 = false,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
};

/* Register Configuration (address + data) */
uint16_t reg_config_default[31][2] = {
	/** General configuration */
	{0x0001, 0x000F},

	/** AFE Path */
	{0x0101, 0x40DA},
	{0x0121, 0x40DA},
	{0x0141, 0x40DA},
	{0x0161, 0x40DA},
	{0x0181, 0x40DA},
	{0x01A1, 0x40DA},
	{0x01C1, 0x40DA},
	{0x01E1, 0x40DA},

	/** 3us AFE width double sided */
	{0x010A, 0x0003},
	{0x012A, 0x0003},
	{0x014A, 0x0003},
	{0x016A, 0x0003},
	{0x018A, 0x0003},
	{0x01AA, 0x0003},
	{0x01CA, 0x0003},
	{0x01EA, 0x0003},

	/** 2048 digital offset */
	{0x010F, 0x8000},
	{0x012F, 0x8000},
	{0x014F, 0x8000},
	{0x016F, 0x8000},
	{0x018F, 0x8000},
	{0x01AF, 0x8000},
	{0x01CF, 0x8000},
	{0x01EF, 0x8000},
};

#else
/* Timeslots Configuration */
static struct adpd410x_timeslot_init ts_init_tab[] = {
	{
		.ts_inputs = {
			.option = ADPD410X_INaCH1_INbCH2,
			.pair = ADPD410X_INP12,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x40,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x40,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.enable_ch2 = true,
		.precon_option = ADPD410X_VC1,
		.afe_trim_opt = ADPD410X_TIA_VREF_0V8855,
		.vref_pulse_opt = ADPD410X_TIA_VREF_0V8855,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_50K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 4,
		.dec_factor = 0,
		.repeats_no = 32,
		.adc_cycles = 1,
	},
	{
		.ts_inputs = {
			.option = ADPD410X_INaCH1_INbCH2,
			.pair = ADPD410X_INP34,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x00,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x40,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x40,
			},
		},
		.enable_ch2 = true,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
	{
		.ts_inputs = {
			.option = ADPD410X_INaCH1_INbCH2,
			.pair = ADPD410X_INP56,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x40,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x40,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.enable_ch2 = true,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
	{
		.ts_inputs = {
			.option = ADPD410X_INaCH1_INbCH2,
			.pair = ADPD410X_INP78,
		},
		.led1 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led2 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_A,
				.let_current_select = 0x00,
			},
		},
		.led3 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x40,
			},
		},
		.led4 = {
			.fields = {
				.led_output_select = ADPD410X_OUTPUT_B,
				.let_current_select = 0x40,
			},
		},
		.enable_ch2 = true,
		.precon_option = ADPD410X_TIA_VREF,
		.afe_trim_opt = ADPD410X_TIA_VREF_1V256,
		.vref_pulse_opt = ADPD410X_TIA_VREF_1V256,
		.chan2 = ADPD410X_TIA_VREF_50K,
		.chan1 = ADPD410X_TIA_VREF_100K,
		.pulse4_subtract = 0xA,
		.pulse4_reverse = 0xA,
		.byte_no = 3,
		.dec_factor = 0,
		.repeats_no = 1,
		.adc_cycles = 1,
	},
};

/* Register Configuration (address + data) */
uint16_t reg_config_default[13][2] = {
	/** General configuration */
	{0x0001, 0x000F},

	/** AFE Path */
	{0x0101, 0x40DA},
	{0x0121, 0x40DA},
	{0x0141, 0x40DA},
	{0x0161, 0x40DA},

	/** 3us AFE width double sided */
	{0x010A, 0x0003},
	{0x012A, 0x0003},
	{0x014A, 0x0003},
	{0x016A, 0x0003},

	/** 2048 digital offset */
	{0x010F, 0x8000},
	{0x012F, 0x8000},
	{0x014F, 0x8000},
	{0x016F, 0x8000},
};

#endif

#endif /* ADPD410X_APP_CONFIG_H_ */
