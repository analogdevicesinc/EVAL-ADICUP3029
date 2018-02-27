/*!
 *****************************************************************************
 * @file:    Communication.h
 * @brief:
 * @version: $Revision$
 * @date:    $Date$
 *-----------------------------------------------------------------------------
 *
Copyright (c) 2015-2017 Analog Devices, Inc.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Modified versions of the software must be conspicuously marked as such.
  - This software is licensed solely and exclusively for use with processors
    manufactured by or for Analog Devices, Inc.
  - This software may not be combined or merged with other code in any manner
    that would cause the software to become subject to terms and conditions
    which differ from those listed here.
  - Neither the name of Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.
  - The use of this software may or may not infringe the patent rights of one
    or more patent holders.  This license does not release you from the
    requirement that you obtain separate licenses from these patent holders
    to use this software.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF CLAIMS OF INTELLECTUAL
PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include <stdint.h>
#include "drivers/uart/adi_uart.h"
#include "drivers/pwr/adi_pwr.h"
#include "drivers/dma/adi_dma.h"

/* The serial port may be connected to P0.1, P0.2 or to P0.6, P0.7 */

/* UART pins */
#define UART_PINS_12            1  /* Connected to P0.1, P0.2 */
#define UART_PINS_67            2  /* Connected to P0.6, P0.7 */

/* ------------------------------------------------------------------------- */

/* UART available modes */
#define UART_INT_MODE         1 /* Enables using both RX & TX interrupts */
#define UART_TX_INT_MODE      2 /* Enables using TX interrupts */
#define UART_RX_INT_MODE      3 /* Enables using RX interrupts */


#define UART_PINS    UART_PINS_12

/* The serial port may be used in polling or interrupt mode */
#define UART_MODE UART_INT_MODE

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

/* Write data mode */
enum en_write_data {
   UART_WRITE_NO_INT = 1,            /* Write data when interrupts are disabled */
   UART_WRITE_IN_INT               /* Write data while in an interrupt routine */
};

enum uart_baudrate {
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
};

/* Buffer size for UART Tx and Rx */
#define UART_TX_BUFFER_SIZE      1024       // UART transmit buffer size
#define UART_RX_BUFFER_SIZE      256        // UART receive buffer size

extern unsigned int		uart_rpos, uart_rcnt, uart_tpos, uart_tcnt;
extern unsigned int		uart_echo, uart_cmd, uart_ctrlc, uart_tbusy;

extern unsigned char	uart_rx_buffer[UART_RX_BUFFER_SIZE];
extern unsigned char	uart_tx_buffer[UART_TX_BUFFER_SIZE];


/* Initializes the UART communication peripheral. */
int16_t UART_Init(enum uart_baudrate baudrate,
		uint8_t iBits);

/* Reads one character from UART.. */
int16_t UART_ReadChar(uint8_t *data);

/* Writes one character to UART. */
int16_t UART_WriteChar(uint8_t data,
		enum en_write_data mode);

/* Writes one character string to UART. */
int16_t UART_WriteString(uint8_t* string);

#endif /* _COMMUNICATION_H_ */
