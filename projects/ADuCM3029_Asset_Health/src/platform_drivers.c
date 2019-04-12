/***************************************************************************//**
 *   @file   platform_drivers.c
 *   @brief  Implementation of Generic Platform Drivers.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2017(c) Analog Devices, Inc.
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
#include <stdint.h>
#include "platform_drivers.h"
#include <drivers/spi/adi_spi.h>

uint8_t SPI0_DevMem[ADI_SPI_MEMORY_SIZE];
uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE];


/* SPI device handles */
ADI_SPI_HANDLE	hSPI0MasterDev;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
void pinIntCallback(void* pCBParam, uint32_t Port,  void* PinIntData);

/**
 * @brief Initialize the SPI communication peripheral.
 * @param desc - The SPI descriptor.
 * @param init_param - The structure that contains the SPI parameters.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t spi_init(spi_desc **desc,
		 spi_init_param param)
{
	if (desc) {
		// Unused variable - fix compiler warning
	}

	if(adi_spi_Open(param.id, &SPI0_DevMem[0], ADI_SPI_MEMORY_SIZE,
			&hSPI0MasterDev) != ADI_SPI_SUCCESS)
		return ADI_SPI_FAILURE;

	/* Enable underflow errors */
	if(adi_spi_SetTransmitUnderflow(hSPI0MasterDev, true) != ADI_SPI_SUCCESS)
		return ADI_SPI_FAILURE;

	/* Set the SPI clock rate */
	if(adi_spi_SetBitrate(hSPI0MasterDev, param.max_speed_hz) != ADI_SPI_SUCCESS)
		return ADI_SPI_FAILURE;

	/* Set the chip select */
	if(adi_spi_SetChipSelect(hSPI0MasterDev, param.chip_select) != ADI_SPI_SUCCESS)
		return ADI_SPI_FAILURE;

	/* Set the SPI clock polarity */
	if(adi_spi_SetClockPolarity(hSPI0MasterDev, false) != ADI_SPI_SUCCESS)
		return ADI_SPI_FAILURE;

	/* Set the SPI clock phase */
	if(adi_spi_SetClockPhase(hSPI0MasterDev, false) != ADI_SPI_SUCCESS)
		return ADI_SPI_FAILURE;

	/* Set master mode */
	if(adi_spi_SetMasterMode(hSPI0MasterDev, true) != ADI_SPI_SUCCESS)
		return ADI_SPI_FAILURE;

	if(adi_spi_SetContinuousMode(hSPI0MasterDev, true) != ADI_SPI_SUCCESS)
		return ADI_SPI_FAILURE;

	return SUCCESS;
}

/**
 * @brief Free the resources allocated by spi_init().
 * @param desc - The SPI descriptor.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t spi_remove(spi_desc *desc)
{
	if (desc) {
		// Unused variable - fix compiler warning
	}

	return SUCCESS;
}

/**
 * @brief Write and read data to/from SPI.
 * @param desc - The SPI descriptor.
 * @param data - The buffer with the transmitted/received data.
 * @param bytes_number - Number of bytes to write/read.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t spi_write_and_read(spi_desc *desc,
			   uint8_t *data,
			   uint8_t bytes_number)
{
	ADI_SPI_RESULT eResult = ADI_SPI_SUCCESS;  /* assume the best */
	ADI_SPI_TRANSCEIVER  transceive;

	/* initialize data attributes */
	transceive.pTransmitter = data;
	transceive.pReceiver = data;

	/* link transceive data size to the remaining count */
	transceive.TransmitterBytes = bytes_number;
	transceive.ReceiverBytes = bytes_number;

	/* auto increment both buffers */
	transceive.nTxIncrement = true;
	transceive.nRxIncrement = true;

	transceive.bDMA = false;
	transceive.bRD_CTL = false;

	//eResult = adi_spi_MasterReadWrite(hSPI0MasterDev, &transceive);
	eResult = adi_spi_MasterSubmitBuffer (hSPI0MasterDev, &transceive);

	return eResult;
}

/**
 * @brief Obtain the GPIO decriptor.
 * @param desc - The GPIO descriptor.
 * @param gpio_number - The number of the GPIO.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_get(gpio_desc **desc,
		 uint8_t gpio_number)
{
	if (desc) {
		// Unused variable - fix compiler warning
	}

	if (gpio_number) {
		// Unused variable - fix compiler warning
	}

	return 0;
}

/**
 * @brief Free the resources allocated by gpio_get().
 * @param desc - The SPI descriptor.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_remove(gpio_desc *desc)
{
	if (desc) {
		// Unused variable - fix compiler warning
	}

	return SUCCESS;
}

/**
 * @brief Enable the input direction of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_direction_input(gpio_desc *desc)
{
	if (desc) {
		// Unused variable - fix compiler warning
	}

	return 0;
}

/**
 * @brief Enable the output direction of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param value - The value.
 *                Example: GPIO_HIGH
 *                         GPIO_LOW
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_direction_output(gpio_desc *desc,
			      uint8_t value)
{
	if (desc) {
		// Unused variable - fix compiler warning
	}

	if (value) {
		// Unused variable - fix compiler warning
	}

	return 0;
}

/**
 * @brief Get the direction of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param direction - The direction.
 *                    Example: GPIO_OUT
 *                             GPIO_IN
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_get_direction(gpio_desc *desc,
			   uint8_t *direction)
{
	if (desc) {
		// Unused variable - fix compiler warning
	}

	if (direction) {
		// Unused variable - fix compiler warning
	}

	return 0;
}

/**
 * @brief Set the value of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param value - The value.
 *                Example: GPIO_HIGH
 *                         GPIO_LOW
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_set_value(gpio_desc *desc,
		       uint8_t value)
{
	if (desc) {
		// Unused variable - fix compiler warning
	}

	if (value) {
		// Unused variable - fix compiler warning
	}

	return 0;
}

/**
 * @brief Get the value of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param value - The value.
 *                Example: GPIO_HIGH
 *                         GPIO_LOW
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t gpio_get_value(gpio_desc *desc,
		       uint8_t *value)
{
	if (desc) {
		// Unused variable - fix compiler warning
	}

	if (value) {
		// Unused variable - fix compiler warning
	}

	return 0;
}

/**
 * @brief Generate miliseconds delay.
 * @param msecs - Delay in miliseconds.
 * @return None.
 */
void mdelay(uint32_t msecs)
{
	if (msecs) {
		timer_sleep(msecs);
	}
}

/**
 * @brief Enable the gpio pins and set default state.
 * @return None.
 */
int32_t init_gpio(void)
{
	int32_t ret = 0;

	/* Set interrupt pin */
	ret = adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE); //initialize gpio
//	ret |= adi_gpio_OutputEnable(INTACC_PORT, INTACC_PIN, false);
	ret |= adi_gpio_InputEnable(INTACC_PORT, INTACC_PIN,
				    true);    // Set INTACC_PORT as input
//	ret |= adi_gpio_PullUpEnable(INTACC_PORT, INTACC_PIN, false); 	// Disable pull-up resistors
	ret |= adi_gpio_SetGroupInterruptPolarity(INTACC_PORT, INTACC_PIN);
	ret |= adi_gpio_SetGroupInterruptPins(INTACC_PORT,  SYS_GPIO_INTA_IRQn,
					      INTACC_PIN);
	ret |= adi_gpio_RegisterCallback (SYS_GPIO_INTA_IRQn, pinIntCallback, NULL );

	/*Turn off leds DS3 and DS4 for low power*/
	ret |= adi_gpio_OutputEnable(DS3_PORT, DS3_PIN, true);
	ret |= adi_gpio_SetLow(DS3_PORT, DS3_PIN);
	ret |= adi_gpio_OutputEnable(DS4_PORT, DS4_PIN, true);
	ret |= adi_gpio_SetLow(DS4_PORT, DS4_PIN);

	return ret;
}
