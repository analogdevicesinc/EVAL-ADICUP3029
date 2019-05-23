/***************************************************************************//**
*   @file   cn0418.h
*   @brief  CN0418 driver application header.
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

#ifndef CN0418_H_
#define CN0418_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "platform_drivers.h"
#include "swuart.h"
#include "ad5755.h"
#include "ad5700.h"
#include "memory.h"

#define _NC 0
#define _BS 8
#define _CR 13
#define _LF 10
#define _TB 9

#define HOLDING_REGS_PREFIX '4'

#define HART_BUFF_SIZE 256
#define HART_COMMAND_ZERO_SIZE 5
#define HART_TERMINATOR_CHARACTER_SIZE 1
#define HART_NOTHING_RECEIVED -3
#define HART_PREAMBLE_CHAR 0xFF
#define HART_SHORT_ADDR_RESPONSE 0x06
#define HART_LONG_ADDR_RESPONSE 0x86

#define CHANNEL_NUMBER 4

#define HELP_SHORT_COMMAND true
#define HELP_LONG_COMMAND false

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* HART statuses */
enum hart_status {
	HART_DISABLE,
	HART_ENABLE
};

/* HART channels */
enum hart_channels {
	CH1,
	CH2,
	CH3,
	CH4,
	HART_CHANNEL_NO
};

/* Device initialization parameters */
struct cn0418_init_param {
	/* Devices */
	struct ad5700_init_param ad5700_init;
	struct ad5755_init_param ad5755_init;
	struct uart_init_param usr_uart_init;
	struct memory_init_param memory_init;
	/* GPIO */
	uint8_t hart_mult_a0;
	uint8_t hart_mult_a1;
};

/* Device descriptor */
struct cn0418_dev {
	/* Devices */
	struct ad5755_dev *ad5755_dev;
	struct uart_desc  *usr_uart_dev;
	struct ad5700_dev *ad5700_device;
	struct memory_desc *memory_device;
	/* GPIO */
	struct gpio_desc *hart_mult_a0;
	struct gpio_desc *hart_mult_a1;
	/* Varibles */
	uint8_t hart_buffer[HART_BUFF_SIZE];
	uint16_t hart_rec_size;
};

typedef  int32_t (*cmd_func)(struct cn0418_dev*, uint8_t*);

/* Initializes the cn0418 device. */
int32_t cn0418_setup(struct cn0418_dev **device,
		     struct cn0418_init_param *init_param);

/* Free the resources allocated by cn0418_setup(). */
int32_t cn0418_remove(struct cn0418_dev *dev);

/* Display help options in the CLI. */
int32_t cn0418_help(struct cn0418_dev *dev, uint8_t *arg);

/* Display the status of the application. */
int32_t cn0418_status(struct cn0418_dev *dev, uint8_t *arg);

/* Do a software reset of the device. */
int32_t cn0418_reset(struct cn0418_dev *dev, uint8_t *arg);

/* Enable HART modem. */
int32_t cn0418_hart_enable(struct cn0418_dev *dev, uint8_t* arg);

/* Disable HART modem. */
int32_t cn0418_hart_disable(struct cn0418_dev *dev, uint8_t* arg);

/* Change the active HART channel. */
int32_t cn0418_hart_change_channel(struct cn0418_dev *dev, uint8_t* arg);

/* Make a HART transmission. */
int32_t cn0418_hart_transmit(struct cn0418_dev *dev, uint8_t* arg);

/* Receive a HART transmission. */
int32_t cn0418_hart_get_rec(struct cn0418_dev *dev, uint8_t* arg);

/* Send HART command zero. */
int32_t cn0418_hart_send_command_zero(struct cn0418_dev *dev, uint8_t* arg);

/* This method sends the provided test byte through the HART link continuously
 * until stopped by pressing q. Function used to test the HART physical
 * layer. */
int32_t cn0418_hart_phy_test(struct cn0418_dev *dev, uint8_t* arg);

/* Set the range of a channel. */
int32_t cn0418_set_channel_range(struct cn0418_dev *dev, uint8_t *arg);

/* Set voltage on a channel that has a voltage range selected. */
int32_t cn0418_set_channel_voltage(struct cn0418_dev *dev, uint8_t *arg);

/* Set current on a channel that has a current range selected. */
int32_t cn0418_set_channel_current(struct cn0418_dev *dev, uint8_t *arg);

/* Set code on a channel. */
int32_t cn0418_set_channel_code(struct cn0418_dev *dev, uint8_t *arg);

/* Set code on a channel. */
int32_t cn0418_set_channel_rset(struct cn0418_dev *dev, uint8_t *arg);

/* Display EEPROM address. */
int32_t cn0418_mem_display_addr(struct cn0418_dev *dev, uint8_t* arg);

/* Implements CLI feedback. */
int32_t cn0418_parse(struct cn0418_dev *dev);

/* Implements the CLI logic. */
int32_t cn0418_process(struct cn0418_dev *dev);

/* Get the CLI commands and correlate them to functions. */
void cn0418_find_command(struct cn0418_dev *dev, uint8_t *command,
			 cmd_func* function);

/* Display command prompt for the user on the CLI at the beginning of the
 * program. */
int32_t cn0418_cmd_prompt(struct cn0418_dev *dev);

/* cn0418_hart_change_channel helper function. */
int32_t cn0418_hart_change_chan_helper(struct cn0418_dev *dev,
				       enum hart_channels channel);

/* Convert floating point value to ASCII. Maximum 8 decimals. */
void cn0418_ftoa(uint8_t *buffer, float value);

/* Discover the first EEPROM present on the board. */
int32_t cn0418_mem_discover(struct cn0418_dev *dev, uint8_t start_addr,
			    uint8_t* address);

/* cn0418_status helper function. */
int32_t cn0418_status_helper(struct cn0418_dev *dev, uint8_t index);

#endif /* CN0418_H_ */
