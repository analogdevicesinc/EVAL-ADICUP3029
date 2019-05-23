/***************************************************************************//**
 *   @file   platform_drivers.h
 *   @brief  Header file of Generic Platform Drivers.
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

#ifndef PLATFORM_DRIVERS_H_
#define PLATFORM_DRIVERS_H_

#include <drivers/gpio/adi_gpio.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define CN415_SUCCESS		0
#define CN415_FAILURE		-1

#define	SPI_CPHA	0x01
#define	SPI_CPOL	0x02

#define GPIO_OUT	0x01
#define GPIO_IN		0x00

#define GPIO_HIGH	0x01
#define GPIO_LOW	0x00

/* UART device number */
#define UART_DEVICE_NUM 0u
/* Execution status */
#define UART_SUCCESS             0
#define UART_FAILURE            -1
#define UART_NO_TX_SPACE        -2
#define UART_NO_RX_SPACE        -3
#define UART_INIT_FAILURE		-4
/* UART status */
#define UART_TRUE               1
#define UART_FALSE              0

#define DMA_USE		true
#define DMA_NOT_USE false

#define FLASH_BUFFER_SIZE 2048
#define FLASH_PAGE_ADDR   0X20000

#define TMR0_INT TMR0_EVT_IRQn
#define TMR1_INT TMR1_EVT_IRQn
#define TMR2_INT TMR2_EVT_IRQn
#define HART_CD_INT SYS_GPIO_INTB_IRQn
#define RX_INT SYS_GPIO_INTA_IRQn
#define UART_INT UART_EVT_IRQn
#define UART_DMA_INT DMA0_CH9_DONE_IRQn

#define UART_REC_TIMEOUT_RET -6
#define UART_REC_TIMEOUT 26000

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

typedef enum i2c_type {
	GENERIC_I2C
} i2c_type;

typedef struct i2c_init_param {
	enum i2c_type type;
	uint32_t	  id;
	uint32_t	  max_speed_hz;
	uint8_t		  slave_address;
} i2c_init_param;

typedef struct i2c_desc {
	enum i2c_type type;
	uint32_t	  id;
	uint32_t	  max_speed_hz;
	uint8_t		  slave_address;
} i2c_desc;

typedef enum spi_type {
	ADICUP3029_SPI
} spi_type;

typedef enum spi_mode {
	SPI_MODE_0 = (0 | 0),
	SPI_MODE_1 = (0 | SPI_CPHA),
	SPI_MODE_2 = (SPI_CPOL | 0),
	SPI_MODE_3 = (SPI_CPOL | SPI_CPHA)
} spi_mode;

typedef enum en_spi_channel {
	SPI_ARDUINO,	/* SPI0 - used for ARDUINO connector on ADICUP3029 board */
	SPI_PMOD,	/* SPI1 - used for PMOD connector on ADICUP3029 board */
	SPI_BLE		/* SPI2 - used to send BLE commands to EM9304 */
} en_spi_channel;

typedef struct spi_init_param {
	enum spi_type type;
	uint32_t	  id;
	uint32_t	  max_speed_hz;
	enum spi_mode mode;
	uint8_t		  chip_select;
} spi_init_param;

typedef struct spi_desc {
	enum spi_type type;
	uint32_t	  id;
	uint32_t	  max_speed_hz;
	enum spi_mode mode;
	struct gpio_desc *chip_select;
} spi_desc;

typedef enum gpio_type {
	ADICUP3029_GPIO
} gpio_type;

typedef struct gpio_desc {
	enum gpio_type type;
	uint32_t  id;
	uint8_t	  number;
} gpio_desc;

typedef enum uart_baudrate {
	bd9600,
	bd19200,
	bd38400,
	bd57600,
	bd115200,
	bd230400,
	bd460800,
	bd921600,
	bd1000000,
	bd1500000
} uart_baudrate;

typedef enum uart_en_write_data {
	UART_WRITE_NO_INT = 1, /* Write data in blocking mode */
	UART_WRITE_IN_INT,	  /* Write data in non-blocking mode */
	UART_WRITE_CALLBK      /* Write data in callback mode */
} uart_en_write_data;

typedef struct uart_init_param {
	enum uart_baudrate baudrate;
	uint8_t bits_no;
} uart_init_param;

typedef struct uart_desc {
	enum uart_baudrate baudrate;
	uint8_t bits_no;
} uart_desc;

typedef struct rb_index {
	uint8_t index;
	uint8_t oddpass;
} rb_index;

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initialize the I2C communication peripheral. */
int32_t i2c_init(i2c_desc **desc, const i2c_init_param *param);

/* Free the resources allocated by i2c_init(). */
int32_t i2c_remove(i2c_desc *desc);

/* Write data to a slave device. */
int32_t i2c_write(i2c_desc *desc, uint8_t *data, uint8_t bytes_number,
		  uint8_t stop_bit);

/* Read data from a slave device. */
int32_t i2c_read(i2c_desc *desc, uint8_t *data, uint8_t bytes_number,
		 uint8_t stop_bit);

/* Set address that the device is sending before a transaction. */
int32_t i2c_set_address(i2c_desc *desc, uint8_t new_address);

/* Initialize the SPI communication peripheral. */
int32_t spi_init(spi_desc **desc, const spi_init_param *param);

/* Free the resources allocated by spi_init() */
int32_t spi_remove(spi_desc *desc);

/* Write and read data to/from SPI. */
int32_t spi_write_and_read(spi_desc *desc, uint8_t *data, uint8_t bytes_number);

/* Change SPI mode. */
int32_t spi_change_mode(spi_desc *desc);

/* Obtain the GPIO decriptor. */
int32_t gpio_get(gpio_desc **desc, uint8_t gpio_number);

/* Free the resources allocated by gpio_get() */
int32_t gpio_remove(gpio_desc *desc);

/* Enable the input direction of the specified GPIO. */
int32_t gpio_direction_input(gpio_desc *desc);

/* Enable the output direction of the specified GPIO. */
int32_t gpio_direction_output(gpio_desc *desc, uint8_t value);

/* Set the value of the specified GPIO. */
int32_t gpio_set_value(gpio_desc *desc, uint8_t value);

/* Get the value of the specified GPIO. */
int32_t gpio_get_value(gpio_desc *desc, uint16_t *value);

/* Generate miliseconds delay. */
void mdelay(uint32_t msecs);

/* Initialize the UART communication peripheral. */
int32_t usr_uart_init(struct uart_desc **descriptor,
		      struct uart_init_param init_param);

/* Free the resources allocated by uart_init() */
int32_t usr_uart_remove(struct uart_desc *desc);

/* Write one character to the UART */
int32_t usr_uart_write_char(struct uart_desc *desc, uint8_t data,
			    enum uart_en_write_data mode);

/* Poll TX status to make a non-blocking write call blocking. */
void usr_uart_poll_tx(struct uart_desc *desc);

/* Write a string of characters to the UART */
int32_t usr_uart_write_string(struct uart_desc *desc,
			      uint8_t *string);

/* Read one character from the UART */
int32_t usr_uart_read_char(struct uart_desc *desc, uint8_t *data);

/* Read character from UART non-blocking */
int32_t usr_uart_read_nb(struct uart_desc *desc, volatile uint8_t *data,
			 int16_t size, volatile uint8_t *rdy);

/* Stop RX transfers and flush the buffers. */
int32_t usr_uart_flush_rx_buffers(struct uart_desc *desc);

/* Read a string of characters from UART in non-blocking call. */
int32_t usr_uart_read_string(struct uart_desc *desc, volatile uint8_t *data,
			     int16_t size);

/* Update the user ring buffer pointer to match the application received
 * pointer. */
void usr_uart_ignore_input(struct uart_desc *desc);

/* Initialize flash controller */
int32_t flash_init();

/* Write to the flash */
int32_t flash_write(uint32_t *array, uint32_t array_size);

/* Read from the flash */
void flash_read(uint32_t *array, uint32_t size);

#endif // PLATFORM_DRIVERS_H_
