/*!
 *****************************************************************************
 * @file:    Communication.c
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

/***************************** Include Files **********************************/
#include "Communication.h"

/************************** Variable Definitions ******************************/

unsigned char           uart_rx_buffer[UART_RX_BUFFER_SIZE];
unsigned char           uart_tx_buffer[UART_TX_BUFFER_SIZE];

ADI_UART_HANDLE p_device_handler __attribute__ ((aligned (4)));
uint8_t			uart_buffer[ADI_UART_BIDIR_MEMORY_SIZE] __attribute__ ((aligned (4)));

unsigned int         uart_rpos, uart_rcnt, uart_tpos, uart_tcnt;
unsigned int         uart_echo, uart_cmd, uart_ctrlc, uart_tbusy;

/* Values for the baudrate generation registers when the baudrate is one of
 * the standard values and clock is 26MHz */
const uint8_t baudrate_osr_26mhz[] = {3, 3, 3, 3, 3, 3, 3, 2, 2, 2};
const uint16_t baudrate_divc_26mhz[] = {24, 12, 8, 4, 4, 2, 1, 1, 1, 1};
const uint8_t baudrate_divm_26mhz[] = {3, 3, 2, 3, 1, 1, 1, 1, 1, 1};
const uint16_t baudrate_divn_26mhz[] = {1078, 1078, 1321, 1078, 1563, 1563,
									   1563, 1563, 1280, 171};

/************************* Functions Definitions ******************************/

/**
   @brief UART initialization

**/
int16_t UART_Init(enum uart_baudrate baudrate,
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

	/* Configure power controller */
	/* Initialize clock configuration registers */
	status = adi_pwr_Init();
	if(status != ADI_PWR_SUCCESS)
		return UART_INIT_FAILURE;
	/* Enable external 26MHz oscillator as clock source */
	status = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1);
	if(status != ADI_PWR_SUCCESS)
		return UART_INIT_FAILURE;
	/* Enable clock to the UART module */
	status = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1);
	if(status != ADI_PWR_SUCCESS)
		return UART_INIT_FAILURE;

	/* Configure UART module */
	/* Open UART module*/
	status = adi_uart_Open(uart_device_number,
				  ADI_UART_DIR_BIDIRECTION,
				  (void*)uart_buffer,
				  (const uint32_t)ADI_UART_BIDIR_MEMORY_SIZE,
				  (ADI_UART_HANDLE *const)&p_device_handler);
	if(status != ADI_UART_SUCCESS)
		return UART_INIT_FAILURE;
	/* Set baudrate */
	status = adi_uart_ConfigBaudRate((ADI_UART_HANDLE const)p_device_handler,
							baudrate_divc_26mhz[baudrate],
							baudrate_divm_26mhz[baudrate],
							baudrate_divn_26mhz[baudrate],
							baudrate_osr_26mhz[baudrate]);
	if(status != ADI_UART_SUCCESS)
		return UART_INIT_FAILURE;
	/* Set configuration of UART transmission */
	status = adi_uart_SetConfiguration((ADI_UART_HANDLE const)p_device_handler,
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
int16_t UART_WriteChar(uint8_t data,
		enum en_write_data mode)
{
	bool tx_available = 0;
	uint32_t hw_error;
	const uint32_t buf_size = 1;

	/* Send character with interrupt */
	if(mode == UART_WRITE_IN_INT) {
		/* Check if not busy */
		adi_uart_IsTxBufferAvailable((ADI_UART_HANDLE const)p_device_handler,
					   (bool* const)&tx_available);
		if (!tx_available) {
			return UART_NO_TX_SPACE;
		}
		/* Start transmission */
		adi_uart_SubmitTxBuffer((ADI_UART_HANDLE const)p_device_handler,
			   (void* const)&data,
			   buf_size,
			   DMA_NOT_USE);

		return UART_SUCCESS;

	/* Send character without interrupt (blocking) */
	} else {
		/* Do transmission */
		adi_uart_Write((ADI_UART_HANDLE const)p_device_handler,
    			  (void* const)&data,
				  buf_size,
				  DMA_NOT_USE,
				  &hw_error);
	}
	return UART_SUCCESS;
}

/**
   @brief Writes string to UART.

   @param string - string to write.

   @return UART_SUCCESS or error code.

**/
int16_t UART_WriteString(uint8_t *string)
{
	int16_t result = UART_SUCCESS;

	while(*string != '\0') {
		result = UART_WriteChar(*string++,
				UART_WRITE_NO_INT);

		if(result != UART_SUCCESS) {
			break;
		}
	}

	return result;
}

/**
   @brief Read character from UART.

   @param data - data to read.

**/
int16_t UART_ReadChar(uint8_t *data)
{
	const uint32_t buf_size = 1;
	uint32_t error;
	adi_uart_Read((ADI_UART_HANDLE const)p_device_handler,
			data,
			buf_size,
			DMA_NOT_USE,
			&error);

   return UART_SUCCESS;
}

