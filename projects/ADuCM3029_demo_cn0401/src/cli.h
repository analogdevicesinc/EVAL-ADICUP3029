/***************************************************************************//**
*   @file   cli.h
*   @brief  CLI header.
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

#ifndef CLI_H_
#define CLI_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "platform_source/platform_drivers.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define _NC 0
#define _BS 8
#define _CR 13
#define _LF 10
#define _TB 9
#define _ESC 27
#define _SP 32
#define _HM 2

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

typedef  int32_t (*cmd_func)(void *, uint8_t *);

struct cli_init_param {
	struct uart_init_param uart_init;
};

struct cli_desc {
	struct uart_desc *uart_device;
	uint8_t **cmd_commands;
	cmd_func *v_cmd_fun;
	void *device_descriptor;
	uint8_t *command_size;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Setup the CLI module of the application. */
int32_t cli_setup(struct cli_desc **device, struct cli_init_param *init_param);

/* Free resources allocated by cli_setup(). */
int32_t cli_remove(struct cli_desc *dev);

/* Implements CLI feedback. */
int32_t cli_parse(struct cli_desc *dev);

/* Implements the CLI logic. */
int32_t cli_process(struct cli_desc *dev);

/* Get the CLI commands and correlate them to functions. */
void cli_find_command(struct cli_desc *dev, uint8_t *command,
		      cmd_func* function);

/* Display command prompt for the user on the CLI at the beginning of the
 * program. */
int32_t cli_cmd_prompt(struct cli_desc *dev, uint8_t *app_name);

/* Update the current command vector. */
void cli_load_command_vector(struct cli_desc *dev, cmd_func *command_vector);

/* Update the current command call vector. */
void cli_load_command_calls(struct cli_desc *dev, uint8_t **command_calls);

/* Update the current command size vector. */
void cli_load_command_sizes(struct cli_desc *dev, uint8_t *command_sizes);

/* Update the current handler pointer. */
void cli_load_descriptor_pointer(struct cli_desc *dev,
				 void *command_descriptor);

#endif /* CLI_H_ */
