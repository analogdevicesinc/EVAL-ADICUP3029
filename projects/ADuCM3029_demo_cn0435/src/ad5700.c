/***************************************************************************//**
*   @file   ad5700.c
*   @brief  AD5700 Driver source.
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
#include "ad5700.h"
#include "ad5700_gpio.h"

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/

/* CD pin interrupt flag */
volatile bool modem_rec_flag = false;

/******************************************************************************/
/************************* Functions Definitions ******************************/
/******************************************************************************/

/**
 * CD pin interrupt callback function.
 *
 * Asserts CD interrupt software flag. Standard format for callback functions.
 *
 * @param [in] pCBParam - Pointer to callback parameter list.
 * @param [in] nEvent   - Interrupt source identifier.
 * @param [in/out] pArg - Random argument list.(chosen by the application)
 *
 * @return none
 */
static void receive_modem_callback(void *cb_param, uint32_t event, void *arg)
{
	/* Set receive flag */
	modem_rec_flag = true;
}

/**
 * Initialize device structure for AD5700 used to manage methods for the device.
 *
 * Allocate memory for the descriptor, initialize the software UART and
 * initialize the GPIOs.
 *
 * @param [out] device    - Pointer to the device structure.
 * @param [in] init_param - Pointer to the initialization structure.
 *
 * @return 0 for success or negative error code.
 */
int32_t ad5700_setup(struct ad5700_dev **device,
		     struct ad5700_init_param *init_param)
{
	int32_t ret;
	struct ad5700_dev *dev;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	ret = swuart_init(&dev->swuart_desc, &init_param->swuart_init);
	if(ret < 0)
		goto error;

	ret = gpio_get(&dev->gpio_nrts, init_param->gpio_nrts);
	if(ret < 0)
		goto error;
	ret = gpio_get(&dev->gpio_cd, init_param->gpio_cd);
	if(ret < 0)
		goto error;
	ret = gpio_direction_output(dev->gpio_nrts, GPIO_HIGH);
	if(ret < 0)
		goto error;
	ret = gpio_direction_input(dev->gpio_cd);
	if(ret < 0)
		goto error;
	ret = ad5700_register_gpio_callback(dev, receive_modem_callback);
	if(ret < 0)
		goto error;
	ret = ad5700_set_cd_interrupt(dev);
	if(ret < 0)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free memory allocated in ad5700_setup().
 *
 * @param [in] dev - Pointer to the device structure.
 *
 * @return 0 for success or negative error code.
 */
int32_t ad5700_remove(struct ad5700_dev *dev)
{
	int32_t ret;

	ret = swuart_remove(dev->swuart_desc);
	if(ret < 0)
		return ret;

	ret = gpio_remove(dev->gpio_nrts);
	if(ret < 0)
		return ret;
	ret = gpio_remove(dev->gpio_cd);
	if(ret < 0)
		return ret;

	free(dev);

	return ret;
}

/**
 * Make a HART transmission using the AD5700 and software UART.
 *
 * Assert the nRTS pin, transmit the message and deassert nRTS pin.
 *
 * @param [in] dev  - Pointer to the device structure.
 * @param [in] data - Pointer to the data to be transmitted.
 * @param [in] size - Size in bytes of the transmission.
 *
 * @return 0 for success or negative error code.
 */
int32_t ad5700_transmit(struct ad5700_dev *dev, uint8_t *data, uint32_t size)
{
	int32_t ret;

	ret = gpio_set_value(dev->gpio_nrts, GPIO_LOW);
	if(ret < 0)
		return ret;

	mdelay(6);

	ret = swuart_write_string(dev->swuart_desc, data, size);
	if(ret < 0)
		return ret;

	mdelay(6);

	return gpio_set_value(dev->gpio_nrts, GPIO_HIGH);
}
