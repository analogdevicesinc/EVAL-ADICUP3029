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
#include "platform_drivers.h"

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

/* Handle for UART device */
ADI_UART_HANDLE h_uart_device __attribute__ ((aligned (4)));
/* Memory for  UART driver */
uint8_t uart_device_mem[ADI_UART_BIDIR_MEMORY_SIZE]
__attribute__ ((aligned (4)));
/* User UART memory */
uint8_t uart_current_line[256];
uint8_t uart_previous_line[256];
uint8_t uart_line_index = 0;
uint8_t uart_cmd = 0;
uint8_t uart_flag = 0;
uint8_t uart_ping_flag = 0;
uint8_t uart_pong_flag = 0;
uint8_t ping, pong;
uint8_t *ping_ptr = &ping;
uint8_t *pong_ptr = &pong;

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

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Initialize the SPI communication peripheral.
 *
 * @param [out] desc       - The SPI descriptor.
 * @param [in]  init_param - The structure that contains the SPI parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t spi_init(struct spi_desc **desc, struct spi_init_param param)
{
	/* Variable for storing the return code from SPI device */
	int32_t ret;
	struct spi_desc *dev;

	dev = (struct spi_desc *)malloc(sizeof(*dev));
	if (!dev)
		return FAILURE;

	dev->chip_select = param.chip_select;
	dev->id = param.id;
	dev->max_speed_hz = param.max_speed_hz;
	dev->mode = param.mode;
	dev->type = param.type;

	ret = adi_spi_Open(dev->id, master_spi_device_mem, ADI_SPI_MEMORY_SIZE,
			   &h_spi_device);
	if(ret != ADI_SPI_SUCCESS)
		goto error;

	ret = adi_spi_SetBitrate(h_spi_device, dev->max_speed_hz);
	if(ret != ADI_SPI_SUCCESS)
		goto error;

	/* Set the chip select. */
	/* If no GPIO has been chosen use hardware CS */
	if (dev->chip_select != 0xFF) {
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
		ret = adi_spi_SetChipSelect(h_spi_device, ADI_SPI_CS_NONE);
		if(ret != ADI_SPI_SUCCESS)
			goto error;
	}
	ret = adi_spi_SetContinuousMode(h_spi_device, true);
	if(ret != ADI_SPI_SUCCESS)
		goto error;

	ret = adi_spi_SetIrqmode(h_spi_device, true);
	if(ret != ADI_SPI_SUCCESS)
		goto error;

	ret = adi_spi_SetClockPolarity(h_spi_device, (dev->mode & 0x02) >> 1);
	if(ret != ADI_SPI_SUCCESS)
		goto error;

	ret = adi_spi_SetClockPhase(h_spi_device, (dev->mode & 0x01) >> 0);
	if(ret != ADI_SPI_SUCCESS)
		goto error;

	*desc = dev;

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

	ret = adi_spi_Close(h_spi_device);
	if(ret != ADI_SPI_SUCCESS)
		return ret;

	free(desc);

	return ret;
}

/**
 * Write and read data to/from SPI.
 *
 * @param [in]     desc         - The SPI descriptor.
 * @param [in/out] data         - The buffer with the transmitted/received data.
 * @param [in]     bytes_number - Number of bytes to write/read.
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

	/* Wait till the data transmission is over */
	ret = adi_spi_MasterReadWrite(h_spi_device, &m_spi_transceive);
	if(ret != ADI_SPI_SUCCESS)
		return ret;

	return ret;
}

/**
 * Get port and pin number from the GPIO number.
 *
 * Return the port and pin numbers from the GPIO number to use with the
 * ADuCM3029 drivers.
 *
 * @param [in]  desc - GPIO descriptor.
 * @param [out] pin  - Pin number.
 * @param [out] port - Port number.
 *
 * @return void
 */
static void gpio_get_portpin(gpio_desc *desc, uint8_t *port, uint16_t *pin)
{
	*pin = (ADI_GPIO_DATA)(0x0001 << (desc->number & 0x0F));
	*port = (desc->number & 0xF0) >> 4;
}

/**
 * Obtain the GPIO decriptor.
 *
 * @param [out] desc        - The GPIO descriptor.
 * @param [in]  gpio_number - The number of the GPIO.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_get(gpio_desc **desc, uint8_t gpio_number)
{
	int32_t ret = CN415_SUCCESS;
	struct gpio_desc *dev;

	dev = malloc(sizeof(*dev));
	if (!dev)
		return FAILURE;

	dev->type   = ADICUP3029_GPIO;
	dev->id     = 0;
	dev->number = gpio_number;

	/* If this is the first GPIO initialize GPIO controller */
	if(gpio_init_flag == 0)
		ret = adi_gpio_Init(mem_gpio_handler, ADI_GPIO_MEMORY_SIZE);
	if(ret != ADI_GPIO_SUCCESS)
		goto error;

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
	int32_t ret = CN415_SUCCESS;

	free(desc);

	/* Decrement number of GPIOs */
	gpio_init_flag--;

	/* If no more GPIOs free driver memory */
	if (gpio_init_flag == 0)
		ret = adi_gpio_UnInit();

	return ret;
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

	gpio_get_portpin(desc, &port, &pin);

	return adi_gpio_InputEnable(port, pin, true);
}

/**
 * Enable the output direction of the specified GPIO.
 *
 * @param [in] desc  - The GPIO descriptor.
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

	gpio_get_portpin(desc, &port, &pin);

	ret = adi_gpio_OutputEnable(port, pin, true);
	if(ret != ADI_GPIO_SUCCESS)
		return ret;

	/* Initialize pin with a value */
	if(value == GPIO_HIGH)
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

	gpio_get_portpin(desc, &port, &pin);

	if(value == GPIO_HIGH)
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

	gpio_get_portpin(desc, &port, &pin);

	return adi_gpio_GetData(port, pin, value);
}

/**
 * Generate miliseconds delay.
 *
 * @param [in] msecs - Delay in miliseconds.
 *
 * @return void
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

	desc = malloc(sizeof(*desc));
	if (!desc)
		return FAILURE;

	desc->baudrate = init_param.baudrate;
	desc->bits_no = init_param.bits_no;

	/* Values for the baudrate generation registers when the baudrate is one of
	 * the standard values and clock is 26MHz */
	const uint8_t baudrate_osr_26mhz[] = {3, 3, 3, 3, 3, 3, 3, 2, 2, 2};
	const uint8_t baudrate_divc_26mhz[] = {24, 12, 8, 4, 4, 2, 1, 1, 1, 1};
	const uint8_t baudrate_divm_26mhz[] = {3, 3, 2, 3, 1, 1, 1, 1, 1, 1};
	const uint16_t baudrate_divn_26mhz[] = {1078, 1078, 1321, 1078, 1563, 1563,
						1563, 1563, 1280, 171};

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

	/* Baud rate div values are calculated for PCLK 26Mhz */
	ret = adi_uart_ConfigBaudRate(h_uart_device,
				      baudrate_divc_26mhz[desc->baudrate],
				      baudrate_divm_26mhz[desc->baudrate],
				      baudrate_divn_26mhz[desc->baudrate],
				      baudrate_osr_26mhz[desc->baudrate]);
	if(ret != ADI_UART_SUCCESS)
		goto error;

	*descriptor = desc;

	return CN415_SUCCESS;
error:
	free(desc);

	return CN415_FAILURE;
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
		return CN415_FAILURE;

	ret = adi_uart_Close(h_uart_device);
	if(ret != ADI_UART_SUCCESS)
		return ret;

	free(desc);

	return CN415_SUCCESS;
}

/**
 * Transmit a character through UART.
 *
 * @param [in] desc - User UART device structure descriptor.
 * 		  [in] data - Data to be transmitted.
 * 		  [in] mode - Mode of transmission (blocking or non-blocking)
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_write_char(struct uart_desc *desc, uint8_t data,
			    enum uart_en_write_data mode)
{
	bool tx_available = false;
	uint32_t hw_error;
	const uint32_t buf_size = 1;

	/* Send character with interrupt */
	if(mode == UART_WRITE_IN_INT) {
		/* Check if not busy */
		adi_uart_IsTxBufferAvailable((ADI_UART_HANDLE const)h_uart_device,
					     (bool* const)&tx_available);
		if (!tx_available)
			return UART_NO_TX_SPACE;
		/* Start transmission */
		return adi_uart_SubmitTxBuffer((ADI_UART_HANDLE const)h_uart_device,
					       (void* const)&data, buf_size, DMA_NOT_USE);

		/* Send character without interrupt (blocking) */
	} else {
		/* Do transmission */
		return adi_uart_Write((ADI_UART_HANDLE const)h_uart_device,
				      (void* const)&data, buf_size, DMA_NOT_USE, &hw_error);
	}
}

/**
 * Transmit an array of characters through UART.
 *
 * @param [in] desc   - User UART device structure descriptor.
 * 		  [in] string - Pointer to the data array to be transmitted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_write_string(struct uart_desc *desc, uint8_t *string)
{
	int32_t ret = CN415_SUCCESS;

	while(*string != '\0') {
		ret = usr_uart_write_char(desc, *string++, UART_WRITE_NO_INT);

		if(ret != CN415_SUCCESS)
			break;
	}

	return ret;
}

/**
 * Wait for and read a character in a non-blocking call.
 *
 * @param [in]  desc - User UART device structure descriptor.
 * 		  [out] data - Pointer to data container.
 * 		  [out] rdy  - 1 if a character has been received;
 * 		  	         - 0 if no character has been received.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_read_nb(struct uart_desc *desc, uint8_t *data, uint8_t *rdy)
{
	const uint32_t buf_size = 1;
	int32_t ret;
	uint32_t error;
	bool ready;

	/* Submit buffer for read */
	if (uart_ping_flag == 0) {
		ret = adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
					      ping_ptr, buf_size, DMA_NOT_USE);
		if(ret != 0)
			return ret;
		uart_ping_flag = 1;
	}
	if (uart_pong_flag == 0) {
		ret = adi_uart_SubmitRxBuffer((ADI_UART_HANDLE const)h_uart_device,
					      pong_ptr, buf_size, DMA_NOT_USE);
		if(ret != 0)
			return ret;
		uart_pong_flag = 1;
	}

	/* If a character has been received get it */
	ret = adi_uart_IsRxBufferAvailable(h_uart_device, &ready);
	if(ret != 0)
		return ret;
	if(ready) {
		if (uart_flag == 0) {
			ret = adi_uart_GetRxBuffer(h_uart_device, (void **const)&ping_ptr,
						   &error);
			if(ret != 0)
				return ret;
			*data = *ping_ptr;
			uart_ping_flag = 0;
			uart_flag = 1;
			*rdy = ready;
			return ret;
		}
		if (uart_flag == 1) {
			ret = adi_uart_GetRxBuffer(h_uart_device, (void **const)&pong_ptr,
						   &error);
			if(ret != 0)
				return ret;
			*data = *pong_ptr;
			uart_pong_flag = 0;
			uart_flag = 0;
			*rdy = ready;
			return ret;
		}
	}

	return ret;
}

/**
 * Wait for and read a character in a blocking call.
 *
 * @param [in]  desc - User UART device structure descriptor.
 * 		  [out] data - Pointer to data container.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t usr_uart_read_char(struct uart_desc *desc, uint8_t *data)
{
	const uint32_t buf_size = 1;
	uint32_t error;
	int32_t ret;

	/* If already in program */
	if(uart_ping_flag == 1 || uart_pong_flag == 1) {
		/* Inactivate ping pong buffers */
		/* Prompt for a character to clear remaining flying buffer */
		ret = usr_uart_write_string(desc,
					    (uint8_t*)"Start calibration sequence. Press enter to start typing...");
		if(ret != 0)
			return ret;
		ret = adi_uart_GetRxBuffer(h_uart_device, (void **const)&ping_ptr,
					   &error);
		if(ret != 0)
			return ret;
		ret = adi_uart_GetRxBuffer(h_uart_device, (void **const)&pong_ptr,
					   &error);
		if(ret != 0)
			return ret;

		/* Reset UART software system */
		uart_ping_flag = 0;
		uart_pong_flag = 0;
		uart_flag = 0;

		/* Escape character to clear active line */
		ret = usr_uart_write_string(desc, (uint8_t*)"\33[2K\r");
		if(ret != 0)
			return ret;
		/* Show the comforting line starter */
		ret = usr_uart_write_string(desc, (uint8_t*)">");
		if(ret != 0)
			return ret;
	}

	/* Submit buffer for read */
	return adi_uart_Read((ADI_UART_HANDLE const)h_uart_device, data, buf_size,
			     DMA_NOT_USE, &error);
}

/**
 * Initialize flash controller.
 *
 * @param void
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t flash_init()
{
	int32_t ret;

	/* Open flash device */
	ret = adi_fee_Open(0, fee_device_mem, ADI_FEE_MEMORY_SIZE, &h_fee_device);
	if(ret != ADI_FEE_SUCCESS)
		return ret;

	/* Get the start page */
	ret = adi_fee_GetPageNumber(h_fee_device, FLASH_PAGE_ADDR, &nStartPage);
	if(ret != ADI_FEE_SUCCESS)
		return ret;

	/* Get the end page */
	ret = adi_fee_GetPageNumber(h_fee_device,
				    (FLASH_PAGE_ADDR + FLASH_BUFFER_SIZE - 1), &nEndPage);
	if(ret != ADI_FEE_SUCCESS)
		return ret;

	/* Get block number */
	return adi_fee_GetBlockNumber(h_fee_device, FLASH_PAGE_ADDR, &nBlockNum);
}

/**
 * Write data in flash memory.
 *
 * @param [in] array      - Pointer to the data to be written.
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
	if(ret != ADI_FEE_SUCCESS)
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
