/***************************************************************************//**
*   @file   cn0414.h
*   @brief  CN0414 driver application header.
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

#ifndef CN0414_H_
#define CN0414_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "platform_drivers.h"
#include "swuart.h"
#include "ad5700.h"
#include "adc_update_timer.h"
#include "memory.h"
#include "ad717x.h"
#include "config.h"

#define ADC_CHANNEL_NO 8
#define ADC_VOLTAGE_CHAN_NO 4
#define ADC_VOLTAGE_CHANNEL true
#define ADC_CURRENT_CHANNEL false
#define OPEN_WIRE_DETECT_ENABLED 1
#define OPEN_WIRE_DETECT_DISABLED 0

#define ERROR -1

#define HART_BUFF_SIZE 256
#define HART_COMMAND_ZERO_SIZE 5
#define HART_TERMINATOR_CHARACTER_SIZE 1
#define HART_NOTHING_RECEIVED 1
#define HART_PREAMBLE_CHAR 0xFF
#define HART_SHORT_ADDR_RESPONSE 0x06
#define HART_LONG_ADDR_RESPONSE 0x86

#define HELP_SHORT_COMMAND true
#define HELP_LONG_COMMAND false

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* ADC channels */
enum adc_channels {
	VIN1,
	VIN2,
	VIN3,
	VIN4,
	IIN1,
	IIN2,
	IIN3,
	IIN4
};

/* HART channels */
enum hart_channels_cn0414 {
	CH1_CN0414,
	CH2_CN0414,
	CH3_CN0414,
	CH4_CN0414
};

enum odr_values {
	SPS_31K250 = 5,
	SPS_15K625,
	SPS_10K417,
	SPS_5208,
	SPS_2597,
	SPS_1007,
	SPS_503P8,
	SPS_381,
	SPS_200P3,
	SPS_100P5,
	SPS_59P52,
	SPS_49P68,
	SPS_20P01,
	SPS_16P63,
	SPS_10,
	SPS_5,
	SPS_2P5,
	SPS_1P25
};

/* Device initialization parameters */
struct cn0414_ini_param {
	/* Device peripheral */
	struct ad5700_init_param ad5700_init;
	struct uart_init_param uart_ini;
	struct adc_update_init adc_update_init;
	/* GPIO */
	uint8_t gpio_hart_chan0;
	uint8_t gpio_hart_chan1;
	/* Board components */
	ad717x_init_param ad4111_ini;
	struct memory_init_param memory_init;
};

/* Device descriptor */
struct cn0414_dev {
	/* Device peripheral */
	struct ad5700_dev 	   *ad5700_device;
	struct modbus_slave    *slavemb_desc;
	struct uart_desc 	   *uart_descriptor;
	struct adc_update_desc *adc_update_desc;
	/* GPIO */
	struct gpio_desc *gpio_hart_chan0;
	struct gpio_desc *gpio_hart_chan1;
	/* Board components */
	ad717x_dev  *ad4111_device;
	struct memory_desc *memory_device;
	uint32_t channel_output[ADC_CHANNEL_NO]; /* Channel data registers */
	uint32_t open_wire_temp_buffer; /* Buffer used with open-wire detection */
	uint8_t open_wire_first_done; /* Flag used for open-wire detection */
	uint8_t chan_voltage_status[ADC_VOLTAGE_CHAN_NO]; /* Voltage chan status */
	uint8_t channel_index; /* The next channel register to be updated */
	uint8_t open_wire_detect_enable; /* Open-Wire Detection enable flag */
	uint8_t hart_buffer[HART_BUFF_SIZE]; /* HART receive buffer */
	uint16_t hart_rec_size; /* Size of the last received HART transmission */
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

int32_t cn0414_setup_minimum(struct cn0414_dev **device,
			     struct cn0414_ini_param *init_param);

//TODO: COMMENT cn0414_setup_adc_verify_id
int32_t cn0414_setup_adc_verify_id(struct cn0414_dev *dev);

/* Initializes the cn0414 device. */
int32_t cn0414_setup(struct cn0414_dev **device,
		     struct cn0414_ini_param *init_param);

int32_t cn0414_remove_minimum(struct cn0414_dev *dev);

/* Free the resources allocated by cn0414_setup(). */
int32_t cn0414_remove(struct cn0414_dev *dev);

/* Display help options in the CLI. */
int32_t cn0414_help(struct cn0414_dev *dev, uint8_t* arg);

/* Display the status of the application. */
int32_t cn0414_status(struct cn0414_dev *dev, uint8_t* arg);

/* Display the value of the selected channel. */
int32_t cn0414_channel_display(struct cn0414_dev *dev, uint8_t* arg);

/* Set update rate of the channel value registers. It is recoemended that this
 * rate be at most 80 times lower than the output data rate of the ADC. */
int32_t cn0414_channel_set_update_rate(struct cn0414_dev *dev, uint8_t* arg);

/* Enable HART modem. */
int32_t cn0414_hart_enable(struct cn0414_dev *dev, uint8_t* arg);

/* Disable HART modem. */
int32_t cn0414_hart_disable(struct cn0414_dev *dev, uint8_t* arg);

/* cn0414_hart_change_channel helper function. */
int32_t cn0414_hart_change_chan_helper(struct cn0414_dev *dev,
				       enum hart_channels_cn0414 channel);

/* Change the active HART channel. */
int32_t cn0414_hart_change_channel(struct cn0414_dev *dev, uint8_t* arg);

/* Make a HART transmission. */
int32_t cn0414_hart_transmit(struct cn0414_dev *dev, uint8_t* arg);

/* Receive a HART transmission. */
int32_t cn0414_hart_get_rec(struct cn0414_dev *dev, uint8_t* arg);

/* Send HART command zero. */
int32_t cn0414_hart_send_command_zero(struct cn0414_dev *dev, uint8_t* arg);

/* This method sends the provided test byte through the HART link continuously
 * until stopped by pressing q. Function used to test the HART physical
 * layer. */
int32_t cn0414_hart_phy_test(struct cn0414_dev *dev, uint8_t* arg);

/* Read a register of the ADC. */
int32_t cn0414_adc_read_reg(struct cn0414_dev *dev, uint8_t* arg);

/* Write a register of the ADC. */
int32_t cn0414_adc_write_reg(struct cn0414_dev *dev, uint8_t* arg);

/* Get a number of samples from a specified channel. */
int32_t cn0414_adc_get_samples(struct cn0414_dev *dev, uint8_t* arg);

/* Change ADC output data rate. It is recommended to be at least 80 times
 * greater than the channel register update rate. */
int32_t cn0414_adc_set_odr(struct cn0414_dev *dev, uint8_t* arg);

/* Set filter options of the ADC. */
int32_t cn0414_adc_set_filter(struct cn0414_dev *dev, uint8_t* arg);

/* Enable ADC postfilter option. */
int32_t cn0414_adc_en_postfilt(struct cn0414_dev *dev, uint8_t* arg);

/* Disable ADC postfilter option. */
int32_t cn0414_adc_dis_postfilt(struct cn0414_dev *dev, uint8_t* arg);

/* Change the ADC postfilter option. */
int32_t cn0414_adc_set_postfilt(struct cn0414_dev *dev, uint8_t* arg);

//TODO: COMMENT cn0414_adc_set_output_coding_helper
int32_t cn0414_adc_set_output_coding_helper(struct cn0414_dev *dev,
		uint8_t i);

/* Set ADC output coding (unipolar/bipolar). */
int32_t cn0414_adc_set_output_coding(struct cn0414_dev *dev, uint8_t* arg);

/* Enables open-wire detection. */
int32_t cn0414_adc_open_wire_enable(struct cn0414_dev *dev, uint8_t* arg);

/* Disables open-wire detection. */
int32_t cn0414_adc_open_wire_disable(struct cn0414_dev *dev, uint8_t* arg);

/* Display EEPROM address. */
int32_t cn0414_mem_display_addr(struct cn0414_dev *dev, uint8_t* arg);

/* Implements the CLI logic. */
int32_t cn0414_process(struct cn0414_dev *dev);

/* Read one of the channels on the board. */
int32_t cn0414_read_channel(struct cn0414_dev *dev,
			    enum adc_channels channel);

/* Compute value in volts or amperes from an ADC sample. */
int32_t cn0414_compute_adc_value(struct cn0414_dev *dev, uint32_t code,
				 bool ncurr_or_volt, float *result);

/* Convert floating point value to ASCII. Maximum 4 decimals. */
void cn0414_ftoa(uint8_t *buffer, float value);

/* Discover the first EEPROM present on the board. */
int32_t cn0414_mem_discover(struct cn0414_dev *dev, uint8_t start_addr,
			    uint8_t* address);

/* Take two samples from the ADC and determine if the input is floating. */
int32_t cn0414_open_wire_detect(struct cn0414_dev *dev, uint32_t voltage_chan1,
				uint32_t voltage_chan2, uint8_t *floating);

/* Test function. Displays on the terminal all ADC registers at the point where
 * it's inserted in code. */
int32_t cn0414_show_regs(struct cn0414_dev *dev);

/* Test function. Updates all registers with actual values. */
void cn0414_update_regs(struct cn0414_dev *dev);

#endif /* CN0414_H_ */
