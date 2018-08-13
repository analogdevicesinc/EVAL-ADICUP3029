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

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
NON-INFRINGEMENT, TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF
CLAIMS OF INTELLECTUAL PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_


/******************************** Internal defines ****************************/
#define UART_TX_BUFFER_SIZE      1024       // UART transmit buffer size
#define UART_RX_BUFFER_SIZE      256        // UART receive buffer size

/* Execution status */
#define UART_SUCCESS			0
#define UART_FAILURE            -1
#define UART_NO_TX_SPACE        -2
#define UART_NO_RX_SPACE        -3
#define UART_INIT_FAILURE		-4

#define SPI_SUCCESS				0
#define SPI_FAILURE				-1

/* SPI configuration constants */
/* SPI phase */
#define SPI_PHASE_TRAILING_EDGE	true
#define SPI_PHASE_LEADING_EDGE	false
/* SPI polarity */
#define SPI_POLARITY_IDLE_HIGH	true
#define SPI_POLARITY_IDLE_LOW	false
/* SPI transmit underflow */
#define SPI_SEND_ZERO			true
#define SPI_SEND_LAST_BYTE		false
/* SPI receive overflow */
#define SPI_REC_OVERWRITE		true
#define SPI_REC_DISCARD			false
/* SPI operation mode */
#define SPI_CONT_MODE			true
#define SPI_DISCONT_MODE		false

/* SPI chosen baud rate */
#define SPI_BAUDRATE			4000000 /* 4MHz */

/* UART DMA usage */
#define DMA_USE		true
#define DMA_NOT_USE false

/* UART status */
#define UART_TRUE                1
#define UART_FALSE               0

#define _CR                      13      /* <ENTER> */
#define _LF                      10      /* <New line> */
#define _SPC                     32      /* <Space> */
#define _BS                      8       /* <Backspace> */

#define READ_DATA_REG            0x58
/****************************** Global Data ***********************************/

extern uint32_t           uart_rpos, uart_rcnt, uart_tpos, uart_tcnt;
extern uint32_t           uart_echo, uart_cmd, uart_ctrlc, uart_tbusy;

extern uint8_t          uart_rx_buffer[UART_RX_BUFFER_SIZE];
extern uint8_t          uart_tx_buffer[UART_TX_BUFFER_SIZE];

/****************************** Internal types ********************************/

/* Number of received bytes */
enum en_read_bytes {
    ONE_BYTE = 1,                 /* Read one byte */
    TWO_BYTES                /* Read two bytes */
};


/* Write data mode */
enum en_write_data {
    UART_WRITE_NO_INT = 1, /* Write data when interrupts are disabled */
    UART_WRITE_IN_INT	   /* Write data while in an interrupt routine */
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

/* Write data mode */
enum en_spi_write {
    SPI_WRITE_REG = 1,      /* Write data to ADC */
    SPI_WRITE_RESET        /* Write RESET to ADC */
};

enum en_spi_read {
    SPI_READ_REG = 1,            /* Read one ADC register */
    SPI_READ_DATA               /* Read  ADC registers for three bytes */
};

enum en_spi_channel {
    SPI_ARDUINO,	// SPI0 - used for ARDUINO connector on ADICUP3029 board
    SPI_PMOD,	// SPI1 - used for PMOD connector on ADICUP3029 board
    SPI_BLE		// SPI2 - used to send BLE commands to EM9304
};

enum spi_irqmode {
    spi_mode_tx1rx1 = 0,
    spi_mode_tx2rx2,
    spi_mode_tx3rx3,
    spi_mode_tx4rx4,
    spi_mode_tx5rx5,
    spi_mode_tx6rx6,
    spi_mode_tx7rx7,
    spi_mode_tx8rx8
};

/*************************** Functions prototypes *****************************/
int8_t SPI_Init(void);
//uint32_t SPI_Read(uint8_t ui8address, enum en_spi_read enRegs);
uint32_t SPI_Read(uint8_t ui8address, uint8_t ui8bytes);
int8_t SPI_Write(uint8_t ui8address, uint32_t ui32data, uint8_t ui8bytes);
int8_t UART_Init(enum uart_baudrate baudrate,
                 uint8_t iBits);
int8_t UART_ReadChar(uint8_t *data);
int8_t UART_WriteChar(uint8_t data,
                      enum en_write_data mode);

/**************************** Configuration parameters ************************/

#endif /* _COMMUNICATION_H_ */
