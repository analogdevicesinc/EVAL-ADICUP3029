/***************************************************************************//**
*   @file   ad5770r_pmdz.c
*   @brief  Application source code.
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
#include <math.h>
#include "ad5770r_pmdz.h"
#include "error.h"
#include "timer.h"
#include "delay.h"

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

/* Command vector */
static cmd_func ad5770r_fnc_ptr[] = {
	(cmd_func)ad5770r_pmdz_help,
	(cmd_func)ad5770r_pmdz_set_in_reg,
	(cmd_func)ad5770r_pmdz_update_out,
	(cmd_func)ad5770r_pmdz_set_chan,
	(cmd_func)ad5770r_pmdz_set_range,
	(cmd_func)ad5770r_pmdz_prod_test,
	NULL
};

/* Command call vector */
static char *ad5770r_fnc_calls[] = {
	"help",
	"h",
	"set_in_reg ",
	"sir ",
	"update_out",
	"uo",
	"set_chan ",
	"sc ",
	"set_range ",
	"sr ",
	"prod_test",
	"t",
	""
};

/* Command size vector */
static uint8_t ad5770r_fnc_call_size[] = {
	5, 2, 11, 4, 11, 3, 9, 3, 10, 3, 10, 2, 1
};

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Help command helper function. Display help function prompt.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad5770r_pmdz_help_prompt(struct ad5770r_pmdz_dev *dev,
					bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"\tCN0424 application.\n");
		if(ret != SUCCESS)
			return ret;
		usr_uart_write_string(dev->cli_device->uart_device,
				      (uint8_t*)
				      "For commands with options as arguments typing the command and 'space' without arguments\n");
		if(ret != SUCCESS)
			return ret;
		usr_uart_write_string(dev->cli_device->uart_device,
				      (uint8_t*)"will show the list of options.\n");
		if(ret != SUCCESS)
			return ret;
		return usr_uart_write_string(dev->cli_device->uart_device,
					     (uint8_t*)"Available verbose commands.\n\n");
	} else {
		return usr_uart_write_string(dev->cli_device->uart_device,
					     (uint8_t*)"\nAvailable short commands:\n\n");
	}
}

/**
 * Display DAC channel specific functions.
 *
 * ad5770r_pmdz_help() helper function.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad5770r_pmdz_help_dac(struct ad5770r_pmdz_dev *dev,
				     bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    " set_in_reg <chan> <value> - Set the input register of the chosen DAC channel.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                             <chan> = channel to update; values are:\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c0;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c1;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c2;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c3;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c4;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c5.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                             <value> = update value in decimal; between 0 and 16383.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                             Example: set_in_reg c0 8192\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    " update_out                - Update the DAC output with the channel input registers value.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                             This is done with the nLDAC GPIO if it's available and with the SW register otherwise.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    " set_chan <chan> <value>   - Set the value of the chosen DAC channel.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                             <chan> = channel to update; values are:\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c0;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c1;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c2;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c3;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c4;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c5.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                             <value> = update value in decimal; between 0 and 16383.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                             Example: set_chan c0 8192\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    " set_range <chan> <opt>    - Set the range of the DAC output channels.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                             <chan> = channel to update; values are:\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c0;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c1;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c2;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c3;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c4;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - c5.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                             <opt> = option chosen for the channel: values are:\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                                       - opt1 - only for channels 0 and 1;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - opt2;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                       - opt3.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                     Options are better explained in the datasheet:\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                     https://www.analog.com/media/en/technical-documentation/data-sheets/AD5770R.pdf\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                             Example: set_range c0 opt1\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    " prod_test                 - Set channels at production test levels.\n");
		if(ret != SUCCESS)
			return ret;
		return usr_uart_write_string(dev->cli_device->uart_device,
					     (uint8_t*)"                             Example: prod_test\n");
	} else {
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    " sir <chan> <value> - Set the input register of the chosen DAC channel.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                      <chan> = channel to update; values are:\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c0;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c1;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c2;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c3;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c4;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c5.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                      <value> = update value in decimal; between 0 and 16383.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                      Example: set_in_reg c0 8192\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    " uo                 - Update the DAC output with the channel input registers value.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                      This is done with the nLDAC GPIO if it's available and with the SW register otherwise.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)" sc <chan> <value>  - Set the value of the chosen DAC channel.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                      <chan> = channel to update; values are:\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c0;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c1;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c2;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c3;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c4;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c5.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                      <value> = update value in decimal; between 0 and 16383.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                      Example: set_chan c0 8192\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)" sr <chan> <opt>    - Set the range of the DAC output channels.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                      <chan> = channel to update; values are:\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c0;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c1;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c2;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c3;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c4;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                                - c5.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                      <opt> = option chosen for the channel: values are:\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "                               - opt1 - only for channels 0 and 1;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                               - opt2;\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                               - opt3.\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"              Options are better explained in the datasheet:\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)
					    "              https://www.analog.com/media/en/technical-documentation/data-sheets/AD5770R.pdf\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)"                      Example: set_range c0 opt1\n");
		if(ret != SUCCESS)
			return ret;
		ret = usr_uart_write_string(dev->cli_device->uart_device,
					    (uint8_t*)" t                  - Set channels at production test levels.\n");
		if(ret != SUCCESS)
			return ret;
		return usr_uart_write_string(dev->cli_device->uart_device,
					     (uint8_t*)"                      Example: t\n");
	}
}

/**
 * Display help options in the CLI.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad5770r_pmdz_help(struct ad5770r_pmdz_dev *dev, uint8_t *arg)
{
	int32_t ret;

	ret = ad5770r_pmdz_help_prompt(dev, HELP_LONG_COMMAND);
	if(ret != SUCCESS)
		return ret;

	ret = ad5770r_pmdz_help_dac(dev, HELP_LONG_COMMAND);
	if(ret != SUCCESS)
		return ret;

	ret = ad5770r_pmdz_help_prompt(dev, HELP_SHORT_COMMAND);
	if(ret != SUCCESS)
		return ret;

	return ad5770r_pmdz_help_dac(dev, HELP_SHORT_COMMAND);
}

/**
 * Display "Set Input Register" command error.
 *
 * ad5770r_pmdz_set_in_reg() helper function that displays a tooltip to using
 * this specific command.
 *
 * @param [in] dev      - Application software handler.
 * @param [in] chan_tab - Table to the existinf channels.
 *
 * @return void
 */
static void ad5770r_pmdz_set_in_reg_err(struct ad5770r_pmdz_dev *dev,
					uint8_t **chan_tab)
{
	int8_t i = 0;
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"Set Input Register command has the following syntax:\n");
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"\tsir <chan> <value>\n");
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"Where:\n");
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"<chan> is the DAC channel to be set and can be:\n");
	while(chan_tab[i] != 0) {
		usr_uart_write_string(dev->cli_device->uart_device, (uint8_t*)" - ");
		usr_uart_write_string(dev->cli_device->uart_device, chan_tab[i++]);
		usr_uart_write_string(dev->cli_device->uart_device, (uint8_t*)";\n");
	}
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"<value> the value to be set and must be between 0 and 16383.\n");
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"Example: sir c0 8192\n");
}

/**
 * Set input register for a DAC channel.
 *
 * @param [in] dev - Application software handler.
 * @param [in] arg - Pointer to the channel argument.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad5770r_pmdz_set_in_reg(struct ad5770r_pmdz_dev *dev, uint8_t *arg)
{
	uint8_t *chan, *data;
	uint8_t *chan_tab[] = {
		(uint8_t *)"c0", (uint8_t *)"c1", (uint8_t *)"c2", (uint8_t *)"c3",
		(uint8_t *)"c4", (uint8_t *)"c5", 0
	};
	int8_t chan_index = 0;
	uint16_t data_code;

	if(!arg) {
		ad5770r_pmdz_set_in_reg_err(dev, chan_tab);
		return SUCCESS;
	}

	chan = arg;
	data = (uint8_t *)strchr((const char *)arg, ' ');
	*data = 0;
	data += 1;
	if(strlen((const char *)chan) != 2) {
		ad5770r_pmdz_set_in_reg_err(dev, chan_tab);
		return SUCCESS;
	}

	while(chan_tab[chan_index] != 0)
		if(strncmp((char *)chan, (char *)chan_tab[chan_index++], 2) == 0)
			break;
	if(chan_index >= 6) {
		ad5770r_pmdz_set_in_reg_err(dev, chan_tab);
		return SUCCESS;
	}

	data_code = atoi((char *)data);

	return ad5770r_set_dac_input(dev->ad770r_device, data_code, chan_index);
}

/**
 * Update the DAC output registers with the values in the input registers.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad5770r_pmdz_update_out(struct ad5770r_pmdz_dev *dev, uint8_t *arg)
{
	int32_t ret;
	const struct ad5770r_channel_switches chan = {
		.en0 = true,
		.en1 = true,
		.en2 = true,
		.en3 = true,
		.en4 = true,
		.en5 = true,
	};

	if(dev->ad770r_device->nldac) {
		ret = gpio_set_value(dev->ad770r_device->nldac, GPIO_LOW);
		if(ret != SUCCESS)
			return ret;
		mdelay(1);
		return gpio_set_value(dev->ad770r_device->nldac, GPIO_HIGH);
	} else {
		return ad5770r_set_sw_ldac(dev->ad770r_device, &chan);
	}
}

/**
 * Set output of a channel.
 *
 * Does an input register write and a channel update.
 *
 * @param [in] dev - Application software handler.
 * @param [in] arg - Pointer to the channel argument.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad5770r_pmdz_set_chan(struct ad5770r_pmdz_dev *dev, uint8_t *arg)
{
	int32_t ret;

	ret = ad5770r_pmdz_set_in_reg(dev, arg);
	if(ret != SUCCESS)
		return ret;

	return ad5770r_pmdz_update_out(dev, NULL);
}

/**
 * Display error message and tooltip for the "set_range" command.
 *
 * ad5770r_pmdz_set_range() helper function.
 *
 * @param [in] dev      - Application software handler.
 * @param [in] chan_tab - Table with the channel names.
 * @param [in] chan_opt - Table with the channel ranges (specific to every
 *                        channel).
 *
 * @return void
 */
static void ad5770r_pmdz_set_range_err(struct ad5770r_pmdz_dev *dev,
				       uint8_t **chan_tab, uint8_t **chan_opt)
{
	int8_t i = 0;
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"Set DAC Channel Range command has the following syntax:\n");
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"\tsr <chan> <opt>\n");
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"Where:\n");
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"<chan> is the DAC channel to be set and can be:\n");
	while(chan_tab[i] != 0) {
		usr_uart_write_string(dev->cli_device->uart_device, (uint8_t*)" - ");
		usr_uart_write_string(dev->cli_device->uart_device, chan_tab[i++]);
		usr_uart_write_string(dev->cli_device->uart_device, (uint8_t*)";\n");
	}
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"<opt> is the option chosen for the selected channel and can be:\n");
	i = 0;
	while(chan_opt[i] != 0) {
		usr_uart_write_string(dev->cli_device->uart_device, (uint8_t*)" - ");
		usr_uart_write_string(dev->cli_device->uart_device, chan_opt[i++]);
		if(i == 0)
			usr_uart_write_string(dev->cli_device->uart_device,
					      (uint8_t*)" - Only for channels 0 and 1;\n");
		else
			usr_uart_write_string(dev->cli_device->uart_device, (uint8_t*)";\n");
	}
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"Options are better explained in the datasheet:\n");
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)
			      "https://www.analog.com/media/en/technical-documentation/data-sheets/AD5770R.pdf\n");
	usr_uart_write_string(dev->cli_device->uart_device,
			      (uint8_t*)"Example: sr c0 opt1\n");
}

/**
 * Set output range for a channel.
 *
 * @param [in] dev - Application software handler.
 * @param [in] arg - Pointer to the range option argument.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad5770r_pmdz_set_range(struct ad5770r_pmdz_dev *dev, uint8_t *arg)
{
	uint8_t *chan, *opt;
	uint8_t *chan_tab[] = {
		(uint8_t *)"c0", (uint8_t *)"c1", (uint8_t *)"c2", (uint8_t *)"c3",
		(uint8_t *)"c4", (uint8_t *)"c5", 0
	};
	uint8_t *chan_opt[] = {
		(uint8_t *)"opt1", (uint8_t *)"opt2", (uint8_t *)"opt3", 0
	};
	int8_t chan_index = 0, opt_index = 0;
	struct ad5770r_output_range range;

	chan = arg;
	opt = (uint8_t *)strchr((const char *)arg, ' ');
	*opt = 0;
	opt += 1;

	while(chan_tab[chan_index] != 0)
		if(strncmp((char *)chan, (char *)chan_tab[chan_index++], 2) == 0)
			break;
	if(chan_index >= 6) {
		ad5770r_pmdz_set_range_err(dev, chan_tab, chan_opt);
		return SUCCESS;
	}

	while(chan_opt[opt_index] != 0)
		if(strncmp((char *)opt, (char *)chan_tab[opt_index++], 4) == 0)
			break;
	if(chan_index >= 3) {
		ad5770r_pmdz_set_range_err(dev, chan_tab, chan_opt);
		return SUCCESS;
	}

	if((opt_index == 3) && (chan_index != 0) && (chan_index != 1)) {
		ad5770r_pmdz_set_range_err(dev, chan_tab, chan_opt);
		return SUCCESS;
	}

	range.output_scale = 0;
	range.output_range_mode = opt_index;

	return ad5770r_set_output_mode(dev->ad770r_device, &range, chan_index);
}

/**
 * Production test routine.
 *
 * Set the the channels from 10mA to 60mA in order.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad5770r_pmdz_prod_test(struct ad5770r_pmdz_dev *dev, uint8_t *arg)
{
	struct ad5770r_output_range range = {0, 0};
	int8_t i;
	int32_t ret;
	uint16_t data_code;
	float fscale, lsb;

	for(i = AD5770R_CH0; i <= AD5770R_CH5; i++) {
		if(i == AD5770R_CH0) {
			range.output_range_mode = 0;
			fscale = 300;
		} else if(i == AD5770R_CH1) {
			range.output_range_mode = 2;
		} else {
			range.output_range_mode = 1;
		}

		lsb = fscale / pow(2, 14);
		data_code = (10 * i) / lsb;

		ret = ad5770r_set_output_mode(dev->ad770r_device, &range, i);
		if(ret != SUCCESS)
			return FAILURE;

		ret = ad5770r_set_dac_input(dev->ad770r_device, data_code, i);
		if(ret != SUCCESS)
			return FAILURE;
		ret = ad5770r_pmdz_update_out(dev, NULL);
		if(ret != SUCCESS)
			return FAILURE;
	}

	return SUCCESS;
}

/**
 * Provide default configuration for the application initialization handler.
 *
 * @param [out] init_param - Pointer to the initialization structure.
 *
 * @return void.
 */
void ad5770r_pmdz_get_config(struct ad5770r_pmdz_init_param *init_param)
{
	init_param->ad5770r_init.alarm_config.background_crc_en = true;
	init_param->ad5770r_init.alarm_config.background_crc_msk = false;
	init_param->ad5770r_init.alarm_config.iref_fault_msk = false;
	init_param->ad5770r_init.alarm_config.neg_ch0_msk = false;
	init_param->ad5770r_init.alarm_config.open_drain_en = false;
	init_param->ad5770r_init.alarm_config.over_temp_msk = false;
	init_param->ad5770r_init.alarm_config.temp_warning_msk = false;
	init_param->ad5770r_init.alarm_config.thermal_shutdown_en = true;

	init_param->ad5770r_init.channel_config.en0 = true;
	init_param->ad5770r_init.channel_config.en1 = true;
	init_param->ad5770r_init.channel_config.en2 = true;
	init_param->ad5770r_init.channel_config.en3 = true;
	init_param->ad5770r_init.channel_config.en4 = true;
	init_param->ad5770r_init.channel_config.en5 = true;
	init_param->ad5770r_init.channel_config.sink0 = true;

	init_param->ad5770r_init.dac_value[0] = 0x2000;
	init_param->ad5770r_init.dac_value[1] = 0x2000;
	init_param->ad5770r_init.dac_value[2] = 0x2000;
	init_param->ad5770r_init.dac_value[3] = 0x2000;
	init_param->ad5770r_init.dac_value[4] = 0x2000;
	init_param->ad5770r_init.dac_value[5] = 0x2000;

	init_param->ad5770r_init.dev_spi_settings.addr_ascension = false;
	init_param->ad5770r_init.dev_spi_settings.single_instruction = false;
	init_param->ad5770r_init.dev_spi_settings.stream_mode_length = 0;

	init_param->ad5770r_init.external_reference = false;
	init_param->ad5770r_init.reference_selector = AD5770R_INT_REF_1_25_V_OUT_ON;

	init_param->ad5770r_init.input_value[0] = 0x2000;
	init_param->ad5770r_init.input_value[1] = 0x2000;
	init_param->ad5770r_init.input_value[2] = 0x2000;
	init_param->ad5770r_init.input_value[3] = 0x2000;
	init_param->ad5770r_init.input_value[4] = 0x2000;
	init_param->ad5770r_init.input_value[5] = 0x2000;

	init_param->ad5770r_init.mask_channel_sel.en0 = false;
	init_param->ad5770r_init.mask_channel_sel.en1 = false;
	init_param->ad5770r_init.mask_channel_sel.en2 = false;
	init_param->ad5770r_init.mask_channel_sel.en3 = false;
	init_param->ad5770r_init.mask_channel_sel.en4 = false;
	init_param->ad5770r_init.mask_channel_sel.en5 = false;
	init_param->ad5770r_init.mask_channel_sel.sink0 = false;

	init_param->ad5770r_init.mask_hw_ldac.en0 = false;
	init_param->ad5770r_init.mask_hw_ldac.en1 = false;
	init_param->ad5770r_init.mask_hw_ldac.en2 = false;
	init_param->ad5770r_init.mask_hw_ldac.en3 = false;
	init_param->ad5770r_init.mask_hw_ldac.en4 = false;
	init_param->ad5770r_init.mask_hw_ldac.en5 = false;
	init_param->ad5770r_init.mask_hw_ldac.sink0 = false;

	init_param->ad5770r_init.mon_setup.ib_ext_en = false;
	init_param->ad5770r_init.mon_setup.monitor_channel = AD5770R_CH0;
	init_param->ad5770r_init.mon_setup.monitor_function = AD5770R_DISABLE;
	init_param->ad5770r_init.mon_setup.mux_buffer = false;

	init_param->ad5770r_init.nalarm = (typeof(init_param->ad5770r_init.nalarm))
					  calloc(1, sizeof(*init_param->ad5770r_init.nalarm));
	init_param->ad5770r_init.nldac = (typeof(init_param->ad5770r_init.nldac))calloc(
			1, sizeof(*init_param->ad5770r_init.nldac));
	init_param->ad5770r_init.nalarm->number = 0x10;
	init_param->ad5770r_init.nalarm->extra = NULL;
	init_param->ad5770r_init.nldac->number = 0x0E;
	init_param->ad5770r_init.nldac->extra = NULL;
	init_param->ad5770r_init.nreset = NULL;

	init_param->ad5770r_init.output_filter[0] =
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM;
	init_param->ad5770r_init.output_filter[1] =
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM;
	init_param->ad5770r_init.output_filter[2] =
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM;
	init_param->ad5770r_init.output_filter[3] =
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM;
	init_param->ad5770r_init.output_filter[4] =
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM;
	init_param->ad5770r_init.output_filter[5] =
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM;

	init_param->ad5770r_init.output_mode[0].output_range_mode = 0;
	init_param->ad5770r_init.output_mode[0].output_scale = 0;
	init_param->ad5770r_init.output_mode[1].output_range_mode = 0;
	init_param->ad5770r_init.output_mode[1].output_scale = 0;
	init_param->ad5770r_init.output_mode[2].output_range_mode = 0;
	init_param->ad5770r_init.output_mode[2].output_scale = 0;
	init_param->ad5770r_init.output_mode[3].output_range_mode = 0;
	init_param->ad5770r_init.output_mode[3].output_scale = 0;
	init_param->ad5770r_init.output_mode[4].output_range_mode = 0;
	init_param->ad5770r_init.output_mode[4].output_scale = 0;
	init_param->ad5770r_init.output_mode[5].output_range_mode = 0;
	init_param->ad5770r_init.output_mode[5].output_scale = 0;

	init_param->ad5770r_init.page_mask.dac_data_page_mask = 0x2000;
	init_param->ad5770r_init.page_mask.input_page_mask = 0x2000;

	init_param->ad5770r_init.sw_ldac.en0 = false;
	init_param->ad5770r_init.sw_ldac.en1 = false;
	init_param->ad5770r_init.sw_ldac.en2 = false;
	init_param->ad5770r_init.sw_ldac.en3 = false;
	init_param->ad5770r_init.sw_ldac.en4 = false;
	init_param->ad5770r_init.sw_ldac.en5 = false;
	init_param->ad5770r_init.sw_ldac.sink0 = false;

	init_param->ad5770r_init.spi_init.chip_select = 0xFF;
	init_param->ad5770r_init.spi_init.extra = NULL;
	init_param->ad5770r_init.spi_init.id = SPI_PMOD;
	init_param->ad5770r_init.spi_init.max_speed_hz = 9000000;
	init_param->ad5770r_init.spi_init.mode = SPI_MODE_0;
	init_param->ad5770r_init.spi_init.type = ADICUP3029_SPI;

	init_param->cli_init.uart_init.baudrate = bd115200;
	init_param->cli_init.uart_init.bits_no = 8;
	init_param->cli_init.uart_init.has_callback = true;
}

/**
 * Application main process.
 *
 * Runs the Command Line Interpretor.
 *
 * @param [in] dev - Pointer to the application handler.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad5770r_pmdz_process(struct ad5770r_pmdz_dev *dev)
{
	return cli_process(dev->cli_device);
}

/**
 * Allocate memory for the application handlers and initialize the system.
 *
 * @param [out] device     - Pointer to the application handler.
 * @param [in]  init_param - Pointer to the initialization structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad5770r_pmdz_setup(struct ad5770r_pmdz_dev **device,
			   struct ad5770r_pmdz_init_param *init_param)
{
	int32_t ret;
	struct ad5770r_pmdz_dev *dev;
	uint8_t app_name[] = {
		'A', 'D', '5', '7', '7', '0', 'R', '_', 'P', 'M', 'D', 'Z', 0
	};

	dev = calloc(1, sizeof *dev);
	if(!dev)
		return FAILURE;

	ret = timer_start();
	if(ret != SUCCESS)
		goto error;

	ret = cli_setup(&dev->cli_device, &init_param->cli_init);
	if(ret != SUCCESS)
		goto error;
	cli_load_command_vector(dev->cli_device, ad5770r_fnc_ptr);
	cli_load_command_calls(dev->cli_device, (uint8_t **)ad5770r_fnc_calls);
	cli_load_command_sizes(dev->cli_device, ad5770r_fnc_call_size);
	cli_load_descriptor_pointer(dev->cli_device, dev);
	ret = cli_cmd_prompt(dev->cli_device, app_name);
	if(ret != SUCCESS)
		goto error;

	ret = ad5770r_init(&dev->ad770r_device, &init_param->ad5770r_init);
	free(init_param->ad5770r_init.nalarm);
	free(init_param->ad5770r_init.nldac);
	if(ret != SUCCESS)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free memory allocated by ad5770r_pmdz_setup().
 *
 * @param [in] device - Pointer to the application handler.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad5770r_pmdz_remove(struct ad5770r_pmdz_dev *dev)
{
	int32_t ret;

	if(!dev)
		return FAILURE;

	ret = ad5770r_remove(dev->ad770r_device);
	if(ret != SUCCESS)
		return ret;

	ret = cli_remove(dev->cli_device);
	if(ret != SUCCESS)
		return ret;

	free(dev);

	return ret;
}
