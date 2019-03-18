/***************************************************************************//**
*   @file   ad5700.h
*   @brief  AD5700 Driver header.
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

#ifndef AD5700_H_
#define AD5700_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "platform_drivers.h"
#include "swuart.h"

#define AD5700_SUCCESS 0

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
struct ad5700_init_param {
	/* Device peripheral */
	struct swuart_init swuart_init;
	/* GPIOs */
	uint8_t gpio_nrts;
	uint8_t gpio_cd;
};

struct ad5700_dev {
	/* Device peripheral */
	struct swuart_dev *swuart_desc;
	/* GPIOs */
	struct gpio_desc *gpio_nrts;
	struct gpio_desc *gpio_cd;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initialize device structure for AD5700 used to manage methods for the device.
 * Allocate memory for the descriptor, initialize the software UART and
 * initialize the GPIOs. */
int32_t ad5700_setup(struct ad5700_dev **device,
		     struct ad5700_init_param *init_param);

/* Free memory allocated in ad5700_setup(). */
int32_t ad5700_remove(struct ad5700_dev *dev);

/* Make a HART transmission using the AD5700 and software UART. Assert the nRTS
 * pin, transmit the message and deassert nRTS pin. */
int32_t ad5700_transmit(struct ad5700_dev *dev, uint8_t *data, uint32_t size);

#endif /* AD5700_H_ */
