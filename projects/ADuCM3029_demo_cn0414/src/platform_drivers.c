/***************************************************************************//**
 *   @file   platform_drivers.c
 *   @brief  Implementation of Generic Platform Drivers.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2017(c) Analog Devices, Inc.
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
#include <stdint.h>
#include <stdlib.h>
#include <drivers/gpio/adi_gpio.h>
#include "timer.h"
#include <drivers/dma/adi_dma.h>
#include <drivers/spi/adi_spi.h>
#include <drivers/uart/adi_uart.h>
#include <drivers/flash/adi_flash.h>
#include <drivers/i2c/adi_i2c.h>
#include "platform_drivers.h"

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

/* Handle for UART device */
ADI_UART_HANDLE h_uart_device __attribute__ ((aligned (4)));
/* Memory for  UART driver */
uint8_t uart_device_mem[ADI_UART_BIDIR_MEMORY_SIZE] __attribute__ ((aligned (
			4)));
/* User UART memory */
uint8_t uart_current_line[256]; /* CLI current buffer */
uint8_t uart_previous_line[256]; /* CLI previous buffer */
uint8_t uart_line_index = 0; /* CLI buffer index */
uint8_t uart_cmd = 0; /* CLI command receive flag */
uint8_t uart_flag = 0;
uint8_t uart_ping_flag = 0;
uint8_t uart_pong_flag = 0;
volatile uint8_t ping, pong;
volatile uint8_t *ping_ptr = &ping;
volatile uint8_t *pong_ptr = &pong;
static volatile int8_t uart_tx_flying = 0;
uint8_t spi_init_flag = 0;

/* Master SPI device handle */
ADI_SPI_HANDLE h_spi_device;
/* Device memory required for operate Master SPI device */
uint8_t master_spi_device_mem[ADI_SPI_MEMORY_SIZE];

/* Handle for Flash device */
ADI_FEE_HANDLE h_fee_device;
/* Memory for Flash device */
uint8_t fee_device_mem[ADI_FEE_MEMORY_SIZE];
/* Hardware report variable */
uint32_t fee_hardware_error;
/* Flash write buffer */
uint32_t fee_data_buffer[FLASH_BUFFER_SIZE] __attribute__ ((aligned (4)));
/* Pointer to flash page address */
uint32_t *page_address = (uint32_t*)FLASH_PAGE_ADDR;
/* Flash parameters */
uint32_t nStartPage, nEndPage, nBlockNum;

/* Memory for GPIO device */
uint8_t mem_gpio_handler[ADI_GPIO_MEMORY_SIZE];
uint8_t gpio_init_flag = 0;

/* Master I2C device handle */
ADI_I2C_HANDLE master_i2c_dev;
/* Device memory required for operate Master I2C device */
uint8_t master_i2c_device_memory[ADI_I2C_MEMORY_SIZE];

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * UART callback function.
 *
 * @param [in] cb_param - Callback parameter passed by the application.
 * @param [in] event    - Trigger ID code.
 * @param [in] arg      - Callback parameter passed by the driver.
 *
 * @return void
 */
static void uart_callback(void *cb_param, uint32_t event, void *arg)
{
	switch(event) {
	case ADI_UART_EVENT_RX_BUFFER_PROCESSED:
		if(arg == (void *)&ping)
			uart_ping_flag = 2;
		if(arg == (void *)&pong)
			uart_pong_flag = 2;
		break;
	case ADI_UART_EVENT_NO_RX_BUFFER_EVENT:
		if (uart_ping_flag == 0) {
			adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
						(void *)ping_ptr, 1, DMA_NOT_USE);
			uart_ping_flag = 1;
		}
		if (uart_pong_flag == 0) {
			adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
						(void *)pong_ptr, 1, DMA_NOT_USE);
			uart_pong_flag = 1;
		}
		break;
	case ADI_UART_EVENT_TX_BUFFER_PROCESSED:
		uart_tx_flying--;
		break;
	default:
		break;
	}
}

/**
 * Initialize the I2C communication peripheral.
 *
 * @param [out] desc      - The I2C descriptor.
 * @param [in] init_param - The structure that contains the I2C parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t i2c_init(i2c_desc **desc, const i2c_init_param *param)
{
	int32_t ret;
	struct i2c_desc *dev;

	/* Allocate memory for I2C device descriptor */
	dev = calloc(1, sizeof *dev);
	if (!dev)
		return FAILURE;

	/* Initialize parameters */
	dev->id = param->id;
	dev->max_speed_hz = param->max_speed_hz;
	dev->slave_address = param->slave_address;
	dev->type = param->type;

	/* If this is the first GPIO initialize GPIO controller */
	if(gpio_init_flag == 0) {
		ret = adi_gpio_Init(mem_gpio_handler, ADI_GPIO_MEMORY_SIZE);
		if(ret != ADI_GPIO_SUCCESS)
			goto error;
	}

	/* Driving strength must be enabled for I2C pins */
	ret = adi_gpio_DriveStrengthEnable(ADI_GPIO_PORT0,
					   ADI_GPIO_PIN_4 | ADI_GPIO_PIN_5, true);
	if(ret != ADI_GPIO_SUCCESS)
		goto error;

	/* If no more GPIOs free driver memory */
	if (gpio_init_flag == 0) {
		ret = adi_gpio_UnInit();
		if(ret != ADI_GPIO_SUCCESS)
			goto error;
	}

	/* Open Master Device */
	ret = adi_i2c_Open(dev->id, master_i2c_device_memory, ADI_I2C_MEMORY_SIZE,
			   &master_i2c_dev);
	if(ret != ADI_I2C_SUCCESS)
		goto error;

	/* Set bit rate */
	ret = adi_i2c_SetBitRate(master_i2c_dev, dev->max_speed_hz);
	if(ret != ADI_I2C_SUCCESS)
		goto error;

	*desc = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free the resources allocated by i2c_init().
 *
 * @param [in] desc - The I2C descriptor.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t i2c_remove(i2c_desc *desc)
{
	/* Free memory for device descriptor */
	free(desc);

	/* Close I2C device */
	return adi_i2c_Close(master_i2c_dev);
}

/**
 * Write data to a slave device.
 *
 * @param [in] desc         - The I2C descriptor.
 * @param [in] data         - Buffer that stores the transmission data.
 * @param [in] bytes_number - Number of bytes to write.
 * @param [in] stop_bit     - Stop condition control.
 *                          Example: 0 - A stop condition will not be generated;
 *                                   1 - A stop condition will be generated.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t i2c_write(i2c_desc *desc, uint8_t *data, uint8_t bytes_number,
		  uint8_t stop_bit)
{
	ADI_I2C_TRANSACTION trans;
	uint32_t error;

	trans.bReadNotWrite = false;
	if(stop_bit == 0)
		trans.bRepeatStart = true;
	else
		trans.bRepeatStart = false;
	trans.nDataSize = bytes_number;
	trans.nPrologueSize = 0;
	trans.pData = data;
	trans.pPrologue = NULL;

	return adi_i2c_ReadWrite(master_i2c_dev, &trans, &error);
}

/**
 * Read data from a slave device.
 *
 * @param [in] desc         - The I2C descriptor.
 * @param [out] data        - Buffer that will store the received data.
 * @param [in] bytes_number - Number of bytes to read.
 * @param [in] stop_bit     - Stop condition control.
 *                          Example: 0 - A stop condition will not be generated;
 *                                   1 - A stop condition will be generated.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t i2c_read(i2c_desc *desc, uint8_t *data, uint8_t bytes_number,
		 uint8_t stop_bit)
{
	ADI_I2C_TRANSACTION trans;
	uint32_t error;

	trans.bReadNotWrite = true;
	trans.bRepeatStart = true;
	if(stop_bit)
		trans.bRepeatStart = false;
	trans.nDataSize = bytes_number;
	trans.nPrologueSize = 0;
	trans.pData = data;
	trans.pPrologue = NULL;

	return adi_i2c_ReadWrite(master_i2c_dev, &trans, &error);
}

/**
 * Set address that the device is sending before a transaction.
 *
 * @param [in] desc        - The I2C descriptor.
 * @param [in] new_address - New slave address.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t i2c_set_address(i2c_desc *desc, uint8_t new_address)
{
	return adi_i2c_SetSlaveAddress(master_i2c_dev, new_address);
}

/**
 * Initialize the SPI communication peripheral.
 *
 * @param [out] desc      - The SPI descriptor.
 * @param [in] init_param - The structure that contains the SPI parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t spi_init(struct spi_desc **desc, const struct spi_init_param *param)
{
	/* Variable for storing the return code from SPI device */
	int32_t ret;
	struct spi_desc *dev;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return FAILURE;

	dev->chip_select = NULL;
	dev->id = param->id;
	dev->max_speed_hz = param->max_speed_hz;
	dev->mode = param->mode;
	dev->type = param->type;

	/* To cope with more than one SPI device descriptor and SPI controller on
	 * the first pass and only the descriptors on subsequent passes */
	if(spi_init_flag != 0) {
		*desc = dev;
		spi_init_flag++;
		return 0;
	}

	/* Open the SPI device. It opens in Master mode by default */
	ret = adi_spi_Open(dev->id, master_spi_device_mem, ADI_SPI_MEMORY_SIZE,
			   &h_spi_device);
	if(ret != ADI_SPI_SUCCESS)
		goto error;

	/* Set the bit rate  */
	ret = adi_spi_SetBitrate(h_spi_device, dev->max_speed_hz);
	if(ret != ADI_SPI_SUCCESS)
		goto error;

	/* Set the chip select. */
	/* If no GPIO has been chosen use hardware CS */
	if (param->chip_select == 0xFF) {
		if(dev->id == SPI_ARDUINO) {
			/* SPI_ARDUINO uses CS1 */
			ret = adi_spi_SetChipSelect(h_spi_device, ADI_SPI_CS1);
			if(ret != ADI_SPI_SUCCESS)
				goto error;
		} else {
			/* SPI_BLE or SPI_PMOD */
			/* SPI_BLE and SPI_PMOD use CS0 */
			ret = adi_spi_SetChipSelect(h_spi_device, ADI_SPI_CS0);
			if(ret != ADI_SPI_SUCCESS)
				goto error;
		}
	} else {
		ret = adi_spi_SetChipSelect(h_spi_device, ADI_SPI_CS0);
		if(ret != ADI_SPI_SUCCESS)
			goto error;
		ret = gpio_get(&dev->chip_select, param->chip_select);
		if(ret != ADI_SPI_SUCCESS)
			goto error;
		ret = gpio_direction_output(dev->chip_select, GPIO_LOW);
		if(ret != ADI_SPI_SUCCESS)
			goto error;
	}
	ret = adi_spi_SetContinuousMode(h_spi_device, true);
	if(ret != ADI_SPI_SUCCESS)
		goto error;
	ret = adi_spi_SetIrqmode(h_spi_device, true);
	if(ret != ADI_SPI_SUCCESS)
		goto error;
	/* set Clock polarity low */
	ret = adi_spi_SetClockPolarity(h_spi_device, (dev->mode & 0x02) >> 1);
	if(ret != ADI_SPI_SUCCESS)
		goto error;
	/* set Clock phase leading */
	ret = adi_spi_SetClockPhase(h_spi_device, (dev->mode & 0x1) >> 0);
	if(ret != ADI_SPI_SUCCESS)
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
 * Free the resources allocated by spi_init().
 *
 * @param [in] desc - The SPI descriptor.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t spi_remove(spi_desc *desc)
{
	int32_t ret;

	if(!desc)
		return -1;

	if(desc->chip_select) {
		ret = gpio_remove(desc->chip_select);
		if(ret != 0)
			return ret;
	}

	free(desc);

	/* If not the last SPI device to disconnect don't close the controller
	 * yet */
	if(spi_init_flag > 1) {
		spi_init_flag--;
		return 0;
	}

	ret = adi_spi_Close(h_spi_device);
	if(ret != ADI_SPI_SUCCESS)
		return ret;

	spi_init_flag--;

	return ret;
}

/**
 * Write and read data to/from SPI.
 *
 * @param [in] desc         - The SPI descriptor.
 * @param [in/out] data     - The buffer with the transmitted/received data.
 * @param [in] bytes_number - Number of bytes to write/read.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t spi_write_and_read(spi_desc *desc, uint8_t *data, uint8_t bytes_number)
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
		if(ret != 0)
			return ret;
	}

	/* Wait till the data transmission is over */
	ret = adi_spi_MasterReadWrite(h_spi_device, &m_spi_transceive);
	if(ret != ADI_SPI_SUCCESS)
		return ret;

	if(desc->chip_select) {
		ret = gpio_set_value(desc->chip_select, GPIO_HIGH);
		if(ret != 0)
			return ret;
	}

	return ret;
}

/**
 * Obtain the GPIO decriptor.
 *
 * @param [out] desc        - The GPIO descriptor.
 * @param [in]  gpio_number - The number of the GPIO.
 *
 * @return void
 */
static void gpio_get_portpin(struct gpio_desc *desc, uint16_t *pin,
			     uint8_t *port)
{
	*pin = (ADI_GPIO_DATA)(0x0001 << (desc->number & 0x0F));
	*port = (desc->number & 0xF0) >> 4;
}

/**
 * Obtain the GPIO decriptor.
 *
 * @param [out] desc       - The GPIO descriptor.
 * @param [in] gpio_number - The number of the GPIO.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_get(gpio_desc **desc, uint8_t gpio_number)
{
	int32_t ret = 0;
	struct gpio_desc *dev;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	dev->type = ADICUP3029_GPIO;
	dev->id = 0;
	dev->number = gpio_number;

	/* If this is the first GPIO initialize GPIO controller */
	if(gpio_init_flag == 0) {
		ret = adi_gpio_Init(mem_gpio_handler, ADI_GPIO_MEMORY_SIZE);
		if(ret != ADI_GPIO_SUCCESS)
			goto error;
	}

	/* Increment number of GPIOs */
	gpio_init_flag++;

	*desc = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free the resources allocated by gpio_get().
 *
 * @param [in] desc - The SPI descriptor.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_remove(gpio_desc *desc)
{
	if(!desc)
		return -1;

	free(desc);

	/* Decrement number of GPIOs */
	gpio_init_flag--;

	/* If no more GPIOs free driver memory */
	if (gpio_init_flag == 0)
		return adi_gpio_UnInit();

	return 0;
}

/**
 * Enable the input direction of the specified GPIO.
 *
 * @param [in] desc - The GPIO descriptor.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_direction_input(gpio_desc *desc)
{
	uint16_t pin;
	uint8_t port;

	/* Get pin port and number from pin number */
	gpio_get_portpin(desc, &pin, &port);

	/* Enable input driver */
	return adi_gpio_InputEnable(port, pin, true);
}

/**
 * Enable the output direction of the specified GPIO.
 *
 * @param [in] desc  - The GPIO descriptor.
 * @param [in] value - The value.
 *                     Example: GPIO_HIGH;
 *                              GPIO_LOW.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_direction_output(gpio_desc *desc, uint8_t value)
{
	int32_t ret;
	uint16_t pin;
	uint8_t port;

	/* Get pin port and number from pin number */
	gpio_get_portpin(desc, &pin, &port);

	/* Enable output driver */
	ret = adi_gpio_OutputEnable(port, pin, true);
	if(ret != ADI_GPIO_SUCCESS)
		return ret;

	/* Initialize pin with a value */
	if(value == 1)
		return adi_gpio_SetHigh(port, pin);
	else
		return adi_gpio_SetLow(port, pin);
}

/**
 * Set the value of the specified GPIO.
 *
 * @param [in] desc  - The GPIO descriptor.
 * @param [in] value - The value.
 *                     Example: GPIO_HIGH
 *                              GPIO_LOW
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_set_value(gpio_desc *desc, uint8_t value)
{
	uint16_t pin;
	uint8_t port;

	/* Get pin port and number from pin number */
	gpio_get_portpin(desc, &pin, &port);

	/* Change pin value */
	if(value == 1)
		return adi_gpio_SetHigh(port, pin);
	else
		return adi_gpio_SetLow(port, pin);
}

/**
 * Get the value of the specified GPIO.
 *
 * @param [in]  desc  - The GPIO descriptor.
 * @param [out] value - The value.
 *                      Example: GPIO_HIGH
 *                               GPIO_LOW
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_get_value(gpio_desc *desc, uint16_t *value)
{
	uint16_t pin;
	uint8_t port;

	/* Get pin port and number from pin number */
	gpio_get_portpin(desc, &pin, &port);

	/* Read pin value */
	return adi_gpio_GetData(port, pin, value);
}

/**
 * Generate miliseconds delay.
 *
 * @param [in] msecs - Delay in miliseconds.
 *
 * @return None.
 */
void mdelay(uint32_t msecs)
{
	timer_sleep(msecs);
}

/**
 * Initialize and start UART communication and UART controller descriptor
 * structure.
 *
 * @param [out] descriptor - User UART device structure descriptor.
 * @param [in]  init_param - Initilaization parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_init(struct uart_desc **descriptor,
		      struct uart_init_param *init_param)
{
	/* Variable for storing the return code from UART device */
	int32_t  ret;
	uint8_t bits_nr;
	struct uart_desc *desc;

	desc = calloc(1, sizeof *desc);
	if (!desc)
		return FAILURE;

	desc->baudrate = init_param->baudrate;
	desc->bits_no = init_param->bits_no;
	desc->has_callback = init_param->has_callback;

	/* Values for the baudrate generation registers when the baudrate is one of
	 * the standard values and clock is 26MHz */
	const uint8_t baudrate_osr_26mhz[] = {3, 3, 3, 3, 3, 3, 3, 2, 2, 2};
	const uint8_t baudrate_divc_26mhz[] = {24, 12, 8, 4, 4, 2, 1, 1, 1, 1};
	const uint8_t baudrate_divm_26mhz[] = {3, 3, 2, 3, 1, 1, 1, 1, 1, 1};
	const uint16_t baudrate_divn_26mhz[] = {
		1078, 1078, 1321, 1078, 1563, 1563, 1563, 1563, 1280, 171
	};

	/* Get bits number */
	switch(desc->bits_no) {
	case 5:
		bits_nr = ADI_UART_WORDLEN_5BITS;
		break;
	case 6:
		bits_nr = ADI_UART_WORDLEN_6BITS;
		break;
	case 7:
		bits_nr = ADI_UART_WORDLEN_7BITS;
		break;
	case 8:
		bits_nr = ADI_UART_WORDLEN_8BITS;
		break;
	default:
		bits_nr = ADI_UART_WORDLEN_8BITS;
	}

	/* Open the UART device.Data transfer is bidirectional with NORMAL mode by
	 * default. */
	ret = adi_uart_Open(UART_DEVICE_NUM, ADI_UART_DIR_BIDIRECTION,
			    uart_device_mem, ADI_UART_BIDIR_MEMORY_SIZE, &h_uart_device);
	if(ret != ADI_UART_SUCCESS)
		goto error;

	/* Configure  UART device with NO-PARITY, ONE STOP BIT and 8bit word
	 * length. */
	ret = adi_uart_SetConfiguration(h_uart_device, ADI_UART_NO_PARITY,
					ADI_UART_ONE_STOPBIT, bits_nr);
	if(ret != ADI_UART_SUCCESS)
		goto error;

	ret = adi_uart_ConfigBaudRate(h_uart_device,
				      baudrate_divc_26mhz[desc->baudrate],
				      baudrate_divm_26mhz[desc->baudrate],
				      baudrate_divn_26mhz[desc->baudrate],
				      baudrate_osr_26mhz[desc->baudrate]);
	if(ret != ADI_UART_SUCCESS)
		goto error;

	if(desc->has_callback) {
		/* Register callback */
		ret = adi_uart_RegisterCallback(h_uart_device, uart_callback, NULL);
		if(ret != ADI_UART_SUCCESS)
			goto error;
	}

	/* Submit buffer for read */
	if (uart_ping_flag == 0) {
		ret = adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
					      (void *)ping_ptr, 1, DMA_NOT_USE);
		if(ret != ADI_UART_SUCCESS)
			goto error;
		uart_ping_flag = 1;
	}
	if (uart_pong_flag == 0) {
		ret = adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
					      (void *)pong_ptr, 1, DMA_NOT_USE);
		if(ret != ADI_UART_SUCCESS)
			goto error;
		uart_pong_flag = 1;
	}

	*descriptor = desc;

	return ret;
error:
	free(desc);

	return ret;
}

/**
 * Close UART communication and free memory from UART descriptor.
 *
 * @param [in] desc - User UART device structure descriptor.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_remove(struct uart_desc *desc)
{
	if(!desc)
		return -1;

	return adi_uart_Close(h_uart_device);
}

/**
 * Write a character trought UART in a blocking call.
 *
 * usr_uart_write_char() helper function. The blocking call differs depending
 * on the existence of a callback function.
 *
 * @param [in] desc - User UART device structure descriptor.
 * @param [in] data - Data to be transmitted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t usr_uart_write_block(struct uart_desc *desc, uint8_t data)
{
	int32_t ret;
	const uint32_t buf_size = 1;
	uint32_t hw_error;

	if(desc->has_callback) {
		ret = adi_uart_SubmitTxBuffer((ADI_UART_HANDLE const)h_uart_device,
					      &data, buf_size, DMA_NOT_USE);
		if(ret != ADI_UART_SUCCESS)
			return ret;
		uart_tx_flying++;
		while(uart_tx_flying > 0);
		if(uart_tx_flying == 0)
			return 0;
		else
			return -1;
	} else {
		return adi_uart_Write((ADI_UART_HANDLE const)h_uart_device, &data,
				      buf_size, DMA_NOT_USE, &hw_error);
	}
}

/**
 * Transmit a character through UART.
 *
 * @param [in] desc - User UART device structure descriptor.
 * @param [in] data - Data to be transmitted.
 * @param [in] mode - Mode of transmission (blocking or non-blocking)
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_write_char(struct uart_desc *desc, uint8_t data,
			    enum uart_comm_mode mode)
{
	int32_t ret;
	bool tx_available;
	const uint32_t buf_size = 1;

	switch(mode) {
	case UART_BLOCKING:
		ret = usr_uart_write_block(desc, data);
		break;
	case UART_NON_BLOCKING:
		ret = adi_uart_IsTxBufferAvailable((ADI_UART_HANDLE const)h_uart_device,
						   (bool* const)&tx_available);
		if(ret != ADI_UART_SUCCESS)
			return ret;
		if (!tx_available)
			return ret;
		/* Start transmission */
		ret = adi_uart_SubmitTxBuffer((ADI_UART_HANDLE const)h_uart_device,
					      &data, buf_size, DMA_NOT_USE);
		break;
	default:
		break;
	}

	return ret;
}

/**
 * Transmit an array of characters through UART.
 *
 * @param [in] desc   - User UART device structure descriptor.
 * @param [in] string - Pointer to the data array to be transmitted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_write_string(struct uart_desc *desc, uint8_t *string)
{
	int32_t ret = 0;

	while(*string != '\0') {
		ret = usr_uart_write_char(desc, *string++, UART_BLOCKING);

		if(ret != 0)
			break;
	}

	return ret;
}

/**
 * Read a character through UART in a blocking call.
 *
 * usr_uart_read_char() helper function. Takes in consideration if the
 * application submits an UART callback or not.
 *
 * @param [in]  desc - User UART device structure descriptor.
 * @param [out] data - Pointer to data container.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t usr_uart_read_block(struct uart_desc *desc, uint8_t *data)
{
	const uint32_t buf_size = 1;
	uint32_t error;

	if(!desc->has_callback) {
		if(uart_ping_flag == 0 && uart_pong_flag == 0)
			adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
						data, buf_size, DMA_NOT_USE);

		return adi_uart_GetRxBuffer((ADI_UART_HANDLE const)h_uart_device,
					    (void **)&data, &error);
	} else {
		while(uart_ping_flag != 2 && uart_pong_flag != 2);
		if(uart_ping_flag == 2) {
			*data = *ping_ptr;
			uart_ping_flag = 0;
		}
		if(uart_pong_flag == 2) {
			*data = *pong_ptr;
			uart_pong_flag = 0;
		}

		return 0;
	}
}

/**
 * Read a character through UART.
 *
 * @param [in]  desc - User UART device structure descriptor.
 * @param [out] data - Pointer to data container.
 * @param [out] rdy  - 1 if there is new data, 0 otherwise.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t usr_uart_read_nonblock(struct uart_desc *desc, uint8_t *data,
				      uint8_t *rdy)
{
	uint32_t error;
	bool rx_done;
	int32_t ret;

	if(!desc->has_callback) {
		ret = adi_uart_IsRxBufferAvailable((ADI_UART_HANDLE const)h_uart_device,
						   &rx_done);
		if(ret != ADI_UART_SUCCESS)
			return ret;
		if(!rx_done)
			return 0;
		*rdy = 1;

		return adi_uart_GetRxBuffer((ADI_UART_HANDLE const)h_uart_device,
					    (void **)&data, &error);
	} else {
		if(uart_ping_flag == 2) {
			*data = *ping_ptr;
			*rdy = 1;
			uart_ping_flag = 0;
		}
		if(uart_pong_flag == 2) {
			*data = *pong_ptr;
			*rdy = 1;
			uart_pong_flag = 0;
		}

		return 0;
	}
}

/**
 * Read a character through UART.
 *
 * @param [in]  desc - User UART device structure descriptor.
 * @param [out] data - Pointer to data container.
 * @param [out] rdy  - 1 if there is new data, 0 otherwise.
 * @param [in]  mode - Determines if BLOCKING or NON-BLOCKING.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_read_char(struct uart_desc *desc, uint8_t *data, uint8_t *rdy,
			   enum uart_comm_mode mode)
{
	int32_t ret;

	*rdy = 0;

	switch(mode) {
	case UART_BLOCKING:
		ret = usr_uart_read_block(desc, data);
		*rdy = 1;
		break;
	case UART_NON_BLOCKING:
		ret = usr_uart_read_nonblock(desc, data, rdy);
		break;
	default:
		break;
	}

	return ret;
}
