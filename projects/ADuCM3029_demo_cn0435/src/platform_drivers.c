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
#include <drivers/dma/adi_dma.h>
#include <drivers/spi/adi_spi.h>
#include <drivers/uart/adi_uart.h>
#include <drivers/flash/adi_flash.h>
#include <drivers/i2c/adi_i2c.h>
#include "platform_drivers.h"
#include "timer.h"
#include "config.h"

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

/* Handle for UART device */
ADI_UART_HANDLE h_uart_device __attribute__ ((aligned (4)));
/* Memory for  UART driver */
uint8_t uart_device_mem[ADI_UART_BIDIR_MEMORY_SIZE] __attribute__ ((aligned (
			4)));
/* User UART memory */
uint8_t uart_current_line[100]; /* CLI current buffer */
uint8_t uart_previous_line[100]; /* CLI previous buffer */
uint8_t uart_line_index = 0; /* CLI buffer index */
uint8_t uart_cmd = 0; /* CLI command receive flag */
uint8_t uart_flag = 0;
uint8_t uart_ping_flag = 0;
uint8_t uart_pong_flag = 0;
#if defined(CLI_INTEFACE)
volatile uint8_t ping, pong;
volatile uint8_t *ping_ptr = &ping;
volatile uint8_t *pong_ptr = &pong;
#elif defined(MODBUS_INTERFACE)
volatile uint8_t ping, pong;
volatile uint8_t *ping_ptr = &ping;
volatile uint8_t *pong_ptr = &pong;
volatile uint8_t ring_buff[256];
volatile struct rb_index user_index = {0, 0}, app_index = {0, 0};
volatile struct rb_index *user_ptr = &user_index, *app_ptr = &app_index;
volatile uint8_t tx_flag = 0;
#endif
uint8_t uart_init_flag = 0;

/* Master SPI device handle */
ADI_SPI_HANDLE h_spi_device;
/* Device memory required for operate Master SPI device */
uint8_t master_spi_device_mem[ADI_SPI_MEMORY_SIZE];
uint8_t spi_init_flag = 0;

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
uint8_t i2c_init_flag = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
#if defined(MODBUS_INTERFACE)
/**
 * Increment the ring buffer pointer structure by an integer number.
 *
 * @param [in] a - ring buffer pointer structure to be incremented.
 * @param [in] b - integer increment.
 *
 * @return new ring buffer pointer structure.
 */
static struct rb_index add_index(struct rb_index a, uint8_t b)
{
	struct rb_index r;

	if((a.index + b) > 255)
		r.oddpass = a.oddpass ^ 1;
	else
		r.oddpass = a.oddpass;
	r.index = a.index + b;

	return r;
}

/**
 * See if two ring buffer pointer structures are pointing to the same place.
 *
 * @param [in] user - First pointer.
 * @param [in] app  - Second pointer.
 *
 * @return true if the pointers point to the same place in the ring buffer,
 *         false otherwise.
 */
static bool index_user_equal_app(struct rb_index user, struct rb_index app)
{
	if(user.index == app.index && user.oddpass == app.oddpass)
		return true;
	else
		return false;
}

/**
 * See if one ring buffer pointer is ahead of the other.
 *
 * @param [in] user - First pointer.
 * @param [in] app  - Second pointer.
 *
 * @return true if the first pointer is ahead than the second, false otherwise.
 */
static bool index_user_greater_app(struct rb_index user, struct rb_index app)
{
	if((user.index > app.index && user.oddpass == app.oddpass) ||
	    (user.index < app.index && user.oddpass != app.oddpass))
		return true;
	else
		return false;
}
#endif
/**
 * UART callback function.
 *
 * Used only in the CLI interface mode.
 *
 * @param [in] cb_param - Pointer to the application passed parameter.
 * @param [in] event    - ID code of the interrupt trigger.
 * @param [in] arg      - Pointer to the driver passed parameter.
 *
 * @return void
 */
static void uart_callback(void *cb_param, uint32_t event, void *arg)
{
#if defined(CLI_INTEFACE)
	switch(event) {
	case ADI_UART_EVENT_RX_BUFFER_PROCESSED:
		if(arg == (void *)&ping) {
			uart_ping_flag = 2;
		}
		if(arg == (void *)&pong) {
			uart_pong_flag = 2;
		}
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
	default:
		break;
	}
#elif defined(MODBUS_INTERFACE)
	switch(event) {
	case ADI_UART_EVENT_RX_BUFFER_PROCESSED:
		if(arg == (void *)ping_ptr) {
			ring_buff[app_index.index] = *ping_ptr;
			app_index = add_index(app_index, 1);
		}
		if(arg == (void *)&pong) {
			ring_buff[app_index.index] = *pong_ptr;
			app_index = add_index(app_index, 1);
		}
		break;
	case ADI_UART_EVENT_NO_RX_BUFFER_EVENT:
		adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
					(void *)ping_ptr, 1, DMA_NOT_USE);
		adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
					(void *)pong_ptr, 1, DMA_NOT_USE);
		break;
	case ADI_UART_EVENT_TX_BUFFER_PROCESSED:
		tx_flag = 1;
		break;
	default:
		break;
	}
#endif
}

/**
 * Initialize the I2C communication peripheral.
 *
 * @param [out] desc       - The I2C descriptor.
 * @param [in]  init_param - The structure that contains the I2C parameters.
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
		return -1;

	/* Initialize parameters */
	dev->id = param->id;
	dev->max_speed_hz = param->max_speed_hz;
	dev->slave_address = param->slave_address;
	dev->type = param->type;

	/* If this is the first GPIO initialize GPIO controller */
	if(gpio_init_flag == 0) {
		ret = adi_gpio_Init(mem_gpio_handler, ADI_GPIO_MEMORY_SIZE);
		if(ret < 0)
			goto error;
	}

	/* Driving strength must be enabled for I2C pins */
	ret = adi_gpio_DriveStrengthEnable(ADI_GPIO_PORT0,
					   ADI_GPIO_PIN_4 | ADI_GPIO_PIN_5, true);
	if(ret < 0)
		goto error;

	/* If no more GPIOs free driver memory */
	if (gpio_init_flag == 0) {
		ret = adi_gpio_UnInit();
		if(ret < 0)
			goto error;
	}

	/* Open Master Device */
	ret = adi_i2c_Open(dev->id, master_i2c_device_memory, ADI_I2C_MEMORY_SIZE,
			   &master_i2c_dev);
	if(ret < 0)
		goto error;

	/* Set bit rate */
	ret = adi_i2c_SetBitRate(master_i2c_dev, dev->max_speed_hz);
	if(ret < 0)
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
	if(!desc)
		return -1;

	free(desc);

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
 * @param [in] data         - Buffer that will store the received data.
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
 * @param [out] desc       - The SPI descriptor.
 * @param [in]  init_param - The structure that contains the SPI parameters.
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
		return -1;

	dev->chip_select = NULL;
	dev->id = param->id;
	dev->max_speed_hz = param->max_speed_hz;
	dev->mode = param->mode;
	dev->type = param->type;

	/* To cope with more than one SPI device descriptor and SPI controller on
	 * the first pass and only the descriptors on subsequent passes */
	if (spi_init_flag == 0) {
		/* Open the SPI device. It opens in Master mode by default */
		ret = adi_spi_Open(dev->id, master_spi_device_mem, ADI_SPI_MEMORY_SIZE,
				   &h_spi_device);
		if(ret < 0)
			goto error;
	}
	/* Set the bit rate  */
	ret = adi_spi_SetBitrate(h_spi_device, dev->max_speed_hz);
	if(ret < 0)
		goto error;

	/* Set the chip select. */
	/* If no GPIO has been chosen use hardware CS */
	if (param->chip_select == 0xFF) {
		if(dev->id == SPI_ARDUINO) {
			/* SPI_ARDUINO uses CS1 */
			ret = adi_spi_SetChipSelect(h_spi_device, ADI_SPI_CS1);
			if(ret < 0)
				goto error;
		} else {
			/* SPI_BLE or SPI_PMOD */
			/* SPI_BLE and SPI_PMOD use CS0 */
			ret = adi_spi_SetChipSelect(h_spi_device, ADI_SPI_CS0);
			if(ret < 0)
				goto error;
		}
	} else {
		ret = adi_spi_SetChipSelect(h_spi_device, ADI_SPI_CS0);
		if(ret < 0)
			goto error;
		ret = gpio_get(&dev->chip_select, param->chip_select);
		if(ret < 0)
			goto error;
		ret = gpio_direction_output(dev->chip_select, GPIO_HIGH);
		if(ret < 0)
			goto error;
	}
	ret = adi_spi_SetContinuousMode(h_spi_device, true);
	if(ret < 0)
		goto error;
	ret = adi_spi_SetIrqmode(h_spi_device, true);
	if(ret < 0)
		goto error;
	/* set Clock polarity low */
	ret = adi_spi_SetClockPolarity(h_spi_device, (dev->mode & 0x02) >> 1);
	if(ret < 0)
		goto error;
	/* set Clock phase leading */
	ret = adi_spi_SetClockPhase(h_spi_device, (dev->mode & 0x01) >> 0);
	if(ret < 0)
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
		if(ret < 0)
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
	if(ret < 0)
		return ret;

	spi_init_flag--;

	return ret;
}

/**
 * Write and read data to/from SPI.
 *
 * @param [in] desc         - The SPI descriptor.
 * @param [in] data         - The buffer with the transmitted/received data.
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

	if(desc->chip_select)
		gpio_set_value(desc->chip_select, GPIO_LOW);

	/* Wait till the data transmission is over */
	ret = adi_spi_MasterReadWrite(h_spi_device, &m_spi_transceive);
	if(ret < 0) {
		if(desc->chip_select)
			gpio_set_value(desc->chip_select, GPIO_HIGH);
		return ret;
	}

	if(desc->chip_select)
		gpio_set_value(desc->chip_select, GPIO_HIGH);

	return ret;
}

/**
 * Change SPI mode.
 *
 * @param [in] desc - The SPI descriptor.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t spi_change_mode(spi_desc *desc)
{
	int32_t ret;

	ret = adi_spi_SetClockPolarity(h_spi_device, (desc->mode & 0x02) >> 1);
	if(ret < 0)
		return ret;

	return adi_spi_SetClockPhase(h_spi_device, (desc->mode & 0x01) >> 0);
}

/**
 * Obtain the GPIO port and pin from the handler.
 *
 * @param [in] desc  - The GPIO descriptor.
 * @param [out] pin  - The number of the pin of the GPIO.
 * @param [out] port - The number of the port of the GPIO.
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
	int32_t ret;
	struct gpio_desc *dev;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	dev->type	= ADICUP3029_GPIO;
	dev->id 	= 0;
	dev->number = gpio_number;

	/* If this is the first GPIO initialize GPIO controller */
	if(gpio_init_flag == 0) {
		ret = adi_gpio_Init(mem_gpio_handler, ADI_GPIO_MEMORY_SIZE);
		if(ret < 0)
			goto error;
	}

	/* Increment number of GPIOs */
	gpio_init_flag++;

	*desc = dev;

	return 0;
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
	int32_t ret;

	if(!desc)
		return -1;

	free(desc);

	/* Decrement number of GPIOs */
	gpio_init_flag--;

	/* If no more GPIOs free driver memory */
	if (gpio_init_flag == 0) {
		ret = adi_gpio_UnInit();
		if(ret < 0)
			return ret;
	}

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
 * @param [in] desc - The GPIO descriptor.
 * @param [in] value - The value.
 *                     Example: GPIO_HIGH
 *                              GPIO_LOW
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
	if(ret < 0)
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
 * @param [in] desc   - The GPIO descriptor.
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
		      struct uart_init_param init_param)
{
	/* Variable for storing the return code from UART device */
	int32_t ret;
	uint8_t bits_nr;
	struct uart_desc *desc;

	desc = calloc(1, sizeof *desc);
	if (!desc)
		return -1;

	desc->baudrate = init_param.baudrate;
	desc->bits_no = init_param.bits_no;

	/* Values for the baudrate generation registers when the baudrate is one of
	 * the standard values and clock is 26MHz */
	const uint8_t baudrate_osr_26mhz[] = {3, 3, 3, 3, 3, 3, 3, 2, 2, 2};
	const uint16_t baudrate_divc_26mhz[] = {24, 12, 8, 4, 4, 2, 1, 1, 1, 1};
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
	if(uart_init_flag++ == 0) {
		ret = adi_uart_Open(UART_DEVICE_NUM, ADI_UART_DIR_BIDIRECTION,
				    uart_device_mem, ADI_UART_BIDIR_MEMORY_SIZE, &h_uart_device);
		if(ret < 0)
			goto error;
	}

	/* Configure  UART device with NO-PARITY, ONE STOP BIT and 8bit word
	 * length. */
	ret = adi_uart_SetConfiguration(h_uart_device, ADI_UART_NO_PARITY,
					ADI_UART_ONE_STOPBIT, bits_nr);
	if(ret < 0)
		goto error;

	/* Baud rate div values are calculated for PCLK 26Mhz */
	ret = adi_uart_ConfigBaudRate(h_uart_device,
				      baudrate_divc_26mhz[desc->baudrate],
				      baudrate_divm_26mhz[desc->baudrate],
				      baudrate_divn_26mhz[desc->baudrate],
				      baudrate_osr_26mhz[desc->baudrate]);
	if(ret < 0)
		goto error;

//#if defined(CLI_INTEFACE)
	/* Register callback */
	ret = adi_uart_RegisterCallback(h_uart_device, uart_callback, NULL);
	if(ret < 0)
		goto error;
#if defined(MODBUS_INTERFACE)
	ret = adi_uart_SetRxFifoTriggerLevel(h_uart_device,
					     ADI_UART_RX_FIFO_TRIG_LEVEL_1BYTE);
	if(ret < 0)
		goto error;

	ret = adi_uart_EnableFifo(h_uart_device, true);
	if(ret < 0)
		goto error;
#endif
	/* Submit buffer for read */
#if defined(CLI_INTEFACE)
	if (uart_ping_flag == 0) {
		ret = adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
					      (void *)ping_ptr, 1, DMA_NOT_USE);
		if(ret < 0)
			goto error;
		uart_ping_flag = 1;
	}
	if (uart_pong_flag == 0) {
		ret = adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
					      (void *)pong_ptr, 1, DMA_NOT_USE);
		if(ret < 0)
			goto error;
		uart_pong_flag = 1;
	}
#elif defined(MODBUS_INTERFACE)
	ret = adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
				      (void *)ping_ptr, 1, DMA_NOT_USE);
	if(ret < 0)
		goto error;
	ret = adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
				      (void *)pong_ptr, 1, DMA_NOT_USE);
	if(ret < 0)
		goto error;
#endif
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
	int32_t ret;

	if(!desc)
		return -1;

	if (--uart_init_flag == 0) {
		ret = adi_uart_Close(h_uart_device);
		if(ret < 0)
			return ret;
	}

	return 0;
}

/**
 * Write a character through UART.
 *
 * @param [in] desc - User UART device structure descriptor.
 * @param [in] data - Data to be transmitted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_write_char(struct uart_desc *desc, uint8_t data,
			    enum uart_en_write_data mode)
{
	int32_t ret;
#if defined(CLI_INTEFACE)
	bool tx_available;
	uint32_t hw_error;
#endif
	const uint32_t buf_size = 1;
#if defined(MODBUS_INTERFACE)
	tx_flag = 0;
	ret = adi_uart_SubmitTxBuffer((ADI_UART_HANDLE const)h_uart_device, &data,
				      buf_size, DMA_NOT_USE);
	if(ret < 0)
		return ret;
	while(tx_flag == 0);

	return ret;
#elif defined(CLI_INTEFACE)
	/* Send character with interrupt */
	if(mode == UART_WRITE_IN_INT) {
		/* Check if not busy */
		ret = adi_uart_IsTxBufferAvailable((ADI_UART_HANDLE const)h_uart_device,
						   (bool* const)&tx_available);
		if(ret < 0)
			return ret;
		if (!tx_available) {
			return UART_NO_TX_SPACE;
		}
		/* Start transmission */
		return adi_uart_SubmitTxBuffer((ADI_UART_HANDLE const)h_uart_device,
					       (void* const)&data, buf_size, DMA_NOT_USE);
		/* Send character without interrupt (blocking) */
	} else {
		/* Do transmission */
		return adi_uart_Write((ADI_UART_HANDLE const)h_uart_device,
				      (void* const)&data, buf_size, DMA_NOT_USE, &hw_error);
	}
#endif
}

/**
 * Poll TX status to make a non-blocking write call blocking.
 *
 * @param [in] desc - Pointer to the UART handler.
 *
 * @return void
 */
void usr_uart_poll_tx(struct uart_desc *desc)
{
	bool tx_complete;

	do {
		adi_uart_IsTxComplete((ADI_UART_HANDLE const)h_uart_device,
				      &tx_complete);
	} while(!tx_complete);
}

/**
 * Transmit an array of characters through UART.
 *
 * @param [in] desc   - User UART device structure descriptor.
 * @param [in] string - Pointer to the data array to be transmitted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_write_string(struct uart_desc *desc,
			      uint8_t *string)
{
	int16_t result = 0;

	while(*string != '\0') {
		result = usr_uart_write_char(desc, *string++, UART_WRITE_NO_INT);

		if(result != 0) {
			break;
		}
	}

	return result;
}

/**
 * Read a character in a non-blocking call.
 *
 * @param [in] desc  - User UART device structure descriptor.
 * @param [out] data - Pointer to data container.
 * @param [out] rdy  - 1 if a character has been received;
 * 		  	         - 0 if no character has been received.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_read_nb(struct uart_desc *desc, volatile uint8_t *data,
			 int16_t size, volatile uint8_t *rdy)
{
	*rdy = 0;
#if defined(MODBUS_INTERFACE)
	uint8_t i, j = 0;

	if(size == 0)
		return 0;

	if(index_user_equal_app(user_index, app_index) ||
	    index_user_greater_app(add_index(user_index, size), app_index))
		return 0;

	for(i = user_index.index; i != add_index(user_index, size).index; i++) {
		data[j++] = ring_buff[i];
	}

	user_index = add_index(user_index, size);
	*rdy = 1;

	return 0;
#elif defined(CLI_INTEFACE)
	if(uart_ping_flag == 2) {
		*data = *ping_ptr;
		*rdy = 1;
		uart_ping_flag = 0;
		return 0;
	}
	if(uart_pong_flag == 2) {
		*data = *pong_ptr;
		*rdy = 1;
		uart_pong_flag = 0;
		return 0;
	}

	return 0;
#endif
}

/**
 * Wait for and read a character in a blocking call.
 *
 * @param [in] desc  - User UART device structure descriptor.
 * @param [out] data - Pointer to data container.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_read_char(struct uart_desc *desc, uint8_t *data)
{
	const uint32_t buf_size = 1;
	uint32_t error;

	/* Submit buffer for read */
	return adi_uart_Read((ADI_UART_HANDLE const)h_uart_device, data, buf_size,
			     DMA_NOT_USE, &error);
}

/**
 * Stop RX transfers and flush the buffers.
 *
 * Used to reset the channel.
 *
 * @param [in] desc - UART device handler.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_flush_rx_buffers(struct uart_desc *desc)
{
	return adi_uart_FlushRxChannel((ADI_UART_HANDLE const)h_uart_device);
}

#if defined(MODBUS_INTERFACE)
/**
 * Read a string of characters from UART in non-blocking call.
 *
 * The functions exits on reading the requested number of bytes or on timeout.
 *
 * @param [in] desc  - UART device handler.
 * @param [out] data - Pointer to the read data.
 * @param [in] size  - Number of bytes to read.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_read_string(struct uart_desc *desc, volatile uint8_t *data,
			     int16_t size)
{
	uint8_t i, j = 0;
	uint32_t timeout = UART_REC_TIMEOUT;

	if(size == 0)
		return 0;

	while(index_user_equal_app(user_index, app_index) ||
	      index_user_greater_app(add_index(user_index, size), app_index)) {
		timeout--;
		if(timeout == 0)
			break;
	}

	if(timeout == 0)
		return UART_REC_TIMEOUT_RET;

	for(i = user_index.index; i != add_index(user_index, size).index; i++) {
		data[j++] = ring_buff[i];
	}
	user_index = add_index(user_index, size);

	return 0;
}

/**
 * Update the user ring buffer pointer to match the application received
 * pointer.
 *
 * This is done to effectively ignore all UART input from the moment of calling
 * this. This is done so that the MODBUS slave does not hang in incomplete
 * commands from the master or the responses of other slaves.
 *
 * @param [in] desc  - UART device handler.
 *
 * @return void
 */
void usr_uart_ignore_input(struct uart_desc *desc)
{
	user_ptr->index = app_ptr->index;
	user_ptr->oddpass = app_ptr->oddpass;
}
#endif
/**
 * Initialize flash controller.
 *
 * @param none.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t flash_init()
{
	int32_t ret;

	/* Open flash device */
	ret = adi_fee_Open(0, fee_device_mem, ADI_FEE_MEMORY_SIZE, &h_fee_device);
	if(ret < 0)
		return ret;

	/* Get the start page */
	ret = adi_fee_GetPageNumber(h_fee_device, FLASH_PAGE_ADDR, &nStartPage);
	if(ret < 0)
		return ret;

	/* Get the end page */
	ret = adi_fee_GetPageNumber(h_fee_device,
				    (FLASH_PAGE_ADDR + FLASH_BUFFER_SIZE - 1),
				    &nEndPage);
	if(ret < 0)
		return ret;

	/* Get block number */
	return adi_fee_GetBlockNumber(h_fee_device, FLASH_PAGE_ADDR, &nBlockNum);
}

/**
 * Write data in flash memory.
 *
 * @param [in] array 	  - Pointer to the data to be written.
 * @param [in] array_size - Size of the written data.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t flash_write(uint32_t *array, uint32_t array_size)
{
	ADI_FEE_TRANSACTION transaction;
	uint32_t i;
	int32_t ret;

	transaction.bUseDma = false;
	transaction.nSize = array_size * sizeof(*array);
	transaction.pWriteAddr = page_address;
	transaction.pWriteData = fee_data_buffer;

	for(i = 0; i < array_size; i++)
		fee_data_buffer[i] = array[i];

	/* First erase page */
	ret = adi_fee_PageErase(h_fee_device, nStartPage, nEndPage,
				&fee_hardware_error);
	if(ret < 0)
		return ret;

	/* Then write */
	return adi_fee_Write(h_fee_device, &transaction, &fee_hardware_error);
}

/**
 * Read data from the flash memory.
 *
 * @param [out] array - Pointer to the container for the read data.
 * @param [in]  size  - Size of the read data.
 *
 * @return void
 */
void flash_read(uint32_t *array, uint32_t size)
{
	uint32_t *data_in_flash = (uint32_t *)page_address;
	uint32_t *data		    = array;

	for (uint32_t i = 0; i < size; i++)
		*data++ = *data_in_flash++;
}
