/*!
 *******************************************************************************
 * @file:    Communication.c
 * @brief:
 * @version: $Revision$
 * @date:    $Date$
 *------------------------------------------------------------------------------
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

/***************************** Include Files **********************************/
#include <stdio.h>
#include <stdint.h>

#include "ADuCM3029.h"
#include "drivers/spi/adi_spi.h"
#include "drivers/gpio/adi_gpio.h"
#include "drivers/uart/adi_uart.h"
#include "Timer.h"

#include "Communication.h"


/********************************* Global data ********************************/


uint8_t			uart_rx_buffer[UART_RX_BUFFER_SIZE];
uint8_t			uart_tx_buffer[UART_TX_BUFFER_SIZE];

/* Handle for UART device */
ADI_UART_HANDLE	p_uart_device_handler __attribute__ ((aligned (4)));
/* Memory for  UART driver */
uint8_t			uart_device_memory[ADI_UART_BIDIR_MEMORY_SIZE] __attribute__ ((aligned (4)));

uint32_t		uart_rpos, uart_rcnt, uart_tpos, uart_tcnt;
uint32_t		uart_echo, uart_cmd, uart_ctrlc, uart_tbusy;

/* Values for the baudrate generation registers when the baudrate is one of
 * the standard values and clock is 26MHz */
const uint8_t	baudrate_osr_26mhz[]  = {3, 3, 3, 3, 3, 3, 3, 2, 2, 2};
const uint16_t	baudrate_divc_26mhz[] = {24, 12, 8, 4, 4, 2, 1, 1, 1, 1};
const uint8_t	baudrate_divm_26mhz[] = {3, 3, 2, 3, 1, 1, 1, 1, 1, 1};
const uint16_t	baudrate_divn_26mhz[] = {1078, 1078, 1321, 1078, 1563, 1563,
                                         1563, 1563, 1280, 171
                                       };

/* Master SPI device handle */
ADI_SPI_HANDLE	p_spi_device_handler;
/* Device memory required for operate Master SPI device */
uint8_t			spi_device_memory[ADI_SPI_MEMORY_SIZE];
/* SPI transceiver telegram */
ADI_SPI_TRANSCEIVER spi_telegram;

#define SPI_DEVICE_NR SPI_PMOD
#define SPI_CS_PIN_NR ADI_SPI_CS0 /* CS pin for the PMOD connector */

/************************* Functions Definitions ******************************/

/**
   @brief SPI initialization

   @return none

**/
int8_t SPI_Init(void)
{
	ADI_SPI_RESULT status;

	/* Open SPI module */
	status = adi_spi_Open(SPI_DEVICE_NR,
	                      spi_device_memory,
	                      ADI_SPI_MEMORY_SIZE,
	                      &p_spi_device_handler);
	if(status != ADI_SPI_SUCCESS)
		return SPI_FAILURE;

	/* Set baud rate */
	status = adi_spi_SetBitrate(p_spi_device_handler,
	                            SPI_BAUDRATE);
	if(status != ADI_SPI_SUCCESS)
		return SPI_FAILURE;

	/* Set receive data on leading edge of clock */
	status = adi_spi_SetClockPhase(p_spi_device_handler,
	                               SPI_PHASE_TRAILING_EDGE);
	if(status != ADI_SPI_SUCCESS)
		return SPI_FAILURE;

	/* Set clock to be idle low */
	status = adi_spi_SetClockPolarity(p_spi_device_handler,
	                                  SPI_POLARITY_IDLE_HIGH);
	if(status != ADI_SPI_SUCCESS)
		return SPI_FAILURE;

	/* Set SPI to transmit zero on underflow */
	status = adi_spi_SetTransmitUnderflow(p_spi_device_handler,
	                                      SPI_SEND_ZERO);
	if(status != ADI_SPI_SUCCESS)
		return SPI_FAILURE;

	/* Set SPI to overwrite old data with new data on receive overflow */
	status = adi_spi_SetReceiveOverflow(p_spi_device_handler,
	                                    SPI_REC_OVERWRITE);
	if(status != ADI_SPI_SUCCESS)
		return SPI_FAILURE;

	/* Set SPI in continuous transfer mode */
	status = adi_spi_SetContinuousMode(p_spi_device_handler,
	                                   SPI_DISCONT_MODE);
	if(status != ADI_SPI_SUCCESS)
		return SPI_FAILURE;

	/* Set SPI to interrupt after 2 transmitted bytes and 2 or more received
	 * bytes */
	status = adi_spi_SetIrqmode(p_spi_device_handler,
	                            spi_mode_tx2rx2);
	if(status != ADI_SPI_SUCCESS)
		return SPI_FAILURE;

	/* Configure SPI for Arduino connectors */
	status = adi_spi_SetChipSelect(p_spi_device_handler,
	                               ADI_SPI_CS0);
	if(status != ADI_SPI_SUCCESS)
		return SPI_FAILURE;

	return SPI_SUCCESS;
}

/**
   @brief Reads a specified register address in the converter via SPI.

   @param ui8address - register address
   @param enRegs - register number

   @return reading result

**/
uint32_t SPI_Read(uint8_t ui8address, uint8_t ui8bytes)
{
	uint32_t ui32AdcCodes = 0, status;
	uint8_t size = ui8bytes + 1;
	uint8_t ui8counter, rx_buff[size];
	uint8_t tx_buff[size];
	static uint8_t ui8read_rx;

	tx_buff[0] = ui8address;
	for(ui8counter = 1; ui8counter <= ui8bytes; ui8counter++)
		tx_buff[ui8counter] = 0xAA;

	/* Configure SPI telegram */
	spi_telegram.pTransmitter		= tx_buff;
	spi_telegram.TransmitterBytes	= size; /* Transmit 1 byte of address*/
	spi_telegram.nTxIncrement		= 1; /* Address not incremented */
	spi_telegram.pReceiver 			= rx_buff;
	spi_telegram.ReceiverBytes 		= size;
	spi_telegram.nRxIncrement 		= 1; /* No data alignment */
	spi_telegram.bDMA 				= false; /* No DMA */
	spi_telegram.bRD_CTL 			= false; /* Full-duplex */

	/* Transmit register address and receive the read */
	status = adi_spi_MasterReadWrite(p_spi_device_handler,
	                                 &spi_telegram);
	if(status != ADI_SPI_SUCCESS)
		return SPI_FAILURE;

	/* Compile data in an 32 bit unsigned integer format */
	for(ui8counter = 1; ui8counter <= ui8bytes; ui8counter++) {
		ui8read_rx = rx_buff[ui8counter];
		ui32AdcCodes = (uint32_t)((ui32AdcCodes << 8) | ui8read_rx);
	}

	return ui32AdcCodes;
}

/**
   @brief Writes a register to the Converter via SPI.

   @param ui8address - ACC register address
   @param ui8Data - value to be written
   @enMode ui8Data - write mode

   @return none

**/
int8_t SPI_Write(uint8_t ui8address, uint32_t ui32data, uint8_t ui8bytes)
{
	uint8_t ui8counter, ui8write[ui8bytes+1];
	int32_t status;

	/* Configure SPI telegram */
	spi_telegram.pTransmitter		= ui8write;
	spi_telegram.TransmitterBytes	= ui8bytes+1;
	spi_telegram.nTxIncrement 		= 1; /* No data alignment */
	spi_telegram.pReceiver 			= NULL; /* No received data */
	spi_telegram.ReceiverBytes 		= 0;
	spi_telegram.nRxIncrement 		= 0;
	spi_telegram.bDMA 				= false; /* No DMA */
	spi_telegram.bRD_CTL 			= false; /* Full-duplex */

	/* Separate data into 8 bits values */
	if(ui8bytes != 4) {
		ui8write[0] = ui8address;
		for(ui8counter = 1; ui8counter <= ui8bytes; ui8counter++) {
			ui8write[ui8counter] = (ui32data >>
			                        ((ui8bytes - ui8counter) * 8));
		}

		/* Transmit data */
		status = adi_spi_MasterReadWrite(p_spi_device_handler,
		                                 &spi_telegram);
		if(status != ADI_SPI_SUCCESS)
			return SPI_FAILURE;

		/* Transmit 4 0xFF bytes */
	} else {
		for(ui8counter = 1; ui8counter <= ui8bytes; ui8counter++)
			ui8write[ui8counter - 1] = 0xFF;

		/* Transmit data */
		status = adi_spi_MasterReadWrite(p_spi_device_handler,
		                                 &spi_telegram);
		if(status != ADI_SPI_SUCCESS)
			return SPI_FAILURE;
	}
	return SPI_SUCCESS;
}

/**
  @brief UART initialization

  @param lBaudrate - UART baud rate
  @param iBits - data length (5 - 8 bits)

  @return none

**/
int8_t UART_Init(enum uart_baudrate baudrate,
                 uint8_t iBits)
{
	uint32_t status;
	const uint32_t uart_device_number = 0;
	uint8_t bits_nr;

	/* Get bits number */
	switch(iBits) {
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

	/* Configure UART module */
	/* Open UART module*/
	status = adi_uart_Open(uart_device_number,
	                       ADI_UART_DIR_BIDIRECTION,
	                       (void*)uart_device_memory,
	                       (const uint32_t)ADI_UART_BIDIR_MEMORY_SIZE,
	                       (ADI_UART_HANDLE *const)&p_uart_device_handler);
	if(status != ADI_UART_SUCCESS)
		return UART_INIT_FAILURE;
	/* Set baudrate */
	status = adi_uart_ConfigBaudRate((ADI_UART_HANDLE const)p_uart_device_handler,
	                                 baudrate_divc_26mhz[baudrate],
	                                 baudrate_divm_26mhz[baudrate],
	                                 baudrate_divn_26mhz[baudrate],
	                                 baudrate_osr_26mhz[baudrate]);
	if(status != ADI_UART_SUCCESS)
		return UART_INIT_FAILURE;
	/* Set configuration of UART transmission */
	status = adi_uart_SetConfiguration((ADI_UART_HANDLE const)p_uart_device_handler,
	                                   ADI_UART_NO_PARITY,
	                                   ADI_UART_ONE_STOPBIT,
	                                   bits_nr);
	if(status != ADI_UART_SUCCESS)
		return UART_INIT_FAILURE;

	return UART_SUCCESS;
}

/**
  @brief Writes one character to UART.

  @param data - Character to write.
  @param mode - Write mode

  @return UART_SUCCESS or error code.

**/
int8_t UART_WriteChar(uint8_t data,
                      enum en_write_data mode)
{
	bool tx_available;
	uint32_t hw_error;
	const uint32_t buf_size = 1;

	/* Send character with interrupt */
	if(mode == UART_WRITE_IN_INT) {
		/* Check if not busy */
		adi_uart_IsTxBufferAvailable((ADI_UART_HANDLE const)p_uart_device_handler,
		                             (bool* const)&tx_available);
		if (!tx_available) {
			return UART_NO_TX_SPACE;
		}
		/* Start transmission */
		adi_uart_SubmitTxBuffer((ADI_UART_HANDLE const)p_uart_device_handler,
		                        (void* const)&data,
		                        buf_size,
		                        DMA_NOT_USE);

		return UART_SUCCESS;

		/* Send character without interrupt (blocking) */
	} else {
		/* Do transmission */
		adi_uart_Write((ADI_UART_HANDLE const)p_uart_device_handler,
		               (void* const)&data,
		               buf_size,
		               DMA_NOT_USE,
		               &hw_error);
	}
	return UART_SUCCESS;
}


/**
  @brief Read character from UART.

  @param data - data that is received.

  @return none

**/
int8_t UART_ReadChar(uint8_t *data)
{
	int32_t status;
	const uint32_t buf_size = 1;
	uint32_t error;
	/* Read data from UART */
	status = adi_uart_Read((ADI_UART_HANDLE const)p_uart_device_handler,
	                       data,
	                       buf_size,
	                       DMA_NOT_USE,
	                       &error);
	if(status != ADI_UART_SUCCESS)
		return UART_FAILURE;
	return UART_SUCCESS;
}

/**
  @brief Internal printf function with semihosting via UART.

  @param ptr - data to write.

  @return UART_SUCCESS or error code.

**/
int _write (int fd, char *ptr, int len)
{
	char *p = ptr;

	int res = UART_SUCCESS;

	(void)fd;
	(void)len;

	while (*p != '\n') {
		res = UART_WriteChar(*p++, UART_WRITE_NO_INT);

		if (res != UART_SUCCESS)
			break;

		if(*p == '\t')
			break;
	}

	if(*p == '\n') {
		UART_WriteChar('\r', 1);
		UART_WriteChar('\n', 1);
	}

	return res;
}

