/***************************************************************************//**
 *   @file   cn0401.h
 *   @author Andrei Drimbarean (Andrei.Drimbarean@analog.com)
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

#ifndef CN0401_H_
#define CN0401_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "can_obj_layer.h"
#include "cli.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define HELP_SHORT_COMMAND true
#define HELP_LONG_COMMAND false

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

enum can_ctrl_state_machine {
	CAN_IDLE,
	CAN_SLEEP,
	CAN_FD_EN
};

struct cn0401_init_param {
	struct cli_init_param cli_init;
	struct can_ctrl_init_param can_ctrl_init;
	uint8_t aux_gpio;
	uint8_t silent_gpio;
};

struct cn0401_dev {
	struct cli_desc *cli_handler;
	struct can_ctrl_dev *can_ctrl_handler;
	struct intr_dev *intr_handler;
	struct gpio_desc *aux_gpio;
	struct gpio_desc *silent_gpio;
	enum can_ctrl_state_machine state;

	volatile bool rec_flag;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Provide default configuration for the application initialization handler. */
void cn0401_get_config(struct cn0401_init_param *init_param);

/* Display help options in the CLI. */
int32_t cn0401_help(struct cn0401_dev *dev, uint8_t *arg);

/* Transmit a CAN message up to 64 characters. */
int32_t cn0401_can_send(struct cn0401_dev *dev, uint8_t *arg);

/* Set the standard ID that the messages sent use. */
int32_t cn0401_can_set_tx_sid(struct cn0401_dev *dev, uint8_t *arg);

/* CLI command used to call the CAN receive routine. */
int32_t cn0401_can_get_rec(struct cn0401_dev *dev, uint8_t *arg);

/* Production test routine called in CLI. */
int32_t cn0401_can_test_proc(struct cn0401_dev *dev, uint8_t *arg);

/* The CN0401 application main process. */
int32_t cn0401_process(struct cn0401_dev *dev);

/* The application initialization routine. */
int32_t cn0401_setup(struct cn0401_dev **device,
		     struct cn0401_init_param *init_param);

/* Free memory allocated by the cn0401_setup() function. */
int32_t cn0401_remove(struct cn0401_dev *dev);

#endif /* CN0401_H_ */
