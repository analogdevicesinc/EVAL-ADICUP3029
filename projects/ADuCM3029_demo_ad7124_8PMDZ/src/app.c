/***************************************************************************//**
*   @file   app.c
*   @brief  Application source code.
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
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "app.h"
#include "platform_init.h"
#include "error.h"
#include "timer.h"
#include "irq_extra.h"
#include "util.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Help command helper function. Display help function prompt.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad7124_8pmdz_help_prompt(struct ad7124_8pmdz_dev *dev,
					bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = cli_write_string(dev->cli_handler,
				       (uint8_t*)"\tCN0424 application.\n");
		if(ret != SUCCESS)
			return ret;
		ret = cli_write_string(dev->cli_handler,
				       (uint8_t*)
				       "For commands with options as arguments typing the command and 'space' without arguments\n");
		if(ret != SUCCESS)
			return ret;
		ret = cli_write_string(dev->cli_handler,
				       (uint8_t*)"will show the list of options.\n");
		if(ret != SUCCESS)
			return ret;
		return cli_write_string(dev->cli_handler,
					(uint8_t*)"Available verbose commands.\n");
	} else {
		return cli_write_string(dev->cli_handler,
					(uint8_t*)"\nAvailable short commands:\n\n");
	}
}

/**
 * @brief Display application commands tooltip.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] short_command - false to display verbose commands;
 *                             true to display the short ones.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_help_general(struct ad7124_8pmdz_dev *dev,
		bool short_command)
{
	if (!short_command) {
		return cli_write_string(dev->cli_handler,
					(uint8_t*)" help                       - Display command menu.\r\n"
					" reset <opt>                - Reset the AD7124 device.\r\n"
					"                              <opt> = 'dev' to perform only a device reset; do not include to perform an application reset.\r\n");
	} else {
		return cli_write_string(dev->cli_handler,
					(uint8_t*)" h                - Display command menu.\r\n"
					" rst <opt>        - Reset the application.\r\n"
					"                    <opt> = 'dev' to perform only a device reset; do not include to perform an application reset.\r\n");
	}
}

/**
 * @brief Display AD7124 device commands tooltip.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] short_command - false to display verbose commands;
 *                             true to display the short ones.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_help_adc(struct ad7124_8pmdz_dev *dev,
				     bool short_command)
{
	if (!short_command) {
		return cli_write_string(dev->cli_handler,
					(uint8_t*)
					" adc_reg_read <addr>        - Read an ADC register of a specific address.\n\r"
					"                              <addr> = Address of the register to be read in hexadecimal base.\n\r"
					"                              Example: adc_reg_read 2a\n\r"
					" adc_reg_write <addr> <val> - Write an ADC register of a specific address with a new value.\n\r"
					"                              <addr> = Address of the register to be written in hexadecimal base.\n\r"
					"                              <val> = New value of the register.\n\r"
					"                              Example: adc_reg_write 9 8002\n\r"
					" adc_get_samples <no>       - Get a number of samples per enabled channels. If the operation takes too long press 'q' to abort.\n\r"
					"                              <no> = Number of samples (maximum 2048).\n\r"
					"                              Example: adc_get_samples 20\n\r"
					" adc_chan_en_set <mask>     - Choose ADC channels to be activated.\n\r"
					"                              <mask> = 16-bit mask of the channels to be activated. Can be hexadecimal or binary.\n\r"
					"                              A bit of 1 means activated the channel, a bit of 0 means deactivate the channel.\n\r"
					"                              Example 1: adc_chan_en_set 0xAAAA = activate every other channel. '0x' is necessary for hexadecimal interpretation.\n\r"
					"                              Example 2: adc_chan_en_set 0b1010101010101010 = activate every other channel. '0b' is necessary for binary interpretation.\n\r"
					" adc_chan_en_get            - Get enable status of ADC channels. Returns a hexadecimal 16-bit mask where bits of 1 represent enabled channels,\n\r"
					"                              and bits of 0 represent disabled channels.\n\r"
					" adc_pga_set <chan> <opt>   - Set PFA for a channel.\n\r"
					"                              <chan> = ID of the channel to be changed.\n\r"
					"                              <opt> = PGA option; values are: opt0, opt1, ... opt7 corresponding to the datasheet.\n\r"
					"                              Example: adc_pga_set 0 opt3 - set ADC channel 0 to PGA 3, gain value of 8.\n\r"
					" adc_pga_get <chan>         - Display a channel's PGA option; return values are: opt0, opt1, ... opt7 corresponding to the datasheet.\n\r"
					"                              <chan> = ID of the channel to be read.\n\r"
					"                              Example: adc_pga_set 0 - read the PGA value of channel 0.\n\r"
					" adc_odr_set <odr>          - Set ADC sample rate. Filter option, power mode and reference clock must be taken into consideration.\n\r"
					"                              <odr> = New sample rate value.\n\r"
					"                              Example: adc_odr_set 2000 - set sample rate to 2000 samples per second.\n\r"
					" adc_odr_get                - Read the current sample rate.\n\r"
					" adc_flt_set <opt> <post>   - Set ADC filter option.\n\r"
					"                              <opt> = filter option; can be: 'sinc4', 'sinc3', 'fflt4', 'fflt3' and 'postf'.\n\r"
					"                              <post> = post-filter option; can be: 'opt0', 'opt1', ... 'opt3'; add only when opt=postf.\n\r"
					"                              Example: adc_flt_set fflt4 - set filter option to fast settling sinc4.\n\r"
					" adc_flt_get                - Read the current filter.\n\r");
	} else {
		return cli_write_string(dev->cli_handler,
					(uint8_t*)" arr <addr>       - Read an ADC register of a specific address.\n\r"
					"                    <addr> - Address of the register to be read in hexadecimal base.\n\r"
					"                    Example: arr 2a\n\r"
					" arw <addr> <val> - Write an ADC register of a specific address with a new value.\n\r"
					"                    <addr> - Address of the register to be written in hexadecimal base.\n\r"
					"                    <val> - New value of the register.\n\r"
					"                    Example: arw 9 8002\n\r"
					" ags <no>         - Get a number of samples per enabled channels. If the operation takes too long press 'q' to abort.\n\r"
					"                    <no> - Number of samples (maximum 2048). If sample rate is smaller than 3200 setting the argument 0 means continuous streaming.\n\r"
					"                    Example: ags 20\n\r"
					" aces <mask>      - Choose ADC channels to be activated.\n\r"
					"                    <mask> - 16-bit mask of the channels to be activated. Can be hexadecimal or binary.\n\r"
					"                    A bit of 1 means activated the channel, a bit of 0 means deactivate the channel.\n\r"
					"                    Example 1: aces 0xAAAA = activate every other channel. '0x' is necessary for hexadecimal interpretation.\n\r"
					"                    Example 2: aces 0b1010101010101010 = activate every other channel. '0b' is necessary for binary interpretation.\n\r"
					" aceg             - Get enable status of ADC channels. Returns a hexadecimal 16-bit mask where bits of 1 represent enabled channels,\n\r"
					"                    and bits of 0 represent disabled channels.\n\r"
					" aps <chan> <opt> - Set PGA for a channel.\n\r"
					"                    <chan> = ID of the channel to be changed.\n\r"
					"                    <opt> = PGA option; values are: opt0, opt1, ... opt7 corresponding to the datasheet.\n\r"
					"                    Example: aps 0 opt3 - set ADC channel 0 to PGA 3, gain value of 8.\n\r"
					" apg <chan>       - Display a channel's PGA option; return values are: opt0, opt1, ... opt7 corresponding to the datasheet.\n\r"
					"                    <chan> = ID of the channel to be read.\n\r"
					"                    Example: apg 0 - read the PGA value of channel 0.\n\r"
					" aos <odr>        - Set ADC sample rate. Filter option, power mode and reference clock must be taken into consideration.\n\r"
					"                    <odr> = New sample rate value.\n\r"
					"                    Example: aos 2000 - set sample rate to 2000 samples per second.\n\r"
					" aog              - Read the current sample rate.\n\r"
					" afs <opt> <post> - Set ADC filter option.\n\r"
					"                    <opt> = filter option; can be: 'sinc4', 'sinc3', 'fflt4', 'fflt3' and 'postf'.\n\r"
					"                    <post> = post-filter option; can be: 'opt0', 'opt1', ... 'opt3'; add only when opt=postf.\n\r"
					"                    Example: afs fflt4 - set filter option to fast settling sinc4.\n\r"
					" afg              - Read the current filter.\n\r");
	}
}

/**
 * Display help options in the CLI.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7124_8pmdz_help(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	int32_t ret;

	ret = ad7124_8pmdz_help_prompt(dev, HELP_LONG_COMMAND);
	if(ret != 0)
		return ret;

	ret = ad7124_8pmdz_help_general(dev, HELP_LONG_COMMAND);
	if(ret != SUCCESS)
		return ret;

	ret = ad7124_8pmdz_help_adc(dev, HELP_LONG_COMMAND);
	if(ret != SUCCESS)
		return ret;

	ret = ad7124_8pmdz_help_prompt(dev, HELP_SHORT_COMMAND);
	if(ret != 0)
		return ret;

	ret = ad7124_8pmdz_help_general(dev, HELP_SHORT_COMMAND);
	if(ret != SUCCESS)
		return ret;

	return ad7124_8pmdz_help_adc(dev, HELP_SHORT_COMMAND);
}

/**
 * @brief CLI command to read and display a device register.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - The address of the register to be displayed.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_reg_read(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	uint8_t reg_addr;
	int32_t ret;
	uint8_t buff[50];

	reg_addr = strtol((char *)arg, NULL, 16);

	if (reg_addr >= AD7124_REG_NO)
		return cli_write_string(dev->cli_handler,
					(uint8_t*)"Wrong register address.\n");
	ret = ad7124_read_register(dev->ad7124_handler,
				   &dev->ad7124_handler->regs[reg_addr]);
	if(ret != SUCCESS)
		return cli_write_string(dev->cli_handler,
					(uint8_t*)"SPI read error.\n");

	sprintf((char *)buff, "Register 0x%s value is: 0x%.6lx\n\r",
		arg, dev->ad7124_handler->regs[reg_addr].value);

	return cli_write_string(dev->cli_handler, buff);
}

/**
 * @brief CLI command to write a device register.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - The address and new value of the register to be displayed.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_reg_write(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	uint8_t reg_addr;
	uint8_t *reg_val_ptr;
	int32_t ret, reg_val;

	reg_val_ptr = (uint8_t *)strchr((char *)arg, ' ');
	*reg_val_ptr = 0;
	reg_val_ptr++;

	reg_addr = strtol((char *)arg, NULL, 16);

	if(reg_addr == AD7124_REG_NO)
		return cli_write_string(dev->cli_handler,
					(uint8_t*)"Wrong register address.\n");
	if(dev->ad7124_handler->regs[reg_addr].rw == AD7124_R)
		return cli_write_string(dev->cli_handler,
					(uint8_t*)"Register is read only.\n");

	reg_val = strtol((char *)reg_val_ptr, NULL, 16);

	dev->ad7124_handler->regs[reg_addr].value = reg_val;
	ret = ad7124_write_register(dev->ad7124_handler,
				    dev->ad7124_handler->regs[reg_addr]);
	if(ret != SUCCESS)
		return cli_write_string(dev->cli_handler,
					(uint8_t*)"Write failed.\n");

	return ret;
}

/**
 * @brief Get number and ID of active channels.
 * @param [in] dev - Pointer to the application handler.
 * @param [out] chann_mask - Mask of the enabled channels: a 1 bit represents an
 *                           active channel and a 0 bit represents a non-active
 *                           one.
 * @param [out] chann_no - Number of active channels.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_get_en_chann(struct ad7124_8pmdz_dev *dev,
		uint16_t *chann_mask, int8_t *chann_no)
{
	int8_t i;
	struct ad7124_st_reg *regs;
	int32_t ret;

	regs = dev->ad7124_handler->regs;
	*chann_mask = 0;
	*chann_no = 0;

	for (i = 0; i < 16; i++) {
		ret = ad7124_read_register(dev->ad7124_handler,
					   &regs[(AD7124_Channel_0 + i)]);
		if (ret != SUCCESS)
			return FAILURE;
		if (regs[(AD7124_Channel_0 + i)].value &
		    AD7124_CH_MAP_REG_CH_ENABLE) {
			*chann_mask |= 1 << i;
			*chann_no += 1;
		}
	}

	return SUCCESS;
}

/**
 * @brief Set ADC in single or continuous conversion, or idle mode.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] enable - true to put device in one of the conversion modes;
 *                      false to put de device in idle.
 * @param [in] sample_no - 1 to put the device in single conversion mode;
 *                         > 1 to put the device in continuous conversion mode.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_data_read_conv_en(struct ad7124_8pmdz_dev *dev,
		bool enable, uint32_t sample_no)
{
	struct ad7124_st_reg *regs;
	int32_t ret;

	regs = dev->ad7124_handler->regs;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &regs[AD7124_ADC_Control]);
	if (ret != SUCCESS)
		return FAILURE;
	regs[AD7124_ADC_Control].value &=
		~AD7124_ADC_CTRL_REG_MODE(0xF);
	if (enable) {
		if (sample_no == 1)
			regs[AD7124_ADC_Control].value |=
				AD7124_ADC_CTRL_REG_MODE(1);
	} else {
		regs[AD7124_ADC_Control].value |=
			AD7124_ADC_CTRL_REG_MODE(2);
	}

	return ad7124_write_register(dev->ad7124_handler,
				     regs[AD7124_ADC_Control]);
}

/**
 * @brief Get the AD7124 reference clock.
 * @param [in] dev - Pointer to the application handler.
 * @param [out] f_clk - Pointer to the clock frequency container.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_fclk_logic_get(struct ad7124_8pmdz_dev *dev,
		float *f_clk)
{
	int32_t ret;
	const float	f_clk_fp = 614400,
			f_clk_mp = 153600,
			f_clk_lp = 76800;
	struct ad7124_st_reg *reg_map;

	reg_map = dev->ad7124_handler->regs;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_ADC_Control]);
	if (ret != SUCCESS)
		return FAILURE;

	switch ((reg_map[AD7124_ADC_Control].value &
		 AD7124_ADC_CTRL_REG_POWER_MODE(3)) >> 6) {
	case 0:
		*f_clk = f_clk_lp;
		break;
	case 1:
		*f_clk = f_clk_mp;
		break;
	case 2:
	case 3:
		*f_clk = f_clk_fp;
		break;
	default:
		return FAILURE;
	}

	return SUCCESS;
}

/**
 * @brief Get the filter coefficient for the sample rate.
 * @param [in] dev - Pointer to the application handler.
 * @param [out] flt_coff - Pointer to the filter coefficient container.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_fltcoff_logic_get(struct ad7124_8pmdz_dev *dev,
		uint16_t *flt_coff)
{
	int32_t ret;
	struct ad7124_st_reg *reg_map;
	uint16_t power_mode;

	reg_map = dev->ad7124_handler->regs;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_ADC_Control]);
	if (ret != SUCCESS)
		return FAILURE;

	power_mode = (reg_map[AD7124_ADC_Control].value &
		      AD7124_ADC_CTRL_REG_POWER_MODE(3)) >> 6;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_Filter_0]);
	if (ret != SUCCESS)
		return FAILURE;

	*flt_coff = 32;
	if (reg_map[AD7124_Filter_0].value & AD7124_FILT_REG_SINGLE_CYCLE) {
		if ((reg_map[AD7124_Filter_0].value &
		     AD7124_FILT_REG_FILTER(7)) ==
		    AD7124_FILT_REG_FILTER(0))
			*flt_coff *= 4;
		if ((reg_map[AD7124_Filter_0].value &
		     AD7124_FILT_REG_FILTER(7)) ==
		    AD7124_FILT_REG_FILTER(2))
			*flt_coff *= 3;
	}
	if ((reg_map[AD7124_Filter_0].value & AD7124_FILT_REG_FILTER(7)) ==
	    AD7124_FILT_REG_FILTER(4)) {
		if (power_mode == 0)
			*flt_coff *= 11;
		else
			*flt_coff *= 19;
	}
	if ((reg_map[AD7124_Filter_0].value & AD7124_FILT_REG_FILTER(7)) ==
	    AD7124_FILT_REG_FILTER(5)) {
		if (power_mode == 0)
			*flt_coff *= 10;
		else
			*flt_coff *= 18;
	}

	return SUCCESS;
}

/**
 * @brief Calculate current sample rate based on the FILTER0 register and
 *        ADC_CONTROL register.
 * @param [in] dev - Pointer to the application handler.
 * @param [out] odr - Pointer to the output data rate container.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_odr_logic_get(struct ad7124_8pmdz_dev *dev,
		float *odr)
{
	int32_t ret;
	float f_clk;
	struct ad7124_st_reg *reg_map;
	uint16_t fs_value, flt_coff;

	reg_map = dev->ad7124_handler->regs;

	ret = ad7124_8pmdz_fclk_logic_get(dev, &f_clk);
	if (ret != SUCCESS)
		return FAILURE;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_Filter_0]);
	if (ret != SUCCESS)
		return FAILURE;

	fs_value = reg_map[AD7124_Filter_0].value & AD7124_FILT_REG_FS(0x7FF);

	if ((reg_map[AD7124_Filter_0].value & AD7124_FILT_REG_FILTER(7)) ==
	    AD7124_FILT_REG_FILTER(7)) {
		switch ((reg_map[AD7124_Filter_0].value &
			 AD7124_FILT_REG_POST_FILTER(7)) >> 17) {
		case 2:
			*odr = 27.27;
			return SUCCESS;
		case 3:
			*odr = 25;
			return SUCCESS;
		case 5:
			*odr = 20;
			return SUCCESS;
		case 6:
			*odr = 16.7;
			return SUCCESS;
		default:
			return FAILURE;
		}
	}

	ret = ad7124_8pmdz_fltcoff_logic_get(dev, &flt_coff);
	if (ret != SUCCESS)
		return FAILURE;

	*odr = f_clk / (float)(flt_coff * fs_value);

	return SUCCESS;
}

/**
 * @brief Display samples when the CLI can keep up with ODR. Burst and
 *        continuous mode.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] sample_no - Number of samples to display.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_data_read_nomem(struct ad7124_8pmdz_dev *dev,
		uint32_t sample_no)
{
	int32_t ret, i, data_temp;
	uint16_t chann_mask, mask_temp;
	int8_t chann_no, chan_idx;
	uint8_t ascii_buff[50], stop_char, temp_indx;
	struct ad7124_st_reg *reg_map;

	if (sample_no == 0)
		sample_no = 0xFFFFFFFF;

	ret = ad7124_8pmdz_get_en_chann(dev, &chann_mask, &chann_no);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: SPI communication.\n\r");
		return FAILURE;
	}

	reg_map = dev->ad7124_handler->regs;

	for (i = 0; i < 16; i++) {
		if (chann_mask & (1 << i)) {
			sprintf((char *)ascii_buff, "CH%-4ld ", i);
			cli_write_string(dev->cli_handler, ascii_buff);
		}
	}
	cli_write_string(dev->cli_handler, (uint8_t*)"\n\r");

	ret = ad7124_8pmdz_data_read_conv_en(dev, true, 2);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR: SPI communication.\n\r");
		return FAILURE;
	}

	while (1) {
		chan_idx = 0;
		mask_temp = chann_mask;
		for (i = 0; i < chann_no; i++) {
			ret = ad7124_wait_for_conv_ready(dev->ad7124_handler,
							 100000);
			if (ret != SUCCESS)
				return FAILURE;
			ret = ad7124_read_data(dev->ad7124_handler, &data_temp);
			if (ret != SUCCESS)
				return FAILURE;
			chan_idx += (int8_t)find_first_set_bit(mask_temp);
			if ((reg_map[AD7124_Status].value &
			     AD7124_STATUS_REG_CH_ACTIVE(0xF)) ==
			    chan_idx) {
				sprintf((char *)ascii_buff, "%.6lx ",
					data_temp);
			} else {
				sprintf((char *)ascii_buff, "%.6lx ", 0ul);
			}
			cli_write_string(dev->cli_handler, ascii_buff);
			mask_temp >>= (find_first_set_bit(mask_temp) + 1);
			chan_idx++;
		}
		cli_write_string(dev->cli_handler, (uint8_t*)"\n\r");
		if (sample_no != 0xFFFFFFFF)
			sample_no--;
		if (sample_no == 0)
			break;
		cli_get_last_character(dev->cli_handler, &stop_char);
		if (stop_char == 'q') {
			cli_get_fifo_indx(dev->cli_handler, &temp_indx);
			cli_set_fifo_indx(dev->cli_handler, (temp_indx - 1));
			break;
		}
	}

	ret = ad7124_8pmdz_data_read_conv_en(dev, false, 0);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: SPI communication.\n\r");
		return FAILURE;
	}

	return SUCCESS;
}

/**
 * @brief Display samples when ODR is too fast for the CLI. Only burst mode.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] sample_no - Number of samples to display.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_data_read_mem(struct ad7124_8pmdz_dev *dev,
		uint32_t sample_no)
{
	int32_t ret, i, j, data_temp;
	struct ad7124_st_reg *reg_map;
	uint16_t chann_mask, mask_temp;
	int8_t chann_no, chan_idx;
	const uint32_t data_tab_max = 2048;
	uint8_t ascii_buff[50], stop_char, temp_indx;
	uint32_t *data_tab;

	reg_map = dev->ad7124_handler->regs;

	ret = ad7124_8pmdz_get_en_chann(dev, &chann_mask, &chann_no);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: SPI communication.\n\r");
		return SUCCESS;
	}

	if ((sample_no * chann_no) > data_tab_max) {
		sprintf((char *)ascii_buff,
			"ERROR: Maximum of %lu samples at once.\n\r",
			(data_tab_max / chann_no));
		cli_write_string(dev->cli_handler, ascii_buff);
		return SUCCESS;
	}

	data_tab = (uint32_t *)calloc(data_tab_max, sizeof(*data_tab));
	if (!data_tab) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: Out of memory.\n\r");
		return SUCCESS;
	}

	ret = ad7124_8pmdz_data_read_conv_en(dev, true, sample_no);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR: SPI communication.\n\r");
		goto exit;
	}

	j = 0;
	while (sample_no--) {
		for (i = 0; i < chann_no; i++) {
			ret = ad7124_wait_for_conv_ready(dev->ad7124_handler,
							 100000);
			if (ret != SUCCESS)
				goto exit;
			ret = ad7124_read_data(dev->ad7124_handler, &data_temp);
			if (ret != SUCCESS)
				goto exit;
			chan_idx = reg_map[AD7124_Status].value &
				   AD7124_STATUS_REG_CH_ACTIVE(0xF);
			data_tab[j + i] = data_temp | (chan_idx << 24);
		}
		j += i;
		cli_get_last_character(dev->cli_handler, &stop_char);
		if (stop_char == 'q') {
			cli_get_fifo_indx(dev->cli_handler, &temp_indx);
			cli_set_fifo_indx(dev->cli_handler, (temp_indx - 1));
			break;
		}
	}
	ret = ad7124_8pmdz_data_read_conv_en(dev, false, sample_no);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: SPI communication.\n\r");
		goto exit;
	}

	for (i = 0; i < 16; i++) {
		if (chann_mask & (1 << i)) {
			sprintf((char *)ascii_buff, "CH%-4ld ", i);
			cli_write_string(dev->cli_handler, ascii_buff);
		}
	}
	cli_write_string(dev->cli_handler, (uint8_t*)"\n\r");

	sample_no = 0;
	j /= chann_no;
	while (j--) {
		mask_temp = chann_mask;
		chan_idx = 0;
		for (i = 0; i < chann_no; i++) {
			chan_idx += (int8_t)find_first_set_bit(mask_temp);
			if ((data_tab[sample_no + i] >> 24) == chan_idx) {
				sprintf((char *)ascii_buff, "%.6lx ",
					(data_tab[sample_no + i] & 0xFFFFFF));
			} else {
				sprintf((char *)ascii_buff, "%.6lx ", 0ul);
			}
			cli_write_string(dev->cli_handler, ascii_buff);
			mask_temp >>= (find_first_set_bit(mask_temp) + 1);
			chan_idx++;
		}
		if (mask_temp != 0)
			cli_write_string(dev->cli_handler,
					 (uint8_t*)"ERROR: Display channel.");
		sample_no += i;
		cli_write_string(dev->cli_handler, (uint8_t*)"\n\r");
	}

exit:
	free(data_tab);

	return SUCCESS;
}

/**
 * @brief CLI command to read and display data from the ADC.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - Number of samples to read.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_data_read(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	uint32_t sample_no;
	float odr;

	if ((arg == NULL) || (*arg == 0))
		sample_no = 0;
	else
		sample_no = atoi((char *)arg);

	ad7124_8pmdz_odr_logic_get(dev, &odr);
	if (odr <= 3200)
		return ad7124_8pmdz_data_read_nomem(dev, sample_no);
	else
		return ad7124_8pmdz_data_read_mem(dev, sample_no);
}

/**
 * @brief CLI command to enable channels.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - Mask of the channels to enable (hexa or binary).
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_channel_en_set(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	int32_t ret;
	uint16_t chann_mask;
	uint8_t *err_ptr;
	int8_t i;
	struct ad7124_st_reg *regs;

	if (strncmp((char *)arg, "0b", 2) == 0) {
		chann_mask = strtol((char *)(arg + 2), (char **)&err_ptr, 2);
	} else if (strncmp((char *)arg, "0x", 2) == 0) {
		chann_mask = strtol((char *)(arg + 2), (char **)&err_ptr, 16);
	} else {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: Incorrect mask format.\n\r");

		return FAILURE;
	}
	if (err_ptr == (arg + 2)) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: Incorrect mask format.\n\r");

		return FAILURE;
	}

	regs = dev->ad7124_handler->regs;

	for (i = 0; i < 16; i++) {
		ret = ad7124_read_register(dev->ad7124_handler,
					   &regs[(AD7124_Channel_0 + i)]);
		if (ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t*)"ERROR: SPI communication.\n\r");

			return FAILURE;
		}
		if ((1 << i) & chann_mask)
			regs[(AD7124_Channel_0 + i)].value |=
				AD7124_CH_MAP_REG_CH_ENABLE;
		else
			regs[(AD7124_Channel_0 + i)].value &=
				~AD7124_CH_MAP_REG_CH_ENABLE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    regs[(AD7124_Channel_0 + i)]);
		if (ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t*)"ERROR: SPI communication.\n\r");

			return FAILURE;
		}
	}

	return SUCCESS;
}

/**
 * @brief CLI command to read the enabled channels.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_channel_en_get(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	int32_t ret;
	uint16_t chann_mask;
	int8_t chann_no;
	uint8_t buff[50];

	ret = ad7124_8pmdz_get_en_chann(dev, &chann_mask, &chann_no);
	if (ret != SUCCESS)
		return cli_write_string(dev->cli_handler,
					(uint8_t*)"ERROR: SPI communication.\n\r");

	sprintf((char *)buff, "Enabled channels mask is 0x%x", chann_mask);

	return cli_write_string(dev->cli_handler, buff);
}

/**
 * @brief CLI command to do a device reset. The device is reset to datasheet
 *        default values.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - Choose no option to reset the application;
 *                   choose 'dev' option to reset just the device.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_reboot(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	int32_t ret, reg_nr;
	struct ad7124_st_reg *reg_map;
	uint8_t dev_flag = 0;

	if (strncmp((char *)arg, "dev", 4) == 0)
		dev_flag = 1;

	ret = ad7124_reset(dev->ad7124_handler);
	if (ret != SUCCESS)
		return FAILURE;

	reg_map = dev->ad7124_handler->regs;

	for(reg_nr = AD7124_Status; (reg_nr < AD7124_REG_NO) && !(ret < 0);
	    reg_nr++) {
		if (reg_map[reg_nr].rw != AD7124_W) {
			ret = ad7124_read_register(dev->ad7124_handler,
						   &reg_map[reg_nr]);
			if (ret != SUCCESS)
				break;
		}

		/* Get CRC State and device SPI interface settings */
		if (reg_nr == AD7124_Error_En) {
			ad7124_update_crcsetting(dev->ad7124_handler);
			ad7124_update_dev_spi_settings(dev->ad7124_handler);
		}
	}

	if (!dev_flag) {
		reg_map[AD7124_ADC_Control].value = 0x05C0;
		reg_map[AD7124_Channel_0].value = 0x0001;
		reg_map[AD7124_Channel_1].value = 0x0043;
		reg_map[AD7124_Channel_2].value = 0x0085;
		reg_map[AD7124_Channel_3].value = 0x00C7;
		reg_map[AD7124_Channel_4].value = 0x0109;
		reg_map[AD7124_Channel_5].value = 0x014B;
		reg_map[AD7124_Channel_6].value = 0x018D;
		reg_map[AD7124_Channel_7].value = 0x01CF;
		reg_map[AD7124_Channel_8].value = 0x0011;
		reg_map[AD7124_Channel_9].value = 0x0031;
		reg_map[AD7124_Channel_10].value = 0x0051;
		reg_map[AD7124_Channel_11].value = 0x0071;
		reg_map[AD7124_Channel_12].value = 0x0091;
		reg_map[AD7124_Channel_13].value = 0x00B1;
		reg_map[AD7124_Channel_14].value = 0x00D1;
		reg_map[AD7124_Channel_15].value = 0x00F1;
		reg_map[AD7124_Config_0].value = 0x0860;
		reg_map[AD7124_Config_1].value = 0x0861;
		reg_map[AD7124_Config_2].value = 0x0862;
		reg_map[AD7124_Config_3].value = 0x0863;
		reg_map[AD7124_Config_4].value = 0x0864;
		reg_map[AD7124_Config_5].value = 0x0865;
		reg_map[AD7124_Config_6].value = 0x0866;
		reg_map[AD7124_Config_7].value = 0x0867;
		reg_map[AD7124_Filter_0].value = 0x060001;
		reg_map[AD7124_Filter_1].value = 0x060001;
		reg_map[AD7124_Filter_2].value = 0x060001;
		reg_map[AD7124_Filter_3].value = 0x060001;
		reg_map[AD7124_Filter_4].value = 0x060001;
		reg_map[AD7124_Filter_5].value = 0x060001;
		reg_map[AD7124_Filter_6].value = 0x060001;
		reg_map[AD7124_Filter_7].value = 0x060001;

		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_ADC_Control]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_0]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_1]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_2]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_3]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_4]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_5]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_6]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_7]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_8]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_9]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_10]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_11]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_12]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_13]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_14]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Channel_15]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Config_0]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Config_1]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Config_2]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Config_3]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Config_4]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Config_5]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Config_6]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Config_7]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Filter_0]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Filter_1]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Filter_2]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Filter_3]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Filter_4]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Filter_5]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Filter_6]);
		if (ret != SUCCESS)
			return FAILURE;
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Filter_7]);
		if (ret != SUCCESS)
			return FAILURE;
	}

	return SUCCESS;
}

/**
 * @brief Set PGA for a channel.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - Channel ID and PGA option.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_chann_pga_set(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	int8_t chann_no, opt_no;
	uint8_t *err_ptr;
	const uint8_t *opt_tab[] = {
		(uint8_t *)"opt0",
		(uint8_t *)"opt1",
		(uint8_t *)"opt2",
		(uint8_t *)"opt3",
		(uint8_t *)"opt4",
		(uint8_t *)"opt5",
		(uint8_t *)"opt6",
		(uint8_t *)"opt7"
	};
	struct ad7124_st_reg *reg_map;
	int32_t ret;

	chann_no = strtol((char *)arg, (char **)&err_ptr, 10);
	if ((chann_no < 0) || (chann_no > 15) || (arg == err_ptr)) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: Incorrect parameters.\n\r");

		return FAILURE;
	}

	opt_no = 0;
	err_ptr++;
	while (opt_no < 8) {
		if (strncmp((char *)err_ptr, (char *)opt_tab[opt_no], 5) == 0)
			break;
		opt_no++;
	}
	if (opt_no >= 8) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: Incorrect option.\n\r");

		return FAILURE;
	}

	reg_map = dev->ad7124_handler->regs;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_Channel_0 + chann_no]);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: Device error.\n\r");

		return FAILURE;
	}
	reg_map[AD7124_Channel_0 + chann_no].value &=
		~AD7124_CH_MAP_REG_SETUP(7);
	reg_map[AD7124_Channel_0 + chann_no].value |=
		AD7124_CH_MAP_REG_SETUP(opt_no);
	ret = ad7124_write_register(dev->ad7124_handler,
				    reg_map[AD7124_Channel_0 + chann_no]);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: Device error.\n\r");

		return FAILURE;
	}

	return SUCCESS;
}

/**
 * @brief Read and display PGA of a channel.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - Channel ID.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_chann_pga_get(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	int8_t chann_no, opt_no;
	uint8_t *err_ptr, ascii_buff[50];
	struct ad7124_st_reg *reg_map;
	int32_t ret;

	chann_no = strtol((char *)arg, (char **)&err_ptr, 10);
	if ((chann_no < 0) || (chann_no > 15) || (arg == err_ptr)) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: Incorrect parameters.\n\r");

		return FAILURE;
	}

	reg_map = dev->ad7124_handler->regs;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_Channel_0 + chann_no]);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: Device error.\n\r");

		return FAILURE;
	}
	opt_no = (reg_map[AD7124_Channel_0 + chann_no].value &
		  AD7124_CH_MAP_REG_SETUP(7)) >> 12;

	sprintf((char *)ascii_buff, "Channel %d PGA option %d; gain of %d\r\n",
		chann_no, opt_no, (int16_t)pow(2,opt_no));

	return cli_write_string(dev->cli_handler, ascii_buff);
}

/**
 * @brief Set output data rate.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] odr - New output data rate.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_odr_logic_set(struct ad7124_8pmdz_dev *dev,
		float odr)
{
	int32_t ret;
	float f_clk;
	uint16_t flt_coff, fs_value;
	struct ad7124_st_reg *reg_map;
	uint8_t i;

	reg_map = dev->ad7124_handler->regs;

	ret = ad7124_8pmdz_fclk_logic_get(dev, &f_clk);
	if (ret != SUCCESS)
		return FAILURE;

	ret = ad7124_8pmdz_fltcoff_logic_get(dev, &flt_coff);
	if (ret != SUCCESS)
		return FAILURE;

	fs_value = (uint16_t)(f_clk / (flt_coff * odr));
	if (fs_value == 0)
		fs_value = 1;
	if (fs_value > 2047)
		fs_value = 2047;

	for (i = 0; i < 8; i++) {
		ret = ad7124_read_register(dev->ad7124_handler,
					   &reg_map[AD7124_Filter_0 + i]);
		if (ret != SUCCESS)
			return FAILURE;
		reg_map[AD7124_Filter_0 + i].value &= ~AD7124_FILT_REG_FS(0x7FF);
		reg_map[AD7124_Filter_0 + i].value |= AD7124_FILT_REG_FS(fs_value);

		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Filter_0 + i]);
		if (ret != SUCCESS)
			return FAILURE;
	}

	return SUCCESS;
}

/**
 * @brief CLI command to set the sampling rate of the device.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - New dievice sampling rate.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_odr_set(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	int32_t ret;
	float odr;
	uint8_t *err_ptr;

	if (!arg || *arg == 0) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR: Incorrect parameters.\n\r");

		return FAILURE;
	}

	odr = strtod((char *)arg, (char **)&err_ptr);
	if (err_ptr == arg) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR: Incorrect parameters.\n\r");

		return FAILURE;
	}

	ret = ad7124_8pmdz_odr_logic_set(dev, odr);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR: Can't set ODR.");

		return FAILURE;
	}

	return ad7124_8pmdz_odr_get(dev, NULL);
}

/**
 * @brief CLI command to get and display the sample rate.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_odr_get(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	int32_t ret;
	float odr;
	uint8_t ascii_buff[50];

	ret = ad7124_8pmdz_odr_logic_get(dev, &odr);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR: Can't get ODR.");

		return FAILURE;
	}
	sprintf((char *)ascii_buff, "New ODR is: %.3f", odr);

	return cli_write_string(dev->cli_handler, ascii_buff);
}

/**
 * @brief Update the filter options for each configuration.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] flt_opt - New filter option.
 * @param [in] post_flt_opt - New post-filter option.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_flt_logic_set(struct ad7124_8pmdz_dev *dev,
		int8_t flt_opt, int8_t post_flt_opt)
{
	int8_t i;
	struct ad7124_st_reg *reg_map;
	int32_t ret;

	reg_map = dev->ad7124_handler->regs;

	for (i = 0; i < 8; i++) {
		ret = ad7124_read_register(dev->ad7124_handler,
					   &reg_map[AD7124_Filter_0 + i]);
		if (ret != SUCCESS)
			return FAILURE;
		reg_map[AD7124_Filter_0 + i].value &=
			~AD7124_FILT_REG_FILTER(7);
		reg_map[AD7124_Filter_0 + i].value |=
			AD7124_FILT_REG_FILTER(flt_opt);
		if (post_flt_opt != 0xAA) {
			reg_map[AD7124_Filter_0 + i].value &=
				~AD7124_FILT_REG_POST_FILTER(7);
			reg_map[AD7124_Filter_0 + i].value |=
				AD7124_FILT_REG_POST_FILTER(post_flt_opt);
		}
		ret = ad7124_write_register(dev->ad7124_handler,
					    reg_map[AD7124_Filter_0 + i]);
		if (ret != SUCCESS)
			return FAILURE;
	}

	return SUCCESS;
}

/**
 * @brief CLI command to update the filter option.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - New filter and post-filter option.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_flt_set(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	const uint8_t *flt_opt_tab[] = {
		(uint8_t *)"sinc4",
		(uint8_t *)"reserved",
		(uint8_t *)"sinc3",
		(uint8_t *)"reserved",
		(uint8_t *)"fflt4",
		(uint8_t *)"fflt3",
		(uint8_t *)"reserved",
		(uint8_t *)"postf "
	};
	const uint8_t *post_flt_opt_tab[] = {
		(uint8_t *)"reserved",
		(uint8_t *)"reserved",
		(uint8_t *)"opt0",
		(uint8_t *)"opt1",
		(uint8_t *)"reserved",
		(uint8_t *)"opt2",
		(uint8_t *)"opt3",
		(uint8_t *)"reserved"
	};
	int8_t flt_opt, post_flt_opt = 0xAA;
	int32_t ret;

	flt_opt = 0;
	while (flt_opt < 8) {
		if (strncmp((char *)arg, (char *)flt_opt_tab[flt_opt], 6) == 0)
			break;
		flt_opt++;
	}
	if (flt_opt >= 8) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR: Incorrect parameters.");

		return FAILURE;
	}

	if (flt_opt == 7) {
		post_flt_opt = 0;
		while (post_flt_opt < 8) {
			if (strncmp((char *)(arg + 6),
				    (char *)post_flt_opt_tab[post_flt_opt],
				    5) == 0)
				break;
			post_flt_opt++;
		}
		if (post_flt_opt >= 8) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR: Incorrect parameters.");

			return FAILURE;
		}
	}

	ret = ad7124_8pmdz_flt_logic_set(dev, flt_opt, post_flt_opt);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR: Can't update device.");

		return FAILURE;
	}

	return SUCCESS;
}

/**
 * @brief Read filter and post-filter options from the device.
 * @param [in] dev - Pointer to the application handler.
 * @param [out] flt_opt - Pointer to the filter option container.
 * @param [out] post_flt_opt - Pointer to the post-filter option container.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_flt_logic_get(struct ad7124_8pmdz_dev *dev,
		int8_t *flt_opt, int8_t *post_flt_opt)
{
	struct ad7124_st_reg *reg_map;
	int32_t ret;

	reg_map = dev->ad7124_handler->regs;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_Filter_0]);
	if (ret != SUCCESS)
		return FAILURE;
	*flt_opt = (reg_map[AD7124_Filter_0].value &
		    AD7124_FILT_REG_FILTER(7)) >> 21;
	*post_flt_opt = (reg_map[AD7124_Filter_0].value &
			 AD7124_FILT_REG_POST_FILTER(7)) >> 17;

	return SUCCESS;
}

/**
 * @brief CLI command to read and display the active filter option.
 * @param [in] dev - Pointer to the application handler.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_flt_get(struct ad7124_8pmdz_dev *dev, uint8_t *arg)
{
	int32_t ret;
	int8_t flt_opt, post_flt_opt;
	uint8_t ascii_buff[50];
	const uint8_t *flt_opt_tab[] = {
		(uint8_t *)"sinc4",
		(uint8_t *)"reserved",
		(uint8_t *)"sinc3",
		(uint8_t *)"reserved",
		(uint8_t *)"sinc4 + sinc1",
		(uint8_t *)"sinc3 + sinc1",
		(uint8_t *)"reserved",
		(uint8_t *)"post-filter"
	};
	const uint8_t *post_flt_opt_tab[] = {
		(uint8_t *)"reserved",
		(uint8_t *)"reserved",
		(uint8_t *)"27.27",
		(uint8_t *)"25",
		(uint8_t *)"reserved",
		(uint8_t *)"20",
		(uint8_t *)"16.7",
		(uint8_t *)"reserved"
	};

	ret = ad7124_8pmdz_flt_logic_get(dev, &flt_opt, &post_flt_opt);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR: Can't read device.");

		return FAILURE;
	}

	sprintf((char *)ascii_buff, "Filter option is: %s",
		flt_opt_tab[flt_opt]);
	cli_write_string(dev->cli_handler, ascii_buff);

	if (flt_opt == 7) {
		sprintf((char *)ascii_buff,
			"; with the post-filter sample rate of %s.\r\n",
			post_flt_opt_tab[post_flt_opt]);
		cli_write_string(dev->cli_handler, ascii_buff);
	} else {
		cli_write_string(dev->cli_handler, (uint8_t *)"\r\n");
	}

	return SUCCESS;
}

/**
 * @brief Allocate memory for the CLI vectors and populate them.
 * @param [in] dev - Pointer to the application handler.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_init_tabs(struct ad7124_8pmdz_dev *dev)
{
	dev->cmd_fun_tab = calloc((AD7124_8PMDZ_CMD_NO + 1),
				  sizeof *dev->cmd_fun_tab);
	if(!dev->cmd_fun_tab)
		return FAILURE;
	dev->cmd_fun_tab[0] = (cmd_func)ad7124_8pmdz_help;
	dev->cmd_fun_tab[1] = (cmd_func)ad7124_8pmdz_reg_read;
	dev->cmd_fun_tab[2] = (cmd_func)ad7124_8pmdz_reg_write;
	dev->cmd_fun_tab[3] = (cmd_func)ad7124_8pmdz_data_read;
	dev->cmd_fun_tab[4] = (cmd_func)ad7124_8pmdz_channel_en_set;
	dev->cmd_fun_tab[5] = (cmd_func)ad7124_8pmdz_channel_en_get;
	dev->cmd_fun_tab[6] = (cmd_func)ad7124_8pmdz_reboot;
	dev->cmd_fun_tab[7] = (cmd_func)ad7124_8pmdz_chann_pga_set;
	dev->cmd_fun_tab[8] = (cmd_func)ad7124_8pmdz_chann_pga_get;
	dev->cmd_fun_tab[9] = (cmd_func)ad7124_8pmdz_odr_set;
	dev->cmd_fun_tab[10] = (cmd_func)ad7124_8pmdz_odr_get;
	dev->cmd_fun_tab[11] = (cmd_func)ad7124_8pmdz_flt_set;
	dev->cmd_fun_tab[12] = (cmd_func)ad7124_8pmdz_flt_get;

	dev->cmd_name_size_tab = calloc((AD7124_8PMDZ_CMD_NO * 2 + 1),
					sizeof *dev->cmd_name_size_tab);
	if(!dev->cmd_name_size_tab)
		return FAILURE;
	dev->cmd_name_size_tab[AD7124_8PMDZ_CMD_NO * 2] = 1;
	dev->cmd_name_size_tab[0] = 5;
	dev->cmd_name_size_tab[1] = 2;
	dev->cmd_name_size_tab[2] = 13;
	dev->cmd_name_size_tab[3] = 4;
	dev->cmd_name_size_tab[4] = 14;
	dev->cmd_name_size_tab[5] = 4;
	dev->cmd_name_size_tab[6] = 16;
	dev->cmd_name_size_tab[7] = 4;
	dev->cmd_name_size_tab[8] = 16;
	dev->cmd_name_size_tab[9] = 5;
	dev->cmd_name_size_tab[10] = 16;
	dev->cmd_name_size_tab[11] = 5;
	dev->cmd_name_size_tab[12] = 5;
	dev->cmd_name_size_tab[13] = 3;
	dev->cmd_name_size_tab[14] = 12;
	dev->cmd_name_size_tab[15] = 4;
	dev->cmd_name_size_tab[16] = 12;
	dev->cmd_name_size_tab[17] = 4;
	dev->cmd_name_size_tab[18] = 12;
	dev->cmd_name_size_tab[19] = 4;
	dev->cmd_name_size_tab[20] = 12;
	dev->cmd_name_size_tab[21] = 4;
	dev->cmd_name_size_tab[22] = 12;
	dev->cmd_name_size_tab[23] = 4;
	dev->cmd_name_size_tab[24] = 12;
	dev->cmd_name_size_tab[25] = 4;

	dev->cmd_name_tab = calloc((AD7124_8PMDZ_CMD_NO * 2 + 1),
				   sizeof *dev->cmd_name_tab);
	if(!dev->cmd_name_tab)
		return FAILURE;
	dev->cmd_name_tab[AD7124_8PMDZ_CMD_NO * 2] = (uint8_t *)"";
	dev->cmd_name_tab[0] = (uint8_t *)"help";
	dev->cmd_name_tab[1] = (uint8_t *)"h";
	dev->cmd_name_tab[2] = (uint8_t *)"adc_reg_read ";
	dev->cmd_name_tab[3] = (uint8_t *)"arr ";
	dev->cmd_name_tab[4] = (uint8_t *)"adc_reg_write ";
	dev->cmd_name_tab[5] = (uint8_t *)"arw ";
	dev->cmd_name_tab[6] = (uint8_t *)"adc_get_samples ";
	dev->cmd_name_tab[7] = (uint8_t *)"ags ";
	dev->cmd_name_tab[8] = (uint8_t *)"adc_chan_en_set ";
	dev->cmd_name_tab[9] = (uint8_t *)"aces ";
	dev->cmd_name_tab[10] = (uint8_t *)"adc_chan_en_get";
	dev->cmd_name_tab[11] = (uint8_t *)"aceg";
	dev->cmd_name_tab[12] = (uint8_t *)"reset";
	dev->cmd_name_tab[13] = (uint8_t *)"rst";
	dev->cmd_name_tab[14] = (uint8_t *)"adc_pga_set ";
	dev->cmd_name_tab[15] = (uint8_t *)"aps ";
	dev->cmd_name_tab[16] = (uint8_t *)"adc_pga_get ";
	dev->cmd_name_tab[17] = (uint8_t *)"apg ";
	dev->cmd_name_tab[18] = (uint8_t *)"adc_odr_set ";
	dev->cmd_name_tab[19] = (uint8_t *)"aos ";
	dev->cmd_name_tab[20] = (uint8_t *)"adc_odr_get";
	dev->cmd_name_tab[21] = (uint8_t *)"aog";
	dev->cmd_name_tab[22] = (uint8_t *)"adc_flt_set ";
	dev->cmd_name_tab[23] = (uint8_t *)"afs ";
	dev->cmd_name_tab[24] = (uint8_t *)"adc_flt_get";
	dev->cmd_name_tab[25] = (uint8_t *)"afg";

	return SUCCESS;
}

/**
 * TODO: might not be necessary of good.
 * @brief Do full device calibration.
 * @param [in] dev - Pointer to the application handler.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t ad7124_8pmdz_init_calibration(struct ad7124_8pmdz_dev *dev)
{
	int32_t ret;
	struct ad7124_st_reg *reg_map;
	uint16_t flag_data;

	reg_map = dev->ad7124_handler->regs;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_Channel_0]);
	if(ret != SUCCESS)
		return FAILURE;
	reg_map[AD7124_Channel_0].value |= AD7124_CH_MAP_REG_CH_ENABLE;
	ret = ad7124_write_register(dev->ad7124_handler,
				    reg_map[AD7124_Channel_0]);
	if(ret != SUCCESS)
		return FAILURE;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_ADC_Control]);
	if(ret != SUCCESS)
		return FAILURE;
	reg_map[AD7124_ADC_Control].value &= ~AD7124_ADC_CTRL_REG_MODE(0xf);
	reg_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(0x5);
	ret = ad7124_write_register(dev->ad7124_handler,
				    reg_map[AD7124_ADC_Control]);
	if(ret != SUCCESS)
		return FAILURE;
	do {
		ret = ad7124_read_register(dev->ad7124_handler,
					   &reg_map[AD7124_ADC_Control]);
		if(ret != SUCCESS)
			return FAILURE;
		flag_data = (reg_map[AD7124_ADC_Control].value &
			     AD7124_ADC_CTRL_REG_MODE(0xf)) >> 2;
	} while (flag_data == 4);

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_ADC_Control]);
	if(ret != SUCCESS)
		return FAILURE;
	reg_map[AD7124_ADC_Control].value &= ~AD7124_ADC_CTRL_REG_MODE(0xf);
	reg_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(0x6);
	ret = ad7124_write_register(dev->ad7124_handler,
				    reg_map[AD7124_ADC_Control]);
	if(ret != SUCCESS)
		return FAILURE;
	do {
		ret = ad7124_read_register(dev->ad7124_handler,
					   &reg_map[AD7124_ADC_Control]);
		if(ret != SUCCESS)
			return FAILURE;
		flag_data = (reg_map[AD7124_ADC_Control].value &
			     AD7124_ADC_CTRL_REG_MODE(0xf)) >> 2;
	} while (flag_data == 4);

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_ADC_Control]);
	if(ret != SUCCESS)
		return FAILURE;
	reg_map[AD7124_ADC_Control].value &= ~AD7124_ADC_CTRL_REG_MODE(0xf);
	reg_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(0x6);
	ret = ad7124_write_register(dev->ad7124_handler,
				    reg_map[AD7124_ADC_Control]);
	if(ret != SUCCESS)
		return FAILURE;
	do {
		ret = ad7124_read_register(dev->ad7124_handler,
					   &reg_map[AD7124_ADC_Control]);
		if(ret != SUCCESS)
			return FAILURE;
		flag_data = (reg_map[AD7124_ADC_Control].value &
			     AD7124_ADC_CTRL_REG_MODE(0xf)) >> 2;
	} while (flag_data == 4);

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_Channel_0]);
	if(ret != SUCCESS)
		return FAILURE;
	reg_map[AD7124_Channel_0].value &= ~AD7124_CH_MAP_REG_CH_ENABLE;
	ret = ad7124_write_register(dev->ad7124_handler,
				    reg_map[AD7124_Channel_0]);
	if(ret != SUCCESS)
		return FAILURE;

	ret = ad7124_read_register(dev->ad7124_handler,
				   &reg_map[AD7124_ADC_Control]);
	if(ret != SUCCESS)
		return FAILURE;
	reg_map[AD7124_ADC_Control].value &= ~AD7124_ADC_CTRL_REG_MODE(0xf);
	reg_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(0x3);
	ret = ad7124_write_register(dev->ad7124_handler,
				    reg_map[AD7124_ADC_Control]);
	if(ret != SUCCESS)
		return FAILURE;

	return SUCCESS;
}

/**
 * @brief Setup the application.
 * @param [out] device - Pointer to the device handler.
 * @param [in] init_param - Pointer to the initialization structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_init(struct ad7124_8pmdz_dev **device,
			  struct ad7124_8pmdz_init_param *init_param)
{
	int32_t ret;
	struct ad7124_8pmdz_dev *dev;
	struct callback_desc cli_cb;

	pwr_setup();

	dev = calloc(1, sizeof *dev);
	if(!dev)
		return FAILURE;

	ret = cli_setup(&dev->cli_handler, &init_param->cli_initial);
	if(ret != SUCCESS)
		goto error_dev;
	ret = ad7124_8pmdz_init_tabs(dev);
	if(ret != SUCCESS)
		goto error_cli;
	cli_load_command_vector(dev->cli_handler, dev->cmd_fun_tab);
	cli_load_command_calls(dev->cli_handler, dev->cmd_name_tab);
	cli_load_command_sizes(dev->cli_handler, dev->cmd_name_size_tab);
	cli_load_descriptor_pointer(dev->cli_handler, dev);

	ret = irq_ctrl_init(&dev->irq_handler, &init_param->irq_init);
	if (IS_ERR_VALUE(ret))
		goto error_cli;
	cli_cb.callback = cli_uart_callback;
	cli_cb.config = dev->cli_handler->uart_device;
	cli_cb.ctx = dev->cli_handler;
	ret = irq_register_callback(dev->irq_handler, ADUCM_UART_INT_ID,
				    &cli_cb);
	if (IS_ERR_VALUE(ret))
		goto error_irq;
	ret = irq_global_enable(dev->irq_handler);
	if (IS_ERR_VALUE(ret))
		goto error_irq;
	ret = irq_enable(dev->irq_handler, ADUCM_UART_INT_ID);
	if (IS_ERR_VALUE(ret))
		goto error_irq;

	ret = ad7124_setup(&dev->ad7124_handler, init_param->ad7124_initial);
	if(ret != SUCCESS)
		goto error_irq;

//	ret = ad7124_8pmdz_init_calibration(dev);
//	if(ret != SUCCESS)
//		goto error_cli;

	*device = dev;

	return SUCCESS;

error_irq:
	irq_ctrl_remove(dev->irq_handler);
error_cli:
	cli_remove(dev->cli_handler);
error_dev:
	free(dev);

	return FAILURE;
}

/**
 * @brief Free memory allocated by ad7124_8pmdz_init().
 * @param [in] dev - Pointer to the application handler.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_remove(struct ad7124_8pmdz_dev *dev)
{
	int32_t ret;

	if(!dev)
		return FAILURE;

	ret = cli_remove(dev->cli_handler);
	if(ret != SUCCESS)
		return FAILURE;

	ret = irq_ctrl_remove(dev->irq_handler);
	if(ret != SUCCESS)
		return FAILURE;

	ret = ad7124_remove(dev->ad7124_handler);
	if(ret != SUCCESS)
		return FAILURE;

	free(dev);

	return SUCCESS;
}

/**
 * @brief Application process.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_process(struct ad7124_8pmdz_dev *dev)
{
	return cli_process(dev->cli_handler);
}

/**
 * @brief Application prompt wrapper used as API for layers above the
 *        application.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t ad7124_8pmdz_prompt(struct ad7124_8pmdz_dev *dev)
{
	uint8_t app_name[] = {
		'A', 'D', '7', '1', '2', '4', '_', '8', 'P', 'M', 'D', 'Z', '\0'
	};

	return cli_cmd_prompt(dev->cli_handler, app_name);
}
