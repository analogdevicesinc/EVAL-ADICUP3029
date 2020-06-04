/***************************************************************************//**
*   @file   ad5770r_pmdz.h
*   @brief  Application header.
*   @author Andrei Drimbarean (andrei.drimbarean@analog.com)
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

#ifndef AD5770R_PMDZ_H_
#define AD5770R_PMDZ_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad5770r.h"
#include "cli.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define HELP_SHORT_COMMAND true
#define HELP_LONG_COMMAND false

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

struct ad5770r_pmdz_init_param {
	struct ad5770r_init_param ad5770r_init;
	struct cli_init_param cli_init;
};

struct ad5770r_pmdz_dev {
	struct ad5770r_dev *ad770r_device;
	struct cli_desc *cli_device;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Display help options in the CLI. */
int32_t ad5770r_pmdz_help(struct ad5770r_pmdz_dev *dev, uint8_t *arg);

/* Display "Set Input Register" command error. */
int32_t ad5770r_pmdz_set_in_reg(struct ad5770r_pmdz_dev *dev, uint8_t *arg);

/* Update the DAC output registers with the values in the input registers. */
int32_t ad5770r_pmdz_update_out(struct ad5770r_pmdz_dev *dev, uint8_t *arg);

/* Set output of a channel. */
int32_t ad5770r_pmdz_set_chan(struct ad5770r_pmdz_dev *dev, uint8_t *arg);

/* Set output range for a channel. */
int32_t ad5770r_pmdz_set_range(struct ad5770r_pmdz_dev *dev, uint8_t *arg);

/* Production test routine. */
int32_t ad5770r_pmdz_prod_test(struct ad5770r_pmdz_dev *dev, uint8_t *arg);

/* Provide default configuration for the application initialization handler. */
void ad5770r_pmdz_get_config(struct ad5770r_pmdz_init_param *init_param);

/* Application main process. */
int32_t ad5770r_pmdz_process(struct ad5770r_pmdz_dev *dev);

/* Allocate memory for the application handlers and initialize the system. */
int32_t ad5770r_pmdz_setup(struct ad5770r_pmdz_dev **device,
			   struct ad5770r_pmdz_init_param *init_param);

/* Free memory allocated by ad5770r_pmdz_setup(). */
int32_t ad5770r_pmdz_remove(struct ad5770r_pmdz_dev *dev);

#endif /* AD5770R_PMDZ_H_ */
