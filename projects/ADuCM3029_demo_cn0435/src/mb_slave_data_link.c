/***************************************************************************//**
*   @file   mb_slave_data_link.c
*   @brief  MODBUS data link source code.
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
#include "mb_slave_data_link.h"
#include <stdlib.h>
#include <string.h>
#include "timer.h"

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

uint8_t commands_array[] = {
	READ_DISCRETE_OUTPUT_COIL,
	READ_DISCRETE_INPUT_CONTACTS,
	READ_OUTPUT_HOLDING_REGISTERS,
	READ_ANALOG_INPUT_REGISTERS,
	WRITE_SINGLE_DISCRETE_OUTPUT_COIL,
	WRITE_SINGLE_OUTPUT_HOLDING_REGISTER,
	WRITE_MULTI_DISCRETE_OUTPUT_COILS,
	WRITE_MULTI_OUTPUT_HOLDING_REGISTER
};

extern uint32_t uart_baud_values[];

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Get the MODBUS slave address from the hardware pins.
 *
 * modbus_setup() helper function.
 *
 * @param [in]  device     - Pointer to the device descriptor.
 * @param [in]  init_param - Pointer to the initialization structure.
 * @param [out] id         - ID of the MODBUS slave.
 *
 * @return 0 for success, negative error code otherwise.
 */
static int32_t modbus_setup_get_slave_addr(struct modbus_slave *dev,
		struct modbus_slave_init_param *init_param, uint8_t *id)
{
	int16_t i;
	struct gpio_desc *temp_gpio;
	uint16_t gpio_value;
	int32_t ret;

	*id = 0;

	for(i = 0; i < SLAVE_ID_BITS_NUMBER; i++) {
		ret = gpio_get(&temp_gpio, init_param->slave_id_gpio[i]);
		if(ret < 0)
			return ret;
		ret = gpio_direction_input(temp_gpio);
		if(ret < 0)
			return ret;
		ret = gpio_get_value(temp_gpio, &gpio_value);
		if(ret < 0)
			return ret;
		if(gpio_value != GPIO_LOW)
			*id |= (uint8_t)((uint8_t)1 << i);
		else
			continue;
		ret = gpio_remove(temp_gpio);
		if(ret < 0)
			return ret;
	}

	*id += 1;

	return ret;
}

/**
 * Initialize the data link layer descriptor for the MODBUS functionality.
 *
 * @param [out] device     - Pointer to the device structure.
 * @param [in]  init_param - Pointer to the initialization structure.
 *
 * @return 0 for success, negative error code otherwise.
 */
int32_t modbus_setup(struct modbus_slave **device,
		     struct modbus_slave_init_param *init_param)
{
	int32_t ret;
	struct modbus_slave *dev;

	/* Allocate memory for descriptor */
	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	/* Initialize descriptor */
	ret = modbus_setup_get_slave_addr(dev, init_param, &dev->slave_id);
	if(ret < 0)
		goto error;
	dev->state = MBSLAVE_IDLE;
	dev->exception = MODBUS_EXCEPTION_NO_PROBLEM;
	dev->request = NULL;
	dev->discrete_out_coils = NULL;
	dev->discrete_in_contacts = NULL;
	dev->analog_in_regs = NULL;
	dev->out_holding_regs = NULL;
	dev->out_coils_range = 0;
	dev->in_contacts_range = 0;
	dev->in_reg_range = 0;
	dev->holding_reg_range = 0;
	dev->mb_write_flag = 0;
	dev->last_function = 0;
	dev->register_address = 0;
	dev->register_number = 0;

	/* Initialize UART functionality */
	ret = usr_uart_init(&dev->phy_layer, init_param->phy_layer);
	if(ret < 0)
		goto error;

	ret = gpio_get(&dev->de_nre, init_param->de_nre_no);
	if(ret < 0)
		goto error;

	ret = gpio_direction_output(dev->de_nre, GPIO_LOW);
	if(ret < 0)
		goto error;

	/* Return device pointer */
	*device  = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free memory allocated by modbus_setup().
 *
 * @param [in] dev - Pointer to the device structure.
 *
 * @return 0 for success, negative error code otherwise.
 */
int32_t modbus_remove(struct modbus_slave *dev)
{
	int32_t ret;

	/* Free device sub-structures */
	ret = usr_uart_remove(dev->phy_layer);
	if(ret < 0)
		return ret;

	/* Free device structure */
	free(dev);

	return ret;
}

/**
 * Calculate CRC for a MODBUS message.
 *
 * @param [in]  msg        - Pointer to the message on which to calculate CRC.
 * @param [in]  msg_length - Length of the message on which to calculate CRC.
 * @param [out] hi_crc     - MSB of the calculated CRC.
 * @param [out] lo_crc     - LSB of the calculated CRC.
 *
 * @return void
 */
void modbus_crc16(uint8_t *msg, uint8_t msg_length, uint8_t *hi_crc,
		  uint8_t *lo_crc)
{
	uint8_t i;
	uint16_t crc = 0xFFFF;

	while(msg_length--) {
		crc = crc ^ *msg++;
		for(i = 0; i < 8; i++) {
			if(crc & 1)
				crc = (crc >> 1) ^ 0xA001;
			else
				crc = crc >> 1;
		}
	}

	*hi_crc = (uint8_t)((crc >> 8) & 0x00FF);
	*lo_crc = (uint8_t)(crc & 0x00FF);
}

/**
 * Check if the CRC of a MODBUS message is correct.
 *
 * @param [in] msg        - Pointer to the message.
 * @param [in] msg_length - Length of the message.
 *
 * @return 0 for success, negative error code otherwise.
 */
int32_t modbus_check_crc(uint8_t *msg, uint8_t msg_length)
{
	int32_t ret = 0;
	uint16_t crc_calculated, crc_received;
	uint8_t crc_hi, crc_lo;

	/* If the message is too short return error */
	if(msg_length < 2)
		return -1;

	/* Calculate CRC based on received message */
	modbus_crc16(msg, msg_length - 2, &crc_hi, &crc_lo);
	crc_calculated = ((uint16_t)crc_hi << 8) | (uint16_t)crc_lo;
	/* Get received CRC */
	crc_received   = ((uint16_t)msg[msg_length - 1] << 8) |
			 (uint16_t)msg[msg_length - 2];

	/* Compare the received CRC with the calculated one */
	if(crc_calculated == crc_received)
		ret = msg_length;
	else
		ret = -1;

	return ret;
}

/**
 * Build the header of a MODBUS message using the function and the slave ID.
 *
 * @param [in]  dev      - Pointer to the MODBUS slave device descriptor.
 * @param [in]  function - Function code for the message.
 * @param [out] rsp      - Pointer to the new message.
 *
 * @return 0 for success, negative error code otherwise.
 */
int32_t modbus_build_header(struct modbus_slave *dev, uint8_t function,
			    uint8_t* rsp)
{
	rsp[0] = dev->slave_id;
	rsp[1] = function;

	return MODBUS_HEADER_LENGTH;
}

/**
 * Transmit a MODBUS message using the available physical layer.
 *
 * @param [in]  dev        - Pointer to the MODBUS slave device descriptor.
 * @param [in]  msg        - Pointer to the message to be transmitted.
 * @param [out] msg_length - Message length.
 *
 * @return 0 for success, negative error code otherwise.
 */
int32_t modbus_send_msg(struct modbus_slave *dev, uint8_t *msg,
			uint8_t msg_length)
{
	int32_t ret;
	uint8_t i;
	uint8_t crc_hi, crc_lo;
	uint16_t crc;

	modbus_crc16(msg, msg_length, &crc_hi, &crc_lo);
	crc = ((uint16_t)crc_hi << 8) | ((uint16_t)crc_lo & 0x00FF);

	msg[msg_length++] = crc & 0x00FF;
	msg[msg_length++] = crc >> 8;

	ret = gpio_set_value(dev->de_nre, GPIO_HIGH);
	if(ret < 0)
		return ret;

	for(i = 0; i < msg_length; i++) {
		ret = usr_uart_write_char(dev->phy_layer, msg[i], UART_WRITE_NO_INT);
		if(ret < 0)
			break;
	}

	usr_uart_poll_tx(dev->phy_layer);

	ret = gpio_set_value(dev->de_nre, GPIO_LOW);
	if(ret < 0)
		return ret;

	dev->state = MBSLAVE_IDLE;

	return ret;
}

/**
 * Build an exception MODBUS message response.
 *
 * @param [in]  dev            - Pointer to the MODBUS slave device descriptor.
 * @param [in]  function       - MODBUS function code.
 * @param [in]  exception_code - Exception code.
 * @param [out] rsp            - Pointer to the response.
 *
 * @return 0 for success, negative error code otherwise.
 */
int32_t modbus_response_exception(struct modbus_slave *dev, uint8_t function,
				  uint8_t exception_code, uint8_t *rsp)
{
	uint8_t rsp_length;

	rsp_length = modbus_build_header(dev, function | 0x80, rsp);

	/* Positive exception code */
	rsp[rsp_length++] = exception_code;

	return rsp_length;
}

/**
 * Receive a MODBUS message.
 *
 * This is a nonblocking call. First submit a 2 byte buffer to receive the
 * message header and, when received, submit a 3 byte buffer to receive using
 * DMA while the header is processed. After the processing and receiving the
 * next 3 bytes decide how many bytes are left from the message and receive the
 * rest. After this allocate memory to contain this message and return the
 * pointer.
 *
 * @param [in]  dev      - Pointer to the MODBUS slave device descriptor.
 * @param [out] req      - Pointer to the received message.
 * @param [out] req_size - Size of the received message.
 *
 * @return 0 for success, negative error code otherwise.
 */
int32_t modbus_receive(struct modbus_slave *dev, uint8_t **req,
		       uint16_t *req_size)
{
	int32_t ret;
	uint8_t rdy;
	uint8_t buff[MODBUS_RTU_MAX_LENGTH];
	uint16_t req_index = 2;
	uint16_t i;
	uint8_t *temp_ptr;
	int16_t data_size;

	ret = usr_uart_read_nb(dev->phy_layer, (uint8_t *)buff, 1, &rdy);
	if((ret < 0) || (rdy == 0))
		return ret;

	ret = usr_uart_read_string(dev->phy_layer, (uint8_t *)(buff + 1), 1);
	if((ret < 0) || (rdy == 0))
		return ret;

	switch(buff[1]) {
	case READ_DISCRETE_OUTPUT_COIL:
	case READ_DISCRETE_INPUT_CONTACTS:
	case READ_OUTPUT_HOLDING_REGISTERS:
	case READ_ANALOG_INPUT_REGISTERS:
	case WRITE_SINGLE_DISCRETE_OUTPUT_COIL:
	case WRITE_SINGLE_OUTPUT_HOLDING_REGISTER:
		data_size = 6;
		break;
	case WRITE_MULTI_DISCRETE_OUTPUT_COILS:
	case WRITE_MULTI_OUTPUT_HOLDING_REGISTER:
		data_size = 5;
		break;
	default:
		return ret;
	}

	ret = usr_uart_read_string(dev->phy_layer, buff + req_index, data_size);
	if(ret < 0)
		return ret;
	req_index += data_size;
	if((buff[1] == WRITE_MULTI_DISCRETE_OUTPUT_COILS) ||
	    (buff[1] == WRITE_MULTI_OUTPUT_HOLDING_REGISTER)) {
		data_size = buff[(req_index - 1)] + 2;
		ret = usr_uart_read_string(dev->phy_layer, (buff + req_index),
					   data_size);
		if(ret < 0)
			return ret;
		req_index += data_size;
	}

	temp_ptr = calloc(req_index, sizeof *temp_ptr);
	if(!temp_ptr)
		return ret;
	for(i = 0; i < req_index; i++) {
		temp_ptr[i] = buff[i];
	}
	*req = temp_ptr;
	*req_size = req_index;
	dev->state = MBSLAVE_CHECK_REQ;

	return ret;
}

/**
 * Get the MODBUS function code from a telegram.
 *
 * modbus_check_request() helper function.
 *
 * @param [in]  dev        - Pointer to the MODBUS slave device descriptor.
 * @param [in]  req        - Pointer to the received message.
 * @param [out] func_index - Function index in the MODBUS function array.
 *
 * @return void
 */
static void modbus_get_function(struct modbus_slave *dev, uint8_t *req,
				uint8_t *func_index)
{
	uint8_t function_number = sizeof(commands_array) /
				  sizeof(typeof(*commands_array));
	uint8_t i;

	for(i = 0; i < function_number; i++) {
		if(req[MODBUS_FUNCTION_PLACE] == commands_array[i])
			break;
	}
	*func_index = i;
}

/**
 * Check if a MODBUS telegram contains a valid function code.
 *
 * modbus_check_request() helper function.
 *
 * @param [in] dev      - Pointer to the MODBUS slave device descriptor.
 * @param [in] req      - Pointer to the received message.
 * @param [in] req_size - Function index in the MODBUS function array.
 *
 * @return void
 */
static void modbus_check_illegal_func(struct modbus_slave *dev, uint8_t *req,
				      uint16_t req_size)
{
	uint8_t i;
	uint8_t function_number = sizeof(commands_array) /
				  sizeof(typeof(*commands_array));

	modbus_get_function(dev, req, &i);
	if(i == function_number) {
		dev->exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
		dev->state = MBSLAVE_REPLY;
		return;
	}

	switch(commands_array[i]) {
	case READ_OUTPUT_HOLDING_REGISTERS:
	case WRITE_SINGLE_OUTPUT_HOLDING_REGISTER:
	case WRITE_MULTI_OUTPUT_HOLDING_REGISTER:
		if(dev->out_holding_regs == NULL) {
			dev->exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
			dev->state = MBSLAVE_REPLY;
		}
		break;
	case READ_DISCRETE_OUTPUT_COIL:
	case WRITE_SINGLE_DISCRETE_OUTPUT_COIL:
	case WRITE_MULTI_DISCRETE_OUTPUT_COILS:
		if(dev->discrete_out_coils == NULL) {
			dev->exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
			dev->state = MBSLAVE_REPLY;
		}
		break;
	case READ_ANALOG_INPUT_REGISTERS:
		if(dev->analog_in_regs == NULL) {
			dev->exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
			dev->state = MBSLAVE_REPLY;
		}
		break;
	case READ_DISCRETE_INPUT_CONTACTS:
		if(dev->discrete_in_contacts == NULL) {
			dev->exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
			dev->state = MBSLAVE_REPLY;
		}
		break;
	default:
		break;
	}
}

/**
 * Parse the MODBUS telegram and return the addresses of the first and last
 * registers affected the telegram.
 *
 * modbus_check_request() helper function.
 *
 * @param [in]  dev         - Pointer to the MODBUS slave device descriptor.
 * @param [in]  req         - Pointer to the received message.
 * @param [out] start_range - Address of the first register.
 * @param [out] max_range   - Address of the last register.
 *
 * @return void
 */
static void modbus_get_data_range(struct modbus_slave *dev, uint8_t *req,
				  uint16_t *start_range,uint16_t *max_range)
{
	uint8_t i;

	modbus_get_function(dev, req, &i);

	switch(commands_array[i]) {
	case READ_OUTPUT_HOLDING_REGISTERS:
	case WRITE_MULTI_OUTPUT_HOLDING_REGISTER:
	case READ_DISCRETE_OUTPUT_COIL:
	case WRITE_MULTI_DISCRETE_OUTPUT_COILS:
	case READ_ANALOG_INPUT_REGISTERS:
	case READ_DISCRETE_INPUT_CONTACTS:
		*start_range = ((req[MODBUS_REGISTER_ADDR_HI] << 8) |
				req[MODBUS_REGISTER_ADDR_LO]);
		*max_range = *start_range + ((req[MODBUS_REGISTER_OFF_HI] << 8) |
					     req[MODBUS_REGISTER_OFF_LO]) - 1;
		break;
	case WRITE_SINGLE_OUTPUT_HOLDING_REGISTER:
	case WRITE_SINGLE_DISCRETE_OUTPUT_COIL:
		*start_range = ((req[MODBUS_REGISTER_ADDR_HI] << 8) |
				req[MODBUS_REGISTER_ADDR_LO]);
		*max_range = *start_range;
		break;
	default:
		break;
	}
}

/**
 * Check if the first and last registers in a telegram exist in the device
 * register map.
 *
 * modbus_check_request() helper function.
 *
 * @param [in] dev - Pointer to the MODBUS slave device descriptor.
 * @param [in] req - Pointer to the received message.
 *
 * @return void
 */
static void modbus_check_data_range(struct modbus_slave *dev, uint8_t *req)
{
	uint16_t max_range, start_range;
	struct mb_slave_register *strt_reg, *max_reg;
	uint8_t i;

	modbus_get_function(dev, req, &i);

	modbus_get_data_range(dev, req, &start_range, &max_range);

	switch(commands_array[i]) {
	case READ_DISCRETE_OUTPUT_COIL:
	case WRITE_SINGLE_DISCRETE_OUTPUT_COIL:
	case WRITE_MULTI_DISCRETE_OUTPUT_COILS:
		strt_reg = modbus_get_reg(dev, start_range, DISCRETE_OUTPUT_COILS);
		max_reg = modbus_get_reg(dev, max_range, DISCRETE_OUTPUT_COILS);
		if(strt_reg == NULL || max_reg == NULL) {
			dev->exception = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
			dev->state = MBSLAVE_REPLY;
		}
		break;
	case READ_DISCRETE_INPUT_CONTACTS:
		strt_reg = modbus_get_reg(dev, start_range, DISCRETE_INPUT_CONTACTS);
		max_reg = modbus_get_reg(dev, max_range, DISCRETE_INPUT_CONTACTS);
		if(strt_reg == NULL || max_reg == NULL) {
			dev->exception = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
			dev->state = MBSLAVE_REPLY;
		}
		break;
	case READ_ANALOG_INPUT_REGISTERS:
		strt_reg = modbus_get_reg(dev, start_range, ANALOG_INPUT_REGISTERS);
		max_reg = modbus_get_reg(dev, max_range, ANALOG_INPUT_REGISTERS);
		if(strt_reg == NULL || max_reg == NULL) {
			dev->exception = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
			dev->state = MBSLAVE_REPLY;
		}
		break;
	case READ_OUTPUT_HOLDING_REGISTERS:
	case WRITE_SINGLE_OUTPUT_HOLDING_REGISTER:
	case WRITE_MULTI_OUTPUT_HOLDING_REGISTER:
		strt_reg = modbus_get_reg(dev, start_range, OUTPUT_HOLDING_REGISTERS);
		max_reg = modbus_get_reg(dev, max_range, OUTPUT_HOLDING_REGISTERS);
		if(strt_reg == NULL || max_reg == NULL) {
			dev->exception = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
			dev->state = MBSLAVE_REPLY;
		}
		break;
	default:
		break;
	}
}

/**
 * Check a request.
 *
 * Check the slave it is addressed to, if the function code is correct and
 * supported, if the registers are within the register map and if the CRC is
 * correct.
 *
 * @param [in] dev      - Pointer to the MODBUS slave device descriptor.
 * @param [in] req      - Pointer to the received message.
 * @param [in] req_size - Size of the received message.
 *
 * @return void
 */
void modbus_check_request(struct modbus_slave *dev, uint8_t *req,
			  uint16_t req_size)
{
	if(req[MODBUS_ADDRESS_PLACE] != dev->slave_id) {
		dev->state = MBSLAVE_IDLE;
		return;
	}

	if(modbus_check_crc(req, req_size) == -1) {
		dev->state = MBSLAVE_IDLE;
		return;
	}

	modbus_check_illegal_func(dev, req, req_size);
	if(dev->exception != MODBUS_EXCEPTION_NO_PROBLEM)
		return;

	modbus_check_data_range(dev, req);
	if(dev->exception != MODBUS_EXCEPTION_NO_PROBLEM)
		return;

	dev->exception = MODBUS_EXCEPTION_NO_PROBLEM;
	dev->state = MBSLAVE_PROCESS;
}

/**
 * Update a MODBUS register after receiving the proper command.
 *
 * modbus_write() helper function.
 *
 * @param [in] dev           - Pointer to the MODBUS slave device descriptor.
 * @param [in] request       - Pointer to the received message.
 * @param [in] value_pos     - Position of the update value in the telegram.
 * @param [in] vector_ptr    - Pointer to the required register vector.
 * @param [in] vector_size   - Size of the respective register vector.
 * @param [in] start_address - Address of the desired register.
 *
 * @return void
 */
static void modbus_write_one(struct modbus_slave *dev, uint8_t *request,
			     uint8_t value_pos, struct mb_slave_register *vector_ptr,
			     uint16_t vector_size, uint16_t start_address)
{
	uint16_t i;
	uint16_t new_val;

	new_val = (request[value_pos] << 8) |
		  request[value_pos + 1];

	for(i = 0; i < vector_size; i++) {
		if(start_address == vector_ptr[i].address) {
			vector_ptr[i].data = new_val;
			break;
		}
	}
}

/**
 * Execute a register write on a master request and build a response.
 *
 * modbubs_execute_command() helper function.
 *
 * @param [in]  dev      - Pointer to the MODBUS slave device descriptor.
 * @param [in]  request  - Pointer to the received message.
 * @param [out] response - Pointer to the response.
 *
 * @return void
 */
static void modbus_write(struct modbus_slave *dev, uint8_t *request,
			 uint8_t *response)
{
	uint8_t func_index;
	uint16_t i;
	uint16_t start_address;
	uint16_t write_number;

	modbus_get_function(dev, request, &func_index);

	start_address = (request[MODBUS_REGISTER_ADDR_HI] << 8) |
			request[MODBUS_REGISTER_ADDR_LO];

	dev->register_address = start_address;

	switch(commands_array[func_index]) {
	case WRITE_SINGLE_DISCRETE_OUTPUT_COIL:
		modbus_write_one(dev, request, MODBUS_REGISTER_ADDR_LO + 1,
				 dev->discrete_out_coils, dev->out_coils_range,
				 start_address);
		break;
	case WRITE_SINGLE_OUTPUT_HOLDING_REGISTER:
		dev->register_number = 1;
		modbus_write_one(dev, request, MODBUS_REGISTER_ADDR_LO + 1,
				 dev->out_holding_regs, dev->holding_reg_range,
				 start_address);
		break;
	case WRITE_MULTI_DISCRETE_OUTPUT_COILS:
		write_number = request[MODBUS_REGISTER_OFF_LO + 1] / 2;
		for(i = 1; i <= write_number; i = i + 2) {
			modbus_write_one(dev, request, MODBUS_REGISTER_OFF_LO + 1 + i,
					 dev->out_holding_regs, dev->holding_reg_range,
					 start_address);
		}
		break;
	case WRITE_MULTI_OUTPUT_HOLDING_REGISTER:
		write_number = request[MODBUS_WRITE_BYTES_NO];
		dev->register_number = write_number / 2;
		for(i = 0; i < write_number; i = i + 2) {
			modbus_write_one(dev, request, MODBUS_WRITE_BYTES_NO + 1 + i,
					 dev->out_holding_regs, dev->holding_reg_range,
					 start_address++);
		}
		break;
	default:
		break;
	}
	dev->mb_write_flag = 1;
	response[MODBUS_REGISTER_ADDR_HI] = request[MODBUS_REGISTER_ADDR_HI];
	response[MODBUS_REGISTER_ADDR_LO] = request[MODBUS_REGISTER_ADDR_LO];
	response[MODBUS_REGISTER_OFF_HI] = request[MODBUS_REGISTER_OFF_HI];
	response[MODBUS_REGISTER_OFF_LO] = request[MODBUS_REGISTER_OFF_LO];
}

/**
 * Get data from MODBUS registers.
 *
 * modbus_read() helper function.
 *
 * @param [in]  request       - Pointer to the received message.
 * @param [in]  end_address   - Address of the last register.
 * @param [in]  vector_ptr    - Pointer to the required register vector.
 * @param [in]  vector_size   - Size of the respective register vector.
 * @param [in]  start_address - Address of the first register.
 * @param [out] data_array    - Pointer to the returned data.
 *
 * @return void
 */
static void modbus_read_one(uint8_t *data_array, uint16_t start_address,
			    uint16_t end_address, struct mb_slave_register *vector_ptr,
			    uint16_t vector_size)
{
	uint16_t i;
	uint8_t resp_index = MODBUS_READ_RESPONSE_ADDRESS;

	for(i = 0; i < vector_size; i++) {
		if(vector_ptr[i].address == start_address) {
			data_array[resp_index++] = (vector_ptr[i].data & 0xFF00) >> 8;
			data_array[resp_index++] = vector_ptr[i].data & 0x00FF;
			if(start_address == end_address) {
				break;
			}
			start_address++;
		}
	}
}

/**
 * Execute a register read on a master request and build a response.
 *
 * modbubs_execute_command() helper function.
 *
 * @param [in]  dev      - Pointer to the MODBUS slave device descriptor.
 * @param [in]  request  - Pointer to the received message.
 * @param [out] response - Pointer to the response.
 *
 * @return void
 */
static void modbus_read(struct modbus_slave *dev, uint8_t *request,
			uint8_t *response)
{
	uint8_t func_index;
	uint16_t start_address;
	uint16_t end_address;

	modbus_get_function(dev, request, &func_index);

	start_address = (request[MODBUS_REGISTER_ADDR_HI] << 8) |
			request[MODBUS_REGISTER_ADDR_LO];

	end_address = start_address + ((request[MODBUS_REGISTER_OFF_HI] << 8) |
				       request[MODBUS_REGISTER_OFF_LO]) - 1;

	response[MODBUS_READ_RESPONSE_BYTES_NO] =
		(end_address - start_address + 1) * 2;

	switch(commands_array[func_index]) {
	case READ_DISCRETE_OUTPUT_COIL:
		break;
	case READ_DISCRETE_INPUT_CONTACTS:
		break;
	case READ_OUTPUT_HOLDING_REGISTERS:
		modbus_read_one(response, start_address, end_address,
				dev->out_holding_regs, dev->holding_reg_range);
		break;
	case READ_ANALOG_INPUT_REGISTERS:
		modbus_read_one(response, start_address, end_address,
				dev->analog_in_regs, dev->in_reg_range);
		break;
	default:
		break;
	}
}

/**
 * Parse a master request, execute the command and build a response.
 *
 * @param [in]  dev      - Pointer to the MODBUS slave device descriptor.
 * @param [in]  request  - Pointer to the received message.
 * @param [out] response - Pointer to the response.
 *
 * @return void
 */
void modbus_execute_command(struct modbus_slave *dev, uint8_t *request,
			    uint8_t *response)
{
	uint8_t func_index;

	modbus_get_function(dev, request, &func_index);

	response[MODBUS_ADDRESS_PLACE] = dev->slave_id;
	response[MODBUS_FUNCTION_PLACE] = commands_array[func_index];

	if(commands_array[func_index] <= READ_ANALOG_INPUT_REGISTERS)
		modbus_read(dev, request, response);
	else
		modbus_write(dev, request, response);
}

/**
 * Check a request, execute the command and send the response without receiving
 * the telegram.
 *
 * This is done to separate the receiving of the command from the rest of the
 * MODBUS process and make the command available outside the driver.
 *
 * @param [in] dev - Pointer to the MODBUS slave device descriptor.
 *
 * @return void
 */
int32_t modbus_process_open(struct modbus_slave *dev)
{
	uint8_t *response;
	uint16_t resp_size = 0;
	uint8_t func_index;

	modbus_check_request(dev, dev->request, dev->request_size);
	if(dev->state == MBSLAVE_IDLE) {
		free(dev->request);
		return MODBUS_IGNORE;
	}
	if(dev->exception != MODBUS_EXCEPTION_NO_PROBLEM) {
		response = calloc(MODBUS_EXCEPTION_RESPONSE_SIZE, sizeof(uint8_t));
		resp_size = modbus_response_exception(dev,
						      dev->request[MODBUS_FUNCTION_PLACE], dev->exception, response);
		modbus_send_msg(dev, response, resp_size);
		dev->exception = MODBUS_EXCEPTION_NO_PROBLEM;

		free(response);
		free(dev->request);

		return 0;
	}

	modbus_get_function(dev, dev->request, &func_index);

	switch(commands_array[func_index]) {
	case READ_OUTPUT_HOLDING_REGISTERS:
	case READ_ANALOG_INPUT_REGISTERS:
		resp_size = ((dev->request[MODBUS_REGISTER_OFF_HI] << 8) |
			     dev->request[MODBUS_REGISTER_OFF_LO]) * 2;
		resp_size += 5;
		break;
	case WRITE_SINGLE_OUTPUT_HOLDING_REGISTER:
	case WRITE_MULTI_OUTPUT_HOLDING_REGISTER:
		resp_size = 8;
		break;
	default:
		break;
	}
	dev->last_function = func_index;
	response = calloc(resp_size, sizeof(uint8_t));

	modbus_execute_command(dev, dev->request, response);

	dev->last_function = commands_array[func_index];

	modbus_send_msg(dev, response, resp_size - 2);

	free(response);
	free(dev->request);

	return 0;
}

/**
 * Complete MODBUS slave process.
 *
 * Receive a command, check it, execute it and sent the response.
 *
 * @param [in]  dev             - Pointer to the MODBUS slave device descriptor.
 * @param [out] function_update -
 *
 * @return 0 for success, negative error code otherwise.
 */
int32_t modbus_process(struct modbus_slave *dev, uint8_t *function_update)
{
	uint8_t *response;
	uint16_t resp_size = 0;
	int32_t ret;
	uint8_t func_index;

	ret = modbus_receive(dev, &dev->request, &dev->request_size);
	if((dev->state == MBSLAVE_IDLE) || (ret != 0))
		return ret;

	modbus_check_request(dev, dev->request, dev->request_size);
	if(dev->state == MBSLAVE_IDLE)
		return ret;
	if(dev->exception != MODBUS_EXCEPTION_NO_PROBLEM) {
		response = calloc(MODBUS_EXCEPTION_RESPONSE_SIZE, sizeof *response);
		resp_size = modbus_response_exception(dev,
						      dev->request[MODBUS_FUNCTION_PLACE], dev->exception, response);
		modbus_send_msg(dev, response, resp_size);
		dev->exception = MODBUS_EXCEPTION_NO_PROBLEM;
		goto finish;
	}

	modbus_get_function(dev, dev->request, &func_index);

	*function_update = 0;

	switch(commands_array[func_index]) {
	case READ_OUTPUT_HOLDING_REGISTERS:
	case READ_ANALOG_INPUT_REGISTERS:
		resp_size = ((dev->request[MODBUS_REGISTER_OFF_HI] << 8) |
			     dev->request[MODBUS_REGISTER_OFF_LO]) * 2;
		resp_size += 5;
		break;
	case WRITE_SINGLE_OUTPUT_HOLDING_REGISTER:
	case WRITE_MULTI_OUTPUT_HOLDING_REGISTER:
		resp_size = 8;
		*function_update = 1;
		break;
	default:
		break;
	}
	response = calloc(resp_size, sizeof(uint8_t));

	modbus_execute_command(dev, dev->request, response);

	modbus_send_msg(dev, response, resp_size - 2);
finish:
	free(response);

	return ret;
}

/**
 * Return a MODBUS register based on the type and address.
 *
 * @param [in] dev      - Pointer to the MODBUS slave device descriptor.
 * @param [in] reg_addr - Address of the register.
 * @param [in] reg_type - Type of the register.
 *
 * @return pointer to the register of the address and type wanted, or NULL
 *         pointer in case the register does not exist.
 */
struct mb_slave_register *modbus_get_reg(struct modbus_slave *dev,
		uint16_t reg_addr, enum mb_reg_type reg_type)
{
	uint16_t i;

	switch(reg_type) {
	case DISCRETE_OUTPUT_COILS:
		for(i = 0; i < dev->out_coils_range; i++) {
			if(dev->discrete_out_coils[i].address == reg_addr) {
				return (dev->discrete_out_coils + i);
			}
		}
		break;
	case DISCRETE_INPUT_CONTACTS:
		for(i = 0; i < dev->in_contacts_range; i++) {
			if(dev->discrete_in_contacts[i].address == reg_addr) {
				return (dev->discrete_in_contacts + i);
			}
		}
		break;
	case ANALOG_INPUT_REGISTERS:
		for(i = 0; i < dev->in_reg_range; i++) {
			if(dev->analog_in_regs[i].address == reg_addr) {
				return (dev->analog_in_regs + i);
			}
		}
		break;
	case OUTPUT_HOLDING_REGISTERS:
		for(i = 0; i < dev->holding_reg_range; i++) {
			if(dev->out_holding_regs[i].address == reg_addr) {
				return (dev->out_holding_regs + i);
			}
		}
		break;
	default:
		break;
	}

	return NULL;
}
