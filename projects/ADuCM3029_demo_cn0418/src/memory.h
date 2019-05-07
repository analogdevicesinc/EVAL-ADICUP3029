/***************************************************************************//**
*   @file   memory.h
*   @brief  EEPROM memory 24LC32A driver header.
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

#ifndef MEMORY_H_
#define MEMORY_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "platform_drivers.h"

#define WRITE_SIZE_LIMIT 32
#define WRITE_SIZE_ERROR -2
#define MEMORY_SIZE_BYTES 4096
#define MEMORY_PROTECTED_SIZE 2048

#define A0_VDD 0x1
#define A1_VDD 0x2
#define A2_VDD 0x4
#define A0_MASK(x) (((x) & 0x01) >> 0)
#define A1_MASK(x) (((x) & 0x02) >> 1)
#define A2_MASK(x) (((x) & 0x04) >> 2)

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
struct memory_desc {
	struct i2c_desc *i2c_dev;
	uint8_t i2c_address; /* Address of the slave */
};

struct memory_init_param {
	struct i2c_init_param i2c_param;
	uint8_t i2c_address;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initializes the 24LC32A device. */
int32_t memory_setup(struct memory_desc **device,
		     struct memory_init_param *init_param);

/* Free the resources allocated by memory_setup(). */
int32_t memory_remove(struct memory_desc *dev);

/* Writes a number of bytes to memory provided the data does not exceed page
 * boundaries. */
int32_t memory_write_within_page(struct memory_desc *dev, uint16_t address,
				 uint8_t *data, uint32_t size);

/* Writes any number of bytes in EEPROM memory. */
int32_t memory_write(struct memory_desc *dev, uint16_t address, uint8_t *data,
		     uint32_t size);

/* Read any number of bytes from EEPROM memory. */
int32_t memory_read(struct memory_desc *dev, uint16_t address, uint8_t *data,
		    uint32_t size);

/* Change the I2C address used in comunication with the memory. */
int32_t memory_change_i2c_address(struct memory_desc *dev, uint8_t address);

#endif /* MEMORY_H_ */
