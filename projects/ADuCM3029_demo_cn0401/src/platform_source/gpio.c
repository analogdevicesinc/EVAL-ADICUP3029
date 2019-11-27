/***************************************************************************//**
 *   @file   gpio.c
 *   @author DBogdan (dragos.bogdan@analog.com)
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

#include "error.h"
#include "gpio.h"
#include <drivers/gpio/adi_gpio.h>
#include <stdlib.h>

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

/* Memory for GPIO device */
uint8_t mem_gpio_handler[ADI_GPIO_MEMORY_SIZE];
uint8_t gpio_init_flag = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Obtain the GPIO decriptor.
 *
 * @param [out] desc        - The GPIO descriptor.
 * @param [in]  gpio_number - The number of the GPIO.
 *
 * @return void
 */
static inline void gpio_get_portpin(struct gpio_desc *desc, uint16_t *pin,
				    uint8_t *port)
{
	*pin = (ADI_GPIO_DATA)(0x0001 << (desc->number & 0x0F));
	*port = (desc->number & 0xF0) >> 4;
}

/**
 * @brief Obtain the GPIO decriptor.
 * @param desc - The GPIO descriptor.
 * @param gpio_number - The number of the GPIO.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_get(struct gpio_desc **desc,
		 uint8_t gpio_number)
{
	int32_t ret = 0;
	struct gpio_desc *dev;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	dev->type = ADICUP3029_GPIO;
	dev->id = 0;
	dev->number = gpio_number;

	/* If this is the first GPIO initialize GPIO controller */
	if(gpio_init_flag == 0) {
		ret = adi_gpio_Init(mem_gpio_handler, ADI_GPIO_MEMORY_SIZE);
		if(ret != SUCCESS)
			goto error;
	}

	/* Increment number of GPIOs */
	gpio_init_flag++;

	*desc = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * @brief Free the resources allocated by gpio_get().
 * @param desc - The SPI descriptor.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_remove(struct gpio_desc *desc)
{
	if(!desc)
		return -1;

	free(desc);

	/* Decrement number of GPIOs */
	gpio_init_flag--;

	/* If no more GPIOs free driver memory */
	if (gpio_init_flag == 0)
		return adi_gpio_UnInit();

	return SUCCESS;
}

/**
 * @brief Enable the input direction of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_direction_input(struct gpio_desc *desc)
{
	uint16_t pin;
	uint8_t port;

	/* Get pin port and number from pin number */
	gpio_get_portpin(desc, &pin, &port);

	/* Enable input driver */
	return adi_gpio_InputEnable(port, pin, true);
}

/**
 * @brief Enable the output direction of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param value - The value.
 *                Example: GPIO_HIGH
 *                         GPIO_LOW
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_direction_output(struct gpio_desc *desc,
			      uint8_t value)
{
	int32_t ret;
	uint16_t pin;
	uint8_t port;

	/* Get pin port and number from pin number */
	gpio_get_portpin(desc, &pin, &port);

	/* Enable output driver */
	ret = adi_gpio_OutputEnable(port, pin, true);
	if(ret != SUCCESS)
		return ret;

	/* Initialize pin with a value */
	if(value == 1)
		return adi_gpio_SetHigh(port, pin);
	else
		return adi_gpio_SetLow(port, pin);
}

/**
 * @brief Get the direction of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param direction - The direction.
 *                    Example: GPIO_OUT
 *                             GPIO_IN
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_get_direction(struct gpio_desc *desc,
			   uint8_t *direction)
{
	if (desc) {
		// Unused variable - fix compiler warning
	}

	if (direction) {
		// Unused variable - fix compiler warning
	}

	return SUCCESS;
}

/**
 * @brief Set the value of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param value - The value.
 *                Example: GPIO_HIGH
 *                         GPIO_LOW
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_set_value(struct gpio_desc *desc,
		       uint8_t value)
{
	uint16_t pin;
	uint8_t port;

	/* Get pin port and number from pin number */
	gpio_get_portpin(desc, &pin, &port);

	/* Change pin value */
	if(value == 1)
		return adi_gpio_SetHigh(port, pin);
	else
		return adi_gpio_SetLow(port, pin);
}

/**
 * @brief Get the value of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param value - The value.
 *                Example: GPIO_HIGH
 *                         GPIO_LOW
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_get_value(struct gpio_desc *desc,
		       uint8_t *value)
{
	uint16_t pin;
	uint8_t port;
	uint16_t data_temp;

	/* Get pin port and number from pin number */
	gpio_get_portpin(desc, &pin, &port);

	/* Read pin value */
	return adi_gpio_GetData(port, pin, &data_temp);
}

/**
 * Add GPIO to the GPIO interrupt group.
 *
 * @param [in] desc      - The GPIO descriptor.
 * @param [in] int_group - The GPIO interrupt group.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_add_int_group(gpio_desc *desc, uint8_t int_group)
{
	uint8_t port;
	uint16_t pin, temp;
	int32_t ret;

	gpio_get_portpin(desc, &pin, &port);

	ret = adi_gpio_GetGroupInterruptPins(port, int_group, &temp);
	if(ret != ADI_GPIO_SUCCESS)
		return ret;
	temp |= pin;
	return adi_gpio_SetGroupInterruptPins(port, int_group, temp);
}

/**
 * Remove GPIO from the GPIO interrupt group.
 *
 * @param [in] desc      - The GPIO descriptor.
 * @param [in] int_group - The GPIO interrupt group.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_remove_int_group(gpio_desc *desc, uint8_t int_group)
{
	uint8_t port;
	uint16_t pin, temp;
	int32_t ret;

	gpio_get_portpin(desc, &pin, &port);

	ret = adi_gpio_GetGroupInterruptPins(port, int_group, &temp);
	if(ret != ADI_GPIO_SUCCESS)
		return ret;
	temp &= ~pin;

	return adi_gpio_SetGroupInterruptPins(port, int_group, temp);
}

/**
 * Set GPIO interrupt polarity.
 *
 * @param [in] desc     - The GPIO descriptor.
 * @param [in] polarity - Polarity of the GPIO interrupt:
 *                         - true - interrupt on low to high;
 *                         - false - interrupt on high to low.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_set_int_polarity(gpio_desc *desc, bool polarity)
{
	uint8_t port;
	uint16_t pin, temp;
	int32_t ret;

	gpio_get_portpin(desc, &pin, &port);

	ret = adi_gpio_GetGroupInterruptPolarity(port, &temp);
	if(ret != ADI_GPIO_SUCCESS)
		return ret;
	if(polarity)
		temp |= pin;
	else
		temp &= ~pin;

	return adi_gpio_SetGroupInterruptPolarity(port, temp);
}

/**
 * Register callback routine for a GPIO interrupt group.
 *
 * @param [in] int_group - The GPIO interrupt group.
 * @param [in] cb_ptr    - Poiner to the callback function.
 * @param [in] cb_param  - Pointer to the application given parameter.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t gpio_reg_callback(uint8_t int_group, void *cb_ptr, void *cb_param)
{
	return adi_gpio_RegisterCallback(int_group, cb_ptr, cb_param);
}

/**
 * Enable/Disable pull-up for a GPIO.
 *
 * @param [in] desc - The GPIO handler.
 * @param [in] enable - true to enable the pull-up;
 *                      fale to disable the pull-up.
 *
 * @return 0 in case of success, error code otherwise.
 */
int32_t gpio_set_pull_up(gpio_desc *desc, bool enable)
{
	uint16_t pin;
	uint8_t port;

	gpio_get_portpin(desc, &pin, &port);

	return adi_gpio_PullUpEnable(port, pin, enable);
}
