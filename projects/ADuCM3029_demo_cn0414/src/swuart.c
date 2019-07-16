/***************************************************************************//**
*   @file   swuart.c
*   @brief  Software UART driver source.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdlib.h>
#include "timer.h"
#include "platform_drivers.h"
#include "swuart.h"

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
extern uint8_t gpio_init_flag;
extern uint8_t mem_gpio_handler[ADI_GPIO_MEMORY_SIZE];
volatile bool rec_flag = false; /* HART receive flag */
volatile uint32_t delay_flag = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Delay for half of the software UART baudrate period using the timer assigned
 * to the software UART. Load the timer, start counting, wait vfor the timer
 * interrupt and then deactivate the timer.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static inline void swuart_delay(struct swuart_dev *dev, uint32_t usec)
{
	NVIC_EnableIRQ(TMR1_EVT_IRQn);
	delay_flag = usec;
	while(delay_flag != 0);
	NVIC_DisableIRQ(TMR1_EVT_IRQn);
}

/**
 * Software UART RX interrupt callback function. This interrupt is associated
 * with the Group A GPIOs, which is set to include the RX pin. The interrupt is
 * used to wait for the start bit in the read function.
 *
 * @param [in] pCBParam - Pointer to callback parameter list.
 * @param [in] nEvent   - Interrupt source identifier.
 * @param [in/out] pArg - Random argument list.(chosen by the application)
 *
 * @return none
 */
static void software_uart_callback(void *pCBParam, uint32_t nEvent, void *pArg)
{
	rec_flag = true;
}

/**
 * Software UART timer interrupt callback function. The timer is used to
 * implement the timing for the protocol.
 *
 * @param [in] pCBParam - Pointer to callback parameter list.
 * @param [in] nEvent   - Interrupt source identifier.
 * @param [in/out] pArg - Random argument list.(chosen by the application)
 *
 * @return none
 */
static void timer0_delay_callback(void *pCBParam, uint32_t nEvent, void *pArg)
{
	if(delay_flag != 0)
		delay_flag--;
}

/**
 * Initializes the software UART.
 *
 * @param [out] device    - The device structure.
 * @param [in] init_param - Pointer to the structure that contains the device
 *                          initial parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t swuart_init(struct swuart_dev **device, struct swuart_init *init_param)
{
	int32_t ret;
	struct swuart_dev *dev;
	uint16_t val = 0;
	ADI_TMR_CONFIG delay_timer_config;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	dev->id = init_param->id;
	dev->baudrate = init_param->baudrate;
	dev->rx_port_pin.port_number = init_param->rx_port_pin.port_number;
	dev->rx_port_pin.pin_number = init_param->rx_port_pin.pin_number;
	dev->tx_port_pin.port_number = init_param->tx_port_pin.port_number;
	dev->tx_port_pin.pin_number = init_param->tx_port_pin.pin_number;
	dev->type = init_param->type;
	dev->no_of_bits = init_param->no_of_bits;
	dev->parity = init_param->parity;
	dev->delay_timer = init_param->delay_timer;
	dev->half_period = (uint16_t)((SECOND_LOAD_TIMER / dev->baudrate) / 2);

	if(gpio_init_flag == 0) {
		ret = adi_gpio_Init(mem_gpio_handler, ADI_GPIO_MEMORY_SIZE);
		if(ret != ADI_GPIO_SUCCESS)
			goto error;
	}

	gpio_init_flag++;

	ret = adi_gpio_RegisterCallback(ADI_GPIO_INTA_IRQ, software_uart_callback,
					NULL);
	if(ret != ADI_GPIO_SUCCESS)
		goto error;

	ret = adi_gpio_SetGroupInterruptPolarity(dev->rx_port_pin.port_number,
			~dev->rx_port_pin.pin_number);
	if(ret != ADI_GPIO_SUCCESS)
		goto error;

	ret = adi_gpio_InputEnable(dev->rx_port_pin.port_number,
				   dev->rx_port_pin.pin_number, true);
	if(ret != ADI_GPIO_SUCCESS)
		goto error;

	ret = adi_gpio_PullUpEnable(dev->rx_port_pin.port_number,
				    dev->rx_port_pin.pin_number, false);
	if(ret != ADI_GPIO_SUCCESS)
		goto error;

	ret = adi_gpio_OutputEnable(dev->tx_port_pin.port_number,
				    dev->tx_port_pin.pin_number, true);
	if(ret != ADI_GPIO_SUCCESS)
		goto error;

	ret = adi_gpio_SetHigh(dev->tx_port_pin.port_number,
			       dev->tx_port_pin.pin_number);
	if(ret != ADI_GPIO_SUCCESS)
		goto error;

	ret = adi_gpio_GetGroupInterruptPins(dev->rx_port_pin.port_number,
					     ADI_GPIO_INTA_IRQ,  &val);
	if(ret != ADI_GPIO_SUCCESS)
		goto error;
	val |= dev->rx_port_pin.pin_number;
	ret = adi_gpio_SetGroupInterruptPins(dev->rx_port_pin.port_number,
					     ADI_GPIO_INTA_IRQ, val);
	if(ret != ADI_GPIO_SUCCESS)
		goto error;

	/* Disable interrupt */
	NVIC_DisableIRQ(RX_INT);

	ret = adi_tmr_Init(dev->delay_timer, timer0_delay_callback, dev, true);
	if(ret != ADI_TMR_SUCCESS)
		goto error;

	/* Basic timer configuration */
	delay_timer_config.bCountingUp = false;
	delay_timer_config.bPeriodic = true;
	delay_timer_config.bReloading = false;
	delay_timer_config.bSyncBypass = true;
	delay_timer_config.eClockSource = ADI_TMR_CLOCK_PCLK;
	delay_timer_config.nLoad = 260;
	delay_timer_config.nAsyncLoad = 260;
	delay_timer_config.ePrescaler = 0;

	/* Configure delay timer */
	ret = adi_tmr_ConfigTimer(dev->delay_timer, &delay_timer_config);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	NVIC_DisableIRQ(TMR1_EVT_IRQn);

	/* Configure delay timer */
	ret = adi_tmr_Enable(dev->delay_timer, true);
	if(ret != ADI_TMR_SUCCESS)
		return ret;

	*device = dev;

	return 0;

error:
	free(dev);

	return ret;
}

/**
 * Free the resources allocated by swuart_init().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t swuart_remove(struct swuart_dev *dev)
{
	int32_t ret;

	gpio_init_flag--;

	/* If this is the first GPIO initialize GPIO controller */
	if(gpio_init_flag == 0) {
		ret = adi_gpio_UnInit();
		if(ret != ADI_GPIO_SUCCESS)
			return ret;
	}

	free(dev);

	return 0;
}

/**
 * The function implements a read/write operation through software UART started
 * by the application. The UART transmit considers the parameters number of bits
 * per byte and parity.
 *
 * @param [in] dev	     - The device structure.
 * @param [in/out] sbyte - Pointer to the byte to be transmitted and replaced
 *                         with the received one.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t swuart_write_char(struct swuart_dev *dev, uint8_t *sbyte)
{
	uint16_t val = 0;
	int32_t ret = 0;
	uint8_t parity_tx, parity_rx, i;

	/* Parity initialize */
	if(dev->parity == UART_EVEN_PARITY) {
		parity_tx = 0;
		parity_rx = 0;
	} else {
		parity_tx = 1;
		parity_rx = 1;
	}

	/* Transmit a byte and receive one full-duplex */
	adi_gpio_SetLow(dev->tx_port_pin.port_number, dev->tx_port_pin.pin_number);
	swuart_delay(dev, dev->half_period);
	swuart_delay(dev, dev->half_period);
	for(i = 0; i < dev->no_of_bits; i++) {
		if((*sbyte & 0x01) == 0x01) {
			adi_gpio_SetHigh(dev->tx_port_pin.port_number,
					 dev->tx_port_pin.pin_number);
			parity_tx += 1;
		} else {
			adi_gpio_SetLow(dev->tx_port_pin.port_number,
					dev->tx_port_pin.pin_number);
		}
		*sbyte = *sbyte >> 1;
		*sbyte &= 0x7F;
		swuart_delay(dev, dev->half_period);

		adi_gpio_GetData(dev->rx_port_pin.port_number,
				 dev->rx_port_pin.pin_number, &val);
		if(val == 0) {
			*sbyte |= 0x00;
		} else {
			*sbyte |= 0x80;
			parity_rx += 1;
		}
		swuart_delay(dev, dev->half_period);
	}

	if(dev->parity != UART_NO_PARITY) {
		if(parity_tx & 0x01)
			adi_gpio_SetHigh(dev->tx_port_pin.port_number,
					 dev->tx_port_pin.pin_number);
		else
			adi_gpio_SetLow(dev->tx_port_pin.port_number,
					dev->tx_port_pin.pin_number);
		swuart_delay(dev, dev->half_period);
		/* Get parity and check it */
		adi_gpio_GetData(dev->rx_port_pin.port_number,
				 dev->rx_port_pin.pin_number, &val);
		if(((val == 0) && ((parity_rx & 0x01) != 0)) ||
		    ((val != 0) && ((parity_rx & 0x01) == 0)))
			ret |= -1;
		swuart_delay(dev, dev->half_period);
	}

	adi_gpio_SetHigh(dev->tx_port_pin.port_number,
			 dev->tx_port_pin.pin_number);

	*sbyte = *sbyte >> (8 - dev->no_of_bits);

	return ret;
}

/**
 * The function implements a read/write operation through software UART started
 * by another device and not the application. The UART transmit considers the
 * parameters number of bits per byte and parity. The main difference is that
 * this transmission waits for the start bit of the other participant before
 * giving it's own.
 *
 * @param [in] dev	     - The device structure.
 * @param [in/out] sbyte - Pointer to the byte to be transmitted and replaced
 *                         with the received one.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t swuart_read_char(struct swuart_dev *dev, uint8_t *rbyte)
{
	int32_t ret = 0;
	uint32_t timeout = 1370;
	uint16_t val;
	uint8_t parity_tx, parity_rx, i;

	if(dev->parity == UART_EVEN_PARITY) {
		parity_tx = 0;
		parity_rx = 0;
	} else {
		parity_tx = 1;
		parity_rx = 1;
	}

	rec_flag = false;

	/* Disable interrupt */
	NVIC_EnableIRQ(RX_INT);

	/* Wait for the start bit or timeout */
	do {
		swuart_delay(dev, 1);
		timeout--;
	} while((rec_flag == false) && (timeout != 0));
	/* Error if no transmission */
	if(timeout == 0) {
		ret = 1;
		goto finish;
	}

	adi_gpio_SetLow(dev->tx_port_pin.port_number,
			dev->tx_port_pin.pin_number);
	/* Do transmission */
	swuart_delay(dev, dev->half_period);
	adi_gpio_GetData(dev->rx_port_pin.port_number,
			 dev->rx_port_pin.pin_number,
			 &val);
	if(val) {
		ret = 1;
		goto finish;
	}
	swuart_delay(dev, dev->half_period);
	for(i = 0; i < dev->no_of_bits; i++) {
		if((*rbyte & 0x01) == 0x01) {
			adi_gpio_SetHigh(dev->tx_port_pin.port_number,
					 dev->tx_port_pin.pin_number);
			parity_tx += 1;
		} else {
			adi_gpio_SetLow(dev->tx_port_pin.port_number,
					dev->tx_port_pin.pin_number);
		}
		*rbyte = *rbyte >> 1;
		*rbyte &= 0x7F;
		swuart_delay(dev, dev->half_period);
		adi_gpio_GetData(dev->rx_port_pin.port_number,
				 dev->rx_port_pin.pin_number,
				 &val);
		if(val == 0) {
			*rbyte |= 0x00;
		} else {
			*rbyte |= 0x80;
			parity_rx += 1;
		}
		swuart_delay(dev, dev->half_period);
	}

	if(dev->parity != UART_NO_PARITY) {
		if(parity_tx & 0x01)
			/* Transmit parity */
			adi_gpio_SetHigh(dev->tx_port_pin.port_number,
					 dev->tx_port_pin.pin_number);
		else
			adi_gpio_SetLow(dev->tx_port_pin.port_number,
					dev->tx_port_pin.pin_number);
		swuart_delay(dev, dev->half_period);
		/* Get parity and check it */
		adi_gpio_GetData(dev->rx_port_pin.port_number,
				 dev->rx_port_pin.pin_number, &val);
		if(((val == 0) && ((parity_rx & 0x01) != 0)) ||
		    ((val != 0) && ((parity_rx & 0x01) == 0)))
			ret |= -1;
		swuart_delay(dev, dev->half_period);
	}

	*rbyte = *rbyte >> (8 - dev->no_of_bits);

finish:
	adi_gpio_SetHigh(dev->tx_port_pin.port_number,
			 dev->tx_port_pin.pin_number);

	/* Disable interrupt */
	NVIC_DisableIRQ(RX_INT);

	return ret;
}

/**
 * Write a string of bytes through the software UART.
 *
 * @param [in] dev        - The device structure.
 * @param [in/out] string - Pointer to the string to be transmitted and replaced
 *                          with the received one.
 * @param [in] size	      - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t swuart_write_string(struct swuart_dev *dev, uint8_t *string,
			    uint32_t size)
{
	uint32_t i;
	int32_t check;

	for(i = 0; i < size; i++) {
		check = swuart_write_char(dev, &string[i]);
		if(check != 0)
			return check;
		swuart_delay(dev, dev->half_period);
		swuart_delay(dev, dev->half_period);
	}

	return check;
}

/**
 * Read a string of bytes through the software UART.
 *
 * @param [in] dev        - The device structure.
 * @param [in/out] string - Pointer to the string to be transmitted and replaced
 *                          with the received one.
 * @param [in] size	      - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t swuart_read_string(struct swuart_dev *dev, uint8_t *string,
			   uint32_t size)
{
	uint32_t i;
	int32_t check;

	for(i = 0; i < size; i++) {
		check = swuart_read_char(dev, &string[i]);
		if(check != 0)
			return check;
	}

	return check;
}
