/***************************************************************************//**
*   @file   cn0531.h
*   @brief  CN0531 application header.
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

#ifndef CN0531_H_
#define CN0531_H_

#include <stdint.h>
#include "cli.h"
#include "ad5791.h"
#include "aducm3029_adc.h"

#define CN0531_CLI_CMD_NO 4
#define HELP_SHORT_COMMAND true
#define HELP_LONG_COMMAND false

#define CN0531_POSITIVE_REFERENCE (float)5
#define CN0531_NEGATIVE_REFERENCE (float)-5

#define CN0531_HELP_DAC_LONG " dac_reg_read <addr>        - Read a DAC register.\n\r" \
					     "                              <addr> = address of the register to be read in hexadecimal.\n\r" \
					     "                              Example: dac_reg_read 1\n\r" \
					     " dac_reg_write <addr> <val> - Write a DAC register.\n\r" \
					     "                              <addr> = address of the register to be written in hexadecimal.\n\r"\
					     "                              <val> = value to be written to the register in hexadecimal.\n\r" \
					     "                              Example: dac_reg_write 1 18c\n\r" \
					     " dac_out <volt>             - Update the DAC output voltage.\n\r" \
					     "                              <volt> = new voltage value expressed in volts.\n\r" \
					     "                              Example: dac_out 1.4\n\r"
#define CN0531_HELP_DAC_SHORT " drr <addr>       - Read a DAC register.\n\r" \
					     "                    <addr> = address of the register to be read in hexadecimal.\n\r" \
					     "                    Example: dac_reg_read 1\n\r" \
					     " drw <addr> <val> - Write a DAC register.\n\r" \
					     "                    <addr> = address of the register to be written in hexadecimal.\n\r" \
					     "                    <val> = value to be written to the register in hexadecimal.\n\r" \
					     "                    Example: dac_reg_write 1 18c\n\r" \
					     " do <volt>        - Update the DAC output voltage.\n\r" \
					     "                    <volt> = new voltage value expressed in volts.\n\r" \
					     "                    Example: do -2.3\n\r"

struct cn0531_dev {
	struct cli_desc *cli_desc;
	struct ad5791_dev *ad5791_desc;
	struct aducm3029_adc_desc *platform_adc;
	cmd_func *cn0531_cmd_func_tab;
	uint8_t **cn0531_cmd_calls;
	uint8_t *cn0531_cmd_size;
};

struct cn0531_init_param {
	struct cli_init_param cli_param;
	struct ad5791_init_param ad5791_param;
	struct aducm3029_adc_init_param platform_adc;
};

/** Display help options in the CLI. */
int32_t cn0531_help(struct cn0531_dev *dev, uint8_t *arg);

/** CLI command to read a register value. */
int32_t cn0531_reg_read(struct cn0531_dev *dev, uint8_t *arg);

/** CLI command to write a register value. */
int32_t cn0531_reg_write(struct cn0531_dev *dev, uint8_t *arg);

/** CLI command to update the output. */
int32_t cn0531_dac_out(struct cn0531_dev *dev, uint8_t *arg);

/** Application process. Needs to run in a loop. */
int32_t cn0531_process(struct cn0531_dev *dev);

/** Application intialization. */
int32_t cn0531_init(struct cn0531_dev **device,
		    struct cn0531_init_param *param);

/** Free memory allocated by cn0531_init() */
int32_t cn0531_remove(struct cn0531_dev *dev);

#endif /* CN0531_H_ */
