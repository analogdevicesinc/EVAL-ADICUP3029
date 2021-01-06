/***************************************************************************//**
*   @file   cn0531.c
*   @brief  CN0531 application source.
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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "cn0531.h"
#include "error.h"
#include "timer.h"

/**
 * @brief Help command helper function. Display help function prompt.
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0531_help_prompt(struct cn0531_dev *dev, bool short_command)
{
	if (!short_command) {
		return usr_uart_write_string(dev->cli_desc->uart_device,
					     (uint8_t*)"\tCN0531 application.\n\r"
					     "For commands with options as arguments typing the command and 'space' without arguments\n\r"
					     "will show the list of options.\n\r"
					     "Available verbose commands.\n\r\n\r");
	} else {
		return usr_uart_write_string(dev->cli_desc->uart_device,
					     (uint8_t*)"\nAvailable short commands:\n\r\n\r");
	}
}

/**
 * @brief Application generic commends. cn0531_help() helper function.
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0531_help_general(struct cn0531_dev *dev, bool short_command)
{
	if (!short_command) {
		return usr_uart_write_string(dev->cli_desc->uart_device,
					     (uint8_t*)" help                       - Print this tooltip.\n\r"
					     "                              Example: help\n\r");
	} else {
		return usr_uart_write_string(dev->cli_desc->uart_device,
					     (uint8_t*)" h                - Print this tooltip.\n\r"
					     "                    Example: h\n\r");
	}
}

/**
 * @brief DAC specific commands tooltip. cn0531_help() helper function.
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0531_help_dac(struct cn0531_dev *dev, bool short_command)
{
	if (!short_command) {
		return usr_uart_write_string(dev->cli_desc->uart_device,
					     (uint8_t*)CN0531_HELP_DAC_LONG);
	} else {
		return usr_uart_write_string(dev->cli_desc->uart_device,
					     (uint8_t*)CN0531_HELP_DAC_SHORT);
	}
}

/**
 * @brief Display help options in the CLI.
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0531_help(struct cn0531_dev *dev, uint8_t *arg)
{
	int32_t ret;

	ret = cn0531_help_prompt(dev, HELP_LONG_COMMAND);
	if(ret != SUCCESS)
		return FAILURE;

	ret = cn0531_help_general(dev, HELP_LONG_COMMAND);
	if(ret != SUCCESS)
		return FAILURE;

	ret = cn0531_help_dac(dev, HELP_LONG_COMMAND);
	if(ret != SUCCESS)
		return FAILURE;

	ret = cn0531_help_prompt(dev, HELP_SHORT_COMMAND);
	if(ret != SUCCESS)
		return FAILURE;

	ret = cn0531_help_general(dev, HELP_SHORT_COMMAND);
	if(ret != SUCCESS)
		return FAILURE;

	return cn0531_help_dac(dev, HELP_SHORT_COMMAND);
}

/**
 * @brief CLI command to read a register value.
 * @param [in] dev - The device structure.
 * @param [in] arg - Address of the register in ASCII.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0531_reg_read(struct cn0531_dev *dev, uint8_t *arg)
{
	uint8_t addr, *err_ptr;
	int32_t val, ret;
	uint8_t buff[20];

	addr = strtol((char *)arg, (char **)&err_ptr, 16);
	if(err_ptr == arg)
		return FAILURE;

	val = ad5791_get_register_value(dev->ad5791_desc, addr);
	if (val < 0)
		return FAILURE;
	val &= 0xfffff;

	ret = usr_uart_write_string(dev->cli_desc->uart_device,
				    (uint8_t*)"Register 0x");
	if (ret != SUCCESS)
		return FAILURE;
	ret = usr_uart_write_string(dev->cli_desc->uart_device, arg);
	if (ret != SUCCESS)
		return FAILURE;
	ret = usr_uart_write_string(dev->cli_desc->uart_device, (uint8_t*)" = 0x");
	if (ret != SUCCESS)
		return FAILURE;
	itoa(val, (char *)buff, 16);
	ret = usr_uart_write_string(dev->cli_desc->uart_device, buff);
	if (ret != SUCCESS)
		return FAILURE;

	return usr_uart_write_string(dev->cli_desc->uart_device, (uint8_t*)"\n");
}

/**
 * @brief CLI command to write a register value.
 * @param [in] dev - The device structure.
 * @param [in] arg - The address of the register and new value in ASCII.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0531_reg_write(struct cn0531_dev *dev, uint8_t *arg)
{
	uint8_t addr, *val_ptr, *err_ptr;
	int32_t val;

	addr = strtol((char *)arg, (char **)&val_ptr, 16);
	if(val_ptr == arg)
		return FAILURE;

	val = strtol((char *)val_ptr, (char **)&err_ptr, 16);
	if(val_ptr == err_ptr)
		return FAILURE;

	return ad5791_set_register_value(dev->ad5791_desc, addr, (uint32_t)val);
}

/**
 * @brief CLI command to update the output.
 * @param [in] dev - The device structure.
 * @param [in] arg - The new voltage output value in ASCII.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0531_dac_out(struct cn0531_dev *dev, uint8_t *arg)
{
	const uint8_t dac_bit_no = 20;
	float dac_ref_pos = CN0531_POSITIVE_REFERENCE,
	      dac_ref_neg = CN0531_NEGATIVE_REFERENCE, dac_lsb;
	float val;
	uint8_t *err_ptr;
	int32_t dac_code, conf_reg, ret;

	val = strtod((char *)arg, (char **)&err_ptr);
	if(err_ptr == arg)
		return FAILURE;

	conf_reg = ad5791_get_register_value(dev->ad5791_desc, AD5791_REG_CTRL);

	dac_lsb = (dac_ref_pos - dac_ref_neg) / (pow(2, dac_bit_no) - 1);

	if (conf_reg & AD5791_CTRL_RBUF)
		val = val * 2 - 5;

	if (!(conf_reg & AD5791_CTRL_BIN2SC))
		dac_code = val / dac_lsb;
	else
		dac_code = (val - dac_ref_neg) / dac_lsb;

	ret = ad5791_set_dac_value(dev->ad5791_desc, dac_code);
	if (ret != SUCCESS)
		return FAILURE;

	return ad5791_soft_instruction(dev->ad5791_desc, AD5791_SOFT_CTRL_LDAC);
}

/**
 * @brief Application process. Needs to run in a loop.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0531_process(struct cn0531_dev *dev)
{
	return cli_process(dev->cli_desc);
}

/**
 * @brief Initialize the CLI descriptor vectors with commands.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0531_init_cli_load(struct cn0531_dev *dev)
{
	dev->cn0531_cmd_calls = (uint8_t **)calloc((CN0531_CLI_CMD_NO * 2 + 1),
				sizeof *dev->cn0531_cmd_calls);
	if (!dev->cn0531_cmd_calls)
		return FAILURE;
	dev->cn0531_cmd_calls[0] = (uint8_t *)"help";
	dev->cn0531_cmd_calls[1] = (uint8_t *)"h";
	dev->cn0531_cmd_calls[2] = (uint8_t *)"dac_reg_read ";
	dev->cn0531_cmd_calls[3] = (uint8_t *)"drr ";
	dev->cn0531_cmd_calls[4] = (uint8_t *)"dac_reg_write ";
	dev->cn0531_cmd_calls[5] = (uint8_t *)"drw ";
	dev->cn0531_cmd_calls[6] = (uint8_t *)"dac_out ";
	dev->cn0531_cmd_calls[7] = (uint8_t *)"do ";

	dev->cn0531_cmd_func_tab = (cmd_func *)calloc((CN0531_CLI_CMD_NO + 1),
				   sizeof *dev->cn0531_cmd_func_tab);
	if (!dev->cn0531_cmd_func_tab)
		goto error_calls;
	dev->cn0531_cmd_func_tab[0] = (cmd_func)cn0531_help;
	dev->cn0531_cmd_func_tab[1] = (cmd_func)cn0531_reg_read;
	dev->cn0531_cmd_func_tab[2] = (cmd_func)cn0531_reg_write;
	dev->cn0531_cmd_func_tab[3] = (cmd_func)cn0531_dac_out;

	dev->cn0531_cmd_size = (uint8_t *)calloc((CN0531_CLI_CMD_NO * 2 + 1),
			       sizeof *dev->cn0531_cmd_size);
	if (!dev->cn0531_cmd_size)
		goto error_func;
	dev->cn0531_cmd_size[0] = 5;
	dev->cn0531_cmd_size[1] = 2;
	dev->cn0531_cmd_size[2] = 13;
	dev->cn0531_cmd_size[3] = 4;
	dev->cn0531_cmd_size[4] = 14;
	dev->cn0531_cmd_size[5] = 4;
	dev->cn0531_cmd_size[6] = 8;
	dev->cn0531_cmd_size[7] = 3;

	return SUCCESS;

error_func:
	free(dev->cn0531_cmd_func_tab);
error_calls:
	free(dev->cn0531_cmd_calls);

	return FAILURE;
}

/**
 * @brief Unload the CLI vectors.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0531_remove_cli_unload(struct cn0531_dev *dev)
{
	if (dev->cn0531_cmd_calls)
		free(dev->cn0531_cmd_calls);
	if (dev->cn0531_cmd_func_tab)
		free(dev->cn0531_cmd_func_tab);
	if (dev->cn0531_cmd_size)
		free(dev->cn0531_cmd_size);

	return SUCCESS;
}

/**
 * @brief Application intialization.
 * @param [out] device - Pointer to the device structure.
 * @param [in] param - Pointer to the initialization structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0531_init(struct cn0531_dev **device, struct cn0531_init_param *param)
{
	int32_t ret, val;
	struct cn0531_dev *dev;
	uint8_t app_name[] = "CN0531";

	timer_start();

	dev = (struct cn0531_dev *)calloc(1, sizeof *dev);
	if (!dev)
		return FAILURE;

	ret = cli_setup(&dev->cli_desc, &param->cli_param);
	if (ret != SUCCESS)
		goto error_cn;
	ret = cn0531_init_cli_load(dev);
	if (ret != SUCCESS)
		goto error_cli;
	cli_load_command_vector(dev->cli_desc, dev->cn0531_cmd_func_tab);
	cli_load_command_calls(dev->cli_desc, dev->cn0531_cmd_calls);
	cli_load_command_sizes(dev->cli_desc, dev->cn0531_cmd_size);
	cli_load_descriptor_pointer(dev->cli_desc, dev);
	ret = cli_cmd_prompt(dev->cli_desc, app_name);
	if(ret != SUCCESS)
		goto error_cli_ld;

	ret = ad5791_init(&dev->ad5791_desc, param->ad5791_param);
	if (ret != SUCCESS)
		goto error_cli_ld;

	ret = ad5791_set_dac_value(dev->ad5791_desc, 0);
	if (ret != SUCCESS)
		goto error_dac;

	val = ad5791_get_register_value(dev->ad5791_desc, AD5791_REG_CTRL);
	if (val < SUCCESS)
		goto error_dac;
	val &= ~(AD5791_CTRL_OPGND | AD5791_CTRL_RBUF);
	ret = ad5791_set_register_value(dev->ad5791_desc, AD5791_REG_CTRL,
					(uint32_t)val);
	if (ret != SUCCESS)
		goto error_dac;

	ret = aducm3029_adc_init(&dev->platform_adc, &param->platform_adc);
	if (ret != SUCCESS)
		goto error_dac;

	*device = dev;

	return SUCCESS;

error_dac:
	ad5791_remove(dev->ad5791_desc);
error_cli_ld:
	cn0531_remove_cli_unload(dev);
error_cli:
	cli_remove(dev->cli_desc);
error_cn:
	free(dev);

	return FAILURE;
}

/**
 * @brief Free memory allocated by cn0531_init()
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0531_remove(struct cn0531_dev *dev)
{
	if (!dev)
		return FAILURE;

	cn0531_remove_cli_unload(dev);
	cli_remove(dev->cli_desc);
	ad5791_remove(dev->ad5791_desc);
	aducm3029_adc_remove(dev->platform_adc);

	free(dev);

	return SUCCESS;
}
