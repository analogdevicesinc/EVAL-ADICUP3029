/***************************************************************************//**
*   @file   mb_slave_data_link.h
*   @brief  MODBUS data link header file.
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

#ifndef MB_SLAVE_DATA_LINK_H_
#define MB_SLAVE_DATA_LINK_H_

#include <stdint.h>
#include "platform_drivers.h"

#define MODBUS_BROADCAST_ADDRESS 0
#define MODBUS_SLAVE_ADDRESS  	 0
#define MODBUS_ADDRESS_PLACE	 0
#define MODBUS_FUNCTION_PLACE 	 1
#define MODBUS_HEADER_LENGTH  	 2
#define MODBUS_CHECKSUM_LENGTH   2
#define MODBUS_RTU_MAX_LENGTH    100
#define MODBUS_VALID_COMMANDS	 8
#define MODBUS_REGISTER_ADDR_HI  2
#define MODBUS_REGISTER_ADDR_LO  3
#define MODBUS_REGISTER_OFF_HI   4
#define MODBUS_REGISTER_OFF_LO   5
#define MODBUS_WRITE_BYTES_NO	 6

#define MODBUS_READ_RESPONSE_BYTES_NO MODBUS_REGISTER_ADDR_HI
#define MODBUS_READ_RESPONSE_ADDRESS  MODBUS_REGISTER_ADDR_LO

#define SLAVE_ID_BITS_NUMBER 4

#define MODBUS_EXCEPTION_NOT_FOR_ME    -2
#define MODBUS_EXCEPTION_FRAMING_ERROR -3
#define MODBUS_EXCEPTION_RESPONSE_SIZE  5

#define MODBUS_IGNORE -5

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
struct mb_slave_register {
	uint16_t address;
	uint16_t data;
};

enum mb_receive_steps {
	_STEP_FUNCTION = 0x01,
	_STEP_META,
	_STEP_DATA
};

enum mb_slave_state {
	MBSLAVE_IDLE,
	MBSLAVE_CHECK_REQ,
	MBSLAVE_PROCESS,
	MBSLAVE_REPLY
};

enum mb_exception_codes {
	MODBUS_EXCEPTION_NO_PROBLEM,
	MODBUS_EXCEPTION_ILLEGAL_FUNCTION,
	MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,
	MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,
	MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE,
	MODBUS_EXCEPTION_ACKNOWLEDGE,
	MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY,
	MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,
	MODBUS_EXCEPTION_MEMORY_PARITY_ERROR,
	MODBUS_EXCEPTION_GATEWAY_PATH_UNAVAILABLE = 10,
	MODBUS_EXCEPTION_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND
};

enum mb_commands {
	READ_DISCRETE_OUTPUT_COIL = 1,
	READ_DISCRETE_INPUT_CONTACTS,
	READ_OUTPUT_HOLDING_REGISTERS,
	READ_ANALOG_INPUT_REGISTERS,
	WRITE_SINGLE_DISCRETE_OUTPUT_COIL,
	WRITE_SINGLE_OUTPUT_HOLDING_REGISTER,
	WRITE_MULTI_DISCRETE_OUTPUT_COILS = 15,
	WRITE_MULTI_OUTPUT_HOLDING_REGISTER
};

enum mb_reg_type {
	DISCRETE_OUTPUT_COILS,
	DISCRETE_INPUT_CONTACTS,
	ANALOG_INPUT_REGISTERS,
	OUTPUT_HOLDING_REGISTERS
};

struct modbus_slave_init_param {
	struct uart_init_param phy_layer;
	uint8_t de_nre_no;
	uint8_t slave_id_gpio[SLAVE_ID_BITS_NUMBER];
};

struct modbus_slave {
	struct uart_desc *phy_layer;
	struct gpio_desc *de_nre;
	uint8_t slave_id;
	enum mb_slave_state state;
	enum mb_exception_codes exception;
	uint8_t last_function;
	uint16_t register_address;
	uint16_t register_number;
	uint8_t mb_write_flag;
	uint8_t *request;
	uint16_t request_size;
	struct mb_slave_register *discrete_out_coils;
	struct mb_slave_register *discrete_in_contacts;
	struct mb_slave_register *analog_in_regs;
	struct mb_slave_register *out_holding_regs;
	uint16_t out_coils_range;
	uint16_t in_contacts_range;
	uint16_t in_reg_range;
	uint16_t holding_reg_range;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/
/* Initialize the data link layer descriptor for the MODBUS functionality. */
int32_t modbus_setup(struct modbus_slave **device,
		     struct modbus_slave_init_param *init_param);

/* Free memory allocated by modbus_setup(). */
int32_t modbus_remove(struct modbus_slave *dev);

/* Calculate CRC for a MODBUS message. */
void modbus_crc16(uint8_t *msg, uint8_t msg_length, uint8_t *hi_crc,
		  uint8_t *lo_crc);

/* Check if the CRC of a MODBUS message is correct. */
int32_t modbus_check_crc(uint8_t *msg, uint8_t msg_length);

/* Build the header of a MODBUS message using the function and the slave ID. */
int32_t modbus_build_header(struct modbus_slave *dev, uint8_t function,
			    uint8_t* rsp);

/* Transmit a MODBUS message using the available physical layer. */
int32_t modbus_send_msg(struct modbus_slave *dev, uint8_t *msg,
			uint8_t msg_length);

/* Build an exception MODBUS message response. */
int32_t modbus_response_exception(struct modbus_slave *dev, uint8_t function,
				  uint8_t exception_code, uint8_t *rsp);

/* Receive a MODBUS message. */
int32_t modbus_receive(struct modbus_slave *dev, uint8_t **req,
		       uint16_t *req_size);

/* Check a request. */
void modbus_check_request(struct modbus_slave *dev, uint8_t *req,
			  uint16_t req_size);

/* Parse a master request, execute the command and build a response. */
void modbus_execute_command(struct modbus_slave *dev, uint8_t *request,
			    uint8_t *response);

/* Check a request, execute the command and send the response without receiving
 * the telegram. */
int32_t modbus_process_open(struct modbus_slave *dev);

/* Complete MODBUS slave process. */
int32_t modbus_process(struct modbus_slave *dev, uint8_t *function_update);

/* Return a MODBUS register based on the type and address. */
struct mb_slave_register *modbus_get_reg(struct modbus_slave *dev,
		uint16_t reg_addr, enum mb_reg_type reg_type);

#endif /* MB_SLAVE_DATA_LINK_H_ */
