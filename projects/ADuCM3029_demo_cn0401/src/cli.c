/***************************************************************************//**
*   @file   cli.c
*   @brief  CLI source.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "cli.h"

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

extern uint8_t uart_current_line[100];
extern uint8_t uart_previous_line[100];
extern uint8_t uart_line_index;
extern uint8_t uart_cmd;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Disable the other interrupts aside from the UART interrupt to not interfere
 * with the UART reception.
 *
 * @param void
 *
 * @return void
 */
static inline void cli_disable_int(void)
{
	/* Only disable interrupts enabled by the application.
	 * Example: NVIC_DisableIRQ(TMR0_EVT_IRQn); */
}

/**
 * Enable back the interrupts disabled by cli_disable_int().
 *
 * @param void
 *
 * @return void
 */
static inline void cli_enable_int(void)
{
	/* Only enable interrupts disabled by the cli_disable_int().
	 * Example: NVIC_EnableIRQ(TMR0_EVT_IRQn); */
}

/**
 * Setup the CLI module of the application.
 *
 * Allocate memory for the driver handler and initialize the physical layer.
 *
 * @param [out] device    - Pointer to the device handler structure.
 * @param [in] init_param - Pointer to the device initialization structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cli_setup(struct cli_desc **device, struct cli_init_param *init_param)
{
	int32_t ret;
	struct cli_desc *dev;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	ret = usr_uart_init(&dev->uart_device, &init_param->uart_init);
	if(ret < 0)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);
	return ret;
}

/**
 * Free resources allocated by cli_setup().
 *
 * @param [in] dev - Pointer to the device descriptor.
 *
 * @return 0 if success, negative error code otherwise.
 */
int32_t cli_remove(struct cli_desc *dev)
{
	int32_t ret;

	if(!dev)
		return -1;

	ret = usr_uart_remove(dev->uart_device);
	if(ret < 0)
		return ret;

	free(dev);

	return ret;
}

/**
 * Implements CLI feedback.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cli_parse(struct cli_desc *dev)
{
	uint8_t c = 0, rdy = 1;
	int8_t i;
	int32_t ret;

	ret = usr_uart_read_char(dev->uart_device, &c, &rdy, UART_NON_BLOCKING);
	if(ret < 0)
		return ret;

	if(rdy == 1) {
		switch(c) {
		case _LF:
		case _CR:
			uart_cmd = UART_TRUE;
			break;
		case _BS:
			if(uart_line_index == 0)
				break;
			uart_line_index--;
			break;
		case _TB:
			usr_uart_write_string(dev->uart_device, (uint8_t*)"\n>");
			i = 0;
			do {
				uart_current_line[i] = uart_previous_line[i];
				usr_uart_write_char(dev->uart_device, uart_current_line[i],
						    UART_BLOCKING);
			} while(uart_previous_line[i++] != '\0');
			uart_line_index = --i;
			break;
		case _NC:
			break;
		default:
			uart_current_line[uart_line_index++] = c;
			if(uart_line_index == 256) {
				uart_line_index--;
			}
		}

		uart_current_line[uart_line_index] = '\0';
	}

	return ret;
}

/**
 * Implements the CLI logic.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cli_process(struct cli_desc *dev)
{
	cmd_func func = NULL;
	uint8_t i = 0;
	int32_t ret;

	/* Disable interrupts to avoid hanging */
	cli_disable_int();

	/* Get character and identify it */
	ret = cli_parse(dev);
	if(ret < 0)
		return ret;

	/* Enable interrupts */
	cli_enable_int();

	/* Check if <ENTER> key was pressed */
	if (uart_cmd == UART_TRUE) {
		do {
			uart_previous_line[i] = uart_current_line[i];
		} while(uart_current_line[i++] != '\0');
		/* Find needed function based on typed command */
		cli_find_command(dev, uart_current_line, &func);

		/* Check if there is a valid command */
		if (func) {
			usr_uart_write_string(dev->uart_device, (uint8_t*)"\n");
			/* Call the desired function */
			(*func)(dev->device_descriptor,
				(uint8_t*)strchr((char*)uart_current_line, ' ') + 1);
		} else if(strncmp((char*)uart_current_line, "exit", 5) == 0) {
			dev->device_descriptor = NULL;

			/* Check if there is no match for typed command */
		} else if (strlen((char *)uart_current_line) != 0) {
			usr_uart_write_string(dev->uart_device, (uint8_t*)"\n");
			/* Display a message for unknown command */
			usr_uart_write_string(dev->uart_device,
					      (uint8_t*)"Unknown command!");
			usr_uart_write_string(dev->uart_device, (uint8_t*)"\n");
		}
		/* Prepare for next <ENTER> */
		uart_cmd = UART_FALSE;
		ret = cli_cmd_prompt(dev, NULL);
		if(ret < 0)
			return ret;
	}

	return ret;
}

/**
 * Get the CLI commands and correlate them to functions.
 *
 * @param [in] dev       - The device structure.
 * @param [in] command   - Command received from the CLI.
 * @param [out] function - Pointer to the corresponding function.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
void cli_find_command(struct cli_desc *dev, uint8_t *command,
		      cmd_func* function)
{
	uint8_t i = 0;

	while (dev->v_cmd_fun[i/2] != NULL) {
		if(strncmp((char *)command,
			   (char *)dev->cmd_commands[i],
			   dev->command_size[i]) == 0 ||
		    strncmp((char *)command,
			    (char *)dev->cmd_commands[i + 1],
			    dev->command_size[i+1]) == 0) {
			if(dev->command_size == 0) {
				break;
			}
			*function = dev->v_cmd_fun[i / 2];
			break;
		}
		i += 2;
	}
}

/**
 * Display command prompt for the user on the CLI at the beginning of the
 * program.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cli_cmd_prompt(struct cli_desc *dev, uint8_t *app_name)
{
	int32_t ret;
	static uint8_t count = 0;

	ret = usr_uart_write_string(dev->uart_device, (uint8_t*)"\n");

	/* Check first <ENTER> is pressed after reset */
	if(count == 0) {
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " ###########################         ###     ###    ###    ###     ###       #######      ####### \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " ###########################        #####    ####   ###    ####    ###      ###    ###  ###    ### \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " ###########################       ##  ##    #####  ###   ##  ##   ###     ###      ##  ##          \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " #####    ##################       ##   ##   ##  ## ###  ##   ##   ###     ##       ## ###   #####  \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " #####       ###############      #########  ##   #####  ########  ###     ###      ##  ##      ##  \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " #####          ############      ##     ##  ##    #### ###    ### ###      ###   ####  ###    ###  \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " #####             #########     ##      ### ##     ### ##      ## ########   ######      ###### #  \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " #####                ######                                                                        \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " #####                ######     # ####      #######   #      ##  #     ####     #######    ####    \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " #####             #########     #########  ######### ###    ### ###  ###  ###  ########  ###  ###  \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " #####          ############     ##     ##  ##         ##    ##  ### ###     ## ##       ###        \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " #####       ###############     ##     ### ########   ###  ###  ### ##         ########  ######    \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " #####    ##################     ##      ## ###         ##  ##   ### ##         ###          ###### \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " ###########################     ##     ### ##           ####    ### ###     ## ##       ##      ## \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " ###########################     #########  #########    ####    ###  ########  ######### ########  \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)
				      " ###########################                                             ###                ####   \n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)"\n");

		usr_uart_write_string(dev->uart_device, (uint8_t*)"\tWelcome to the ");
		usr_uart_write_string(dev->uart_device, app_name);
		usr_uart_write_string(dev->uart_device, (uint8_t*)" application!\n");
		usr_uart_write_string(dev->uart_device,
				      (uint8_t*)"Type <help> or <h> to see available commands...\n");
		usr_uart_write_string(dev->uart_device, (uint8_t*)"\n");
		count++;
	}

	if(ret == UART_SUCCESS)
		usr_uart_write_char(dev->uart_device, '>', UART_BLOCKING);

	uart_line_index = 0;

	return ret;
}

/**
 * Update the current command vector.
 *
 * The CLI has a vector of function pointers that can be called as commands
 * using the physical layer as an input. Load this vector pointer.
 *
 * @param [in] dev            - Handler structure pointer.
 * @param [in] command_vector - Pointer to the new command vector.
 *
 * @return void
 */
void cli_load_command_vector(struct cli_desc *dev, cmd_func *command_vector)
{
	dev->v_cmd_fun = command_vector;
}

/**
 * Update the current command call vector.
 *
 * Load the pointer to the list of recognized CLI commands.
 *
 * @param [in] dev           - Handler structure pointer.
 * @param [in] command_calls - Pointer to the new command call vector.
 *
 * @return void
 */
void cli_load_command_calls(struct cli_desc *dev, uint8_t **command_calls)
{
	dev->cmd_commands = command_calls;
}

/**
 * Update the current command size vector.
 *
 * Load the pointer to the list of recognized CLI commands sizes.
 *
 * @param [in] dev           - Handler structure pointer.
 * @param [in] command_sizes - Pointer to the new command call sizes vector.
 *
 * @return void
 */
void cli_load_command_sizes(struct cli_desc *dev, uint8_t *command_sizes)
{
	dev->command_size = command_sizes;
}

/**
 * Update the current handler pointer.
 *
 * Load the pointer to the handler that must be passed as argument to the
 * command functions.
 *
 * @param [in] dev           - Handler structure pointer.
 * @param [in] command_sizes - Pointer to the new command call sizes vector.
 *
 * @return void
 */
void cli_load_descriptor_pointer(struct cli_desc *dev, void *command_descriptor)
{
	dev->device_descriptor = command_descriptor;
}
