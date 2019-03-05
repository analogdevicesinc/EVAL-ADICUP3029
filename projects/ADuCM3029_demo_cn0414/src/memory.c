/***************************************************************************//**
*   @file   memory.c
*   @brief  EEPROM memory 24LC32A driver source.
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
#include "memory.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Initializes the 24LC32A device.
 *
 * @param [out] device    - The device structure.
 * @param [in] init_param - Pointer to the structure that contains the device
 *                          initial parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t memory_setup(struct memory_desc **device,
		     struct memory_init_param *init_param)
{
	int32_t ret;
	struct memory_desc *dev;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	ret = i2c_init(&dev->i2c_dev, &init_param->i2c_param);
	if(ret != MEMORY_SUCCESS)
		goto error;

	dev->i2c_address = 0x50 | init_param->i2c_address;
	ret = i2c_set_address(dev->i2c_dev, dev->i2c_address);
	if(ret != MEMORY_SUCCESS)
		goto error;

	*device = dev;

	return ret;

error:
	free(dev);

	return ret;
}

/**
 * Free the resources allocated by memory_setup().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t memory_remove(struct memory_desc *dev)
{
	int32_t ret;

	if(!dev)
		return -1;

	ret = i2c_remove(dev->i2c_dev);
	if(ret != MEMORY_SUCCESS)
		return ret;

	free(dev);

	return ret;
}

/**
 * Writes a number of bytes to memory provided the data does not exceed page
 * boundaries. At the start of the buffer is placed the address at which the
 * write will take place. The write size cannot exceed remaining page size.
 * Maximum page size is 32 bytes. If the size of the write would exceed the
 * remaining size of the page, the function exits with negative error code.
 *
 * @param [in] dev     - The device structure.
 * @param [in] address - Address in memory where the first byte is written.
 * @param [in] data    - Pointer to data to be written.
 * @param [in] size    - Size of the data array.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t memory_write_within_page(struct memory_desc *dev, uint16_t address,
				 uint8_t *data, uint32_t size)
{
	int32_t ret;
	uint8_t *transmit_array, i;
	uint32_t page_remainder;

	/* If the write would exceed page bounds return to avoid corrupting
	 * memory */
	page_remainder = address % WRITE_SIZE_LIMIT;
	page_remainder = WRITE_SIZE_LIMIT - page_remainder;
	if(page_remainder < size)
		return WRITE_SIZE_ERROR;

	transmit_array = calloc(size + 2, sizeof(uint8_t));
	if (!transmit_array)
		return -1;

	transmit_array[0] = (address & 0xff00) >> 8;
	transmit_array[1] = (address & 0x00ff) >> 0;

	for(i = 0; i < size; i++)
		transmit_array[i + 2] = data[i];

	/* Do not test if ret is equal to zero faster than the end of the function
	 * because the "catch" branch would have been the same as the success one */
	ret = i2c_write(dev->i2c_dev, transmit_array, size + 2, true);

	free(transmit_array);

	mdelay(5);

	return ret;
}

/**
 * Writes any number of bytes in EEPROM memory. This function utilizes the
 * memory_write_within_page function and instead of exiting if the write size is
 * too big it splits the write in multiple writes so that it is correctly
 * written in the memory.
 *
 * @param [in] dev     - The device structure.
 * @param [in] address - Address in memory where the first byte is written.
 * @param [in] data    - Pointer to data to be written.
 * @param [in] size    - Size of the data array.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t memory_write(struct memory_desc *dev, uint16_t address, uint8_t *data,
		     uint32_t size)
{
	int32_t ret;
	uint32_t page_remainder;
	uint8_t done = 0;
	uint16_t shifted_address;

	/* The shifter address is address after the protected memory in which the
	 * user data can be written */
	shifted_address = MEMORY_PROTECTED_SIZE + address;
	if((shifted_address + size) > MEMORY_SIZE_BYTES)
		return WRITE_SIZE_ERROR;

	do {
		page_remainder = shifted_address % WRITE_SIZE_LIMIT;
		page_remainder = WRITE_SIZE_LIMIT - page_remainder;

		if(page_remainder > size) {
			ret = memory_write_within_page(dev, shifted_address, data, size);
			if(ret != MEMORY_SUCCESS)
				return ret;
			done = 1;
		} else {
			ret = memory_write_within_page(dev, shifted_address, data,
						       page_remainder);
			if(ret != MEMORY_SUCCESS)
				return ret;
			data += page_remainder;
			shifted_address += page_remainder;
			size -= page_remainder;
		}
	} while(done == 0);

	return ret;
}

/**
 * Read any number of bytes from EEPROM memory. The read can be any number of
 * bytes. Before the read is done the memory index must be placed at the desired
 * location using a write with a repead start then a read.
 *
 * @param [in] dev     - The device structure.
 * @param [in] address - Address in memory where the first byte is written.
 * @param [out] data   - Pointer to the buffer for the read data.
 * @param [in] size    - Size of the data array.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t memory_read(struct memory_desc *dev, uint16_t address, uint8_t *data,
		    uint32_t size)
{
	int32_t ret;
	uint8_t *transmit_array;
	uint16_t shifted_address;

	/* The shifter address is address after the protected memory in which the
	 * user data can be written */
	shifted_address = MEMORY_PROTECTED_SIZE + address;
	if((shifted_address + size) > MEMORY_SIZE_BYTES)
		size = MEMORY_SIZE_BYTES - shifted_address;

	transmit_array = calloc(2, sizeof(uint8_t));
	if (!transmit_array)
		return -1;

	transmit_array[0] = (shifted_address & 0xff00) >> 8;
	transmit_array[1] = (shifted_address & 0x00ff) >> 0;

	ret = i2c_write(dev->i2c_dev, transmit_array, 2, false);
	if(ret != MEMORY_SUCCESS)
		goto finish;

	ret = i2c_read(dev->i2c_dev, data, size, true);

finish:
	free(transmit_array);

	return ret;
}

/**
 * Change the I2C address used in communication with the memory.
 *
 * @param [in] dev     - The device structure.
 * @param [in] address - New I2C address.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t memory_change_i2c_address(struct memory_desc *dev, uint8_t address)
{
	dev->i2c_address = 0x50 | address;
	return i2c_set_address(dev->i2c_dev, dev->i2c_address);
}
