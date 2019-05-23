/***************************************************************************//**
*   @file   system_manager.h
*   @brief  System manager header file.
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

#ifndef SYSTEM_MANAGER_H_
#define SYSTEM_MANAGER_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "platform_drivers.h"
#include "cli.h"
#include "cn0414.h"
#include "cn0418.h"
#if defined(MODBUS_INTERFACE)
#include  "mb_slave_data_link.h"
#endif

#define BOARD_SLOT_NUMBER 4
#define CN0414_MODBUS_INPUT_REG_NUMBER 50
#define CN0414_MODBUS_HOLDING_REG_NUMBER 7
#define CN0418_MODBUS_INPUT_REG_NUMBER 30
#define CN0418_MODBUS_HOLDING_REG_NUMBER 10
#define MODBUS_GLOBAL_UPDATE_RATE_REG_ADDR 0xFE

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

enum cn0414_function_address_ranges {
	CN0414_ADC_OUTPUT_CODING,
	CN0414_ADC_FILTER,
	CN0414_ADC_POSTFILTER,
	CN0414_ADC_ODR,
	CN0414_ADC_OWD,
	CN0414_HART_CMD_ZERO,
	CN0414_HART_CHAN_SELECT
};

enum cn0418_function_address_ranges {
	CN0418_DAC_RANGE_CH1,
	CN0418_DAC_RANGE_CH2,
	CN0418_DAC_RANGE_CH3,
	CN0418_DAC_RANGE_CH4,
	CN0418_DAC_OUTPUT_CH1,
	CN0418_DAC_OUTPUT_CH2,
	CN0418_DAC_OUTPUT_CH3,
	CN0418_DAC_OUTPUT_CH4,
	CN0418_HART_CMD_ZERO,
	CN0418_HART_CHAN_SELECT
};

enum active_board_type {
	CN0414,
	CN0418,
	NONE = 0xFFFF,
	ACTIVE_BOARD_TYPE_SIZE = 3
};

struct system_manager_init {
	struct cn0414_ini_param cn0414_type_init;
	struct cn0418_init_param cn0418_type_init;

	struct ad5700_init_param ad5700_init;
	struct memory_init_param memory_init;
#if defined(CLI_INTEFACE)
	struct cli_init_param cli_init;
#elif defined(MODBUS_INTERFACE)
	struct modbus_slave_init_param mb_slave_init;
#endif

	uint8_t gpio_hart_deco_a0;
	uint8_t gpio_hart_deco_a1;
	uint8_t gpio_cs_deco_a0;
	uint8_t gpio_cs_deco_a1;
	uint8_t gpio_spi_cs; /* Temporary GPIO to test for boards */
};

struct system_manager_dev {
	struct cn0414_dev **cn0414_type_tab;
	struct cn0418_dev **cn0418_type_tab;

	struct ad5700_dev 	   *ad5700_device;
	struct memory_desc 	   *memory_device;
#if defined(CLI_INTEFACE)
	struct cli_desc 	   *cli_descriptor;
#elif defined(MODBUS_INTERFACE)
	struct modbus_slave	   *mb_slave_desc;
	struct adc_update_desc *adc_update_timer;
#endif

	uint8_t active_device;
	enum active_board_type boards[BOARD_SLOT_NUMBER];
	uint8_t board_i2c_mem_address[BOARD_SLOT_NUMBER];
	uint8_t present_boards_number;
	uint8_t board_cs_deco_select[BOARD_SLOT_NUMBER];

	struct gpio_desc *gpio_hart_deco_a0;
	struct gpio_desc *gpio_hart_deco_a1;
	struct gpio_desc *gpio_cs_deco_a0;
	struct gpio_desc *gpio_cs_deco_a1;
	struct gpio_desc *gpio_spi_cs; /* Temporary GPIO to test for boards */
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initialize the PLC or DCS system. */
int32_t system_setup(struct system_manager_dev **device,
		     struct system_manager_init *init_param);

/* Free resources allocated by the system_setup(). */
int32_t system_remove(struct system_manager_dev *dev);

/* Display the command menu. */
int32_t system_help(struct system_manager_dev *dev, uint8_t *arg);

/* Change the active board. */
int32_t system_board_set(struct system_manager_dev *dev, uint8_t *arg);

/* The application process method. */
void system_process(struct system_manager_dev *dev);

/* Change the active board in the system. */
int32_t system_change_active_board(struct system_manager_dev *dev,
				   uint8_t board_no);

/* Discover the first EEPROM present on the board. */
int32_t system_mem_discover(struct system_manager_dev *dev, uint8_t start_addr,
			    uint8_t* address);

#endif /* SYSTEM_MANAGER_H_ */
