/***************************************************************************//**
 *   @file   uart.c
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdlib.h>
#include <drivers/uart/adi_uart.h>
#include "uart.h"
#include "error.h"

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

/* Handle for UART device */
ADI_UART_HANDLE h_uart_device __attribute__ ((aligned (4)));
/* Memory for  UART driver */
uint8_t uart_device_mem[ADI_UART_BIDIR_MEMORY_SIZE]
__attribute__ ((aligned (4)));
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
static volatile int8_t tx_done = 0;

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
	case ADI_UART_EVENT_TX_BUFFER_PROCESSED:
		tx_done--;
		break;
	default:
		break;
	}
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
		tx_done++;
		while(tx_done > 0);
		if(tx_done == 0)
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
		ret = -1;
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

		if(ret < 0)
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
		ret = -1;
		break;
	}

	return ret;
}
