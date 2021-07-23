/***************************************************************************//**
 *   @file   cn0503.h
 *   @author Andrei Drimbarean (andrei.drimbarean@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
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

#ifndef CN0503_H_
#define CN0503_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "cli.h"
#include "flash.h"
#include "irq.h"
#include "adpd410x.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define CN0503_CLI_CMD_NO	30
#define CN0503_RAT_NO		8
#define CN0503_BLOCK_FILT_SIZE	10
#define CN0503_DLPF_DEFAULT	6
#define CN0503_CODE_ODR_DEFAULT	50
#define HELP_SHORT_COMMAND	true
#define HELP_LONG_COMMAND	false

#define CN0503_FLASH_BUFF_SIZE	     512
#define CN0503_FLASH_REG_SIZE	     256
#define CN0503_FLASH_MODE_IDX	     509
#define CN0503_FLASH_ODR_IDX	     510
#define CN0503_FLASH_RATM_IDX	     511
#define CN0503_FLASH_MD_ADDR	     0x3E000
#define CN0503_FLASH_UU_ADDR	     0x3E800
#define CN0503_FLASH_FLUO_CALIB_ADDR 0x3D800
#define CN0503_FLASH_FLUO_PARAM_SIZE 3
#define CN0503_FLASH_FLUO_MAX_PAGES  5

#define CN0503_FLUO_DEFAULT_LED_OFF  52
#define CN0503_IMPRESP_MAX_SAMPLES   1950

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/**
 * @enum cn0503_modes
 * @brief Application modes of data display.
 */
enum cn0503_modes {
	/** Code mode */
	CN0503_CODE,
	/** Absolute Ratio mode */
	CN0503_ARAT,
	/** Relative ratio mode */
	CN0503_RRAT,
	/** Instrument 1 mode */
	CN0503_INS1,
	/** Instrument 2 mode */
	CN0503_INS2
};

/**
 * @struct cn0503_init_param
 * @brief Application initialization structure
 */
struct cn0503_init_param {
	/** CLI initialization structure */
	struct cli_init_param cli_param;
	/** ADPD device initialization structure */
	struct adpd410x_init_param adpd4100_param;
	/** Flash controller initialization structure */
	struct flash_init_param flash_param;
	/** Interrupt controller initialization structure */
	struct irq_init_param irq_param;
	/** Manufacturer default flash page start address */
	uint32_t md_flash_page_addr;
	/** User default flash page start address */
	uint32_t uu_flash_page_addr;
	/** User default flash page address for fluorescence calibration data */
	uint32_t fluo_calib_flash_page_addr;
};

/**
 * @struct cn0503_dev
 * @brief Application handler structure
 */
struct cn0503_dev {
	/** CLI handler */
	struct cli_desc *cli_handler;
	/** ADPD device handler */
	struct adpd410x_dev *adpd4100_handler;
	/** Flash controller handler */
	struct flash_dev *flash_handler;
	/** Interrupt controller handler */
	struct irq_ctrl_desc *irq_handler;
	/** Pointer to the CLI commands vector */
	cmd_func *app_cmd_func_tab;
	/** Pointer to the CLI command names vector */
	uint8_t **app_cmd_calls;
	/** Pointer to the CLI command sizes vector */
	uint8_t *app_cmd_size;
	/** Pointer to an impulse response parameters and data */
	struct cn0503_impulse_response *impulse_response;

	/** Rations */
	/** Active ratios mask */
	uint8_t ratmask;
	/** RPN expression for rations */
	uint8_t *arat[CN0503_RAT_NO];
	/** Low Pass Filter bandwidth */
	float rflt[CN0503_RAT_NO];
	/** Application decimation index */
	uint32_t odr_indx;
	/** Output Data Rate in Hz (from 5 to 0.01) */
	float odr;
	/** Level to set alarm for INS2 */
	float alrm_high[CN0503_RAT_NO];
	/** Level to clear alarm for INS2 */
	float alrm_low[CN0503_RAT_NO];
	/** Ratios alarm states */
	uint8_t alrm_state[CN0503_RAT_NO];
	/** Channel baseline ratio */
	float ratb[CN0503_RAT_NO];

	/** Data table */
	/** Device active time slots */
	uint8_t active_slots;
	/** Device two channel time slots */
	uint16_t two_channel_slots;
	/** Code block filter sample size */
	uint8_t code_filter_size;
	/** Block average data index */
	uint8_t data_channel_indx;
	/** Data digital filter size */
	uint8_t data_sizes[ADPD410X_MAX_SLOT_NUMBER];
	/** Samples after the block filter */
	uint32_t data_channel[ADPD410X_MAX_SLOT_NUMBER * 2];
	/** Samples before the block filter */
	float ch_data_filt[CN0503_RAT_NO][CN0503_BLOCK_FILT_SIZE];
	/** Digital low-pass filter window */
	uint32_t lpf_window[CN0503_RAT_NO];
	/** Absolute ratio before digital low-pass filter */
	float *arat_flt_data[CN0503_RAT_NO];
	/** Absolute ratio after digital low-pass filter */
	float arat_data[CN0503_RAT_NO];
	/** Relative ratio data buffer */
	float rrat_data[CN0503_RAT_NO];
	/** Instrument 1 data buffer */
	float ins1_data[CN0503_RAT_NO];
	/** Instrument 2 data buffer */
	float ins2_data[CN0503_RAT_NO];

	/** Third order polynomial terms for instrument 1 */
	/** Zero order coefficients for instrument 1 */
	float ins1_p0[CN0503_RAT_NO];
	/** First order coefficients for instrument 1 */
	float ins1_p1[CN0503_RAT_NO];
	/** Second order coefficients for instrument 1 */
	float ins1_p2[CN0503_RAT_NO];
	/** Third order coefficients for instrument 1 */
	float ins1_p3[CN0503_RAT_NO];
	/** Fourth order coefficients for instrument 1 */
	float ins1_p4[CN0503_RAT_NO];
	/** Fifth order coefficients for instrument 1 */
	float ins1_p5[CN0503_RAT_NO];

	/** Third order polynomial terms for instrument 2 */
	/** Zero order coefficients for instrument 2 */
	float ins2_p0[CN0503_RAT_NO];
	/** First order coefficients for instrument 2 */
	float ins2_p1[CN0503_RAT_NO];
	/** Second order coefficients for instrument 2 */
	float ins2_p2[CN0503_RAT_NO];
	/** Third order coefficients for instrument 2 */
	float ins2_p3[CN0503_RAT_NO];
	/** Fourth order coefficients for instrument 2 */
	float ins2_p4[CN0503_RAT_NO];
	/** Fifth order coefficients for instrument 2 */
	float ins2_p5[CN0503_RAT_NO];

	/** Chip id */
	uint16_t chip_id;
	/** Current data mode */
	uint8_t mode;
	/** Stream counter */
	uint32_t stream_count;
	/** Stream counter flag */
	uint8_t stream_cnt_en;
	/** Idle state value */
	uint8_t idle_state;
	/** Relative ratio unit subtracting */
	uint8_t rrat_sube[CN0503_RAT_NO];

	/** Flash software buffer */
	uint32_t sw_flash_buffer[CN0503_FLASH_BUFF_SIZE];
	/** Manufacturer default flash page address */
	uint32_t md_flash_page_addr;
	/** User default flash page address */
	uint32_t uu_flash_page_addr;
	/** User default flash page address */
	uint32_t fluo_calib_flash_page_addr;
};

struct cn0503_impulse_response {
	struct timer_desc *timer;
	/** Parameters */
	uint8_t chann_no;

	uint8_t nb_fifo_samples;
	uint8_t average_length;
	uint8_t method;
	uint16_t nb_data_cycles;
	uint16_t nb_samples;

	uint8_t led_width;
	uint8_t led_offset;
	int8_t first_sample_offset;

	uint8_t data_size;

	float sample_period;
	uint8_t sample_period_lower;
	uint8_t sample_period_upper;

	/** Interrupts */
	struct callback_desc* irq_cb;

	/** Configuration */
	struct reg_config *registers;
	uint8_t nb_reg_writes;

	/** For rollback */
	uint8_t prev_active_slots;
	bool interrupt_set;

	/** Data arrays */
	uint32_t *data;
	float *averaged_data;

	/** Counters */
	uint16_t data_cycle;
	bool data_ready;

	/** Callback */
	void (*success_callback)(struct cn0503_dev*);

	/** For fluorescence decay measurement, unused otherwise */
	bool skip_calib;
	uint8_t calib_slot;
	uint8_t ref_chann_no;
	uint8_t ref_led_width;
	int8_t ref_start_time;
	uint8_t ref_method;
	float ref_end_time;
	float ref_sample_period;
};

struct reg_config {
	uint16_t addr;
	uint16_t value;
	uint16_t bitmask;
	uint16_t rollback_value;
};


/**
 * @enum cn0503_code_id
 * @brief Device time slot and channel IDs
 */
enum cn0503_code_id {
	/** Time slot A channel 1 */
	CN0503_A1,
	/** Time slot A channel 2 */
	CN0503_A2,
	/** Time slot B channel 1 */
	CN0503_B1,
	/** Time slot B channel 2 */
	CN0503_B2,
	/** Time slot C channel 1 */
	CN0503_C1,
	/** Time slot C channel 2 */
	CN0503_C2,
	/** Time slot D channel 1 */
	CN0503_D1,
	/** Time slot D channel 2 */
	CN0503_D2,
	/** Time slot E channel 1 */
	CN0503_E1,
	/** Time slot E channel 2 */
	CN0503_E2,
	/** Time slot F channel 1 */
	CN0503_F1,
	/** Time slot F channel 2 */
	CN0503_F2,
	/** Time slot G channel 1 */
	CN0503_G1,
	/** Time slot G channel 2 */
	CN0503_G2,
	/** Time slot H channel 1 */
	CN0503_H1,
	/** Time slot H channel 2 */
	CN0503_H2,
	/** Time slot I channel 1 */
	CN0503_I1,
	/** Time slot I channel 2 */
	CN0503_I2,
	/** Time slot J channel 1 */
	CN0503_J1,
	/** Time slot J channel 2 */
	CN0503_J2,
	/** Time slot K channel 1 */
	CN0503_K1,
	/** Time slot K channel 2 */
	CN0503_K2,
	/** Time slot L channel 1 */
	CN0503_L1,
	/** Time slot L channel 2 */
	CN0503_L2
};

/**
 * @enum cn0503_operations
 * @brief ARAT operation ID
 */
enum cn0503_operations {
	/** Plus operation ID */
	CN0503_PLUS,
	/** Minus operation ID */
	CN0503_MINUS,
	/** Multiplication operation ID */
	CN0503_MULT,
	/** Division operation ID */
	CN0503_DIV
};

/**
 * @enum cn0503_defparam_var
 * @brief DEF command parameter IDs
 */
enum cn0503_defparam_var {
	/** Absolute ratio (ARAT) parameter */
	CN0503_ARAT_VAR,
	/** Rolling filter (RFLT) parameter */
	CN0503_RFLT_VAR,
	/** Alarm parameter */
	CN0503_ALRM_VAR,
	/** Ratio baseline (RATB) parameter */
	CN0503_RATB_VAR,
	/** Instrument 1 parameter */
	CN0503_INS1_VAR,
	/** Instrument 2 parameter */
	CN0503_INS2_VAR,
	/** Subratction enable (SUBE) parameter */
	CN0503_SUBE_VAR,
	/** Maximum number of DEF command parameters */
	CN0503_MAX_VAR_NUMBER
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/** Display the help tooltip for the CLI. */
int32_t cn0503_help(struct cn0503_dev *dev, uint8_t *arg);

/** Display the help tooltip for the CLI flash commands. */
int32_t cn0503_help_flash(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to read a register from the ADPD device. */
int32_t cn0503_reg_read(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to write a register from the ADPD device. */
int32_t cn0503_reg_write(struct cn0503_dev *dev, uint8_t *arg);

/** Display the stream mode of the application. */
int32_t cn0503_mode_get(struct cn0503_dev *dev, uint8_t *arg);

/** Set the stream mode of the application. */
int32_t cn0503_mode_set(struct cn0503_dev *dev, uint8_t *arg);

/** Display stream header at the start of the stream. */
int32_t cn0503_stream(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to display the idle state. */
int32_t cn0503_idle_get(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to set the idle state. */
int32_t cn0503_idle_set(struct cn0503_dev *dev, uint8_t *arg);

/** Display the baseline value used in calculating relative ratio. */
int32_t cn0503_baseline_get(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to get and display the low and high alarm thresholds. */
int32_t cn0503_alarm_get(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to get and display the output data rate. */
int32_t cn0503_odr_get(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to set the output data rate. */
int32_t cn0503_odr_set(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to get and display the optical paths mask. */
int32_t cn0503_ratmask_get(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to set the optical paths mask. */
int32_t cn0503_ratmask_set(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to do software reset of the application. */
int32_t cn0503_reboot(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to clear the flash software buffer. */
int32_t cn0503_flash_swbuff_clear(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to load the flash software buffer from the flash memory. */
int32_t cn0503_flash_swbuff_load(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to program a flash page with the data from the buffer. */
int32_t cn0503_flash_program(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to erase a flash page from the flash memory. */
int32_t cn0503_flash_erase(struct cn0503_dev *dev, uint8_t *arg);

/**
 * CLI command to apply the settings stored in the flash buffer to the
 * program.
 */
int32_t cn0503_flash_apply(struct cn0503_dev *dev, uint8_t *arg);

/** Update a flash buffer parameter. */
int32_t cn0503_flash_write(struct cn0503_dev *dev, uint8_t *arg);

/**
 * Read and display an application parameter stored in the flash software
 * buffer.
 */
int32_t cn0503_flash_read(struct cn0503_dev *dev, uint8_t *arg);

/**
 * CLI command to calibrate LED current with respect to percent of ADC
 * full-scale.
 */
int32_t cn0503_led(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to preset a channel for a specific function. */
int32_t cn0503_channel_preset(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to dump flash software buffer data. */
int32_t cn0503_flash_swbuf_dump(struct cn0503_dev *dev, uint8_t *arg);

/** CLI command to measure a channel's impulse response. */
int32_t cn0503_impulse_response(struct cn0503_dev *dev, uint8_t *arg);

/** Application CLI prompt at the beginning of the program. */
int32_t cn0503_prompt(struct cn0503_dev *dev);

/** Application main process. */
int32_t cn0503_process(struct cn0503_dev *dev);

/** Set values for the program initialization structure. */
void cn0503_get_config(struct cn0503_init_param *init_param);

/** Initial process of the application. */
int32_t cn0503_init(struct cn0503_dev **device,
		    struct cn0503_init_param *init_param);

/** Free memory allocated by cn0503_init(). */
int32_t cn0503_remove(struct cn0503_dev *dev);

#endif /* CN0503_H_ */
