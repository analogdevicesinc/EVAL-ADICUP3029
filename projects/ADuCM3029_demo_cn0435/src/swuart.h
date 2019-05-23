/***************************************************************************//**
*   @file   swuart.h
*   @brief  Software UART driver header.
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

#ifndef SWUART_H_
#define SWUART_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "drivers/gpio/adi_gpio.h"
#include "drivers/tmr/adi_tmr.h"
#include <stdint.h>

#define udelay timer_sleep
#define SECOND_LOAD_TIMER 100000

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
enum swuart_type {
	ADICUP3029_SWUART
};

/*
 * Struct describing port and pin numbers for ADuCM3029
 * 		Example: port_number = ADI_GPIO_PORT0,
 *  						   ADI_GPIO_PORT1,
 *  						   ADI_GPIO_PORT2.
 *
 *  			 pin_number  = ADI_GPIO_PIN_0,
 *  			 			   ADI_GPIO_PIN_1,
 * 							   ADI_GPIO_PIN_2,
 * 							   ADI_GPIO_PIN_3,
 * 							   ADI_GPIO_PIN_4,
 * 							   ADI_GPIO_PIN_5,
 * 							   ADI_GPIO_PIN_6,
 * 							   ADI_GPIO_PIN_7,
 * 							   ADI_GPIO_PIN_8,
 * 							   ADI_GPIO_PIN_9,
 * 							   ADI_GPIO_PIN_10,
 * 							   ADI_GPIO_PIN_11,
 * 							   ADI_GPIO_PIN_12,
 * 							   ADI_GPIO_PIN_13,
 * 							   ADI_GPIO_PIN_14,
 * 							   ADI_GPIO_PIN_15.
 */
struct gpio_port_pin {
	ADI_GPIO_PORT port_number;
	uint32_t pin_number;
};

/* UART parity type */
enum uart_parity {
	UART_EVEN_PARITY,
	UART_ODD_PARITY,
	UART_NO_PARITY
};

/* UART number of bits */
enum uart_bit_no {
	UART_5_BITS = 5,
	UART_6_BITS,
	UART_7_BITS,
	UART_8_BITS
};

/* Software UART device descriptor initializer */
struct swuart_init {
	uint8_t id;
	enum swuart_type type;
	/* RX pin description */
	union {
		struct gpio_port_pin rx_port_pin;
		uint32_t rx_pin_number;
	};
	/* TX pin description */
	union {
		struct gpio_port_pin tx_port_pin;
		uint32_t tx_pin_number;
	};
	uint32_t baudrate;
	enum uart_parity parity;
	uint8_t no_of_bits;
	ADI_TMR_DEVICE delay_timer;
};

/* Software UART device descriptor */
struct swuart_dev {
	uint8_t id;
	enum swuart_type type;
	/* RX pin description */
	union {
		struct gpio_port_pin rx_port_pin;
		uint32_t rx_pin_number;
	};
	/* TX pin description */
	union {
		struct gpio_port_pin tx_port_pin;
		uint32_t tx_pin_number;
	};
	uint32_t baudrate;
	enum uart_parity parity;
	uint8_t no_of_bits; /* Number of bits per byte */
	ADI_TMR_DEVICE delay_timer; /* ID of the timer used for the delay */
	uint16_t half_period;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initializes the software UART. */
int32_t swuart_init(struct swuart_dev **device, struct swuart_init *init_param);

/* Free the resources allocated by swuart_init(). */
int32_t swuart_remove(struct swuart_dev *dev);

/* The function implements a read/write operation through software UART started
 * by the application. The UART transmit considers the parameters number of bits
 * per byte and parity. */
int32_t swuart_write_char(struct swuart_dev *dev, uint8_t *sbyte);

/* The function implements a read/write operation through software UART started
 * by another device and not the application. The UART transmit considers the
 * parameters number of bits per byte and parity. The main difference is that
 * this transmission waits for the start bit of the other participant before
 * giving it's own. */
int32_t swuart_read_char(struct swuart_dev *dev, uint8_t *rbyte);

/* Write a string of bytes through the software UART. */
int32_t swuart_write_string(struct swuart_dev *dev, uint8_t *string,
			    uint32_t size);

/* Read a string of bytes through the software UART. */
int32_t swuart_read_string(struct swuart_dev *dev, uint8_t *string,
			   uint32_t size);

#endif /* SWUART_H_ */
