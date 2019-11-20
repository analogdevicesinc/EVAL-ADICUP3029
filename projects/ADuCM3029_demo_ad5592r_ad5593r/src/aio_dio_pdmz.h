/***************************************************************************//**
 *   @file   aio_dio_pdmz.h
 *   @brief  Header of the I/O analog/digital PMOD.
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

#ifndef AIO_DIO_PDMZ_H_
#define AIO_DIO_PDMZ_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "config.h"
#include "ad5592r.h"
#include "ad5593r.h"
#include "ad5592r-base.h"
#include "cli.h"
#include "timer.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define DIO_HIGH 1
#define DIO_LOW  0

#define HELP_SHORT_COMMAND true
#define HELP_LONG_COMMAND false

#define AD5592_3_INTERNAL_REFERENCE (float)2.5
#define AD5592_3_RESOLUTION_BITS 12

typedef int32_t (*adc_read_ptr)(struct ad5592r_dev *, uint8_t, uint16_t *);
typedef int32_t (*dac_write_ptr)(struct ad5592r_dev *, uint8_t, uint16_t);

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

enum aiodio_channels {
	AIODIO_CH0,
	AIODIO_CH1,
	AIODIO_CH2,
	AIODIO_CH3,
	AIODIO_CH4,
	AIODIO_CH5,
	AIODIO_CH6,
	AIODIO_CH7
};

struct aiodio_dev {
	struct ad5592r_dev *board_device;
	struct i2c_desc *board_i2c;
	struct spi_desc *board_spi;
	struct cli_desc *board_cli;
	struct gpio_desc *blink_led1;
	struct gpio_desc *blink_led2;
	struct timer_counter_desc *mode_delay_timer;
	struct timer_counter_desc *blink_delay_timer;
	adc_read_ptr read_adc;
	dac_write_ptr write_dac;
	uint8_t mirror_mode;
	uint8_t test_mode;
	uint8_t test_success;
};

struct aiodio_init_param {
	struct i2c_init_param i2c_init;
	struct spi_init_param spi_init;
	struct ad5592r_init_param ad5592_3r_param;
	struct cli_init_param cli_init;
	struct timer_counter_init mode_delay_timer;
	struct timer_counter_init blink_delay_timer;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Setup the application. */
int32_t aiodio_setup(struct aiodio_dev **device,
		     struct aiodio_init_param *init_param);

/* Free resources allocated by aiodio_setup(). */
int32_t aiodio_remove(struct aiodio_dev *dev);

/* Display help options in the CLI. */
int32_t aiodio_help(struct aiodio_dev *dev, uint8_t *arg);

/* Display the status of the application. */
int32_t aiodio_status(struct aiodio_dev *dev, uint8_t *arg);

/* Set a channel as analog output (DAC channel) and set the output code. */
int32_t aiodio_analog_out(struct aiodio_dev *dev, uint8_t *arg);

/* Set a channel as analog input (ADC channel) and read the input. */
int32_t aiodio_analog_in(struct aiodio_dev *dev, uint8_t *arg);

/* Set a channel as digital output (GPIO channel) and set the output state. */
int32_t aiodio_digital_out(struct aiodio_dev *dev, uint8_t *arg);

/* Set a channel as digital input (GPIO channel) and read the state. */
int32_t aiodio_digital_in(struct aiodio_dev *dev, uint8_t *arg);

/* Activate mirror mode. */
int32_t aiodio_mirror_mode(struct aiodio_dev *dev, uint8_t *arg);

/* Activate production test mode. */
int32_t aiodio_prod_test_mode(struct aiodio_dev *dev, uint8_t *arg);

/* Application process. */
int32_t aiodio_process(struct aiodio_dev *dev);

/* Stream input data from the ADC inputs. */
int32_t aiodio_analog_in_stream(struct aiodio_dev *dev, uint8_t *arg);

#endif /* AIO_DIO_PDMZ_H_ */
