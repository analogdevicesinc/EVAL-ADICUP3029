/***************************************************************************//**
 *   @file   spi.c
 *   @author DBogdan (dragos.bogdan@analog.com)
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
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
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

#include "../platform_include/spi.h"
#include <drivers/spi/adi_spi.h>
#include "../platform_include/error.h"
#include "gpio.h"
#include "error.h"
#include <stdlib.h>

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

/* Master SPI device handle */
ADI_SPI_HANDLE h_spi_device;
/* Device memory required for operate Master SPI device */
uint8_t master_spi_device_mem[ADI_SPI_MEMORY_SIZE];
uint8_t spi_init_flag = 0; /* Number of SPI controllers initialized */

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Initialize the SPI communication peripheral.
 * @param desc - The SPI descriptor.
 * @param init_param - The structure that contains the SPI parameters.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t spi_init(struct spi_desc **desc,
		 const struct spi_init_param *param)
{
	/* Variable for storing the return code from SPI device */
	int32_t ret;
	struct spi_desc *dev;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return FAILURE;

	dev->chip_select = 0;
	dev->id = param->id;
	dev->max_speed_hz = param->max_speed_hz;
	dev->mode = param->mode;
	dev->type = param->type;
	dev->extra = param->extra;

	/* To cope with more than one SPI device descriptor and SPI controller on
	 * the first pass and only the descriptors on subsequent passes */
	if(spi_init_flag != 0) {
		*desc = dev;
		spi_init_flag++;
		return SUCCESS;
	}

	/* Open the SPI device. It opens in Master mode by default */
	ret = adi_spi_Open(dev->id, master_spi_device_mem, ADI_SPI_MEMORY_SIZE,
			   &h_spi_device);
	if(ret != SUCCESS)
		goto error;

	/* Set the bit rate  */
	ret = adi_spi_SetBitrate(h_spi_device, dev->max_speed_hz);
	if(ret != SUCCESS)
		goto error;

	/* Set the chip select. */
	/* If no GPIO has been chosen use hardware CS */
	if (param->chip_select == 0xFF) {
		if(dev->id == SPI_ARDUINO) {
			/* SPI_ARDUINO uses CS1 */
			ret = adi_spi_SetChipSelect(h_spi_device, ADI_SPI_CS1);
			if(ret != SUCCESS)
				goto error;
		} else {
			/* SPI_BLE or SPI_PMOD */
			/* SPI_BLE and SPI_PMOD use CS0 */
			ret = adi_spi_SetChipSelect(h_spi_device, ADI_SPI_CS0);
			if(ret != SUCCESS)
				goto error;
		}
	} else {
		ret = adi_spi_SetChipSelect(h_spi_device, ADI_SPI_CS0);
		if(ret != SUCCESS)
			goto error;
		ret = gpio_get(&dev->chip_select, param->chip_select);
		if(ret != SUCCESS)
			goto error;
		ret = gpio_direction_output(dev->chip_select, GPIO_LOW);
		if(ret != SUCCESS)
			goto error;
	}
	ret = adi_spi_SetContinuousMode(h_spi_device, true);
	if(ret != SUCCESS)
		goto error;
	ret = adi_spi_SetIrqmode(h_spi_device, true);
	if(ret != SUCCESS)
		goto error;
	/* set Clock polarity low */
	ret = adi_spi_SetClockPolarity(h_spi_device, (dev->mode & 0x02) >> 1);
	if(ret != SUCCESS)
		goto error;
	/* set Clock phase leading */
	ret = adi_spi_SetClockPhase(h_spi_device, (dev->mode & 0x1) >> 0);
	if(ret != SUCCESS)
		goto error;

	/* Return device descriptor */
	*desc = dev;

	/* No longer the first SPI device */
	spi_init_flag++;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * @brief Free the resources allocated by spi_init().
 * @param desc - The SPI descriptor.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t spi_remove(struct spi_desc *desc)
{
	int32_t ret;

	if(!desc)
		return -1;

	if(desc->chip_select) {
		ret = gpio_remove(desc->chip_select);
		if(ret != SUCCESS)
			return ret;
	}

	free(desc);

	/* If not the last SPI device to disconnect don't close the controller
	 * yet */
	if(spi_init_flag > 1) {
		spi_init_flag--;
		return SUCCESS;
	}

	ret = adi_spi_Close(h_spi_device);
	if(ret != SUCCESS)
		return ret;

	spi_init_flag--;

	return ret;
}

/**
 * @brief Write and read data to/from SPI.
 * @param desc - The SPI descriptor.
 * @param data - The buffer with the transmitted/received data.
 * @param bytes_number - Number of bytes to write/read.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t spi_write_and_read(struct spi_desc *desc,
			   uint8_t *data,
			   uint8_t bytes_number)
{
	ADI_SPI_TRANSCEIVER m_spi_transceive;
	int32_t ret;

	m_spi_transceive.pTransmitter = data;
	m_spi_transceive.TransmitterBytes = bytes_number;
	m_spi_transceive.nTxIncrement = 1;
	m_spi_transceive.pReceiver = data;
	m_spi_transceive.ReceiverBytes = bytes_number;
	m_spi_transceive.nRxIncrement = 1;
	m_spi_transceive.bDMA = false;
	m_spi_transceive.bRD_CTL = false;

	if(desc->chip_select) {
		ret = gpio_set_value(desc->chip_select, GPIO_LOW);
		if(ret != SUCCESS)
			return ret;
	}

	/* Wait till the data transmission is over */
	ret = adi_spi_MasterReadWrite(h_spi_device, &m_spi_transceive);
	if(ret != SUCCESS)
		return ret;

	if(desc->chip_select) {
		ret = gpio_set_value(desc->chip_select, GPIO_HIGH);
		if(ret != SUCCESS)
			return ret;
	}

	return ret;
}
