/***************************************************************************//**
*   @file   app.h
*   @brief  Application header.
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

#ifndef APP_H_
#define APP_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "cli.h"
#include "ad7124.h"
#include "irq.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define HELP_SHORT_COMMAND true
#define HELP_LONG_COMMAND false

#define AD7124_8PMDZ_CMD_NO 13

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/**
 * @struct ad7124_8pmdz_init_param
 * @brief Application initialization structure.
 */
struct ad7124_8pmdz_init_param {
	/** CLI initialization structure. */
	struct cli_init_param cli_initial;
	/** AD7124 initialization structure. */
	struct ad7124_init_param ad7124_initial;
	/** Interrupt controller initialization structure. */
	struct irq_init_param irq_init;
};

/**
 * @struct ad7124_8pmdz_dev
 * @brief Application handler structure.
 */
struct ad7124_8pmdz_dev {
	/** CLI handler. */
	struct cli_desc *cli_handler;
	/** AD7124 handler. */
	struct ad7124_dev *ad7124_handler;
	/** CLI function vector. */
	cmd_func *cmd_fun_tab;
	/** CLI function names vector. */
	uint8_t **cmd_name_tab;
	/** CLI function sizes vector. */
	uint8_t *cmd_name_size_tab;
	/** Interrupt controller handler. */
	struct irq_ctrl_desc *irq_handler;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Display help options in the CLI. */
int32_t ad7124_8pmdz_help(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** CLI command to read and display a device register. */
int32_t ad7124_8pmdz_reg_read(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** CLI command to write a device register. */
int32_t ad7124_8pmdz_reg_write(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** CLI command to read and display data from the ADC. */
int32_t ad7124_8pmdz_data_read(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** CLI command to enable channels. */
int32_t ad7124_8pmdz_channel_en_set(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** CLI command to read the enabled channels. */
int32_t ad7124_8pmdz_channel_en_get(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/**
 * CLI command to do a device reset. The device is reset to datasheet default
 * values.
 */
int32_t ad7124_8pmdz_reboot(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** Set PGA for a channel. */
int32_t ad7124_8pmdz_chann_pga_set(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** Read and display PGA of a channel. */
int32_t ad7124_8pmdz_chann_pga_get(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** CLI command to set the sampling rate of the device. */
int32_t ad7124_8pmdz_odr_set(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** CLI command to get and display the sample rate. */
int32_t ad7124_8pmdz_odr_get(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** CLI command to update the filter option. */
int32_t ad7124_8pmdz_flt_set(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** CLI command to read and display the active filter option. */
int32_t ad7124_8pmdz_flt_get(struct ad7124_8pmdz_dev *dev, uint8_t *arg);

/** Setup the application. */
int32_t ad7124_8pmdz_init(struct ad7124_8pmdz_dev **device,
			  struct ad7124_8pmdz_init_param *init_param);

/** Free memory allocated by ad7124_8pmdz_init(). */
int32_t ad7124_8pmdz_remove(struct ad7124_8pmdz_dev *dev);

/** Application process. */
int32_t ad7124_8pmdz_process(struct ad7124_8pmdz_dev *dev);

/** Application prompt wrapper used as API for layers above the application. */
int32_t ad7124_8pmdz_prompt(struct ad7124_8pmdz_dev *dev);

#endif /* APP_H_ */
