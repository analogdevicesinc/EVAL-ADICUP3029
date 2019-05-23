/***************************************************************************//**
*   @file   system_manager.c
*   @brief  System manager source.
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

#include <string.h>
#include <stdlib.h>
#include "system_manager.h"
#include "timer.h"

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/

extern uint8_t *cn0414_cmd_commands[];
extern cmd_func cn0414_v_cmd_fun[];
extern uint8_t cn0414_command_size[];
extern uint8_t *cn0418_cmd_commands[];
extern cmd_func cn0418_v_cmd_fun[];
extern uint8_t cn0418_command_size[];

/* Available CLI commands */
char *system_cmd_commands[] = {
	"help",
	"h",
	"board_set ",
	"bs ",
	""
};

/* Functions for available CLI commands */
cmd_func system_v_cmd_fun[] = {
	(cmd_func)system_help,
	(cmd_func)system_board_set,
	NULL
};

/* CLI command sizes */
uint8_t system_command_size[] = {5, 2, 10, 3, 1};

uint8_t cn0414_chan_index_remain = VIN1;

extern uint8_t *output_coding_options[];
extern uint8_t *fiter_options[];
extern uint8_t *postfiter_options[];
extern uint8_t *odr_options[];
extern uint8_t *cn0414_hart_chan_names[];
extern uint8_t *chan_ranges[];
extern uint8_t chan_range_len[];
extern uint8_t *cn0418_hart_chan_names[];
extern uint8_t *channels[];

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * system_setup() helper function.
 *
 * Initialize the GPIOs for the system. These are the SPI CS address and the
 * HART address GPIOs.
 *
 * @param [in] dev        - The device structure.
 * @param [in] init_param - Pointer to the initialization structure.
 *
 * @return 0 for success of negative error code in case of failure.
 */
static int32_t systemp_setup_gpio(struct system_manager_dev *dev,
				  struct system_manager_init *init_param)
{
	int32_t ret;

	ret = gpio_get(&dev->gpio_cs_deco_a0, init_param->gpio_cs_deco_a0);
	if(ret < 0)
		return ret;
	ret = gpio_get(&dev->gpio_cs_deco_a1, init_param->gpio_cs_deco_a1);
	if(ret < 0)
		return ret;
	ret = gpio_get(&dev->gpio_hart_deco_a0, init_param->gpio_hart_deco_a0);
	if(ret < 0)
		return ret;
	ret = gpio_get(&dev->gpio_hart_deco_a1, init_param->gpio_hart_deco_a1);
	if(ret < 0)
		return ret;
	ret = gpio_get(&dev->gpio_spi_cs, init_param->gpio_spi_cs);
	if(ret < 0)
		return ret;

	ret = gpio_direction_output(dev->gpio_cs_deco_a0, GPIO_LOW);
	if(ret < 0)
		return ret;
	ret = gpio_direction_output(dev->gpio_cs_deco_a1, GPIO_LOW);
	if(ret < 0)
		return ret;
	ret = gpio_direction_output(dev->gpio_hart_deco_a0, GPIO_LOW);
	if(ret < 0)
		return ret;
	ret = gpio_direction_output(dev->gpio_hart_deco_a1, GPIO_LOW);
	if(ret < 0)
		return ret;
	return gpio_direction_output(dev->gpio_spi_cs, GPIO_LOW);
}

/**
 * system_setup() helper function.
 *
 * Pass the device descriptors for the EEPROM and the HART transceiver to the
 * present boards.
 *
 * @param [in] dev        - The device structure.
 * @param [in] cn0414_dev - Pointer to the next CN0414 descriptor.
 * @param [in] cn0418_dev - Pointer to the next CN0418 descriptor.
 *
 * @return void
 */
static void systemp_setup_attach_descriptors(struct system_manager_dev *dev,
		struct cn0414_dev *cn0414_dev, struct cn0418_dev *cn0418_dev)
{
	if(cn0414_dev != NULL) {
		cn0414_dev->ad5700_device = dev->ad5700_device;
		cn0414_dev->memory_device = dev->memory_device;
#if defined(CLI_INTEFACE)
		cn0414_dev->uart_descriptor = dev->cli_descriptor->uart_device;
#endif
	}

	if(cn0418_dev != NULL) {
		cn0418_dev->ad5700_device = dev->ad5700_device;
		cn0418_dev->memory_device = dev->memory_device;
#if defined(CLI_INTEFACE)
		cn0418_dev->usr_uart_dev = dev->cli_descriptor->uart_device;
#endif
	}
}

/**
 * system_setup() helper function.
 *
 * Discover the boards present on the system. Cycle through CS addresses and I2C
 * addresses to discover EEPROM memories present on the board. After a board is
 * discovered try to read the ID register assuming that the board is Cn0414. If
 * the ID register is not discovered try to write then read the main control
 * register assuming the board is CN0418. If none of these operations is
 * successful there is something wrong with the board. Otherwise memorize the
 * type and CS address of the board as well as I2C address of the EEPROM.
 *
 * @param [in] dev        - The device structure.
 * @param [in] init_param - Pointer to the initialization structure.
 *
 * @return void
 */
static int32_t systemp_setup_get_boards(struct system_manager_dev *dev,
					struct system_manager_init *init_param)
{
	uint8_t cs_loop;
	uint8_t mem_address;
	ad717x_st_reg *preg;
	int32_t ret;
	uint8_t board_index;

	board_index = dev->present_boards_number;

	for(cs_loop = 0; cs_loop < 4; cs_loop++) {
		gpio_set_value(dev->gpio_spi_cs, GPIO_LOW);
		gpio_set_value(dev->gpio_cs_deco_a0, (cs_loop & 0x01) >> 0);
		gpio_set_value(dev->gpio_cs_deco_a1, (cs_loop & 0x02) >> 1);

		system_mem_discover(dev, 0, &mem_address);

		if(mem_address > (A0_VDD | A1_VDD | A2_VDD))
			continue;

		gpio_set_value(dev->gpio_spi_cs, GPIO_HIGH);

		dev->board_i2c_mem_address[board_index] = mem_address;
		dev->board_cs_deco_select[board_index] = cs_loop;

		ret = cn0414_setup_minimum((dev->cn0414_type_tab + board_index),
					   &init_param->cn0414_type_init);
		if(ret < 0) {
			return ret;
		}

		preg = AD717X_GetReg((*(dev->cn0414_type_tab + board_index))->
				     ad4111_device, AD717X_ID_REG);

		if((preg->value & AD717X_ID_REG_MASK) == AD4111_ID_REG_VALUE) {
			dev->boards[board_index] = CN0414;
			systemp_setup_attach_descriptors(dev,
							 *(dev->cn0414_type_tab + board_index), NULL);
			dev->present_boards_number = ++board_index;
			continue;
		}

		ret = cn0414_remove_minimum(*(dev->cn0414_type_tab + board_index));
		if(ret < 0) {
			return ret;
		}
		ret = cn0418_setup_minimum((dev->cn0418_type_tab + board_index),
					   &init_param->cn0418_type_init);
		if(ret < 0) {
			return ret;
		}
		ret = cn0418_setup_dac_verify_presence_min(*(dev->cn0418_type_tab +
				board_index));
		if(ret < 0) {
			ret = cn0418_remove_minimum(*(dev->cn0418_type_tab + board_index));
			if(ret < 0)
				return ret;
			continue;
		}
		dev->boards[board_index] = CN0418;
		systemp_setup_attach_descriptors(dev, NULL,
						 *(dev->cn0418_type_tab + board_index));
		dev->present_boards_number = ++board_index;
	}

	return ret;
}

#if defined(CLI_INTEFACE)
/**
 * Display the parameters of a board discovered in the system.
 *
 * @param [in] dev   - System handler.
 * @param [in] index - The index of the board in the discovered boards table of
 *                     the system.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t systemp_setup_display_board_param(struct system_manager_dev *dev,
		uint8_t index)
{
	uint8_t board_i2c_addr;
	uint8_t buff[20];

	itoa(index, (char *)buff, 10);
	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)" Board ");
	usr_uart_write_string(dev->cli_descriptor->uart_device, buff);
	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)":\n");
	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)"    Board type: ");
	if(dev->boards[index] == CN0414) {
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"CN0414\n");
		board_i2c_addr = dev->board_i2c_mem_address[index] | 0x50;
		itoa(board_i2c_addr, (char *)buff, 16);
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"    I2C memory address: 0x");
		usr_uart_write_string(dev->cli_descriptor->uart_device, buff);
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"\n    CS decoder GPIOs:\n");
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"     - A0 = ");
		itoa((dev->board_cs_deco_select[index] & 1), (char *)buff, 10);
		usr_uart_write_string(dev->cli_descriptor->uart_device, buff);
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"\n     - A1 = ");
		itoa((dev->board_cs_deco_select[index] & 2) >> 1, (char *)buff, 10);
		usr_uart_write_string(dev->cli_descriptor->uart_device, buff);
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"\n\n");
		return 0;
	}
	if(dev->boards[index] == CN0418) {
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"CN0418\n");
		board_i2c_addr = dev->board_i2c_mem_address[index] | 0x50;
		itoa(board_i2c_addr, (char *)buff, 16);
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"    I2C memory address: 0x");
		usr_uart_write_string(dev->cli_descriptor->uart_device, buff);
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"\n    CS decoder GPIOs:\n");
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"    - A0 = ");
		itoa((dev->board_cs_deco_select[index] & 1), (char *)buff, 10);
		usr_uart_write_string(dev->cli_descriptor->uart_device, buff);
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"\n    - A1 = ");
		itoa((dev->board_cs_deco_select[index] & 2) >> 1, (char *)buff, 10);
		usr_uart_write_string(dev->cli_descriptor->uart_device, buff);
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"\n\n");
		return 0;
	}
	return -1;
}
#elif defined(MODBUS_INTERFACE)
/**
 * system_setup_modbus_registers() helper function.
 *
 * Initialize the MODBUS registers linked to a CN0414 board present on the
 * board.
 *
 * @param [in] dev               - The device structure.
 * @param [in] input_reg_index   - Pointer to the input registers index.
 * @param [in] holding_reg_index - Pointer to the holding registers index.
 * @param [in] board_index       - Index of the board.
 *
 * @return void
 */
static void system_setup_mb_414_regs(struct system_manager_dev *dev,
				     uint8_t *input_reg_index, uint8_t *holding_reg_index,
				     uint8_t board_index)
{
	uint8_t j;
	uint16_t curr_addr;

	for(j = 5; j < CN0414_MODBUS_INPUT_REG_NUMBER + 5; j++) {
		curr_addr = (dev->board_cs_deco_select[board_index] << 12) +
			    (dev->mb_slave_desc->slave_id << 8) + j;
		dev->mb_slave_desc->analog_in_regs[*input_reg_index].address =
			curr_addr;
		dev->mb_slave_desc->analog_in_regs[(*input_reg_index)++].data = 0;
	}
	for(j = 0; j < CN0414_MODBUS_HOLDING_REG_NUMBER; j++) {
		curr_addr = (dev->board_cs_deco_select[board_index] << 12) +
			    (dev->mb_slave_desc->slave_id << 8) + j;
		dev->mb_slave_desc->out_holding_regs[*holding_reg_index].address =
			curr_addr;
		dev->mb_slave_desc->out_holding_regs[(*holding_reg_index)++].data = 0;
	}
}

/**
 * system_setup_modbus_registers() helper function.
 *
 * Initialize the MODBUS registers linked to a CN0418 board present on the
 * board.
 *
 * @param [in] dev               - The device structure.
 * @param [in] input_reg_index   - Pointer to the input registers index.
 * @param [in] holding_reg_index - Pointer to the holding registers index.
 * @param [in] board_index       - Index of the board.
 *
 * @return void
 */
static void system_setup_mb_418_regs(struct system_manager_dev *dev,
				     uint8_t *input_reg_index, uint8_t *holding_reg_index,
				     uint8_t board_index)
{
	uint8_t j;
	uint16_t curr_addr;

	for(j = 5; j < CN0418_MODBUS_INPUT_REG_NUMBER + 5; j++) {
		curr_addr = (dev->board_cs_deco_select[board_index] << 12) +
			    (dev->mb_slave_desc->slave_id << 8) + j;
		dev->mb_slave_desc->analog_in_regs[*input_reg_index].address =
			curr_addr;
		dev->mb_slave_desc->analog_in_regs[(*input_reg_index)++].data = 0;
	}
	for(j = 0; j < CN0418_MODBUS_HOLDING_REG_NUMBER; j++) {
		curr_addr = (dev->board_cs_deco_select[board_index] << 12) +
			    (dev->mb_slave_desc->slave_id << 8) + j;
		dev->mb_slave_desc->out_holding_regs[*holding_reg_index].address =
			curr_addr;
		dev->mb_slave_desc->out_holding_regs[(*holding_reg_index)++].data = 0;
	}
}

/**
 * system_setup() helper function.
 *
 * Allocate memory and initialize the MODBUS registers based on the number and
 * type of boards discovered in the system.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void system_setup_modbus_registers(struct system_manager_dev *dev)
{
	uint8_t i;
	uint16_t analog_in_reg_nr = 5;
	uint16_t hold_reg_nr = 2;
	uint8_t in_reg_curr_index = 0;
	uint8_t hold_reg_curr_index = 0;
	uint32_t update_rate_int_temp;

	for(i = 0; i < dev->present_boards_number; i++) {
		if(dev->boards[i] == CN0414) {
			analog_in_reg_nr += CN0414_MODBUS_INPUT_REG_NUMBER;
			hold_reg_nr += CN0414_MODBUS_HOLDING_REG_NUMBER;
		}
		if(dev->boards[i] == CN0418) {
			analog_in_reg_nr += CN0418_MODBUS_INPUT_REG_NUMBER;
			hold_reg_nr += CN0418_MODBUS_HOLDING_REG_NUMBER;
		}
	}

	dev->mb_slave_desc->analog_in_regs = calloc(analog_in_reg_nr,
					     sizeof *dev->mb_slave_desc->analog_in_regs);
	dev->mb_slave_desc->out_holding_regs = calloc(hold_reg_nr,
					       sizeof *dev->mb_slave_desc->out_holding_regs);
	dev->mb_slave_desc->in_reg_range = analog_in_reg_nr;
	dev->mb_slave_desc->holding_reg_range = hold_reg_nr;

	/* Populate permanent registers */
	dev->mb_slave_desc->analog_in_regs[in_reg_curr_index].address =
		in_reg_curr_index;
	dev->mb_slave_desc->analog_in_regs[in_reg_curr_index++].data =
		dev->present_boards_number;
	for(i = 1; i < 5; i++) {
		dev->mb_slave_desc->analog_in_regs[i].address = i;
		dev->mb_slave_desc->analog_in_regs[i].data =
			dev->boards[i - 1] | (dev->board_cs_deco_select[i - 1] << 1);
		in_reg_curr_index++;
	}
	update_rate_int_temp = dev->adc_update_timer->f_update * 10000;
	dev->mb_slave_desc->out_holding_regs[hold_reg_curr_index].address =
		MODBUS_GLOBAL_UPDATE_RATE_REG_ADDR;
	dev->mb_slave_desc->out_holding_regs[hold_reg_curr_index++].data =
		(update_rate_int_temp & 0xFFFF0000) >> 16;
	dev->mb_slave_desc->out_holding_regs[hold_reg_curr_index].address =
		MODBUS_GLOBAL_UPDATE_RATE_REG_ADDR + 1;
	dev->mb_slave_desc->out_holding_regs[hold_reg_curr_index++].data =
		update_rate_int_temp & 0x0000FFFF;

	/* Populate registers */
	for(i = 0; i < dev->present_boards_number; i++) {
		if (dev->boards[i] == CN0414) {
			system_setup_mb_414_regs(dev, &in_reg_curr_index,
						 &hold_reg_curr_index, i);
		} else if(dev->boards[i] == CN0418) {
			system_setup_mb_418_regs(dev, &in_reg_curr_index,
						 &hold_reg_curr_index, i);
		}
	}
}
#endif

/**
 * Initialize the PLC or DCS system.
 *
 * @param [out] device    - The device structure.
 * @param [in] init_param - Pointer to the structure that contains the systems
 *                          initial parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t system_setup(struct system_manager_dev **device,
		     struct system_manager_init *init_param)
{
	struct system_manager_dev *dev;
	int32_t ret;
	uint32_t i;

	timer_start();

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	dev->active_device = 0;
	dev->present_boards_number = 0;
	for(i = 0; i < BOARD_SLOT_NUMBER; i++) {
		dev->boards[i] = NONE;
		dev->board_i2c_mem_address[i] = 0x00;
	}
	dev->cn0414_type_tab = calloc(BOARD_SLOT_NUMBER,
				      sizeof *dev->cn0414_type_tab);
	dev->cn0418_type_tab = calloc(BOARD_SLOT_NUMBER,
				      sizeof *dev->cn0418_type_tab);
#if defined(CLI_INTEFACE)
	ret = cli_setup(&dev->cli_descriptor, &init_param->cli_init);
	if(ret < 0)
		goto error;
#elif defined(MODBUS_INTERFACE)
	ret = modbus_setup(&dev->mb_slave_desc, &init_param->mb_slave_init);
	if(ret < 0)
		goto error;

	ret = adc_update_setup(&dev->adc_update_timer,
			       &init_param->cn0414_type_init.adc_update_init);
	if(ret < 0)
		goto error;
#endif
	ret = ad5700_setup(&dev->ad5700_device, &init_param->ad5700_init);
	if(ret < 0)
		goto error;
	NVIC_DisableIRQ(HART_CD_INT);

	ret = memory_setup(&dev->memory_device, &init_param->memory_init);
	if(ret < 0)
		goto error;

	ret = systemp_setup_gpio(dev, init_param);
	if(ret < 0) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"GPIO setup failed.\n");
#endif
		goto error;
	}

	ret = systemp_setup_get_boards(dev, init_param);
	if(ret < 0) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"No boards found or initialization error\n");
#endif
		goto error;
	}
#if defined(MODBUS_INTERFACE)
	system_setup_modbus_registers(dev);
	for (i = 0; i < dev->present_boards_number; ++i) {
		if (dev->boards[i] == CN0414) {
			ret = adc_update_activate(dev->adc_update_timer, true);
			if(ret < 0)
				goto error;
			break;
		}
	}
	for (i = 0; i < dev->present_boards_number; ++i) {
		if (dev->boards[i] == CN0414) {
			dev->cn0414_type_tab[i]->adc_update_desc = dev->adc_update_timer;
			system_change_active_board(dev, i);
			cn0414_hart_enable(dev->cn0414_type_tab[i], NULL);
		}
	}
	system_change_active_board(dev, 0);
#endif
#if defined(CLI_INTEFACE)
	dev->cli_descriptor->device_descriptor = NULL;
#endif
	*device = dev;

	return ret;

error:
	free(dev);

	return ret;
}

/**
 * system_remove() helper function.
 *
 * Cleanly remove the GPIO descriptors.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t systemp_remove_gpio(struct system_manager_dev *dev)
{
	int32_t ret;

	ret = gpio_remove(dev->gpio_cs_deco_a0);
	if(ret < 0)
		return ret;
	ret = gpio_remove(dev->gpio_cs_deco_a1);
	if(ret < 0)
		return ret;
	ret = gpio_remove(dev->gpio_hart_deco_a0);
	if(ret < 0)
		return ret;
	ret = gpio_remove(dev->gpio_hart_deco_a1);
	if(ret < 0)
		return ret;
	return gpio_remove(dev->gpio_spi_cs);
}

/**
 * Free resources allocated by the system_setup().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t system_remove(struct system_manager_dev *dev)
{
	uint8_t i;
	int32_t ret;

	if(!dev)
		return -1;

	for(i = 0; i < BOARD_SLOT_NUMBER; i++) {
		if(dev->boards[i] == CN0414) {
			cn0414_remove(dev->cn0414_type_tab[i]);
		} else if(dev->boards[i] == CN0414) {
			cn0418_remove(dev->cn0418_type_tab[i]);
		}
	}

	if(dev->cn0414_type_tab)
		free(dev->cn0414_type_tab);
	if(dev->cn0418_type_tab)
		free(dev->cn0418_type_tab);

	ret = ad5700_remove(dev->ad5700_device);
	if(ret < 0) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"ad5700_remove failed.\n");
#endif
		return ret;
	}

	ret = memory_remove(dev->memory_device);
	if(ret < 0) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"memory_remove failed.\n");
#endif
		return ret;
	}

	ret = systemp_remove_gpio(dev);
	if(ret < 0) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"systemp_remove_gpio failed.\n");
#endif
		return ret;
	}
#if defined(CLI_INTEFACE)
	ret = cli_remove(dev->cli_descriptor);
	if(ret < 0)
		return ret;
#endif
	free(dev);

	return ret;
}
#if defined(CLI_INTEFACE)
/**
 * Display the user prompt through UART.
 *
 * system_help() function helper.
 *
 * @param [in] dev           - The device structure.
 * @param [in] short_command - true if the prompt to be displayed is for full
 *                             commands, false if the prompt is for short
 *                             commands.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t system_help_prompt(struct system_manager_dev *dev,
				  bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->cli_descriptor->uart_device,
					    (uint8_t*)"PLC Arduino shields input/output system.\n");
		if(ret < 0) {
			return ret;
		}
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)
				      "For commands with options as arguments typing the command and 'space' without arguments\n");
		if(ret < 0) {
			return ret;
		}
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"will show the list of options.\n");
		if(ret < 0) {
			return ret;
		}
		return usr_uart_write_string(dev->cli_descriptor->uart_device,
					     (uint8_t*)"Available verbose commands.\n");
	} else {
		return usr_uart_write_string(dev->cli_descriptor->uart_device,
					     (uint8_t*)"\nAvailable short commands:\n\n");
	}
}

/**
 * Display the command menu.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t system_help(struct system_manager_dev *dev, uint8_t *arg)
{
	system_help_prompt(dev, HELP_LONG_COMMAND);

	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)" help                 - Display command menu.\n");

	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)" board_set <board_no> - Set active board.\n");
	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)"                        <board_no> = board number.\n");

	system_help_prompt(dev, HELP_SHORT_COMMAND);

	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)" h             - Display command menu.\n");

	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)" bs <board_no> - Set active board.\n");
	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)"                 <board_no> = board number.\n");

	return 0;
}

/**
 * Display error message and available boards in case the user tries to select a
 * wrong board.
 *
 * system_board_set() helper function.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void system_board_set_error(struct system_manager_dev *dev)
{
	uint8_t buff[20];
	uint8_t i;

	itoa(dev->present_boards_number, (char *)buff, 10);
	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)"\nThere are ");
	usr_uart_write_string(dev->cli_descriptor->uart_device, buff);
	usr_uart_write_string(dev->cli_descriptor->uart_device,
			      (uint8_t*)" boards currently available.\n");
	for(i = 0; i < dev->present_boards_number;
	    i++) {
		systemp_setup_display_board_param(dev, i);
	}
}

/**
 * Change the active board.
 *
 * Select the desired board by it's index in the found boards table and
 * designate it as the active board. The active board is the board that executes
 * the process method for itself.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - The index of the desired board.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t system_board_set(struct system_manager_dev *dev, uint8_t *arg)
{
	uint8_t board_number;
	int32_t ret;

	if(arg[0] == '\0') {
		system_board_set_error(dev);
		return 0;
	}

	board_number = atoi((char *)arg);

	ret = system_change_active_board(dev, board_number);
	if(ret < 0) {
		system_board_set_error(dev);
	}

	if (arg[0] != '\0') {
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)"\n Board ");
		usr_uart_write_string(dev->cli_descriptor->uart_device, arg);
		usr_uart_write_string(dev->cli_descriptor->uart_device,
				      (uint8_t*)" selected. Type <exit> to return to menu.\n");
	}

	return 0;
}
#elif defined(MODBUS_INTERFACE)
/**
 * Cycle between active devices.
 *
 * This function takes into account how many devices have been discovered and
 * cycles through them by incrementing the "active device index" and rolling
 * over when necessary. system_process() helper function.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void system_process_act_dev_inc(struct system_manager_dev *dev)
{
	if(dev->active_device == dev->present_boards_number - 1)
		dev->active_device = 0;
	else
		dev->active_device++;
}

/**
 * Call a CN0414 routine based on a MODBUS command.
 *
 * @param [in] dev         - The device structure.
 * @param [in] call_offset - Offset to the register address found in the MODBUS
 *                           command.
 *
 * @return void
 */
static void system_process_single_414_call(struct system_manager_dev *dev,
		uint16_t call_offset)
{
	uint16_t reg_function_code;
	float f_temp_container;
	uint32_t i_temp_container;
	struct mb_slave_register *temp_reg = NULL;
	uint8_t board_cs_addr, i;

	temp_reg = modbus_get_reg(dev->mb_slave_desc,
				  (dev->mb_slave_desc->register_address + call_offset),
				  OUTPUT_HOLDING_REGISTERS);

	if(temp_reg == NULL)
		return;

	reg_function_code = dev->mb_slave_desc->register_address + call_offset;
	reg_function_code &= 0x00FF;

	board_cs_addr = (dev->mb_slave_desc->register_address & 0xF000) >> 12;
	for(i = 0; i < dev->present_boards_number; i++) {
		if(dev->board_cs_deco_select[i] == board_cs_addr) {
			break;
		}
	}

	if(i == dev->present_boards_number)
		return;

	switch(reg_function_code) {
	case MODBUS_GLOBAL_UPDATE_RATE_REG_ADDR:
		i_temp_container = (uint32_t)temp_reg->data << 16;
		i_temp_container += (temp_reg + 1)->data;
		f_temp_container = i_temp_container / 10000.0;
		adc_update_set_rate(dev->adc_update_timer, f_temp_container);
		break;
	case CN0414_ADC_OUTPUT_CODING:
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		if(temp_reg->data == 1)
			cn0414_adc_set_output_coding(dev->cn0414_type_tab[i],
						     output_coding_options[2]);
		else if(temp_reg->data == 0)
			cn0414_adc_set_output_coding(dev->cn0414_type_tab[i],
						     output_coding_options[0]);
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0414_ADC_FILTER:
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		cn0414_adc_set_filter(dev->cn0414_type_tab[i],
				      fiter_options[temp_reg->data]);
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0414_ADC_POSTFILTER:
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		if(temp_reg->data == 4) {
			cn0414_adc_dis_postfilt(dev->cn0414_type_tab[i], NULL);
		} else {
			cn0414_adc_en_postfilt(dev->cn0414_type_tab[i], NULL);
			cn0414_adc_set_postfilt(dev->cn0414_type_tab[i],
						postfiter_options[temp_reg->data]);
		}
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0414_ADC_ODR:
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		cn0414_adc_set_odr(dev->cn0414_type_tab[i],
				   odr_options[temp_reg->data]);
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0414_ADC_OWD:
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		if(temp_reg->data == 0) {
			cn0414_adc_open_wire_disable(dev->cn0414_type_tab[i], NULL);
			reg_function_code = (dev->board_cs_deco_select[i] << 12) +
					    (dev->mb_slave_desc->slave_id << 8) + 21;
			temp_reg = modbus_get_reg(dev->mb_slave_desc, reg_function_code,
						  ANALOG_INPUT_REGISTERS);
			for(i = 0; i < ADC_VOLTAGE_CHAN_NO; i++)
				(temp_reg + i)->data = 0;
		} else {
			cn0414_adc_open_wire_enable(dev->cn0414_type_tab[i], NULL);
		}
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0414_HART_CMD_ZERO:
		if(temp_reg->data == 1) {
			i_temp_container = dev->active_device;
			system_change_active_board(dev, i);
			cn0414_hart_send_command_zero(dev->cn0414_type_tab[i],
						      (uint8_t *)"5");
			temp_reg->data = 0;
			system_change_active_board(dev, (uint8_t)i_temp_container);
		}
		break;
	case CN0414_HART_CHAN_SELECT:
		if(temp_reg->data >= 4)
			break;
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		cn0414_hart_change_chan_helper(dev->cn0414_type_tab[i], temp_reg->data);
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	default:
		break;
	}
}

/**
 * Call a CN0418 routine based on a MODBUS command.
 *
 * @param [in] dev         - The device structure.
 * @param [in] call_offset - Offset to the register address found in the MODBUS
 *                           command.
 *
 * @return void
 */
static void system_process_single_418_call(struct system_manager_dev *dev,
		uint16_t call_offset)
{
	uint16_t reg_function_code;
	uint32_t i_temp_container;
	struct mb_slave_register *temp_reg = NULL;
	uint8_t board_cs_addr, i;
	int32_t status_reg_val, data_reg_val;
	uint8_t j = 0;

	temp_reg = modbus_get_reg(dev->mb_slave_desc,
				  (dev->mb_slave_desc->register_address + call_offset),
				  OUTPUT_HOLDING_REGISTERS);

	if(temp_reg == NULL)
		return;

	reg_function_code = dev->mb_slave_desc->register_address + call_offset;
	reg_function_code &= 0x00FF;

	board_cs_addr = (dev->mb_slave_desc->register_address & 0xF000) >> 12;
	for(i = 0; i < dev->present_boards_number; i++) {
		if(dev->board_cs_deco_select[i] == board_cs_addr) {
			break;
		}
	}

	if(i == dev->present_boards_number)
		return;

	switch(reg_function_code) {
	case CN0418_DAC_RANGE_CH1:
		if(temp_reg->data >= 7)
			break;
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		ad5755_set_channel_range(dev->cn0418_type_tab[i]->ad5755_dev,
					 reg_function_code, temp_reg->data);
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0418_DAC_RANGE_CH2:
		if(temp_reg->data >= 7)
			break;
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		ad5755_set_channel_range(dev->cn0418_type_tab[i]->ad5755_dev,
					 reg_function_code, temp_reg->data);
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0418_DAC_RANGE_CH3:
		if(temp_reg->data >= 7)
			break;
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		ad5755_set_channel_range(dev->cn0418_type_tab[i]->ad5755_dev,
					 reg_function_code, temp_reg->data);
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0418_DAC_RANGE_CH4:
		if(temp_reg->data >= 7)
			break;
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		ad5755_set_channel_range(dev->cn0418_type_tab[i]->ad5755_dev,
					 reg_function_code, temp_reg->data);
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0418_DAC_OUTPUT_CH1:
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);

		data_reg_val =
			ad5755_get_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
						  AD5755_RD_DATA_REG(0));
		ad5755_set_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
					  AD5755_DREG_WR_DAC, (reg_function_code - 4), temp_reg->data);
		while(j++ < 20) {
			mdelay(1);
			status_reg_val =
				ad5755_get_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
							  AD5755_RD_STATUS_REG);
			if(status_reg_val) {
				ad5755_set_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
							  AD5755_DREG_WR_DAC, (reg_function_code - 4),
							  (uint16_t)data_reg_val);
				temp_reg->data = (uint16_t)data_reg_val;
			}
		}

		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0418_DAC_OUTPUT_CH2:
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);

		data_reg_val =
			ad5755_get_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
						  AD5755_RD_DATA_REG(1));
		ad5755_set_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
					  AD5755_DREG_WR_DAC, (reg_function_code - 4), temp_reg->data);
		while(j++ < 20) {
			mdelay(1);
			status_reg_val =
				ad5755_get_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
							  AD5755_RD_STATUS_REG);
			if(status_reg_val) {
				ad5755_set_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
							  AD5755_DREG_WR_DAC, (reg_function_code - 4),
							  (uint16_t)data_reg_val);
				temp_reg->data = (uint16_t)data_reg_val;
			}
		}

		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0418_DAC_OUTPUT_CH3:
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);

		data_reg_val =
			ad5755_get_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
						  AD5755_RD_DATA_REG(2));
		ad5755_set_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
					  AD5755_DREG_WR_DAC, (reg_function_code - 4), temp_reg->data);
		while(j++ < 20) {
			mdelay(1);
			status_reg_val =
				ad5755_get_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
							  AD5755_RD_STATUS_REG);
			if(status_reg_val) {
				ad5755_set_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
							  AD5755_DREG_WR_DAC, (reg_function_code - 4),
							  (uint16_t)data_reg_val);
				temp_reg->data = (uint16_t)data_reg_val;
			}
		}

		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0418_DAC_OUTPUT_CH4:
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);

		data_reg_val =
			ad5755_get_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
						  AD5755_RD_DATA_REG(3));
		ad5755_set_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
					  AD5755_DREG_WR_DAC, (reg_function_code - 4), temp_reg->data);
		while(j++ < 20) {
			mdelay(1);
			status_reg_val =
				ad5755_get_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
							  AD5755_RD_STATUS_REG);
			if(status_reg_val) {
				ad5755_set_register_value(dev->cn0418_type_tab[i]->ad5755_dev,
							  AD5755_DREG_WR_DAC, (reg_function_code - 4),
							  (uint16_t)data_reg_val);
				temp_reg->data = (uint16_t)data_reg_val;
			}
		}

		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0418_HART_CMD_ZERO:
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		if(temp_reg->data == 1) {
			cn0418_hart_send_command_zero(dev->cn0418_type_tab[i],
						      (uint8_t *)"5");
			temp_reg->data = 0;
		}
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	case CN0418_HART_CHAN_SELECT:
		if(temp_reg->data >= 4)
			break;
		i_temp_container = dev->active_device;
		system_change_active_board(dev, i);
		cn0418_hart_change_chan_helper(dev->cn0418_type_tab[i], temp_reg->data);
		system_change_active_board(dev, (uint8_t)i_temp_container);
		break;
	default:
		break;
	}
}

/**
 * Call multiple CN0414 routines based on a MODBUS command.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void system_process_multiple_414_call(struct system_manager_dev *dev)
{
	uint16_t i;

	if(dev->mb_slave_desc->register_address == 0xFE) {
		system_process_single_414_call(dev, 0);
		return;
	}

	for(i = 0; i < dev->mb_slave_desc->register_number; i++) {
		system_process_single_414_call(dev, i);
	}
}

/**
 * Call multiple CN0418 routines based on a MODBUS command.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void system_process_multiple_418_call(struct system_manager_dev *dev)
{
	uint16_t i;

	for(i = 0; i < dev->mb_slave_desc->register_number; i++) {
		system_process_single_418_call(dev, i);
	}
}

/**
 * Call routines based on the MODBUS request. Output holding registers are all
 * connected to a specific board routine. When these registers are written the
 * routines should be accessed.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void system_process_call_routine(struct system_manager_dev *dev)
{
	int8_t i;
	uint8_t current_board;
	uint8_t board_compare;

	current_board = (dev->mb_slave_desc->register_address & 0xF000) >> 12;

	for(i = 0; i < dev->present_boards_number; i++) {
		board_compare = dev->board_cs_deco_select[i];
		if(current_board == board_compare) {
			if(dev->boards[i] == CN0414) {
				system_process_multiple_414_call(dev);
			} else  if(dev->boards[i] == CN0418) {
				system_process_multiple_418_call(dev);
			}
		}
	}
}

/**
 * Update the active board as if it is a CN0414 type of board.
 *
 * Run the CN0414 process and if all the channels have been updated and the
 * called methods have been run change the active board to the next index found
 * in the board table.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void system_process_414_update(struct system_manager_dev *dev)
{
	int32_t ret;
	uint16_t input_reg_addr;
	struct mb_slave_register *temp_reg;
	uint8_t i;

	ret = cn0414_process(dev->cn0414_type_tab[dev->active_device]);
	if(ret == 0) {
		input_reg_addr = ((dev->board_cs_deco_select[dev->active_device] <<
				   12) + (dev->mb_slave_desc->slave_id << 8)) + 5 +
				 (dev->cn0414_type_tab[dev->active_device]->
				  channel_index * 2);
		temp_reg = modbus_get_reg(dev->mb_slave_desc, input_reg_addr,
					  ANALOG_INPUT_REGISTERS);
		if(temp_reg == NULL)
			return;
		temp_reg->data = (dev->cn0414_type_tab[dev->active_device]->
				  channel_output[dev->cn0414_type_tab[dev->active_device]->
						 channel_index] & 0xFFFF0000) >> 16;
		temp_reg++;
		temp_reg->data = dev->cn0414_type_tab[dev->active_device]->
				 channel_output[dev->cn0414_type_tab[dev->active_device]->
						channel_index] & 0x0000FFFF;
	}
	if(dev->cn0414_type_tab[dev->active_device]->open_wire_detect_enable ||
	    (dev->cn0414_type_tab[dev->active_device]->channel_index < 4)) {
		input_reg_addr = ((dev->board_cs_deco_select[dev->active_device] <<
				   12) | (dev->mb_slave_desc->slave_id << 8)) + 5 + 16 +
				 dev->cn0414_type_tab[dev->active_device]->
				 channel_index;
		temp_reg = modbus_get_reg(dev->mb_slave_desc, input_reg_addr,
					  ANALOG_INPUT_REGISTERS);
		if(temp_reg == NULL)
			return;
		temp_reg->data = dev->cn0414_type_tab[dev->active_device]->
				 chan_voltage_status[dev->
						     cn0414_type_tab[dev->active_device]->
						     channel_index];
	}
	if(dev->cn0414_type_tab[dev->active_device]->hart_buffer[0] == 1) {
		input_reg_addr = ((dev->board_cs_deco_select[dev->active_device] <<
				   12) | (dev->mb_slave_desc->slave_id << 8)) + 5 + 20;
		temp_reg = modbus_get_reg(dev->mb_slave_desc, input_reg_addr,
					  ANALOG_INPUT_REGISTERS);
		for(i = 0;
		    (i < dev->cn0414_type_tab[dev->active_device]->hart_rec_size) &&
		    (i < CN0418_MODBUS_INPUT_REG_NUMBER);
		    i++) {
			(temp_reg + i)->data = dev->
					       cn0414_type_tab[dev->active_device]->hart_buffer[i + 1];
		}
		dev->cn0414_type_tab[dev->active_device]->hart_buffer[0] = 0;
	}
	if(dev->cn0414_type_tab[dev->active_device]->channel_index == VIN1 &&
	    cn0414_chan_index_remain == IIN4) {
		system_process_act_dev_inc(dev);
		system_change_active_board(dev, dev->active_device);
		if(dev->boards[dev->active_device] == CN0414)
			dev->cn0414_type_tab[dev->active_device]->channel_index = VIN1;
	}
	cn0414_chan_index_remain =
		dev->cn0414_type_tab[dev->active_device]->channel_index;
}

/**
 * Update the active board as if it is a CN0418 type of board.
 *
 * Run the CN0414 process and if all  called methods have been run change the
 * active board to the next index found in the board table.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void system_process_418_update(struct system_manager_dev *dev)
{
	uint16_t input_reg_addr;
	struct mb_slave_register *temp_reg;
	uint8_t i;

	cn0418_process(dev->cn0418_type_tab[dev->active_device]);
	if(dev->cn0418_type_tab[dev->active_device]->hart_buffer[0] == 1) {
		input_reg_addr = ((dev->board_cs_deco_select[dev->active_device] <<
				   12) | (dev->mb_slave_desc->slave_id << 8)) + 5;
		temp_reg = modbus_get_reg(dev->mb_slave_desc, input_reg_addr,
					  ANALOG_INPUT_REGISTERS);
		for(i = 0;
		    (i < dev->cn0418_type_tab[dev->active_device]->hart_rec_size) &&
		    (i < CN0418_MODBUS_INPUT_REG_NUMBER);
		    i++) {
			(temp_reg + i)->data = dev->
					       cn0418_type_tab[dev->active_device]->hart_buffer[i + 1];
		}
		dev->cn0414_type_tab[dev->active_device]->hart_buffer[0] = 0;
	}
	system_process_act_dev_inc(dev);
	system_change_active_board(dev, dev->active_device);
	if(dev->boards[dev->active_device] == CN0414)
		dev->cn0414_type_tab[dev->active_device]->channel_index = VIN1;
}
#endif
/**
 * The application process method.
 *
 * This function runs in endless loop and
 * implements the MODBUS application layer and the ongoing board routines.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
void system_process(struct system_manager_dev *dev)
{
#if defined(CLI_INTEFACE)
	uint8_t i;

	if(dev->cli_descriptor->device_descriptor == NULL) {
		cli_load_command_vector(dev->cli_descriptor, system_v_cmd_fun);
		cli_load_command_calls(dev->cli_descriptor,
				       (uint8_t **)system_cmd_commands);
		cli_load_command_sizes(dev->cli_descriptor, system_command_size);
		cli_load_descriptor_pointer(dev->cli_descriptor, dev);
		for (i = 0; i < dev->present_boards_number; i++) {
			if(dev->boards[i] == CN0414) {
				adc_update_activate(dev->cn0414_type_tab[i]->
						    adc_update_desc, false);
			}
		}
		for (i = 0; i < dev->present_boards_number; ++i) {
			systemp_setup_display_board_param(dev, i);
		}
		usr_uart_write_string(dev->cli_descriptor->uart_device, (uint8_t*)">");
	}

	cli_process(dev->cli_descriptor);

	if(dev->boards[dev->active_device] == CN0414)
		cn0414_process(dev->cn0414_type_tab[dev->active_device]);
	else if(dev->boards[dev->active_device] == CN0418)
		cn0418_process(dev->cn0418_type_tab[dev->active_device]);
#elif defined(MODBUS_INTERFACE)
	static int32_t ret = 0;

	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(TMR1_INT);
	NVIC_DisableIRQ(TMR2_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	ret = modbus_receive(dev->mb_slave_desc, &dev->mb_slave_desc->request,
			     &dev->mb_slave_desc->request_size);
	if(ret < 0)
		usr_uart_ignore_input(dev->mb_slave_desc->phy_layer);

	if(dev->mb_slave_desc->state != MBSLAVE_IDLE)
		ret = modbus_process_open(dev->mb_slave_desc);

	if(dev->mb_slave_desc->mb_write_flag == 1) {
		system_process_call_routine(dev);
		dev->mb_slave_desc->mb_write_flag = 0;
	}

	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(TMR1_INT);
	NVIC_EnableIRQ(TMR2_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	if(dev->boards[dev->active_device] == CN0414)
		system_process_414_update(dev);
	else if(dev->boards[dev->active_device] == CN0418)
		system_process_418_update(dev);
#endif
}

/**
 * Change the active board in the system.
 *
 * Boards in the system cycle between each other to implement their respective
 * timed processes. Change the board by changing the active I2C address, SPI CS
 * address and the SPI mode in case the previous board and the new board are of
 * a different type (CN0414 vs. CN0418).
 *
 * @param [in] dev      - The device structure.
 * @param [in] board_no - Number of the next board.
 *
 * @return 0 for success, or negative error code.
 */
int32_t system_change_active_board(struct system_manager_dev *dev,
				   uint8_t board_no)
{
	int32_t ret;
	uint8_t pin_state;
#if defined(CLI_INTEFACE)
	uint8_t i;
#endif

	if((board_no >= BOARD_SLOT_NUMBER) ||
	    (board_no >= dev->present_boards_number))
		return -1;

	dev->active_device = board_no;
	pin_state = dev->board_cs_deco_select[board_no];
	gpio_set_value(dev->gpio_cs_deco_a0, (pin_state & 0x01) >> 0);
	gpio_set_value(dev->gpio_cs_deco_a1, (pin_state & 0x02) >> 1);
	gpio_set_value(dev->gpio_hart_deco_a0, (pin_state & 0x01) >> 0);
	gpio_set_value(dev->gpio_hart_deco_a1, (pin_state & 0x02) >> 1);
	ret = memory_change_i2c_address(dev->memory_device,
					dev->board_i2c_mem_address[board_no]);
	if(ret < 0)
		return ret;

	if(dev->boards[board_no] == CN0414) {
		ret = spi_change_mode(dev->cn0414_type_tab[board_no]->
				      ad4111_device->spi_desc);
		if(ret < 0) {
			return ret;
		}
#if defined(CLI_INTEFACE)
		ret = adc_update_set_rate(dev->cn0414_type_tab[board_no]->
					  adc_update_desc, dev->cn0414_type_tab[board_no]->
					  adc_update_desc->f_update);
		if(ret < 0) {
			return ret;
		}
		ret = adc_update_activate(dev->cn0414_type_tab[board_no]->
					  adc_update_desc, true);
		if(ret < 0) {
			return ret;
		}
		cli_load_command_vector(dev->cli_descriptor, cn0414_v_cmd_fun);
		cli_load_command_calls(dev->cli_descriptor, cn0414_cmd_commands);
		cli_load_command_sizes(dev->cli_descriptor, cn0414_command_size);
		cli_load_descriptor_pointer(dev->cli_descriptor,
					    dev->cn0414_type_tab[board_no]);
#endif
	} else if(dev->boards[board_no] == CN0418) {
		ret = spi_change_mode(dev->cn0418_type_tab[board_no]->
				      ad5755_dev->spi_desc);
		if(ret < 0) {
			return ret;
		}
#if defined(CLI_INTEFACE)
		for (i = 0; i < dev->present_boards_number; i++) {
			if (dev->boards[i] == CN0414) {
				ret = adc_update_activate(dev->cn0414_type_tab[board_no]->
							  adc_update_desc, false);
			}
		}
		if(ret < 0) {
			return ret;
		}
		cli_load_command_vector(dev->cli_descriptor, cn0418_v_cmd_fun);
		cli_load_command_calls(dev->cli_descriptor, cn0418_cmd_commands);
		cli_load_command_sizes(dev->cli_descriptor, cn0418_command_size);
		cli_load_descriptor_pointer(dev->cli_descriptor,
					    dev->cn0418_type_tab[board_no]);
#endif
	}
	return ret;
}

/**
 * Discover the first EEPROM present on the board.
 *
 * @param [in] dev		  - The device structure.
 * @param [in] start_addr - I2C address from where to start searching (not
 *                          bigger than 7).
 * @param [out] address   - I2C address of the memory encountered.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t system_mem_discover(struct system_manager_dev *dev, uint8_t start_addr,
			    uint8_t* address)
{
	int32_t ret = 0;
	uint8_t i;
	uint8_t backup, temp, test = 0xBB;

	if(start_addr > 7)
		return -1;

	/* Check every available memory address */
	for (i = start_addr; i < (A0_VDD | A1_VDD | A2_VDD) + 1; ++i) {
		/* Change address */
		ret = memory_change_i2c_address(dev->memory_device, i);
		if(ret < 0)
			return ret;

		/* If there is a memory there, make backup */
		memory_read(dev->memory_device, 0x0000, &backup, 1);
		/* Test write */
		memory_write(dev->memory_device, 0x0000, &test, 1);
		mdelay(5);
		/* Test read */
		memory_read(dev->memory_device, 0x0000, &temp, 1);
		if(temp == test) {
			/* Rewrite memory with backup data */
			ret = memory_write(dev->memory_device, 0x0000, &backup, 1);
			if(ret < 0)
				return ret;
			break;
		}
	}

	*address = i;

	return ret;
}
