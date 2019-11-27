/***************************************************************************//**
 *   @file   uart.h
 *   @author Andrei Drimbarean (Andrei.Drimbarean@analog.com)
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

#ifndef PLATFORM_INCLUDE_UART_H_
#define PLATFORM_INCLUDE_UART_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

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

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

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

typedef enum uart_comm_mode {
	UART_BLOCKING,    /* Data is transmitted in blocking call */
	UART_NON_BLOCKING /* Data is transmitted in non-blocking call */
} uart_en_write_data;

typedef struct uart_init_param {
	enum uart_baudrate baudrate;
	uint8_t bits_no;
	bool has_callback;
} uart_init_param;

typedef struct uart_desc {
	enum uart_baudrate baudrate;
	uint8_t bits_no;
	bool has_callback;
} uart_desc;

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initialize the UART communication peripheral. */
int32_t usr_uart_init(struct uart_desc **descriptor,
		      struct uart_init_param *init_param);

/* Free the resources allocated by uart_init() */
int32_t usr_uart_remove(struct uart_desc *desc);

/* Write one character to the UART */
int32_t usr_uart_write_char(struct uart_desc *desc, uint8_t data,
			    enum uart_comm_mode mode);

/* Write a string of characters to the UART */
int32_t usr_uart_write_string(struct uart_desc *desc, uint8_t *string);

/* Read one character from the UART */
int32_t usr_uart_read_char(struct uart_desc *desc, uint8_t *data, uint8_t *rdy,
			   enum uart_comm_mode mode);

#endif /* PLATFORM_INCLUDE_UART_H_ */
