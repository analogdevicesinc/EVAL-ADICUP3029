/***************************************************************************//**
*   @file   ad5700_gpio.c
*   @brief  ad5700 GPIO Abstraction Level source.
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
#include "ad5700_gpio.h"

/******************************************************************************/
/************************* Functions Definitions ******************************/
/******************************************************************************/

/**
 * Sets up callback function for Group B GPIO interrupts.
 *
 * The CD pin will be connected to this interrupt to enable HART reads on
 * interrupt instead of calling the function when input is expected. This method
 * functions as an abstraction wrapper to avoid including ADICUP3029 drivers in
 * higher modules.
 *
 * @param [in] dev     	   - AD5700 device structure.
 * @param [in] pf_callback - The function pointer to the callback function.
 *
 * @return 0 for success or negative error code.
 */
int32_t ad5700_register_gpio_callback(struct ad5700_dev *dev,
				      ADI_CALLBACK const pf_callback)
{
	return adi_gpio_RegisterCallback(ADI_GPIO_INTB_IRQ, pf_callback, NULL);
}

/**
 * Set up Group B GPIO interrupt for CD pin of the AD5700 and set the interrupt
 * polarity so that the interrupt is triggered on a positive edge.
 *
 * @param [in] dev - AD5700 device structure.
 *
 * @return 0 for success or negative error code.
 */
int32_t ad5700_set_cd_interrupt(struct ad5700_dev *dev)
{
	int32_t ret;
	uint8_t cd_port;
	uint16_t cd_pin, gpio_register_data;

	cd_pin = (ADI_GPIO_DATA)(0x0001 << (dev->gpio_cd->number & 0x0F));
	cd_port = (dev->gpio_cd->number & 0xF0) >> 4;

	ret = adi_gpio_GetGroupInterruptPins(cd_port, ADI_GPIO_INTB_IRQ,
					     &gpio_register_data);
	if(ret != AD5700_SUCCESS)
		return ret;

	/* Add CD pin to the group B interrupts */
	gpio_register_data |= cd_pin;

	/* Set GPIO for interrupt */
	ret = adi_gpio_SetGroupInterruptPins(cd_port, ADI_GPIO_INTB_IRQ,
					     gpio_register_data);
	if(ret != AD5700_SUCCESS)
		return ret;

	/* Get group B interrupt polarity */
	ret = adi_gpio_GetGroupInterruptPolarity(cd_port, &gpio_register_data);
	if(ret != AD5700_SUCCESS)
		return ret;

	/* Add CD pin to interrupt on a rising edge */
	gpio_register_data |= cd_pin;

	/* Set CD interrupt polarity */
	return adi_gpio_SetGroupInterruptPolarity(cd_port, gpio_register_data);
}
