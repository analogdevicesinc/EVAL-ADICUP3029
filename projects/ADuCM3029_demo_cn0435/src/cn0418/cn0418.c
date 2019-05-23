/***************************************************************************//**
*   @file   cn0418.c
*   @brief  CN0418 driver application source.
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
#include "cn0418.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "cli.h"

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
extern bool modem_rec_flag;
extern uint32_t unique_id;

/* HART channel names */
char *cn0418_hart_chan_names[] = {
	"cha",
	"chb",
	"chc",
	"chd",
	""
};

/* Channel ranges options */
char *chan_ranges[] = {
	"r05v",
	"r010v",
	"rmp5v",
	"rmp10v",
	"r420ma",
	"r020ma",
	"r024ma",
	""
};

/* Channel range options lengths */
uint8_t chan_range_len[] = {5, 6, 6, 7, 7, 7, 7, 1};

/* Channel select options */
char *channels[] = {
	"cha",
	"chb",
	"chc",
	"chd",
	""
};

/* Rset options */
char *rset_options[] = {
	"ext",
	"int",
	""
};

/* Available commands */
char *cn0418_cmd_commands[] = {
	"help",
	"h",
	"dac_set_range ",
	"dsr ",
	"dac_set_voltage ",
	"dsv ",
	"dac_set_current ",
	"dsc ",
	"hart_change_channel ",
	"hcc ",
	"hart_transmit ",
	"ht ",
	"hart_get_rec",
	"hg",
	"dac_set_code ",
	"dsx ",
	"dac_set_rset ",
	"dse ",
	"hart_enable",
	"he",
	"hart_disable",
	"hd",
	"hart_command_zero ",
	"hcz ",
	"discover_eeprom",
	"de",
	"hart_command_zero ",
	"hcz ",
	"status",
	"stts",
	"hart_phy_test ",
	"hpt ",
	"reset",
	"rst",
	""
};

/* Functions for available commands */
cmd_func cn0418_v_cmd_fun[] = {
	(cmd_func)cn0418_help,
	(cmd_func)cn0418_set_channel_range,
	(cmd_func)cn0418_set_channel_voltage,
	(cmd_func)cn0418_set_channel_current,
	(cmd_func)cn0418_hart_change_channel,
	(cmd_func)cn0418_hart_transmit,
	(cmd_func)cn0418_hart_get_rec,
	(cmd_func)cn0418_set_channel_code,
	(cmd_func)cn0418_set_channel_rset,
	(cmd_func)cn0418_hart_enable,
	(cmd_func)cn0418_hart_disable,
	(cmd_func)cn0418_hart_send_command_zero,
	(cmd_func)cn0418_mem_display_addr,
	(cmd_func)cn0418_hart_send_command_zero,
	(cmd_func)cn0418_status,
	(cmd_func)cn0418_hart_phy_test,
	(cmd_func)cn0418_reset,
	NULL
};

/* CLI command sizes */
uint8_t cn0418_command_size[] = {
	5, 2, 14, 4, 16, 4, 16, 4, 20, 4, 14, 3, 13, 3, 13, 4, 13, 4, 12, 3, 13,
	3, 18, 4, 16, 3, 18, 4, 7, 5, 14, 4, 1, 6, 4,
};

/* HART Command zero */
uint8_t cn0418_hart_command_zero[] = {0x02, 0x80, 0x00, 0x00, 0x82};

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Check if there is a SPI device connected and uses the same format of
 * communication as the AD5755.
 *
 * To do this, read the main control register,
 * change the value, see if the change has been made by reading again and
 * restore the value from before the procedure. This is for usage with the
 * minimal initialization function.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_setup_dac_verify_presence_min(struct cn0418_dev *dev)
{
	uint16_t check_value, sense_reg_value, reg_keep;

	sense_reg_value = (uint16_t)ad5755_get_register_value(dev->ad5755_dev,
			  AD5755_RD_MAIN_CTRL_REG);
	check_value = sense_reg_value;
	reg_keep = sense_reg_value;
	/* Delay between reading and writing the register to allow for internal
	 * operations to finish */
	mdelay(5);
	check_value &= ~AD5755_MAIN_WD(0b11);
	check_value |= AD5755_MAIN_WD(2);
	ad5755_set_control_registers(dev->ad5755_dev, AD5755_CREG_MAIN, 0,
				     check_value);

	sense_reg_value = (uint16_t)ad5755_get_register_value(dev->ad5755_dev,
			  AD5755_RD_MAIN_CTRL_REG);

	if((uint16_t)sense_reg_value != check_value)
		return -1;

	ad5755_set_control_registers(dev->ad5755_dev, AD5755_CREG_MAIN, 0,
				     reg_keep);

	return 0;
}

/**
 * Check if there is a SPI device connected and uses the same format of
 * communication as the AD5755.
 *
 * To do this, read the main control register, change the value, see if the
 * change has been made by reading again and restore the value from before the
 * procedure.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
void cn0418_setup_dac_verify_presence(struct cn0418_dev *dev)
{
	uint16_t check_value, sense_reg_value, reg_keep;

	sense_reg_value = (uint16_t)ad5755_get_register_value(dev->ad5755_dev,
			  AD5755_RD_MAIN_CTRL_REG);
	check_value = sense_reg_value;
	reg_keep = sense_reg_value;
	/* Delay between reading and writing the register to allow for internal
	 * operations to finish */
	mdelay(5);
	check_value &= ~AD5755_MAIN_WD(0b11);
	check_value |= AD5755_MAIN_WD(2);
	ad5755_set_control_registers(dev->ad5755_dev, AD5755_CREG_MAIN, 0,
				     check_value);

	sense_reg_value = (uint16_t)ad5755_get_register_value(dev->ad5755_dev,
			  AD5755_RD_MAIN_CTRL_REG);

	if((uint16_t)sense_reg_value != check_value)
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"ERROR! DAC device not found.\n");
#endif
	ad5755_set_control_registers(dev->ad5755_dev, AD5755_CREG_MAIN, 0,
				     reg_keep);
}

/**
 * Setup GPIOs for the CN0418.
 *
 * @param [in] dev        - Pointer to the device structure.
 * @param [in] init_param - Pointer to the structure that contains the device
 *                          initial parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0418_setup_gpio_setup(struct cn0418_dev *dev,
				       struct cn0418_init_param *init_param)
{
	int32_t ret;

	ret = gpio_get(&dev->hart_mult_a0, init_param->hart_mult_a0);
	if(ret < 0)
		return ret;
	ret = gpio_get(&dev->hart_mult_a1, init_param->hart_mult_a1);
	if(ret < 0)
		return ret;
	ret = gpio_direction_output(dev->hart_mult_a0, GPIO_LOW);
	if(ret < 0)
		return ret;
	ret = gpio_direction_output(dev->hart_mult_a1, GPIO_LOW);
	if(ret < 0)
		return ret;

	return ret;
}

/**
 * Write protected memory with ID values.
 *
 * @param [in] dev - Pointer to the device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0418_setup_memory_content_setup(struct cn0418_dev *dev)
{
#ifdef WRITE_PROTECTED_MEMORY
	int32_t ret;

	return memory_write_within_page(dev->memory_device, 0x0000,
					(uint8_t *)"Coming soon...", 14);
#else
	return 0;
#endif
}

/**
 * Initializes the cn0418 device.
 *
 * This functions initializes only components that are specific to every
 * instance of this driver. It should be used when more instances of this driver
 * have to be used.
 *
 * @param [out] device    - The device structure.
 * @param [in] init_param - The structure that contains the device initial
 * 		       		        parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_setup_minimum(struct cn0418_dev **device,
			     struct cn0418_init_param *init_param)
{
	struct cn0418_dev *dev;
	int32_t ret;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	ret = ad5755_init(&dev->ad5755_dev, init_param->ad5755_init);
	if(ret < 0)
		goto error;

	ret = cn0418_setup_gpio_setup(dev, init_param);
	if(ret < 0)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Initializes the cn0418 device.
 *
 * @param [out] device    - The device structure.
 * @param [in] init_param - The structure that contains the device initial
 * 		       		        parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_setup(struct cn0418_dev **device,
		     struct cn0418_init_param *init_param)
{
	int32_t ret;
	struct cn0418_dev *dev;
	uint32_t i;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	/* Devices */
	ret = usr_uart_init(&dev->usr_uart_dev, init_param->usr_uart_init);
	if(ret < 0)
		goto error;

	ret = ad5700_setup(&dev->ad5700_device, &init_param->ad5700_init);
	if(ret < 0)
		goto error;
	NVIC_DisableIRQ(HART_CD_INT);

	ret = ad5755_init(&dev->ad5755_dev, init_param->ad5755_init);
	if(ret < 0)
		goto error;

	cn0418_setup_dac_verify_presence(dev);

	ret = memory_setup(&dev->memory_device, &init_param->memory_init);
	if(ret < 0)
		goto error;

	ret = cn0418_setup_gpio_setup(dev, init_param);
	if(ret < 0)
		goto error;

	/* Initialize HART buffer */
	for(i = 1; i < HART_BUFF_SIZE; i++)
		dev->hart_buffer[i] = 0xaa;
	dev->hart_buffer[0] = 0;
	dev->hart_rec_size = 0;

	ret = cn0418_setup_memory_content_setup(dev);
	if(ret < 0)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free the resources allocated by cn0418_setup_minimum().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_remove_minimum(struct cn0418_dev *dev)
{
	int32_t ret;

	if(!dev)
		return -1;

	ret = ad5755_remove(dev->ad5755_dev);
	if(ret < 0)
		return ret;

	free(dev);

	return ret;
}

/**
 * Free the resources allocated by cn0418_setup().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_remove(struct cn0418_dev *dev)
{
	int32_t ret;

	if(!dev)
		return -1;

	ret = ad5700_remove(dev->ad5700_device);
	if(ret < 0)
		return ret;
	ret = ad5755_remove(dev->ad5755_dev);
	if(ret < 0)
		return ret;
	ret = usr_uart_remove(dev->usr_uart_dev);
	if(ret < 0)
		return ret;

	ret = gpio_remove(dev->hart_mult_a0);
	if(ret < 0)
		return ret;

	ret = gpio_remove(dev->hart_mult_a1);
	if(ret < 0)
		return ret;

	free(dev);

	return ret;
}
#if defined(CLI_INTEFACE)
/**
 * Help command helper function. Display help function prompt.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0418_help_prompt(struct cn0418_dev *dev, bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "\nFor commands with options as arguments typing the command and 'space' without arguments\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"will show the list of options.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"Available verbose commands:\n\n");
	} else {
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "\nFor commands with options as arguments typing the command and 'space' without arguments\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"will show the list of options.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"Available short commands:\n\n");
	}
}

/**
 * Help command helper function. Display commands specific to the status and
 * usage of the application.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long version of commands,
 *                             false to display the short version.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0418_help_general_commands(struct cn0418_dev *dev,
		bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " help                          - Display available commands.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " reset                         - Reset the device to initial configuration and clear all faults.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)
					     " status                        - Display parameters and fault flags of the application.\n");
	} else {
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " h                  - Display available commands.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " rst                - Reset the device to initial configuration and clear all faults.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)
					     " stts               - Display parameters and fault flags of the application.\n");
	}
}

/**
 * Help command helper function. Display commands specific to the AD5700 and
 * HART communication.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long version of commands,
 *                             false to display the short version.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0418_help_hart_commands(struct cn0418_dev *dev,
		bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " hart_enable                   - Enable HART by setting software control of the slew rate on all channels.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" hart_disable                  - Disable HART.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" hart_change_channel <chan>    - Select wanted channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 <chan> = channel to be selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 Example: hart_change_channel cha - for HART channel a.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" hart_transmit <string>        - Transmit string through HART.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 <string> = string to be transmitted.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " hart_get_rec <nbytes>         - Send the received buffer through UART connection.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " hart_command_zero <pbsize>    - Send command zero with the specified number of FFs in the preambule.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 <pbsize> = size of the preambule (no. of 0xFFs in the beginning).\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " hart_phy_test <byte>          - Send command zero with the specified number of FFs in the preambule.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"                                 <byte> = byte to send in loop.\n");
	} else {
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " he                 - Enable HART by setting software control of the slew rate on all channels.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" hd                 - Disable HART.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" hcc <chan>         - Select wanted channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <chan> = channel to be selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      Example: hcc cha - for HART channel a.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" ht <string>        - Transmit string through HART.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <string> = string to be transmitted.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " hg                 - Send the received buffer through UART connection.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " hcz <pbsize>       - Send command zero with the specified number of FFs in the preambule.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                      <pbsize> = size of the preambule (no. of 0xFFs in the beginning).\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " hpt <byte>         - Send command zero with the specified number of FFs in the preambule.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"                      <byte> = byte to send in loop.\n");
	}
}

/**
 * Help command helper function. Display commands specific to the AD5755.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long version of commands,
 *                             false to display the short version.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0418_help_dac_commands(struct cn0418_dev *dev,
					bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" dac_set_range <chan> <range>  - Set the range of a DAC channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                                 <chan> = DAC channel selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                                 <range> = selected range.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 Example: dac_set_range chb r420ma - change the range on channel B to 4 to 20mA\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " dac_set_voltage <chan> <volt> - Set the voltage on a DAC channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                                 <chan> = DAC channel selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                                 <volt> = value in Volts\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 A range must be first selected for this channel. The value must be within range.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 Example: dac_set_voltage chc 3.27 - change the voltage value on channel C to 3.27V\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " dac_set_current <chan> <curr> - Set the current on a DAC channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                                 <chan> = DAC channel selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                                 <curr> = value in miliamperes\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 A range must be first selected for this channel. The value must be within range.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 Example: dac_set_current cha 12.13 - change the current value on channel A to 12.13mA\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" dac_set_code <chan> <code>    - Set the code on a DAC channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                                 <chan> = DAC channel selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 <code> = code between 0 and 65535.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 A range must be first selected for this channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                                 Example: dac_set_current cha 10000 - change the code on channel A to 10000\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " dac_set_rset <chan> <opt>     - Set external or internal Rset for a channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                                 <chan> = DAC channel selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                                 <opt> = option (ext/int)\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)
					     "                                 Example: dac_set_rset cha int - set Rset option of channel A to internal Rset.\n");
	} else {
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" dsr <chan> <range> - Set the range of a DAC channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <chan> = DAC channel selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <range> = selected range.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                      Example: dsr chb r420ma - change the range on channel B to 4 to 20mA\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" dsv <chan> <volt>  - Set the voltage on a DAC channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <chan> = DAC channel selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <volt> = value in Volts\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                      A range must be first selected for this channel. The value must be within range.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                      Example: dsv chc 3.27 - change the voltage value on channel C to 3.27V\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" dsc <chan> <curr>  - Set the current on a DAC channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <chan> = DAC channel selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <curr> = value in miliamperes\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                      A range must be first selected for this channel. The value must be within range.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                      Example: dsc cha 12.13 - change the current value on channel A to 12.13mA\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)" dsx <chan> <code>  - Set the code on a DAC channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <chan> = DAC channel selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <code> = code between 0 and 65535.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                      A range must be first selected for this channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    "                      Example: dac_set_current cha 10000 - change the code on channel A to 10000\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)
					    " dse <chan> <opt>   - Set external or internal Rset for a channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <chan> = DAC channel selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"                      <opt> = option (ext/int)\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)
					     "                      Example: dse cha int - set Rset option of channel A to internal Rset.\n");
	}
}

/**
 * Help command helper function.
 *
 * Display commands specific to the 24LC32A and I2C interfaced memory.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long version of commands,
 *                             false to display the short version.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0418_help_memory_commands(struct cn0418_dev *dev,
		bool short_command)
{
	if (!short_command) {
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)
					     " discover_eeprom               - Discover EEPROM I2C addresses if there are any.\n");
	} else {
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)
					     " de                 - Discover EEPROM I2C addresses if there are any.\n");
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
int32_t cn0418_help(struct cn0418_dev *dev, uint8_t *arg)
{
	int32_t ret;

	ret = cn0418_help_prompt(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0418_help_general_commands(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0418_help_dac_commands(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0418_help_hart_commands(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0418_help_memory_commands(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0418_help_prompt(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0418_help_general_commands(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0418_help_dac_commands(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0418_help_hart_commands(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	return cn0418_help_memory_commands(dev, HELP_SHORT_COMMAND);
}

/**
 * Display the status of the application.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_status(struct cn0418_dev *dev, uint8_t *arg)
{
	int32_t ret, temp_reg_value;
	uint8_t i;
	char *chan_short_open_fault[] = {
		"Channel A current fault.\n",
		"Channel B current fault.\n",
		"Channel C current fault.\n",
		"Channel D current fault.\n",
		"Channel A voltage fault.\n",
		"Channel B voltage fault.\n",
		"Channel C voltage fault.\n",
		"Channel D voltage fault.\n"
	};
	char *chan_dcdc_fault[] = {
		"Channel A DC-DC converter fault.\n",
		"Channel B DC-DC converter fault.\n",
		"Channel C DC-DC converter fault.\n",
		"Channel D DC-DC converter fault.\n"
	};

	for (i = 0; i < CHANNEL_NUMBER; ++i) {
		ret = cn0418_status_helper(dev, i);
		if(ret < 0) {
			return ret;
		}
	}

	temp_reg_value = ad5755_get_register_value(dev->ad5755_dev,
			 AD5755_RD_STATUS_REG);

	for(i = 0; i < 8; i++)
		if((temp_reg_value & (1 << i)) != 0)
			usr_uart_write_string(dev->usr_uart_dev,
					      (uint8_t *)chan_short_open_fault[i]);

	if((temp_reg_value & (1 << 8)) != 0)
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t *)"Core temperature over 150°C.\n");

	for(i = 0; i < 4; i++)
		if((temp_reg_value & (1 << (12 + i))) != 0)
			usr_uart_write_string(dev->usr_uart_dev,
					      (uint8_t *)chan_dcdc_fault[i]);

	return ret;
}
#endif

/**
 * Do a software reset of the device.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_reset(struct cn0418_dev *dev, uint8_t *arg)
{
	uint8_t channel = 0;
	uint16_t dac_control_buff[4] = {0, 0, 0, 0};
	struct ad5755_dev *tmp;

	tmp = dev->ad5755_dev;

	ad5755_software_reset(tmp);
	mdelay(100);
	/* DC-to-DC configuration. */
	ad5755_set_control_registers(tmp, AD5755_CREG_DC_DC, 0,
				     (tmp->p_ad5755_st->dc_dc_comp_bit * AD5755_DC_DC_COMP) |
				     (AD5755_DC_DC_FREQ(tmp->p_ad5755_st->dc_dc_freq_bit)) |
				     (AD5755_DC_DC_PHASE(tmp->p_ad5755_st->dc_dc_phase_bit)) |
				     (AD5755_DC_DC_MAX_V(tmp->p_ad5755_st->dc_dc_max_vbit)));
	/* Configure the DAC control register on a per channel basis. */
	for(channel = AD5755_DAC_A; channel <= AD5755_DAC_D; channel++) {
		if((tmp->this_device == ID_AD5755) ||
		    (tmp->this_device == ID_AD5755_1)) {
			dac_control_buff[channel] = AD5755_DAC_INT_ENABLE |
						    AD5755_DAC_CLR_EN |
						    tmp->p_ad5755_st->rset_bits[channel] |
						    AD5755_DAC_DC_DC |
						    tmp->p_ad5755_st->ovrng_bits[channel] |
						    AD5755_DAC_R(AD5755_R_0_5_V);
		} else {
			dac_control_buff[channel] = AD5755_DAC_INT_ENABLE |
						    AD5755_DAC_CLR_EN |
						    tmp->p_ad5755_st->rset_bits[channel] |
						    AD5755_DAC_DC_DC |
						    tmp->p_ad5755_st->ovrng_bits[channel] |
						    AD5755_DAC_R(AD5755_R_0_5_V);
		}
		ad5755_set_control_registers(tmp, AD5755_CREG_DAC, channel,
					     dac_control_buff[channel]);
	}
	/* Allow at least 200us before enabling the channel output. */
	mdelay(200);
	/* Enable the channel output. */
	for(channel = AD5755_DAC_A; channel <= AD5755_DAC_D; channel++) {
		/* Write to each DAC data register*/
		ad5755_set_register_value(tmp, AD5755_DREG_WR_DAC, channel, 0x0000);
		ad5755_set_control_registers(tmp, AD5755_CREG_DAC, channel,
					     dac_control_buff[channel] | AD5755_DAC_OUTEN);
	}

	return 0;
}

/**
 * Enable HART modem.
 *
 * Use the software control of the slew rate to make the DAC output compatible
 * with HART switching and enable interrupts on the CD pin to get HART messages.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 - irrelevant in this case, only kept to keep function type.
 */
int32_t cn0418_hart_enable(struct cn0418_dev *dev, uint8_t* arg)
{
	int8_t i;

	NVIC_EnableIRQ(HART_CD_INT);

	for(i = 0; i < HART_CHANNEL_NO; i++)
		ad5755_slew_rate_ctrl(dev->ad5755_dev, i, HART_ENABLE,
				      AD5755_SR_CLK_64K, AD5755_STEP_256);

	return 0;
}

/**
 * Disable HART modem.
 *
 * Disable DAC output software slew rate control and the interrupt on CD.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 - irrelevant in this case, only kept to keep function type.
 */
int32_t cn0418_hart_disable(struct cn0418_dev *dev, uint8_t* arg)
{
	int8_t i;

	NVIC_DisableIRQ(HART_CD_INT);

	for(i = 0; i < HART_CHANNEL_NO; i++)
		ad5755_slew_rate_ctrl(dev->ad5755_dev, i, HART_DISABLE,
				      AD5755_SR_CLK_64K, AD5755_STEP_256);

	return 0;
}

/**
 * Change the active HART channel.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - New desired channel.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_hart_change_channel(struct cn0418_dev *dev, uint8_t* arg)
{
#if defined(CLI_INTEFACE)
	int32_t ret;
#endif
	uint8_t i = 0;

	/* Identify channel */
	while (cn0418_hart_chan_names[i][0] != '\0') {
		if(strncmp((char *)arg, (char *)cn0418_hart_chan_names[i], 4) == 0)
			break;
		i++;
	}

	if(i >= 4) {
#if defined(CLI_INTEFACE)
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"Error. Incorrect channel. Available channels are:\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - cha\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chb\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chc\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"    - chd\n");
#elif defined(MODBUS_INTERFACE)
		return 0;
#endif
	}

	return cn0418_hart_change_chan_helper(dev, i);
}

/**
 * Make a HART transmission.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - The string to be transmitted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_hart_transmit(struct cn0418_dev *dev, uint8_t* arg)
{
	uint32_t size;

	/* Get size of the transmission */
	size = strlen((char *)arg);

	/* Transmit without the "\n" character */
	return ad5700_transmit(dev->ad5700_device, arg, size);
}
#if defined(CLI_INTEFACE)
/**
 * Make a HART transmission.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - The string to be transmitted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_hart_get_rec(struct cn0418_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint32_t i;

	NVIC_DisableIRQ(HART_CD_INT);

	if(dev->hart_buffer[0] != 1) {
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"Nothing received.\n");
		if(ret < 0)
			goto finish;
		return HART_NOTHING_RECEIVED_CN0418;
	}

	for(i = 1; i < dev->hart_rec_size; i++) {
		ret = usr_uart_write_char(dev->usr_uart_dev, dev->hart_buffer[i],
					  UART_WRITE_NO_INT);
		if(ret < 0)
			goto finish;
	}
	dev->hart_buffer[0] = 0;

finish:
	NVIC_EnableIRQ(HART_CD_INT);

	return 0;
}
#endif
/**
 * Process function helper. Receive HART transmission on CD interrupt.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0418_process_hart_int_rec(struct cn0418_dev *dev)
{
	uint8_t i = 1;
	int32_t ret;
	uint16_t cd_val;

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(HART_CD_INT);

	do {
		ret = swuart_read_char(dev->ad5700_device->swuart_desc,
				       &dev->hart_buffer[i]);
		gpio_get_value(dev->ad5700_device->gpio_cd, &cd_val);
		if(ret == 0)
			i++;
	} while((i < HART_BUFF_SIZE) && (cd_val != 0));

	dev->hart_rec_size = i;
#if defined(CLI_INTEFACE)
	usr_uart_write_string(dev->usr_uart_dev,
			      (uint8_t*)"\nReceived HART transmission.\n");
#endif
	dev->hart_buffer[0] = 1;

	modem_rec_flag = false;

	/* Enable interrupts */
	NVIC_EnableIRQ(HART_CD_INT);

	return ret;
}

/**
 * Receive and parse the response of a command zero.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0418_hart_receive_and_parse(struct cn0418_dev *dev,
		uint8_t **message_ptr)
{
	int32_t ret;
	uint8_t i = 1;
	uint32_t timeout = 2000;

	dev->hart_buffer[0] = 0;

	while(!modem_rec_flag && (timeout != 0)) {
		timeout--;
		mdelay(1);
	}

	if(timeout == 0)
		return 0;

	ret = cn0418_process_hart_int_rec(dev);
	if(ret < 0)
		return ret;

	while(dev->hart_buffer[i] == HART_PREAMBLE_CHAR)
		i++;

	if(dev->hart_buffer[i] == HART_SHORT_ADDR_RESPONSE)
		i += 6;
	else if(dev->hart_buffer[i] == HART_LONG_ADDR_RESPONSE)
		i += 10;

	*message_ptr = &dev->hart_buffer[i];
	dev->hart_rec_size -= i;

	return ret;
}
#if defined(CLI_INTEFACE)
/**
 * Receive and parse the response of a command zero.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0418_hart_display_cmd_zero_response(struct cn0418_dev *dev,
		uint8_t *response)
{
	uint8_t buff[20];
	uint32_t long_address = 0;
	uint8_t id_lsb = 0;

	if(dev->hart_buffer[0] != 1)
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"No response.\n");

	usr_uart_write_string(dev->usr_uart_dev,
			      (uint8_t*)"\nProtocol version: ");
	itoa(response[4], (char *)buff, 10);
	usr_uart_write_string(dev->usr_uart_dev, buff);
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");

	usr_uart_write_string(dev->usr_uart_dev,
			      (uint8_t*)"Preamble size: ");
	itoa(response[3], (char *)buff, 10);
	usr_uart_write_string(dev->usr_uart_dev, buff);
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");

	usr_uart_write_string(dev->usr_uart_dev,
			      (uint8_t*)"Manufacturer ID code: 0x");
	itoa(response[1], (char *)buff, 16);
	if(strlen((char *)buff) < 2)
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"0");
	usr_uart_write_string(dev->usr_uart_dev, buff);
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");
	id_lsb = response[1] & 0x3F;

	usr_uart_write_string(dev->usr_uart_dev,
			      (uint8_t*)"Manufacturer device type: ");
	itoa(response[2], (char *)buff, 10);
	usr_uart_write_string(dev->usr_uart_dev, buff);
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");

	long_address = id_lsb;
	usr_uart_write_string(dev->usr_uart_dev,
			      (uint8_t*)"Long address: 0x");
	itoa(long_address, (char *)buff, 16);
	if(strlen((char *)buff) < 2)
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"0");
	usr_uart_write_string(dev->usr_uart_dev, buff);
	long_address = 0;
	long_address |= (response[1] << (3 * 8));
	long_address |= (response[9] << (2 * 8));
	long_address |= (response[10] << (1 * 8));
	long_address |= (response[11] << (0 * 8));
	itoa(long_address, (char *)buff, 16);
	if(strlen((char *)buff) < 8)
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"0");
	usr_uart_write_string(dev->usr_uart_dev, buff);
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");

	return 0;
}
#endif
/**
 * Send HART command zero.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Size of the preambule.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_hart_send_command_zero(struct cn0418_dev *dev, uint8_t* arg)
{
	uint8_t preamb_size;
	uint8_t *telegram;
	int32_t ret;
	uint8_t i;
	uint8_t len;

	preamb_size = atoi((char *)arg);

	if((preamb_size < 3) || (preamb_size > 20)) {
#if defined(CLI_INTEFACE)
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"Preambule must be within 3 and 20 bytes.\n");
#elif defined(MODBUS_INTERFACE)
		return 0;
#endif
	}

	len = preamb_size + HART_COMMAND_ZERO_SIZE;

	telegram = calloc(len, sizeof *telegram);
	if(!telegram)
		return -1;

	for(i = 0; i < preamb_size; i++)
		telegram[i] = 0xFF;
	for(i = 0; i < HART_COMMAND_ZERO_SIZE; i++)
		telegram[i + preamb_size] = cn0418_hart_command_zero[i];

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	/* Transmit without the "\n" character */
	ret = ad5700_transmit(dev->ad5700_device, telegram, len);
	if(ret < 0)
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"Transmission failed!\n");
#elif defined(MODBUS_INTERFACE)
		return 0;
#endif
	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	free(telegram);

	ret = cn0418_hart_receive_and_parse(dev, &telegram);
	if(ret < 0)
		return ret;
#if defined(CLI_INTEFACE)
	ret = cn0418_hart_display_cmd_zero_response(dev, telegram);
	if(ret < 0)
		return ret;
#endif
	dev->hart_buffer[0] = 0;

	return ret;
}
#if defined(CLI_INTEFACE)
/**
 * This method sends the provided test byte through the HART link continuously
 * until stopped by pressing q. Function used to test the HART physical layer.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Byte to be transmitted. (only ASCII characters)
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_hart_phy_test(struct cn0418_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint8_t c = 0, rdy;

	ret = usr_uart_write_string(dev->usr_uart_dev,
				    (uint8_t*)"Press 'q' to exit.\n");
	if(ret < 0)
		return ret;

	do {
		/* Disable interrupts to avoid hanging */
		NVIC_DisableIRQ(TMR0_INT);
		NVIC_DisableIRQ(HART_CD_INT);
		ret = ad5700_transmit(dev->ad5700_device, arg, 1);
		if(ret < 0)
			return ret;
		/* Enable interrupts */
		NVIC_EnableIRQ(TMR0_INT);
		NVIC_EnableIRQ(HART_CD_INT);

		ret = usr_uart_read_nb(dev->usr_uart_dev, &c, 1, &rdy);
		if(ret < 0)
			return ret;
	} while (c != 'q');

	return usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");
}
#endif
/**
 * Set the range of a channel.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - String including the selected channel and desired range as
 *                   separate arguments divided by a space.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_set_channel_range(struct cn0418_dev *dev, uint8_t *arg)
{
	uint8_t *reg_chan_strg, *reg_data_strg;
	uint8_t chan = 0, range = 0;
#if defined(CLI_INTEFACE)
	uint8_t buffer[20];
	uint16_t reg_val_readback;
#endif

	reg_data_strg = (uint8_t*)strchr((char*)arg, ' ') + 1;
	reg_chan_strg = arg;
	reg_chan_strg[strcspn((char*)reg_chan_strg, " ")] = '\0';

	while (channels[chan][0] != '\0') {
		if(strncmp((char *)reg_chan_strg, (char *)channels[chan], 4) == 0)
			break;
		chan++;
	}
	if(chan >= 4) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"Wrong channel option. Valid channel options are:\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - cha\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chb\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chc\n");
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"    - chd\n");
#elif defined(MODBUS_INTERFACE)
		return 0;
#endif
	}

	while (chan_ranges[range][0] != '\0') {
		if(strncmp((char *)reg_data_strg, (char *)chan_ranges[range],
			   chan_range_len[range]) == 0)
			break;
		range++;
	}
	if(range >= 7) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"Wrong range option. Valid range options are:\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - r05v\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - r010v\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - rmp5v\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - rmp10v\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - r420ma\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - r020ma\n");
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"    - r024ma\n");
#elif defined(MODBUS_INTERFACE)
		return 0;
#endif
	}

	ad5755_set_channel_range(dev->ad5755_dev, chan, range);

	/* Display new control register value */
#if defined(CLI_INTEFACE)
	reg_val_readback =
#endif
		ad5755_get_register_value(dev->ad5755_dev,
					  AD5755_RD_CTRL_REG(chan));
#if defined(CLI_INTEFACE)
	itoa(reg_val_readback, (char *)buffer, 16);
	usr_uart_write_string(dev->usr_uart_dev,
			      (uint8_t*)"New value of the DAC control register is: ");
	usr_uart_write_string(dev->usr_uart_dev, buffer);
	return usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");
#elif defined(MODBUS_INTERFACE)
	return 0;
#endif
}

/**
 * Set voltage on a channel that has a voltage range selected.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - String including the selected channel and desired voltage
 *                   as separate arguments divided by a space.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_set_channel_voltage(struct cn0418_dev *dev, uint8_t *arg)
{
	uint8_t *reg_chan_strg, *reg_data_strg;
	uint8_t chan = 0;
	float value;
	uint8_t buffer[20];
	uint16_t reg_readback_val;

	reg_data_strg = (uint8_t*)strchr((char*)arg, ' ') + 1;
	reg_chan_strg = arg;
	reg_chan_strg[strcspn((char*)reg_chan_strg, " ")] = '\0';

	while (channels[chan][0] != '\0') {
		if(strncmp((char *)reg_chan_strg, (char *)channels[chan], 4) == 0)
			break;
		chan++;
	}
#if defined(CLI_INTEFACE)
	if(chan >= 4) {
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"Wrong channel option. Valid channel options are:\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - cha\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chb\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chc\n");
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"    - chd\n");
	}
#endif
	value = atof((char *)reg_data_strg);

	value = ad5755_set_voltage(dev->ad5755_dev, chan, value);

	cn0418_ftoa(buffer, value);
#if defined(CLI_INTEFACE)
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"Actual output value: ");
	usr_uart_write_string(dev->usr_uart_dev, buffer);
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"V\n");
#endif
	reg_readback_val = ad5755_get_register_value(dev->ad5755_dev,
			   AD5755_RD_DATA_REG(chan));
	itoa(reg_readback_val, (char *)buffer, 16);
#if defined(CLI_INTEFACE)
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"Data register value: ");
	usr_uart_write_string(dev->usr_uart_dev, buffer);
	return usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");
#elif defined(MODBUS_INTERFACE)
	return 0;
#endif
}

/**
 * Set current on a channel that has a current range selected.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - String including the selected channel and desired current as
 * 				 separate arguments divided by a space.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_set_channel_current(struct cn0418_dev *dev, uint8_t *arg)
{
	uint8_t *reg_chan_strg, *reg_data_strg;
	uint8_t chan = 0;
	float value;
	uint8_t buffer[20];
	uint16_t reg_readback_val;

	reg_data_strg = (uint8_t*)strchr((char*)arg, ' ') + 1;
	reg_chan_strg = arg;
	reg_chan_strg[strcspn((char*)reg_chan_strg, " ")] = '\0';

	while (channels[chan][0] != '\0') {
		if(strncmp((char *)reg_chan_strg, (char *)channels[chan], 4) == 0)
			break;
		chan++;
	}
#if defined(CLI_INTEFACE)
	if(chan >= 4) {
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"Wrong channel option. Valid channel options are:\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - cha\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chb\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chc\n");
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"    - chd\n");
	}
#endif
	value = atof((char *)reg_data_strg);

	value = ad5755_set_current(dev->ad5755_dev, chan, value);

	cn0418_ftoa(buffer, value);
#if defined(CLI_INTEFACE)
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"Actual output value: ");
	usr_uart_write_string(dev->usr_uart_dev, buffer);
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"mA\n");
#endif
	reg_readback_val = ad5755_get_register_value(dev->ad5755_dev,
			   AD5755_RD_DATA_REG(chan));
	itoa(reg_readback_val, (char *)buffer, 16);
#if defined(CLI_INTEFACE)
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"Data register value: ");
	usr_uart_write_string(dev->usr_uart_dev, buffer);
	return usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");
#elif defined(MODBUS_INTERFACE)
	return 0;
#endif
}

/**
 * Set code on a channel.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - String including the selected channel and desired code as
 *                   separate arguments divided by a space.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_set_channel_code(struct cn0418_dev *dev, uint8_t *arg)
{
	uint32_t code;
	uint8_t *reg_chan_strg, *reg_data_strg;
	uint8_t chan = 0;
	uint8_t buffer[10];
	uint16_t reg_readback_val;

	reg_data_strg = (uint8_t*)strchr((char*)arg, ' ') + 1;
	reg_chan_strg = arg;
	reg_chan_strg[strcspn((char*)reg_chan_strg, " ")] = '\0';

	while (channels[chan][0] != '\0') {
		if(strncmp((char *)reg_chan_strg, (char *)channels[chan], 4) == 0)
			break;
		chan++;
	}
	if(chan >= 4) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"Wrong channel option. Valid channel options are:\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - cha\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chb\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chc\n");
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"    - chd\n");
#elif defined(MODBUS_INTERFACE)
		return 0;
#endif
	}

	code = atoi((char *)reg_data_strg);

	if(code > USHRT_MAX) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"Code value can't be bigger than ");
		itoa(USHRT_MAX, (char *)buffer, 10);
		usr_uart_write_string(dev->usr_uart_dev, buffer);
		return usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)".\n");
#elif defined(MODBUS_INTERFACE)
		return 0;
#endif
	}

	ad5755_set_register_value(dev->ad5755_dev, AD5755_DREG_WR_DAC, chan, code);

	reg_readback_val = ad5755_get_register_value(dev->ad5755_dev,
			   AD5755_RD_DATA_REG(chan));
	itoa(reg_readback_val, (char *)buffer, 16);
#if defined(CLI_INTEFACE)
	usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"Data register value: ");
	usr_uart_write_string(dev->usr_uart_dev, buffer);
	return usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");
#elif defined(MODBUS_INTERFACE)
	return 0;
#endif
}

/**
 * Set code on a channel.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - String including the selected channel and desired code as
 *                   separate arguments divided by a space.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_set_channel_rset(struct cn0418_dev *dev, uint8_t *arg)
{
	uint8_t *reg_chan_strg, *reg_data_strg;
	uint8_t chan = 0;
	uint8_t code = 0;
	int32_t reg_val;
	uint8_t buffer[10];
	uint16_t reg_readback_val;

	reg_data_strg = (uint8_t*)strchr((char*)arg, ' ') + 1;
	reg_chan_strg = arg;
	reg_chan_strg[strcspn((char*)reg_chan_strg, " ")] = '\0';

	while (channels[chan][0] != '\0') {
		if(strncmp((char *)reg_chan_strg, (char *)channels[chan], 4) == 0)
			break;
		chan++;
	}
#if defined(CLI_INTEFACE)
	if(chan >= 4) {
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"Wrong channel option. Valid channel options are:\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - cha\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chb\n");
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"    - chc\n");
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"    - chd\n");
	}
#endif
	while (rset_options[code][0] != '\0') {
		if(strncmp((char *)reg_data_strg, (char *)rset_options[code], 4) == 0)
			break;
		code++;
	}
#if defined(CLI_INTEFACE)
	if(code > 1) {
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"Invalid option. Valid options are:\n");
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"    - ext: use external Rset\n");
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"    - int: use internal Rset\n");
	}
#endif
	reg_val = ad5755_get_register_value(dev->ad5755_dev,
					    AD5755_RD_CTRL_REG(chan));
	reg_val &= ~AD5755_DAC_RSET;
	reg_val |= (code == 0) ? 0 : AD5755_DAC_RSET;
	ad5755_set_control_registers(dev->ad5755_dev, AD5755_CREG_DAC, chan,
				     reg_val);

	reg_readback_val = ad5755_get_register_value(dev->ad5755_dev,
			   AD5755_RD_CTRL_REG(chan));
	itoa(reg_readback_val, (char *)buffer, 16);
#if defined(CLI_INTEFACE)
	usr_uart_write_string(dev->usr_uart_dev,
			      (uint8_t*)"New value of the DAC control regiter is: ");
	usr_uart_write_string(dev->usr_uart_dev, buffer);
	return usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");
#elif defined(MODBUS_INTERFACE)
	return 0;
#endif
}
#if defined(CLI_INTEFACE)
/**
 * Display EEPROM address.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_mem_display_addr(struct cn0418_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint8_t addr;
	uint8_t buffer[5];
	uint8_t i = 0;
	uint8_t found_flag = 0;

	while(1) {
		ret = cn0418_mem_discover(dev, i, &addr);
		if(ret < 0)
			return ret;

		if(addr <= (A0_VDD | A1_VDD | A2_VDD)) {
			usr_uart_write_string(dev->usr_uart_dev,
					      (uint8_t*)"EEPROM discovered at address: 0x5");
			itoa(addr, (char *)buffer, 16);
			usr_uart_write_string(dev->usr_uart_dev, buffer);
			usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");

			usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)" - P13 = ");
			itoa(A0_MASK(addr), (char *)buffer, 10);
			usr_uart_write_string(dev->usr_uart_dev, buffer);
			usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");
			usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)" - P14 = ");
			itoa(A1_MASK(addr), (char *)buffer, 10);
			usr_uart_write_string(dev->usr_uart_dev, buffer);
			usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");
			usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)" - P15 = ");
			itoa(A2_MASK(addr), (char *)buffer, 10);
			usr_uart_write_string(dev->usr_uart_dev, buffer);
			usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)"\n");

			i = addr + 1;
			found_flag = 1;
		} else {
			break;
		}
	}

	if(found_flag != 1) {
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"No EEPROM found.\n");
	}

	return ret;
}
#endif

/**
 * Detect HART command zero from a freshly received buffer.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0418_process_hart_detect_command_zero(struct cn0418_dev *dev)
{
	uint8_t i = 1;
	uint8_t k = 0;

	while(i <= 20) {
		if(dev->hart_buffer[i] == 0xFF) {
			i++;
			continue;
		} else {
			break;
		}
	}

	if(i <= 20)
		for(k = 0; k < HART_COMMAND_ZERO_SIZE; k++)
			if(dev->hart_buffer[i + k] != cn0418_hart_command_zero[k])
				break;

	if(k == HART_COMMAND_ZERO_SIZE) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->usr_uart_dev,
				      (uint8_t*)"Command zero detected.\n");
#endif
	}
}

/**
 * Implements the CLI logic.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_process(struct cn0418_dev *dev)
{
	int32_t ret;

	if(modem_rec_flag == true) {
		ret = cn0418_process_hart_int_rec(dev);
		if(ret < 0)
#if defined(CLI_INTEFACE)
			usr_uart_write_string(dev->usr_uart_dev,
					      (uint8_t*)"ERROR in HART receive.");
#endif
		cn0418_process_hart_detect_command_zero(dev);
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->usr_uart_dev, (uint8_t*)">");
#endif
	}

	return 0;
}

/**
 * cn0418_hart_change_channel helper function.
 *
 * @param [in] dev     - The device structure.
 * @param [in] channel - New selected channel.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_hart_change_chan_helper(struct cn0418_dev *dev,
				       enum hart_channels_cn0418 channel)
{
	int32_t ret;

	ret = gpio_set_value(dev->hart_mult_a1, (channel & 0x2) >> 1);
	if(ret < 0)
		return ret;
	return gpio_set_value(dev->hart_mult_a0, (channel & 0x1) >> 0);
}

/**
 * Convert floating point value to ASCII. Maximum 8 decimals.
 *
 * @param [in] dev   - The device structure.
 * @param [in] value - The floating point value to be converted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
void cn0418_ftoa(uint8_t *buffer, float value)
{
	int32_t fraction;
	uint8_t local_buffer[20];
	float subunit;
	uint8_t i;

	strcpy((char *)buffer, "");

	if((value < 0.0) && (value > -1.0))
		strcat((char *)buffer, "-");

	itoa((int32_t)value, (char*)local_buffer, 10);
	strcat((char *)buffer, (char *)local_buffer);
	strcat((char *)buffer, ".");
	/* Display zeros after the decimal point. Else they would be eluded. */
	subunit = fmod(value, 1.0);
	i = 0;
	while(fabs(subunit) < 0.1 && i < 7) {
		strcat((char*)buffer, "0");
		subunit *= 10;
		i++;
	}
	fraction = (int32_t)(value * 100000000) % 100000000;
	itoa(fabs(fraction), (char*)local_buffer, 10);
	strcat((char*)buffer, (char*)local_buffer);
}

/**
 * Discover the first EEPROM present on the board.
 *
 * Checks every address available by trying to write and read one memory
 * location and, if successful, restore memory after it's done.
 *
 * @param [in] dev		  - The device structure.
 * @param [in] start_addr - I2C address from where to start searching (not
 *                          bigger than 7).
 * @param [out] address   - I2C address of the memory encountered.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_mem_discover(struct cn0418_dev *dev, uint8_t start_addr,
			    uint8_t* address)
{
	int32_t ret;
	uint8_t i;
	uint8_t backup, temp, test = 0xAA;
	uint16_t mem_addr = 0;
	uint8_t test_transfer_size = 1;

	if(start_addr > 7)
		return -1;

	for (i = start_addr; i < (A0_VDD | A1_VDD | A2_VDD) + 1; ++i) {
		ret = memory_change_i2c_address(dev->memory_device, i);
		if(ret < 0)
			return ret;

		memory_read(dev->memory_device, mem_addr, &backup, test_transfer_size);
		memory_write(dev->memory_device, mem_addr, &test, test_transfer_size);
		memory_read(dev->memory_device, mem_addr, &temp, test_transfer_size);
		if(temp == test) {
			ret = memory_write(dev->memory_device, mem_addr, &backup,
					   test_transfer_size);
			if(ret < 0)
				return ret;
			break;
		}
	}

	*address = i;

	return 0;
}
#if defined(CLI_INTEFACE)
/**
 * cn0418_status helper function.
 *
 * @param [in] dev   - The device structure.
 * @param [in] index - Index of the channel.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0418_status_helper(struct cn0418_dev *dev, uint8_t index)
{
	int32_t ret;
	int32_t temp_reg_value;
	uint8_t ascii_buff[20];
	uint8_t range_status;

	switch(index) {
	case 0:
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"Channel A:\n");
		if(ret < 0)
			return ret;
		break;
	case 1:
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"Channel B:\n");
		if(ret < 0)
			return ret;
		break;
	case 2:
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"Channel C:\n");
		if(ret < 0)
			return ret;
		break;
	case 3:
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"Channel D:\n");
		if(ret < 0)
			return ret;
		break;
	default:
		return usr_uart_write_string(dev->usr_uart_dev,
					     (uint8_t*)"Error!\n");
	}

	/* Read DAC control register */
	temp_reg_value = ad5755_get_register_value(dev->ad5755_dev,
			 AD5755_RD_CTRL_REG(index));

	/* Output status */
	ret = usr_uart_write_string(dev->usr_uart_dev,
				    (uint8_t*)"    Output: ");
	if(ret < 0)
		return ret;
	if((temp_reg_value & AD5755_DAC_OUTEN) != 0)
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"ENABLED\n");
	else
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"DISABLED\n");
	if(ret < 0)
		return ret;

	/* RSET status */
	ret = usr_uart_write_string(dev->usr_uart_dev,
				    (uint8_t*)"    RSET: ");
	if(ret < 0)
		return ret;
	if((temp_reg_value & AD5755_DAC_RSET) != 0)
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"INTERNAL\n");
	else
		ret = usr_uart_write_string(dev->usr_uart_dev,
					    (uint8_t*)"EXTERNAL\n");
	if(ret < 0)
		return ret;

	/* Channel range */
	ret = usr_uart_write_string(dev->usr_uart_dev,
				    (uint8_t*)"    Range: ");
	if(ret < 0)
		return ret;
	range_status = temp_reg_value & AD5755_DAC_R(7);
	ret = usr_uart_write_string(dev->usr_uart_dev,
				    (uint8_t*)chan_ranges[range_status]);
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->usr_uart_dev,
				    (uint8_t*)"\n");
	if(ret < 0)
		return ret;

	/* Read DAC control register */
	temp_reg_value = ad5755_get_register_value(dev->ad5755_dev,
			 AD5755_RD_DATA_REG(index));

	/* Display channel code */
	itoa((uint16_t)temp_reg_value, (char *)ascii_buff, 10);
	ret = usr_uart_write_string(dev->usr_uart_dev,
				    (uint8_t*)"    Code on channel is: ");
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->usr_uart_dev, ascii_buff);
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->usr_uart_dev,
				    (uint8_t*)"\n");
	if(ret < 0)
		return ret;

	return 0;
}
#endif
