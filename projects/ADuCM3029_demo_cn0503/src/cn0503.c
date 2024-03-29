/***************************************************************************//**
 *   @file   cn0503.c
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include "cn0503.h"
#include "error.h"
#include "timer.h"
#include "gpio.h"
#include "delay.h"
#include "util.h"
#include "uart_extra.h"
#include "power.h"
#include "irq_extra.h"
#include "spi_extra.h"
#include "app_config.h"

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

static uint8_t *cn0503_ascii_modes[] = {
	(uint8_t *)"CODE",
	(uint8_t *)"ARAT",
	(uint8_t *)"RRAT",
	(uint8_t *)"INS1",
	(uint8_t *)"INS2",
	(uint8_t *)""
};

static uint8_t *cn0503_ascii_code_id[] = {
	(uint8_t *)"A1",
	(uint8_t *)"A2",
	(uint8_t *)"B1",
	(uint8_t *)"B2",
	(uint8_t *)"C1",
	(uint8_t *)"C2",
	(uint8_t *)"D1",
	(uint8_t *)"D2",
	(uint8_t *)"E1",
	(uint8_t *)"E2",
	(uint8_t *)"F1",
	(uint8_t *)"F2",
	(uint8_t *)"G1",
	(uint8_t *)"G2",
	(uint8_t *)"H1",
	(uint8_t *)"H2",
	(uint8_t *)"I1",
	(uint8_t *)"I2",
	(uint8_t *)"J1",
	(uint8_t *)"J2",
	(uint8_t *)"K1",
	(uint8_t *)"K2",
	(uint8_t *)"L1",
	(uint8_t *)"L2",
	(uint8_t *)""
};

static uint8_t cn0503_ascii_op[] = {'+', '-', '*', '/', 0};

static uint8_t *cn0503_defparam_vars[] = {
	(uint8_t *)"ARAT", /* Define the absolute ration math */
	(uint8_t *)"RFLT", /* Define the low pass filter for the ARAT */
	(uint8_t *)"ALRM", /* Set high/low thresholds for alarm triggers */
	(uint8_t *)"RATB", /* Define the baseline ratio */
	(uint8_t *)"INS1", /* Define the polynomial for the first instrument */
	(uint8_t *)"INS2", /* Define the polynomial for the second instrument */
	(uint8_t *)"SUBE", /* Enable/Disable the "1-" in RRAT calculation */
	(uint8_t *)""
};

static uint8_t *cn0503_rations_ascii_id[] = {
	(uint8_t *)"RAT0",
	(uint8_t *)"RAT1",
	(uint8_t *)"RAT2",
	(uint8_t *)"RAT3",
	(uint8_t *)"RAT4",
	(uint8_t *)"RAT5",
	(uint8_t *)"RAT6",
	(uint8_t *)"RAT7",
	(uint8_t *)""
};

int8_t *impresp_method_tab[] = {
	(int8_t *)"IMP",
	(int8_t *)"TIA",
	(int8_t *)"SSI",
	(int8_t *)""
};

int8_t *fluo_preset_tab[] = {
	(int8_t *)"IMPPRESET",
	(int8_t *)"TIAPRESET",
	(int8_t *)"SSIPRESET",
	(int8_t *)"MAXRESPRESET",
	(int8_t *)""
};

enum {
	COLORIMETRY,
	FLUORESCENCE,
	TURBIDITY
};

enum cn0503_impulse_response_methods {
	IMP,
	TIA,
	SSI
};

static uint8_t fluo_start_time_offset_per_method[] =
{7, 1.5, 2};

enum cn0503_fluo_presets {
	IMPPRESET,
	TIAPRESET,
	SSIPRESET,
	MAXRESPRESET
};

enum cn0503_impulse_response_args {
	CHANN_NO,
	LED_WIDTH,
	NB_SAMPLES,
	SAMPLE_PERIOD,
	METHOD,
	AVERAGE_LENGTH,
	FIRST_SAMPLE_OFFSET,
	ACQUISITION_WIDTH,
	CALIB_SLOT
};

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Help command helper function. Display help function prompt.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_help_prompt(struct cn0503_dev *dev)
{
	int32_t ret;

	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)"\tCN0503 application.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "Type a command and press 'Enter'. The commands are not case sensitive.\n");
	if(ret != SUCCESS)
		return ret;
	return cli_write_string(dev->cli_handler,
				(uint8_t*)"Available commands.\n\n");
}

/**
 * @brief Display the help tooltip for the CLI.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_help(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;

	ret = cn0503_help_prompt(dev);
	if(ret != SUCCESS)
		return ret;

	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " REG? XXX                        - Read an ADPD register.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XXX = register address in hexadecimal.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " REG XXX YYYY                    - Write an ADPD register.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XXX = register address in hexadecimal.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   YYYY = register new value in hexadecimal.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " MODE?                             Read the data display mode. Return a 4 character code:\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   CODE - raw data is displayed;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   ARAT - absolute ratio is displayed;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   RRAT - relative ratio is displayed;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   INS1 - data processed for instrument 1 is displayed;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   INS2 - data processed for instrument 2 is displayed.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " MODE XXXX                       - Set the data display mode.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XXXX = 4 character code to describe the data display mode. The options are:\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   CODE - raw data is displayed;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   ARAT - absolute ratio is displayed;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   RRAT - relative ratio is displayed;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   INS1 - data processed for instrument 1 is displayed.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   INS2 - data processed for instrument 2 is displayed.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " STREAM X                        - Start calculating and displaying a stream of data.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   X = If 0 or not present enter continuous streaming mode; if N>0 stream data for N values.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " IDLE?                           - Queries the idle condition.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " IDLE X                          - If X=0 or not listed only the terminal stream is terminated, but the application continues sampling.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   If X=1 stop sampling altogether.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " ALRM?                           - Query alarm status; if returned 0 no alarm is present;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   If bit 0 = 1 instrument 2 value is below lower threshold;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   If bit 1 = 1 instrument 2 value is above higher threshold.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " DEFn? XXXX                      - Query operation parameters.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   n = ID of the required optical path (can be from 0 to 7).\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XXXX = the operation parameter to be queried. The following options exist:\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   ARAT = absolute ratio expression in reverse polish notation (RPN);\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   RFLT = the digital low pass filter bandwidth applied to the absolute ratio measurements;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   ALRM = the high and low alarm threshold for the INS2 measurements;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   RATB = baseline ratio used in calculating the relative ratio;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   INS1 = the coefficients of the fifth order polynomial used to calculate the first instrument measurements;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   INS2 = the coefficients of the fifth order polynomial used to calculate the second instrument measurements.\n");
	if(ret != SUCCESS)
		return ret;

	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " DEFn ARAT RPN                   - Set the absolute ratio expression;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Example: DEF0 ARAT A2A1/ = set the optical path 0 absolute ratio to be calculated as (slotA_ch2 / slotA_ch1);\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " DEFn RFLT XX.XX                 - Set the digital filter bandwidth, in Hz, used for the absolute ratio measurement;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Example: DEF1 RFLT 0.5 = set the optical path 1 digital filter bandwidth to 0.5 Hz. Not all values are possible, query the parameter to get the actual value set;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " DEFn ALRM XXXX YYYY             - Set high and low alarm thresholds for instrument 2 measurements;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Example: DEF0 ALRM 25 15 = set optical path 0 high alarm threshold to 25 and low alarm threshold to 15;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " DEFn RATB X.XXXXX               - Set baseline ratio for to calculate relative ratio of the signal path;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Example: DEF2 RATB 2.3 = set the baseline ratio as 2.3 for signal path 2;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " DEFn INS1 X.XXE+XX Y.YYE+YY ... - Set the fifth order polynomial coefficients for calculating instrument 1, ordered from lowest to highest power;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Example: DEF0 INS1 0 1 0.54 1.23e+0 5.48e-4 44.22 = set coefficients for optical path 0;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " DEFn INS2 X.XXE+XX Y.YYE+YY ... - Set the fifth order polynomial coefficients for calculating instrument 2, ordered from lowest to highest power;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Example: DEF4 INS2 0 1 0.54 1.23e+0 5.48e-4 44.22 = set coefficients for optical path 4;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " BOOT                            - Perform a software reset.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " ODR?                            - Query the Output Data Rate of the STREAM command.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " ODR XX.XX                       - Sets Output Data Rate for the STREAM command.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XX.XX = The new ODR in Hz. It can have only certain values and will round down to them:\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   ODR options between 5 Hz and 0.01 Hz and can be calculated by 5/ODR = integer number.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " RATMASK?                        - Query the active channels mask. Returns a hexadecimal 8 bit number that has 1 for every active channel and 0 for every hidden channel.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Example: 1 - only optical path 0 is turned on; 3 - Optical paths 0 and 1 are turned on and the rest are hidden; etc.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " RATMASK XX                      - Set the active channels mask.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XX = New optical path mask.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Example: 1 - only optical path 0 is turned on; 3 - Optical paths 0 and 1 are turned on and the rest are hidden; etc.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " FL_HELP                         - Display the tooltip for the flash commands.\n");
	if (ret != SUCCESS)
		return FAILURE;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " PCB-LEDn YY.Y XXX.X             - Do an automatic calibration of the specified PCB LED current so that the corresponding ADC channels returns a percentage of the saturation.\n");
	if (ret != SUCCESS)
		return FAILURE;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   n = LED ID from the PCB.\n");
	if (ret != SUCCESS)
		return FAILURE;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   YY.Y = percentage of ADC saturation.\n");
	if (ret != SUCCESS)
		return FAILURE;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XXX.X = maximum LED current. If left not specified will default at 338 mA.\n");
	if (ret != SUCCESS)
		return FAILURE;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " CHANNn XXXX                     - Set one optical path for a predefined type of measurement.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   n = optical path (1, 2, 3 or 4);\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XXXX = measurement option. The following options exist:\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   COLO = colorimetry;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   FLUO = fluorescence;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   TURB = turbidity.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Settings that are changed for the channel are: ARAT expression, SUBE parameter and LED current which is set to default.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   RATB parameter is also defaulted to zero. The user needs to check the jumper positions on the board and consider running a PCB-LED command after.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                      Example: CHANN1 COLO - set channel 1 up for colorimetry measurement.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " IMPRESP N XX YY.YY ZZ.ZZ AAAA [BB] [CC]\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                 - Measure the impulse response of the specified channel channel.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   N = optical path (1, 2, 3, or 4);\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XX = length of initial LED pulse, in microseconds;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   YY.YY = acquisition width, in microseconds;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   ZZ.ZZ = sample period, in microseconds. The sample period has the folowing lower bounds:\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   For impulse response mode (IMP), 2 microseconds;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   For TIA/ADC mode (TIA), 1 microsecond;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   For single-sided integration mode (SSI), 0.03125 microseconds.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   AAAA = sampling method. The following options exist:\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   IMP = impulse response mode;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   TIA = TIA/ADC mode;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   SSI = single-sided integration mode.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   BB [OPTIONAL] = number of trials to average over, default 40;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   CC [OPTIONAL] = Start of sampling, measured from the specified LED turnoff time.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   The default value is the empirically measured end of the LED pulse:\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   For IMP mode, 6 microseconds; For TIA, 0 microseconds; For SSI, 3 microseconds.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "  FLUO_CALIB                     - Perform calibration for the fluorescence decay measurement. \n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   For best results, emulate the experimental setup of subsequent decay measurements, removing any fluorophores.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Data and parameters from the most recent calibration call are stored in flash memory.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   Two calibration curves can be stored in flash: one in calibration \"slot\" 1 and one in calibration \"slot\" 2.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   There are two options for the order of arguments: \n");
	if(ret != SUCCESS)
		return ret;

	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "  FLUO_CALIB XXXX N M YY.YY [ZZ] - Use a preset set of parameters\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XXX = preset option. The following options exist:\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   IMPPRESET = use impulse response mode, with a 50us LED pulse and 2us sample period;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   TIAPRESET = use TIA/ADC mode, with a 50us LED pulse and 1us sample period;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   SSIPRESET = use single-sided integration mode, with a 50us LED pulse and 0.25us sample period;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   MAXRESPRESET = use single-sided integration mode, with a 50us LED pulse and 0.03125us sample period.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   N = calibration slot to read from, 1 or 2;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   M = optical path, 1 or 4;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   YY.YY = Length of acquisition region, in microseconds.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   ZZ [OPTIONAL] Number of trials to average over.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " FLUO_CALIB N M XX YY.YY ZZ.ZZ AAAA [BB] [CC]\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   N = calibration slot to read from, 1 or 2;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   M = optical path, 1 or 4;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                 - See the IMPRESP command for the definition of the subsequent arguments.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " FLUO_DECAY                      - Measure the decay time of a fluorophore in the specified optical path.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   There are three options for the order of arguments:\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " FLUO_DECAY XXXX N M YY.YY [ZZ]    - Use a preset set of parameters. See the FLUO_CALIB command for the definition of each argument.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " FLUO_DECAY N M XX.XX YY.YY [AA] [BB] - Measure the decay time, subtracting out the calibration response.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   The LED pulse width and sampling method are automatically set to their values in the most recent calibration.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   N = calibration slot to read from, 1 or 2;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   M = optical path, 1 or 4;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   XX.XX = width of the acquisition region, in microseconds;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   YY.YY = sample period, with lower bounds specified in the IMPRESP command;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   AA [OPTIONAL] = number of trials to average over, default 40;\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                   BB [OPTIONAL] = start of sampling, offset from LED turnoff time. Defaults to the value set from the most recent calibration.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " FLUO_DECAY NOCALIB N XX YY.YY ZZ.ZZ AAAA [BB] [CC]\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                 - Measure the decay response, skipping calibration.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                                 - See the IMPRESP command for the definition of each argument listed after the NOCALIB keyword.\n");
	if(ret != SUCCESS)
		return ret;

	return cli_write_string(dev->cli_handler,
				(uint8_t*)
				" FLUO_CALIB_DUMP                 - Dump the parameters and data from the most recent calibration call to the CLI.\n");
}

/**
 * @brief Display the help tooltip for the CLI flash commands.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_help_flash(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;

	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)" fl_clearbuf    - Clear the software buffer.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " fl_load <x>    - Load the software buffer with data from flash configuration page.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  <x> = 0 to load from User Update page; 1 to load from the Manufacture Default page.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " fl_program <x> - Program the flash configuration page with data from the software buffer.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  <x> = 0 to program the User Update page; key to program the Manufacture Default page.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)" fl_erase <x>   - Erase the flash configuration page.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  <x> = 0 to erase the User Update page; key to erase the Manufacture Default page.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " fl_apply       - Settings currently in the software buffer are applied to the application and device.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " fl_write <cmd> - Save a specific device or application parameter into the software buffer.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  <cmd> = register or application command to input the relevant parameter.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  For register commands the buffer contains an array of register-value pairs that\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  will be written in order to the device. Calling this function with a register command\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  will add a new register-value pair at the end of the array.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  Example: fl_write reg 10A 3355 = write the register 0x10A to the value 0x3355.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  The only application command supported by this command is the def command. Calling the\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  def command will update that value into the software buffer. It can be later applied to\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)"                  the application.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  Example: fl_write def2 RFLT 0.01 = set filter bandwidth of ration 2 to 0.01Hz\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       " fl_read <cmd>  - Read a specific device or application parameter value from the software buffer.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  <cmd> = register or application command to input the relevant parameter.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  For register commands the buffer contains an array of register-value pairs that\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  will be written in order to the device. Calling this function with a regiter command\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  will display the value of the first occurrence of the register.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  Example: fl_read reg? 10A = return the first value of the register 0x10A\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  The only application command supported by this command is the def command. Calling the\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  def command will read that value from the software buffer.\n");
	if(ret != SUCCESS)
		return ret;
	ret = cli_write_string(dev->cli_handler,
			       (uint8_t*)
			       "                  Example: fl_read def2? RFLT = display the setting of the ration 2 filter bandwidth from the software buffer.\n");
	if(ret != SUCCESS)
		return ret;
	return cli_write_string(dev->cli_handler,
				(uint8_t*)
				" fl_dump        - Print flash buffer in configuration file format for easy saving.\n");
}

/**
 * @brief CLI command to read a register from the ADPD device.
 * @param [in] dev - The device structure.
 * @param [in] arg - The address of the register.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_reg_read(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;
	uint16_t addr, reg_val;
	uint8_t buff[20];

	addr = strtol((char *)arg, NULL, 16);

	ret = adpd410x_reg_read(dev->adpd4100_handler, addr, &reg_val);
	if(ret != SUCCESS)
		return FAILURE;

	itoa(reg_val, (char *)buff, 16);
	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: REG? ");
	cli_write_string(dev->cli_handler, arg);
	cli_write_string(dev->cli_handler, (uint8_t *)"=");
	if(reg_val < 0x1000)
		cli_write_string(dev->cli_handler, (uint8_t *)"0");
	if(reg_val < 0x100)
		cli_write_string(dev->cli_handler, (uint8_t *)"0");
	if(reg_val < 0x10)
		cli_write_string(dev->cli_handler, (uint8_t *)"0");
	cli_write_string(dev->cli_handler, buff);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief CLI command to write a register from the ADPD device.
 * @param [in] dev - The device structure.
 * @param [in] arg - The address of the register and the new value in
 *                   hexadecimal.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_reg_write(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;
	uint8_t *val_ptr;
	uint16_t addr, val;

	val_ptr = (uint8_t *)strchr((char *)arg, ' ');
	if(!val_ptr)
		return FAILURE;
	*val_ptr = 0;
	val_ptr++;

	addr = strtol((char *)arg, NULL, 16);
	val = strtol((char *)val_ptr, NULL, 16);

	ret = adpd410x_reg_write(dev->adpd4100_handler, addr, val);
	if(ret != SUCCESS)
		return FAILURE;

	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: REG ");
	cli_write_string(dev->cli_handler, arg);
	cli_write_string(dev->cli_handler, (uint8_t *)" ");
	cli_write_string(dev->cli_handler, val_ptr);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief Display the stream mode of the application.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_mode_get(struct cn0503_dev *dev, uint8_t *arg)
{
	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: MODE?=");
	cli_write_string(dev->cli_handler, cn0503_ascii_modes[dev->mode]);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief Set the stream mode of the application.
 * @param [in] dev - The device structure.
 * @param [in] arg - New stream mode option.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_mode_set(struct cn0503_dev *dev, uint8_t *arg)
{
	int8_t i = 0;

	while(arg[i]) {
		arg[i] = toupper(arg[i]);
		i++;
	}

	i = 0;
	while(cn0503_ascii_modes[i][0] != 0) {
		if(strncmp((char *)arg, (char *)cn0503_ascii_modes[i], 5) == 0)
			break;
		i++;
	}
	if(i > CN0503_INS2)
		return FAILURE;

	dev->mode = i;

	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: MODE ");
	cli_write_string(dev->cli_handler, cn0503_ascii_modes[dev->mode]);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief Display stream header at the start of the stream.
 * @param [in] dev - The device structure.
 * @param [in] mode - Current stream mode.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_stream_prompt(struct cn0503_dev *dev, uint8_t mode)
{
	int8_t i = 0;

	cli_write_string(dev->cli_handler, (uint8_t *)"STREAM: ");
	if(mode == CN0503_CODE) {
		do {
			cli_write_string(dev->cli_handler,
					 cn0503_ascii_code_id[i * 2]);
			cli_write_string(dev->cli_handler, (uint8_t *)" ");
			if(dev->two_channel_slots & (1 << i)) {
				cli_write_string(dev->cli_handler,
						 cn0503_ascii_code_id[i * 2 + 1]);
				cli_write_string(dev->cli_handler,
						 (uint8_t *)" ");
			}
			i++;
		} while(i < dev->active_slots);
	} else {
		for(i = 0; i < CN0503_RAT_NO; i++) {
			if(dev->ratmask & (1 << i)) {
				cli_write_string(dev->cli_handler,
						 cn0503_rations_ascii_id[i]);
				cli_write_string(dev->cli_handler,
						 (uint8_t *)" ");
			}
		}
	}

	return SUCCESS;
}

/**
 * @brief Update the number of time slots and channels open at the start of the
 *        stream. Also update the size of the sample of each time slot. This is
 *        done to know the length of one data packet.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_update_ts_setting(struct cn0503_dev *dev)
{
	int32_t ret;
	uint16_t temp_data;
	int8_t i;

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OPMODE,
				&temp_data);
	if(ret != SUCCESS)
		return ret;
	temp_data &= BITM_OPMODE_TIMESLOT_EN;
	dev->active_slots = (temp_data >> BITP_OPMODE_TIMESLOT_EN) + 1;

	for(i = 0; i < dev->active_slots; i++) {
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_TS_CTRL(i), &temp_data);
		if(ret != SUCCESS)
			return FAILURE;
		if(temp_data & BITM_TS_CTRL_A_CH2_EN)
			dev->two_channel_slots |= (1 << i);
		else
			dev->two_channel_slots &= ~(1 << i);
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_DATA1(i), &temp_data);
		if(ret != SUCCESS)
			return FAILURE;
		dev->data_sizes[i] = temp_data & BITM_DATA1_A_SIGNAL_SIZE;
	}

	return SUCCESS;
}

/**
 * @brief CLI command to start the data streaming.
 * @param [in] dev - The device structure.
 * @param [in] arg - The number of samples to display. If let empty will stream
 *                   indefinitely.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_stream(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;

	if((arg == (NULL + 1)) || (*arg == 0) || (*arg == '0')) {
		dev->stream_cnt_en = 0;
		dev->stream_count = 1;
	} else {
		dev->stream_cnt_en = 1;
		dev->stream_count = atoi((char *)arg);
		if(!dev->stream_count)
			return FAILURE;
	}

	ret = cn0503_update_ts_setting(dev);
	if(ret != SUCCESS)
		return FAILURE;

	ret = adpd410x_set_opmode(dev->adpd4100_handler, ADPD410X_GOMODE);
	if(ret != SUCCESS)
		return FAILURE;

	dev->idle_state = 0;

	return cn0503_stream_prompt(dev, dev->mode);
}

/**
 * @brief CLI command to display the idle state.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter queried.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_idle_get(struct cn0503_dev *dev, uint8_t *arg)
{
	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: IDLE?=");
	cli_write_string(dev->cli_handler, dev->idle_state ? (uint8_t *)"1\n" :
			 (uint8_t *)"0\n");

	return SUCCESS;
}

/**
 * @brief CLI command to set the idle state.
 * @param [in] dev - The device structure.
 * @param [in] arg - New idle state.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_idle_set(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;
	uint16_t reg_data;

	if((arg == NULL) || (*arg == 0) || (*arg == '0')) {
		if(dev->idle_state == 1) {
			ret = cn0503_update_ts_setting(dev);
			if(ret != SUCCESS)
				return FAILURE;

			ret = adpd410x_set_opmode(dev->adpd4100_handler,
						  ADPD410X_GOMODE);
			if(ret != SUCCESS)
				return FAILURE;
		}
		dev->stream_cnt_en = 0;
		dev->stream_count = 0;
		dev->idle_state = 0;
	} else if(*arg == '1') {
		ret = adpd410x_set_opmode(dev->adpd4100_handler,
					  ADPD410X_STANDBY);
		if(ret != SUCCESS)
			return FAILURE;

		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_FIFO_STATUS, &reg_data);
		if (ret != SUCCESS)
			return FAILURE;
		reg_data |= BITM_INT_STATUS_FIFO_CLEAR_FIFO;

		ret = adpd410x_reg_write(dev->adpd4100_handler,
					 ADPD410X_REG_FIFO_STATUS, reg_data);
		if (ret != SUCCESS)
			return FAILURE;

		dev->stream_cnt_en = 0;
		dev->stream_count = 0;
		dev->idle_state = 1;
	} else {
		return FAILURE;
	}

	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: IDLE ");
	cli_write_string(dev->cli_handler, dev->idle_state ? (uint8_t *)"1\n" :
			 (uint8_t *)"0\n");

	return SUCCESS;
}

/**
 * @brief Get and display the application parameters.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_defparam_get(struct cn0503_dev *dev, uint8_t *arg)
{
	int8_t var_indx = 0, rat_indx = 0;
	extern uint8_t uart_current_line[100];
	uint8_t buff[256];

	while(arg[var_indx] != 0) {
		arg[var_indx] = toupper(arg[var_indx]);
		var_indx++;
	}

	var_indx = 0;
	while (cn0503_defparam_vars[var_indx][0] != 0) {
		if (!strcmp((char *)cn0503_defparam_vars[var_indx],
			    (char *)arg))
			break;
		var_indx++;
	}
	if(var_indx == CN0503_MAX_VAR_NUMBER)
		return FAILURE;

	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: DEF");
	uart_write(dev->cli_handler->uart_device, &uart_current_line[3], 1);
	cli_write_string(dev->cli_handler, (uint8_t *)"? ");

	rat_indx = uart_current_line[3] - 0x30;
	switch (var_indx) {
	case CN0503_ARAT_VAR:
		cli_write_string(dev->cli_handler, (uint8_t *)"ARAT=");
		cli_write_string(dev->cli_handler, dev->arat[rat_indx]);
		cli_write_string(dev->cli_handler, (uint8_t *)"\n");
		break;
	case CN0503_RFLT_VAR:
		cli_write_string(dev->cli_handler, (uint8_t *)"RFLT=");
		sprintf((char *)buff, "%.5f\n", dev->rflt[rat_indx]);
		cli_write_string(dev->cli_handler, buff);
		break;
	case CN0503_ALRM_VAR:
		cli_write_string(dev->cli_handler, (uint8_t *)"ALRM=");
		sprintf((char *)buff, "%f %f\n", dev->alrm_high[rat_indx],
			dev->alrm_low[rat_indx]);
		cli_write_string(dev->cli_handler, buff);
		break;
	case CN0503_RATB_VAR:
		cli_write_string(dev->cli_handler, (uint8_t *)"RATB=");
		sprintf((char *)buff, "%.5f\n", dev->ratb[rat_indx]);
		cli_write_string(dev->cli_handler, buff);
		break;
	case CN0503_INS1_VAR:
		cli_write_string(dev->cli_handler, (uint8_t *)"INS1=");
		sprintf((char *)buff, "%.5E %.5E %.5E %.5E %.5E %.5E\n",
			dev->ins1_p0[rat_indx], dev->ins1_p1[rat_indx],
			dev->ins1_p2[rat_indx], dev->ins1_p3[rat_indx],
			dev->ins1_p4[rat_indx], dev->ins1_p5[rat_indx]);
		cli_write_string(dev->cli_handler, buff);
		break;
	case CN0503_INS2_VAR:
		cli_write_string(dev->cli_handler, (uint8_t *)"INS2=");
		sprintf((char *)buff, "%.5E %.5E %.5E %.5E %.5E %.5E\n",
			dev->ins2_p0[rat_indx], dev->ins2_p1[rat_indx],
			dev->ins2_p2[rat_indx], dev->ins2_p3[rat_indx],
			dev->ins2_p4[rat_indx], dev->ins2_p5[rat_indx]);
		cli_write_string(dev->cli_handler, buff);
		break;
	case CN0503_SUBE_VAR:
		itoa(dev->rrat_sube[rat_indx], (char *)buff, 10);
		cli_write_string(dev->cli_handler, (uint8_t *)"SUBE=");
		cli_write_string(dev->cli_handler, buff);
		cli_write_string(dev->cli_handler, (uint8_t *)"\n");
		break;
	default:
		return FAILURE;
	}

	return SUCCESS;
}

/**
 * @brief Reallocate memory for the rolling filter data buffer.
 * @param [in] dev - The device structure.
 * @param [in] width - New size of the filter buffer width.
 * @param [in] buff_ptr - Reference to the filter buffer pointer.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_defparam_set_rflw(struct cn0503_dev *dev, uint8_t width,
					float **buff_ptr)
{
	float *err_check;

	err_check = (float *)realloc(*buff_ptr, width * sizeof(uint32_t));
	if(!err_check)
		return FAILURE;
	*buff_ptr = err_check;

	return SUCCESS;
}

/**
 * @brief Calculate the size of the rolling average filter data buffer based on
 *        the requested bandwidth.
 * @param [in] dev - The device structure.
 * @param [in] new_freq - New filter bandwidth.
 * @param [in] new_lpf_width - Pointer to the new buffer size.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_defparam_set_lpfbw(struct cn0503_dev *dev, float new_freq,
		uint32_t *new_lpf_width)
{
	const float cutoff_freq_amplitude = 0.708;
	const float sampling_freq = CN0503_CODE_ODR_DEFAULT / CN0503_BLOCK_FILT_SIZE;
	float big_neighbor = 0, small_neigbor = 0, temp_val;
	uint32_t temp_width = 1;

	float a, b, c;

	while(small_neigbor == 0) {
		a = 1.0 / temp_width;
		b = sin(M_PI * new_freq * temp_width / sampling_freq);
		c = sin(M_PI * new_freq / sampling_freq);
		temp_val = a * (b / c);
		if(temp_val < cutoff_freq_amplitude) {
			small_neigbor = temp_val;
		} else {
			big_neighbor = temp_val;
			temp_width++;
		}
	}
	if(small_neigbor == cutoff_freq_amplitude)
		*new_lpf_width = temp_width;
	else if((big_neighbor - cutoff_freq_amplitude) >
		(cutoff_freq_amplitude - small_neigbor))
		*new_lpf_width = temp_width + 1;
	else
		*new_lpf_width = temp_width;

	return SUCCESS;
}

/**
 * @brief Set the application parameters.
 * @param [in] dev - The device structure.
 * @param [in] arg - Changed parameter and new value.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_defparam_set(struct cn0503_dev *dev, uint8_t *arg)
{
	int8_t var_indx = 0, rat_indx = 0, i;
	extern uint8_t uart_current_line[100];
	uint8_t *val_ptr, *val_ptr_alt, *errcheck;
	float temp, temp_alt;
	uint8_t resp[256], buff[100];

	while(arg[var_indx] != 0) {
		arg[var_indx] = toupper(arg[var_indx]);
		var_indx++;
	}

	val_ptr = (uint8_t *)strchr((char *)arg, ' ');
	val_ptr++;

	var_indx = 0;
	while (cn0503_defparam_vars[var_indx][0] != 0) {
		if (!strncmp((char *)cn0503_defparam_vars[var_indx],
			     (char *)arg, 4))
			break;
		var_indx++;
	}
	if(var_indx == CN0503_MAX_VAR_NUMBER) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR invalid arguments.\n");
		return FAILURE;
	}

	rat_indx = uart_current_line[3] - 0x30;
	if(rat_indx >= 8 || rat_indx < 0) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR invalid ration.\n");
		return FAILURE;
	}

	sprintf((char *)resp, "RESP: DEF%c %s ", uart_current_line[3],
		cn0503_defparam_vars[var_indx]);

	switch (var_indx) {
	case CN0503_ARAT_VAR:
		i = strlen((char *)val_ptr) + 1;
		errcheck = (uint8_t *)realloc(dev->arat[rat_indx], i);
		if(!errcheck) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR memory.\n");
			return FAILURE;
		}
		dev->arat[rat_indx] = errcheck;
		memcpy(dev->arat[rat_indx], val_ptr, i);
		sprintf((char *)buff, "%s\n", dev->arat[rat_indx]);
		break;
	case CN0503_RFLT_VAR:
		temp = strtod((char *)val_ptr, (char **)&errcheck);
		if(errcheck == val_ptr) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR invalid arguments.\n");
			return FAILURE;
		}
		cn0503_defparam_set_lpfbw(dev, temp,
					  &dev->lpf_window[rat_indx]);
		dev->rflt[rat_indx] = temp;
		cn0503_defparam_set_rflw(dev, dev->lpf_window[rat_indx],
					 &dev->arat_flt_data[rat_indx]);
		sprintf((char *)buff, "%.5f\n", dev->rflt[rat_indx]);
		break;
	case CN0503_ALRM_VAR:
		temp = strtod((char *)val_ptr, (char **)&val_ptr_alt);
		if(val_ptr_alt == val_ptr) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR invalid high argument. Defaulting.\n");
			temp = 25;
		}
		dev->alrm_high[rat_indx] = temp;
		temp = strtod((char *)val_ptr_alt, (char **)&errcheck);
		if(errcheck == val_ptr_alt) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR invalid low argument. Defaulting.\n");
			temp = 15;
		}
		dev->alrm_low[rat_indx] = temp;
		sprintf((char *)buff, "%f %f\n", dev->alrm_high[rat_indx],
			dev->alrm_low[rat_indx]);
		break;
	case CN0503_RATB_VAR:
		temp = strtod((char *)val_ptr, (char **)&errcheck);
		if(errcheck == val_ptr) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR invalid ratio base.\n");
			return FAILURE;
		}
		dev->ratb[rat_indx] = temp;
		sprintf((char *)buff, "%.5f\n", dev->ratb[rat_indx]);
		break;
	case CN0503_INS1_VAR:
		for (i = 0; i < 3; i++) {
			temp = strtod((char *)val_ptr, (char **)&val_ptr_alt);
			if(val_ptr_alt == val_ptr)
				temp = 0;
			temp_alt = strtod((char *)val_ptr_alt,
					  (char **)&val_ptr);
			if(val_ptr_alt == val_ptr)
				temp_alt = 0;
			if (i == 0) {
				dev->ins1_p0[rat_indx] = temp;
				dev->ins1_p1[rat_indx] = temp_alt;
			} else if (i == 1) {
				dev->ins1_p2[rat_indx] = temp;
				dev->ins1_p3[rat_indx] = temp_alt;
			} else if (i == 2) {
				dev->ins1_p4[rat_indx] = temp;
				dev->ins1_p5[rat_indx] = temp_alt;
			}
		}
		sprintf((char *)buff, "%.5E %.5E %.5E %.5E %.5E %.5E\n",
			dev->ins1_p0[rat_indx], dev->ins1_p1[rat_indx],
			dev->ins1_p2[rat_indx], dev->ins1_p3[rat_indx],
			dev->ins1_p4[rat_indx], dev->ins1_p5[rat_indx]);
		break;
	case CN0503_INS2_VAR:
		for (i = 0; i < 3; i++) {
			temp = strtod((char *)val_ptr, (char **)&val_ptr_alt);
			if(val_ptr_alt == val_ptr)
				temp = 0;
			temp_alt = strtod((char *)val_ptr_alt,
					  (char **)&val_ptr);
			if(val_ptr_alt == val_ptr)
				temp_alt = 0;
			if (i == 0) {
				dev->ins2_p0[rat_indx] = temp;
				dev->ins2_p1[rat_indx] = temp_alt;
			} else if (i == 1) {
				dev->ins2_p2[rat_indx] = temp;
				dev->ins2_p3[rat_indx] = temp_alt;
			} else if (i == 2) {
				dev->ins2_p4[rat_indx] = temp;
				dev->ins2_p5[rat_indx] = temp_alt;
			}
		}
		sprintf((char *)buff, "%.5E %.5E %.5E %.5E %.5E %.5E\n",
			dev->ins2_p0[rat_indx], dev->ins2_p1[rat_indx],
			dev->ins2_p2[rat_indx], dev->ins2_p3[rat_indx],
			dev->ins2_p4[rat_indx], dev->ins2_p5[rat_indx]);
		break;
	case CN0503_SUBE_VAR:
		temp = atoi((char *)val_ptr);
		if((temp != 0) && (temp != 1))
			temp = 1;
		dev->rrat_sube[rat_indx] = temp;
		sprintf((char *)buff, "%d\n", dev->rrat_sube[rat_indx]);
		break;
	default:
		return FAILURE;
	}

	strcat((char *)resp, (char *)buff);
	cli_write_string(dev->cli_handler, resp);

	return SUCCESS;
}

/**
 * @brief CLI command used to interact with the application parameters.
 * @param [in] dev - The device structure.
 * @param [in] arg - Command arguments.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_defparam_route(struct cn0503_dev *dev, uint8_t *arg)
{
	extern uint8_t uart_current_line[100];

	if (uart_current_line[4] == '?')
		return cn0503_defparam_get(dev, arg);
	else
		return cn0503_defparam_set(dev, arg);
}

/**
 * @brief Display the baseline value used in calculating relative ratio.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_baseline_get(struct cn0503_dev *dev, uint8_t *arg)
{
	extern uint8_t uart_current_line[100];
	int8_t i;
	uint8_t buff[20];

	if (uart_current_line[3] != '?' || uart_current_line[4] != 0)
		return SUCCESS;

	i = uart_current_line[2] - 0x30;
	if ((i < 0) || (i >= 8))
		return SUCCESS;
	sprintf((char *)buff, "%.5f", dev->ratb[i]);
	cli_write_string(dev->cli_handler,
			 (uint8_t *)"RESP: RZ");
	uart_write(dev->cli_handler->uart_device, &uart_current_line[2], 1);
	cli_write_string(dev->cli_handler, (uint8_t *)"?=");
	cli_write_string(dev->cli_handler, buff);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief CLI command to get and display the low and high alarm thresholds.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_alarm_get(struct cn0503_dev *dev, uint8_t *arg)
{
	extern uint8_t uart_current_line[100];
	int8_t i, alrm_stts = 0;
	uint8_t buff[20];

	if (uart_current_line[5] != '?' || uart_current_line[6] != 0)
		return SUCCESS;

	i = uart_current_line[2] - 0x30;
	if ((i < 0) || (i >= 8))
		return SUCCESS;
	if(dev->alrm_state[i] & 1)
		alrm_stts |= 1;
	if(dev->alrm_state[i] & 2)
		alrm_stts |= 2;
	itoa(alrm_stts, (char *)buff, 10);
	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: ALRM");
	uart_write(dev->cli_handler->uart_device, &uart_current_line[4], 1);
	cli_write_string(dev->cli_handler, (uint8_t *)"?=");
	cli_write_string(dev->cli_handler, buff);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief CLI command to get and display the output data rate.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_odr_get(struct cn0503_dev *dev, uint8_t *arg)
{
	uint8_t buff[20];

	sprintf((char *)buff, "%.5f", dev->odr);
	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: ODR");
	cli_write_string(dev->cli_handler, (uint8_t *)"?=");
	cli_write_string(dev->cli_handler, buff);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief CLI command to set the output data rate.
 * @param [in] dev - The device structure.
 * @param [in] arg - New output data rate.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_odr_set(struct cn0503_dev *dev, uint8_t *arg)
{
	uint8_t *error, buff[20];
	float check_val;
	uint32_t odr_thresh;

	check_val = strtod((char *)arg, (char **)&error);
	if(error == arg)
		return FAILURE;
	odr_thresh = (float)(CN0503_CODE_ODR_DEFAULT / CN0503_BLOCK_FILT_SIZE) /
		     check_val;
	do {
		dev->odr = (float)(CN0503_CODE_ODR_DEFAULT / CN0503_BLOCK_FILT_SIZE) /
			   odr_thresh;
		odr_thresh++;
	} while (dev->odr > check_val);

	sprintf((char *)buff, "%.5f", dev->odr);
	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: ODR ");
	cli_write_string(dev->cli_handler, buff);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief CLI command to get and display the optical paths mask.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_ratmask_get(struct cn0503_dev *dev, uint8_t *arg)
{
	uint8_t buff[20];

	sprintf((char *)buff, "%x", dev->ratmask);
	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: RATMASK");
	cli_write_string(dev->cli_handler, (uint8_t *)"?=");
	cli_write_string(dev->cli_handler, buff);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief CLI command to set the optical paths mask.
 * @param [in] dev - The device structure.
 * @param [in] arg - New optical paths mask setting.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_ratmask_set(struct cn0503_dev *dev, uint8_t *arg)
{
	uint8_t *error, check_val;

	check_val = strtol((char *)arg, (char **)&error, 16);
	if(error == arg)
		return FAILURE;
	dev->ratmask = check_val;

	cli_write_string(dev->cli_handler, (uint8_t *)"RESP: RATMASK");
	cli_write_string(dev->cli_handler, (uint8_t *)" ");
	cli_write_string(dev->cli_handler, arg);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief Initialize the application handler structure with default values.
 * @param [in] dev - The device structure.
 * @param [in] first_time - true if the function is called after fresh power
 *                          cycle;
 *                          false if it's called after software reset.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static void cn0503_handler_init(struct cn0503_dev *dev, bool first_time)
{
	int16_t i;
	uint8_t *temp_ptr;

	dev->mode = CN0503_INS2;
	dev->stream_count = 0;
	dev->stream_cnt_en = 0;
	dev->idle_state = 1;
	for (i = 0; i < CN0503_FLASH_BUFF_SIZE; i++)
		dev->sw_flash_buffer[i] = 0xFFFFFFFF;
	for (i = 0; i < CN0503_RAT_NO; i++) {
		if(first_time) {
			dev->arat[i] = (uint8_t *)calloc(1, sizeof (uint8_t));
		} else {
			temp_ptr = (uint8_t *)realloc(dev->arat[i], 1);
			if(temp_ptr)
				dev->arat[i] = temp_ptr;
		}
		dev->rflt[i] = 0.5;
		dev->alrm_high[i] = 25;
		dev->alrm_low[i] = 15;
		dev->alrm_state[i] = 0;
		dev->ratb[i] = 1;

		dev->ins1_p0[i] = 0;
		dev->ins1_p1[i] = 1;
		dev->ins1_p2[i] = 0;
		dev->ins1_p3[i] = 0;
		dev->ins1_p4[i] = 0;
		dev->ins1_p5[i] = 0;
		dev->ins2_p0[i] = 0;
		dev->ins2_p1[i] = 1;
		dev->ins2_p2[i] = 0;
		dev->ins2_p3[i] = 0;
		dev->ins2_p4[i] = 0;
		dev->ins2_p5[i] = 0;

		dev->lpf_window[i] = CN0503_DLPF_DEFAULT;
		dev->rrat_sube[i] = 1;
	}
	dev->odr = 1;
	dev->data_channel_indx = 0;
	dev->ratmask = 0xf;
}

/**
 * @brief CLI command to do software reset of the application.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_reboot(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;
	int8_t i, j;

	ret = adpd410x_reset(dev->adpd4100_handler);
	if(ret != SUCCESS)
		return FAILURE;

	cn0503_handler_init(dev, false);

	for(i = 0; i < CN0503_RAT_NO; i++)
		for(j = 0; j < CN0503_DLPF_DEFAULT; j++)
			dev->arat_flt_data[i][j] = 0;

	return SUCCESS;
}

/**
 * @brief CLI command to clear the flash software buffer.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_flash_swbuff_clear(struct cn0503_dev *dev, uint8_t *arg)
{
	int16_t i;

	for (i = 0; i < CN0503_FLASH_BUFF_SIZE; i++)
		dev->sw_flash_buffer[i] = 0xFFFFFFFF;

	cli_write_string(dev->cli_handler,
			 (uint8_t *)"RESP: FL_CLEARBUF\n");

	return SUCCESS;
}

/**
 * @brief CLI command to load the flash software buffer from the flash memory.
 * @param [in] dev - The device structure.
 * @param [in] arg - 0 to load from the user memory space;
 *                   1 to load from the manufacturer memory space;
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_flash_swbuff_load(struct cn0503_dev *dev, uint8_t *arg)
{
	uint8_t opt = atoi((char *)arg);

	if ((arg == NULL) || (*arg == '\0'))
		return cli_write_string(dev->cli_handler,
					(uint8_t*) "Argument error.\n");

	if (opt == 0) {
		flash_read(dev->flash_handler, dev->uu_flash_page_addr,
			   dev->sw_flash_buffer, CN0503_FLASH_BUFF_SIZE);
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"RESP: FL_LOAD 0\n");
	} else if (opt == 1) {
		flash_read(dev->flash_handler, dev->md_flash_page_addr,
			   dev->sw_flash_buffer, CN0503_FLASH_BUFF_SIZE);
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"RESP: FL_LOAD 1\n");
	} else {
		return cli_write_string(dev->cli_handler,
					(uint8_t*) "Argument error.\n");
	}

	return SUCCESS;
}

/**
 * @brief CLI command to program a flash page with the data from the buffer.
 * @param [in] dev - The device structure.
 * @param [in] arg - 0 to program the user memory space;
 *                   <key> to program the manufacturer memory space;
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_flash_program(struct cn0503_dev *dev, uint8_t *arg)
{
	uint32_t opt = atoi((char *)arg);
	int32_t ret;

	if ((arg == NULL) || (*arg == '\0'))
		return cli_write_string(dev->cli_handler,
					(uint8_t*) "Argument error.\n");

	if (opt == 0) {
		ret = flash_write(dev->flash_handler, dev->uu_flash_page_addr,
				  dev->sw_flash_buffer, CN0503_FLASH_BUFF_SIZE);
		if (ret != SUCCESS)
			return FAILURE;
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"RESP: FL_PROGRAM 0\n");
	} else if (opt == 15091994) {
		ret = flash_write(dev->flash_handler, dev->md_flash_page_addr,
				  dev->sw_flash_buffer, CN0503_FLASH_BUFF_SIZE);
		if (ret != SUCCESS)
			return FAILURE;
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"RESP: FL_PROGRAM 15091994\n");
	} else {
		return cli_write_string(dev->cli_handler,
					(uint8_t*) "Access denied.\n");
	}

	return SUCCESS;
}

/**
 * @brief CLI command to erase a flash page from the flash memory.
 * @param [in] dev - The device structure.
 * @param [in] arg - 0 to erase the user memory space;
 *                   <key> to erase the manufacturer memory space;
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_flash_erase(struct cn0503_dev *dev, uint8_t *arg)
{
	uint32_t opt = atoi((char *)arg);
	int16_t i;
	uint32_t temp_buff[CN0503_FLASH_BUFF_SIZE];
	int32_t ret;

	if ((arg == NULL) || (*arg == '\0'))
		return cli_write_string(dev->cli_handler,
					(uint8_t*) "Argument error.\n");

	for (i = 0; i < CN0503_FLASH_BUFF_SIZE; i++)
		temp_buff[i] = 0xFFFFFFFF;

	if (opt == 0) {
		ret = flash_write(dev->flash_handler, dev->uu_flash_page_addr,
				  temp_buff, CN0503_FLASH_BUFF_SIZE);
		if (ret != SUCCESS)
			return FAILURE;
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"RESP: FL_ERASE 1\n");
	} else if (opt == 15091994) {
		ret = flash_write(dev->flash_handler, dev->md_flash_page_addr,
				  temp_buff, CN0503_FLASH_BUFF_SIZE);
		if (ret != SUCCESS)
			return FAILURE;
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"RESP: FL_ERASE 15091994\n");
	} else {
		return cli_write_string(dev->cli_handler,
					(uint8_t*) "Access denied.\n");
	}

	return SUCCESS;
}

/**
 * @brief Apply the ARAT expression stored in the flash buffer to the program.
 *        cn0503_flash_apply() helper function.
 * @param [in] dev - The device structure.
 * @param [in] buff - Pointer to the place in the software buffer where the ARAT
 *                    expression is stored.
 * @param [out] arat_ptr - Pointer to the ARAT expression to change.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_apply_arat(struct cn0503_dev *dev, uint32_t *buff,
				       uint8_t **arat_ptr)
{
	int8_t i, arat_size = 0;
	uint8_t *temp_err;

	for (i = 0; i < 8; i++) {
		if ((buff[i] & 0xFF000000) == 0xFF000000)
			break;
		arat_size++;
		if ((buff[i] & 0xFF0000) == 0xFF0000)
			break;
		arat_size++;
		if ((buff[i] & 0xFF00) == 0xFF00)
			break;
		arat_size++;
		if ((buff[i] & 0xFF) == 0xFF)
			break;
		arat_size++;
	}
	if (!arat_size)
		return SUCCESS;
	temp_err = (uint8_t *)realloc((*arat_ptr), arat_size);
	if (!temp_err)
		return FAILURE;
	*arat_ptr = temp_err;

	i = 0;
	do {
		if ((buff[i] & 0xFF000000) == 0xFF000000)
			break;
		(*arat_ptr)[(i * 4)] = (buff[i] & 0xFF000000) >> 24;
		if ((buff[i] & 0xFF0000) == 0xFF0000)
			break;
		(*arat_ptr)[(i * 4 + 1)] = (buff[i] & 0xFF0000) >> 16;
		if ((buff[i] & 0xFF00) == 0xFF00)
			break;
		(*arat_ptr)[(i * 4 + 2)] = (buff[i] & 0xFF00) >> 8;
		if ((buff[i] & 0xFF) == 0xFF)
			break;
		(*arat_ptr)[(i * 4 + 3)] = buff[i] & 0xFF;
		i++;
	} while (1);

	return SUCCESS;
}

/**
 * @brief CLI command to apply the settings stored in the flash buffer to the
 *        program.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_flash_apply(struct cn0503_dev *dev, uint8_t *arg)
{
	int16_t i, sw_buff_loc;
	int32_t ret;
	uint16_t reg_addr, reg_val;

	for (i = 0; i < CN0503_FLASH_REG_SIZE; i++) {
		if ((dev->sw_flash_buffer[i] & 0xF0000000) != 0)
			break;
		reg_addr = (dev->sw_flash_buffer[i] & 0xFFFF0000) >> 16;
		reg_val  = dev->sw_flash_buffer[i] & 0x0000FFFF;
		ret = adpd410x_reg_write(dev->adpd4100_handler, reg_addr,
					 reg_val);
		if (ret != SUCCESS)
			return FAILURE;
	}
	for (i = 0; i < CN0503_RAT_NO; i++) {
		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28;
		ret = cn0503_flash_apply_arat(dev,
					      &dev->sw_flash_buffer[sw_buff_loc],
					      &dev->arat[i]);
		if (ret != SUCCESS)
			return FAILURE;

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 8;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->rflt[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 9;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->alrm_high[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 10;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->alrm_low[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 11;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ratb[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 12;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->rrat_sube[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 16;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins1_p0[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 17;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins1_p1[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 18;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins1_p2[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 19;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins1_p3[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 20;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins1_p4[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 21;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins1_p5[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 22;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins2_p0[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 23;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins2_p1[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 24;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins2_p2[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 25;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins2_p3[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 26;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins2_p4[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 27;
		if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
			memcpy((void *)&dev->ins2_p5[i],
			       (void *)&dev->sw_flash_buffer[sw_buff_loc],
			       4);
	}

	sw_buff_loc = CN0503_FLASH_ODR_IDX;
	if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
		memcpy((void *)&dev->odr,
		       (void *)&dev->sw_flash_buffer[sw_buff_loc],
		       4);

	sw_buff_loc = CN0503_FLASH_MODE_IDX;
	if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
		dev->mode = dev->sw_flash_buffer[sw_buff_loc];

	sw_buff_loc = CN0503_FLASH_RATM_IDX;
	if (dev->sw_flash_buffer[sw_buff_loc] != 0xFFFFFFFF)
		dev->ratmask = dev->sw_flash_buffer[sw_buff_loc];

	cli_write_string(dev->cli_handler,
			 (uint8_t*)"RESP: FL_APPLY\n");

	return SUCCESS;
}

/**
 * @brief Update the register settings of the flash buffer. cn0503_flash_write()
 *        helper function.
 * @param [in] dev - The device structure.
 * @param [in] arg - ASCII buffer with the register address and value to commit
 *                   to the flash software buffer.
 * @param [out] buff - ASCII response buffer returned to the application.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_write_reg(struct cn0503_dev *dev, uint8_t *arg,
				      uint8_t *buff)
{
	int16_t i;
	uint16_t reg_addr, reg_val;
	uint8_t *buff_ptr;

	for (i = 0; i < CN0503_FLASH_REG_SIZE; i++)
		if ((dev->sw_flash_buffer[i] & 0xFF000000) == 0xFF000000)
			break;
	if (i == CN0503_FLASH_REG_SIZE)
		return FAILURE;

	reg_addr = strtol((char *)arg, (char **)&buff_ptr, 16);
	if (buff_ptr == arg)
		return FAILURE;
	reg_val = strtol((char *)buff_ptr, (char **)&arg, 16);
	if (buff_ptr == arg)
		return FAILURE;
	sprintf((char *)buff, "%x %x", reg_addr, reg_val);
	dev->sw_flash_buffer[i] = (reg_addr << 16) & 0xFFFF0000;
	dev->sw_flash_buffer[i] |= reg_val;

	return SUCCESS;
}

/**
 * @brief Update the ARAT settings of the flash buffer. cn0503_flash_write_def()
 *        helper function.
 * @param [in] dev - The device structure.
 * @param [in] arg - ASCII buffer with the ARAT string to commit to the flash
 *                   software buffer.
 * @param [in] rat_no - Optical path ID.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static void cn0503_flash_write_def_arat(struct cn0503_dev *dev, uint8_t *arg,
					uint8_t rat_no)
{
	int16_t sw_buff_index, i;

	for (i = 0; i < 8; i++) {
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + i;
		dev->sw_flash_buffer[sw_buff_index] = 0xFFFFFFFF;
	}

	for (i = 0; i < 8; i++) {
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + i;
		if (*arg == 0)
			break;
		dev->sw_flash_buffer[sw_buff_index] = (*arg << 24) & 0xFF000000;
		arg++;
		if (*arg == 0)
			break;
		dev->sw_flash_buffer[sw_buff_index] |= (*arg << 16) & 0xFF0000;
		arg++;
		if (*arg == 0)
			break;
		dev->sw_flash_buffer[sw_buff_index] |= (*arg << 8) & 0xFF00;
		arg++;
		if (*arg == 0)
			break;
		dev->sw_flash_buffer[sw_buff_index] |= *arg & 0xFF;
		arg++;
	}
}

/**
 * @brief Update a DEF setting of the flash buffer. cn0503_flash_write()
 *        helper function.
 * @param [in] dev - The device structure.
 * @param [in] arg - ASCII buffer with the DEF value to commit to the flash
 *                   software buffer.
 * @param [out] buff - ASCII response buffer returned to the application.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_write_def(struct cn0503_dev *dev, uint8_t *arg,
				      uint8_t *buff_ret)
{
	int8_t i = 0, rat_no;
	uint8_t *err_ptr;
	int16_t sw_buff_index;
	float temp;
	uint8_t buff[100], temp_ascii[100];

	rat_no = (*arg - 0x30);
	if ((rat_no < 0) || (rat_no >= 8))
		return FAILURE;
	arg += 2;

	while(arg[i] != 0) {
		arg[i] = toupper(arg[i]);
		i++;
	}

	i = 0;
	do {
		if (strncmp((char *)arg,
			    (char *)cn0503_defparam_vars[i],
			    4) == 0)
			break;
		i++;
	} while (cn0503_defparam_vars[i][0] != '\0');
	if (i == CN0503_MAX_VAR_NUMBER)
		return FAILURE;

	sprintf((char *)buff_ret, "%d %s ", rat_no, cn0503_defparam_vars[i]);

	arg += 5;
	switch (i) {
	case CN0503_ARAT_VAR:
		cn0503_flash_write_def_arat(dev, arg, rat_no);
		sprintf((char *)buff, "%s\n", arg);
		break;
	case CN0503_RFLT_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 8;
		temp = strtod((char *)arg, (char **)&err_ptr);
		if (err_ptr == arg)
			return FAILURE;
		sprintf((char *)buff, "%.2f\n", temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		break;
	case CN0503_ALRM_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 9;
		temp = strtod((char *)arg, (char **)&err_ptr);
		if (err_ptr == arg) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR invalid high argument. Defaulting.\n");
			temp = 25;
		}
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		sprintf((char *)buff, "%f", temp);
		temp = strtod((char *)err_ptr, (char **)&arg);
		if (err_ptr == arg) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR invalid low argument. Defaulting.\n");
			temp = 15;
		}
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %f\n", temp_ascii, temp);
		break;
	case CN0503_RATB_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 11;
		temp = strtod((char *)arg, (char **)&err_ptr);
		if (err_ptr == arg)
			return FAILURE;
		sprintf((char *)buff, "%.5f\n", temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		break;
	case CN0503_SUBE_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 12;
		temp = strtol((char *)arg, (char **)&err_ptr, 10);
		if (err_ptr == arg)
			return FAILURE;
		dev->sw_flash_buffer[sw_buff_index] = temp;
		sprintf((char *)buff, "%d\n", (int8_t)temp);
		break;
	case CN0503_INS1_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 16;
		temp = strtod((char *)arg, (char **)&err_ptr);
		if (err_ptr == arg)
			temp = 0;
		sprintf((char *)buff, "%.5E", temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		temp = strtod((char *)err_ptr, (char **)&arg);
		if (err_ptr == arg)
			temp = 0;
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %.5E", temp_ascii, temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		temp = strtod((char *)arg, (char **)&err_ptr);
		if (err_ptr == arg)
			temp = 0;
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %.5E", temp_ascii, temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		temp = strtod((char *)err_ptr, (char **)&arg);
		if (err_ptr == arg)
			temp = 0;
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %.5E", temp_ascii, temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		temp = strtod((char *)arg, (char **)&err_ptr);
		if (err_ptr == arg)
			temp = 0;
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %.5E", temp_ascii, temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		temp = strtod((char *)err_ptr, (char **)&arg);
		if (err_ptr == arg)
			temp = 0;
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %.5E\n", temp_ascii, temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		break;
	case CN0503_INS2_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 22;
		temp = strtod((char *)arg, (char **)&err_ptr);
		if (err_ptr == arg)
			temp = 0;
		sprintf((char *)buff, "%.5E", temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		temp = strtod((char *)err_ptr, (char **)&arg);
		if (err_ptr == arg)
			temp = 0;
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %.5E", temp_ascii, temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		temp = strtod((char *)arg, (char **)&err_ptr);
		if (err_ptr == arg)
			temp = 0;
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %.5E", temp_ascii, temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		temp = strtod((char *)err_ptr, (char **)&arg);
		if (err_ptr == arg)
			temp = 0;
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %.5E", temp_ascii, temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		temp = strtod((char *)arg, (char **)&err_ptr);
		if (err_ptr == arg)
			temp = 0;
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %.5E", temp_ascii, temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		sw_buff_index++;
		temp = strtod((char *)err_ptr, (char **)&arg);
		if (err_ptr == arg)
			temp = 0;
		strcpy((char *)temp_ascii, (char *)buff);
		sprintf((char *)buff, "%s %.5E\n", temp_ascii, temp);
		memcpy((void *)(&dev->sw_flash_buffer[sw_buff_index]),
		       (void *)&temp, 4);
		break;
	default:
		return FAILURE;
	}

	strcat((char *)buff_ret, (char *)buff);

	return SUCCESS;
}

/**
 * @brief Update the MODE  setting of the flash buffer. cn0503_flash_write()
 *        helper function.
 * @param [in] dev - The device structure.
 * @param [in] arg - ASCII buffer with the MODE value to commit to the flash
 *                   software buffer.
 * @param [out] buff - ASCII response buffer returned to the application.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_write_mode(struct cn0503_dev *dev, uint8_t *arg,
				       uint8_t *buff)
{
	int8_t i = 0;

	while(arg[i]) {
		arg[i] = toupper(arg[i]);
		i++;
	}

	i = 0;
	while(cn0503_ascii_modes[i][0] != 0) {
		if(strncmp((char *)arg, (char *)cn0503_ascii_modes[i], 5) == 0)
			break;
		i++;
	}
	if(i > CN0503_INS2)
		return FAILURE;

	sprintf((char *)buff, "%s", cn0503_ascii_modes[i]);

	dev->sw_flash_buffer[CN0503_FLASH_MODE_IDX] = i;

	return SUCCESS;
}

/**
 * @brief Update the ODR setting of the flash buffer. cn0503_flash_write()
 *        helper function.
 * @param [in] dev - The device structure.
 * @param [in] arg - ASCII buffer with the ODR value to commit to the flash
 *                   software buffer.
 * @param [out] buff - ASCII response buffer returned to the application.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_write_odr(struct cn0503_dev *dev, uint8_t *arg,
				      uint8_t *buff)
{
	uint8_t *error, odr_thresh;
	float check_val, temp;

	check_val = strtod((char *)arg, (char **)&error);
	if(error == arg)
		return FAILURE;
	odr_thresh = (float)(CN0503_CODE_ODR_DEFAULT / CN0503_BLOCK_FILT_SIZE) /
		     check_val;
	do {
		temp = (float)(CN0503_CODE_ODR_DEFAULT / CN0503_BLOCK_FILT_SIZE) /
		       odr_thresh;
		odr_thresh++;
	} while (temp > check_val);

	sprintf((char *)buff, "%.2f", temp);

	memcpy((void *)&dev->sw_flash_buffer[CN0503_FLASH_ODR_IDX],
	       (void *)&temp, 4);

	return SUCCESS;
}

/**
 * @brief Update the RATMASK setting of the flash buffer. cn0503_flash_write()
 *        helper function.
 * @param [in] dev - The device structure.
 * @param [in] arg - ASCII buffer with the RATMASK value to commit to the flash
 *                   software buffer.
 * @param [out] buff - ASCII response buffer returned to the application.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_write_ratmask(struct cn0503_dev *dev, uint8_t *arg,
		uint8_t *buff)
{
	uint8_t *error, check_val;

	check_val = strtol((char *)arg, (char **)&error, 16);
	if(error == arg)
		return FAILURE;
	sprintf((char *)buff, "%x", check_val);
	dev->sw_flash_buffer[CN0503_FLASH_RATM_IDX] = check_val;

	return SUCCESS;
}

/**
 * @brief Update a flash buffer parameter.
 * @param [in] dev - The device structure.
 * @param [in] arg - Command and value to update. Homologous to the application
 *                   parameter commands.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_flash_write(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;
	int8_t i = 0;
	uint8_t resp[256], cmd[10], buff[100];

	while(arg[i] != 0) {
		arg[i] = toupper(arg[i]);
		i++;
	}

	sprintf((char *)resp, "RESP: FL_WRITE ");

	if (strncmp((char *)arg, "REG", 3) == 0) {
		ret = cn0503_flash_write_reg(dev, (arg + 4), buff);
		if (ret != SUCCESS)
			return cli_write_string(dev->cli_handler,
						(uint8_t*)"reg command arguments incorrect or no more space.\n");
		sprintf((char *)cmd, "REG ");
	} else if (strncmp((char *)arg, "DEF", 3) == 0) {
		ret = cn0503_flash_write_def(dev, (arg + 3), buff);
		if (ret != SUCCESS)
			return cli_write_string(dev->cli_handler,
						(uint8_t*)"def command arguments incorrect.\n");
		sprintf((char *)cmd, "DEF ");
	} else if (strncmp((char *)arg, "MODE ", 5) == 0) {
		ret = cn0503_flash_write_mode(dev, (arg + 5), buff);
		if (ret != SUCCESS)
			return cli_write_string(dev->cli_handler,
						(uint8_t*)"mode command arguments incorrect.\n");
		sprintf((char *)cmd, "MODE ");
	} else if (strncmp((char *)arg, "ODR ", 4) == 0) {
		ret = cn0503_flash_write_odr(dev, (arg + 4), buff);
		if (ret != SUCCESS)
			return cli_write_string(dev->cli_handler,
						(uint8_t*)"odr command arguments incorrect.\n");
		sprintf((char *)cmd, "ODR ");
	} else if (strncmp((char *)arg, "RATMASK ", 8) == 0) {
		ret = cn0503_flash_write_ratmask(dev, (arg + 8), buff);
		if (ret != SUCCESS)
			return cli_write_string(dev->cli_handler,
						(uint8_t*)"ratmask command arguments incorrect.\n");
		sprintf((char *)cmd, "RATMASK ");
	} else {
		return cli_write_string(dev->cli_handler,
					(uint8_t*)"Error wrong command.\n");
	}

	strcat((char *)resp, (char *)cmd);
	strcat((char *)resp, (char *)buff);
	cli_write_string(dev->cli_handler, resp);

	return SUCCESS;
}

/**
 * @brief Find and display register setting in the flash software buffer.
 * @param [in] dev - The device structure.
 * @param [in] arg - Register address in ASCII form.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_read_reg(struct cn0503_dev *dev, uint8_t *arg)
{
	uint16_t addr, i;
	uint8_t *err_ptr, buff[20];

	addr = strtol((char *)arg, (char **)&err_ptr, 16);
	if (err_ptr == arg)
		return FAILURE;

	cli_write_string(dev->cli_handler,
			 (uint8_t*)"RESP: FL_READ REG? ");
	cli_write_string(dev->cli_handler, arg);
	cli_write_string(dev->cli_handler,
			 (uint8_t*)"=");

	for (i = 0; i < CN0503_FLASH_REG_SIZE; i++) {
		if (((dev->sw_flash_buffer[i] >> 16) & 0xFFFF) == addr)
			break;
		if (((dev->sw_flash_buffer[i] >> 16) & 0xFFFF) == 0xFFFF)
			return 	cli_write_string(dev->cli_handler,
						 (uint8_t*)"N/A\n");
	}
	if (i == CN0503_FLASH_REG_SIZE)
		return 	cli_write_string(dev->cli_handler,
					 (uint8_t*)"N/A\n");
	itoa((dev->sw_flash_buffer[i] & 0xFFFF), (char *)buff, 16);
	cli_write_string(dev->cli_handler, buff);

	return 	cli_write_string(dev->cli_handler,
				 (uint8_t*)"\n");
}

/**
 * @brief Read and display a DEF setting in the flash software buffer.
 * @param [in] dev - The device structure.
 * @param [in] arg - DEF parameter.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_read_def(struct cn0503_dev *dev, uint8_t *arg)
{
	uint8_t rat_no, i = 0, j, buff[60];
	uint16_t sw_buff_index;
	float temp;
	uint8_t temp_char;

	while(arg[i] != 0) {
		arg[i] = toupper(arg[i]);
		i++;
	}

	rat_no = (*arg - 0x30);
	if ((rat_no < 0) || (rat_no >= 8))
		return FAILURE;
	arg += 3;

	i = 0;
	do {
		if (strncmp((char *)arg,
			    (char *)cn0503_defparam_vars[i],
			    4) == 0)
			break;
		i++;
	} while (cn0503_defparam_vars[i][0] != '\0');
	if (i == CN0503_MAX_VAR_NUMBER)
		return FAILURE;

	cli_write_string(dev->cli_handler,
			 (uint8_t *)"RESP: FL_READ DEF");
	uart_write(dev->cli_handler->uart_device, (arg - 3), 1);
	cli_write_string(dev->cli_handler,
			 (uint8_t *)" ");
	switch (i) {
	case CN0503_ARAT_VAR:
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ARAT=");
		for (j = 0; j < 8; j++) {
			sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + j;
			if ((dev->sw_flash_buffer[sw_buff_index] & 0xFF000000) == 0xFF000000)
				break;
			temp_char = (dev->sw_flash_buffer[sw_buff_index] & 0xFF000000) >> 24;
			uart_write(dev->cli_handler->uart_device, &temp_char, 1);
			if ((dev->sw_flash_buffer[sw_buff_index] & 0xFF0000) == 0xFF0000)
				break;
			temp_char = (dev->sw_flash_buffer[sw_buff_index] & 0xFF0000) >> 16;
			uart_write(dev->cli_handler->uart_device, &temp_char, 1);
			if ((dev->sw_flash_buffer[sw_buff_index] & 0xFF00) == 0xFF00)
				break;
			temp_char = (dev->sw_flash_buffer[sw_buff_index] & 0xFF00) >> 8;
			uart_write(dev->cli_handler->uart_device,
				   &temp_char, 1);
			if ((dev->sw_flash_buffer[sw_buff_index] & 0xFF) == 0xFF)
				break;
			temp_char = dev->sw_flash_buffer[sw_buff_index] & 0xFF;
			uart_write(dev->cli_handler->uart_device,
				   &temp_char, 1);
		}
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"\n");
		break;
	case CN0503_RFLT_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 8;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "RFLT=%.5f\n", temp);
		cli_write_string(dev->cli_handler, buff);
		break;
	case CN0503_ALRM_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 9;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "ALRM=%f ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%f\n", temp);
		cli_write_string(dev->cli_handler, buff);
		break;
	case CN0503_RATB_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 11;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "RATB=%.5f\n", temp);
		cli_write_string(dev->cli_handler, buff);
		break;
	case CN0503_SUBE_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 12;
		sprintf((char *)buff, "SUBE=%d\n",
			(int)dev->sw_flash_buffer[sw_buff_index]);
		cli_write_string(dev->cli_handler, buff);
		break;
	case CN0503_INS1_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 16;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "INS1=%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E\n", temp);
		cli_write_string(dev->cli_handler, buff);
		break;
	case CN0503_INS2_VAR:
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 22;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "INS2=%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E\n", temp);
		cli_write_string(dev->cli_handler, buff);
		break;
	default:
		return FAILURE;
	}

	return SUCCESS;
}

/**
 * @brief Read and display the MODE setting in the flash software buffer.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_read_mode(struct cn0503_dev *dev)
{
	cli_write_string(dev->cli_handler,
			 (uint8_t *)"RESP: FL_READ MODE?=");
	cli_write_string(dev->cli_handler,
			 cn0503_ascii_modes[dev->sw_flash_buffer[CN0503_FLASH_MODE_IDX]]);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief Read and display the ODR setting in the flash software buffer.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_read_odr(struct cn0503_dev *dev)
{
	uint8_t buff[20];
	float temp;

	memcpy((void *)&temp,
	       (void *)&dev->sw_flash_buffer[CN0503_FLASH_ODR_IDX], 4);

	sprintf((char *)buff, "%.2f", temp);
	cli_write_string(dev->cli_handler,
			 (uint8_t *)"RESP: FL_READ ODR?=");
	cli_write_string(dev->cli_handler, buff);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief Read and display the RATMASK setting in the flash software buffer.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_read_ratmask(struct cn0503_dev *dev)
{
	uint8_t buff[20];

	sprintf((char *)buff, "%x",
		(unsigned int)dev->sw_flash_buffer[CN0503_FLASH_RATM_IDX]);
	cli_write_string(dev->cli_handler,
			 (uint8_t *)"RESP: RATMASK?=");
	cli_write_string(dev->cli_handler, buff);
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief Read and display an application parameter stored in the flash software
 *        buffer.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter to be read.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_flash_read(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;
	int8_t i = 0;

	while(arg[i] != 0) {
		arg[i] = toupper(arg[i]);
		i++;
	}

	if (strncmp((char *)arg, "REG", 3) == 0) {
		ret = cn0503_flash_read_reg(dev, (arg + 5));
		if (ret != SUCCESS)
			return cli_write_string(dev->cli_handler,
						(uint8_t*)"reg command arguments incorrect or no more space.\n");
	} else if (strncmp((char *)arg, "DEF", 3) == 0) {
		ret = cn0503_flash_read_def(dev, (arg + 3));
		if (ret != SUCCESS)
			return cli_write_string(dev->cli_handler,
						(uint8_t*)"def command error.\n");
	} else if (strncmp((char *)arg, "MODE?", 6) == 0) {
		ret = cn0503_flash_read_mode(dev);
		if (ret != SUCCESS)
			return cli_write_string(dev->cli_handler,
						(uint8_t*)"mode command error.\n");
	} else if (strncmp((char *)arg, "ODR?", 5) == 0) {
		ret = cn0503_flash_read_odr(dev);
		if (ret != SUCCESS)
			return cli_write_string(dev->cli_handler,
						(uint8_t*)"odr command error.\n");
	} else if (strncmp((char *)arg, "RATMASK?", 9) == 0) {
		ret = cn0503_flash_read_ratmask(dev);
		if (ret != SUCCESS)
			return cli_write_string(dev->cli_handler,
						(uint8_t*)"ratmask command error.\n");
	} else {
		return cli_write_string(dev->cli_handler,
					(uint8_t*)"Error wrong command.\n");
	}

	return SUCCESS;
}

/**
 * @brief Iteratively do LED current calibration.
 * @param [in] dev - The device structure.
 * @param [in] chan - Optical channel that the LED luminates.
 * @param [in] led_no - LED ID.
 * @param [in] target - Target percent of ADC full-scale.
 * @param [in] max_curr - Maximum LED current allowed.
 * @param [out] landing - Actual percent of ADC full-scale at which we arrived.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_led_cal(struct cn0503_dev *dev, uint8_t chan,
			      uint8_t led_no, float target, float max_curr, float *landing)
{
	int32_t ret, i;
	uint16_t reg_data, pulse_cnt, no_data_samples = 0;
	uint8_t number_avrg, zero_adj_flag;
	uint16_t fd_reg_addr, fd_mask, fd_bitp, fd_val, fd_cnt = 1;
	uint16_t sd_reg_addr, sd_mask, sd_bitp, sd_val, sd_cnt = 1;
	uint32_t data_buff[ADPD410X_MAX_SLOT_NUMBER * 2], readback;
	float comp;
	const uint16_t adc_sat = 8192;
	uint8_t buff[80];

	if ((led_no == 0) || (led_no == 2)) {
		fd_reg_addr = ADPD410X_REG_LED_POW12(chan);
		sd_reg_addr = ADPD410X_REG_LED_POW12(chan);
	} else if ((led_no == 1) || (led_no == 3)) {
		fd_reg_addr = ADPD410X_REG_LED_POW34(chan);
		sd_reg_addr = ADPD410X_REG_LED_POW34(chan);
	}
	fd_mask = BITM_LED_POW12_A_LED_CURRENT1;
	fd_bitp = BITP_LED_POW12_A_LED_CURRENT1;
	sd_mask = BITM_LED_POW12_A_LED_CURRENT2;
	sd_bitp = BITP_LED_POW12_A_LED_CURRENT2;

	ret = adpd410x_reg_read(dev->adpd4100_handler,
				ADPD410X_REG_COUNTS(chan), &reg_data);
	if (ret != SUCCESS)
		return FAILURE;
	pulse_cnt = ((reg_data & BITM_COUNTS_A_NUM_INT) >>
		     BITP_COUNTS_A_NUM_INT) *
		    (reg_data & BITM_COUNTS_A_NUM_REPEAT);

	ret = adpd410x_reg_read(dev->adpd4100_handler,
				ADPD410X_REG_DECIMATE(chan), &reg_data);
	if (ret != SUCCESS)
		return FAILURE;
	number_avrg = ((reg_data & BITM_DECIMATE_A_DECIMATE_FACTOR) >>
		       BITP_DECIMATE_A_DECIMATE_FACTOR) + 1;

	ret = adpd410x_reg_read(dev->adpd4100_handler,
				ADPD410X_REG_ADC_OFF2(chan), &reg_data);
	if (ret != SUCCESS)
		return FAILURE;
	zero_adj_flag = (reg_data & BITM_ADC_OFF2_A_CH2_ADC_ADJUST) >>
			BITP_ADC_OFF2_A_CH2_ADC_ADJUST;

	ret = cn0503_update_ts_setting(dev);
	if(ret != SUCCESS)
		return FAILURE;

	ret = adpd410x_reg_read(dev->adpd4100_handler, fd_reg_addr, &fd_val);
	if (ret != SUCCESS)
		return FAILURE;
	fd_val &= ~fd_mask;
	fd_val |= (fd_cnt << fd_bitp) & fd_mask;
	ret = adpd410x_reg_write(dev->adpd4100_handler, fd_reg_addr, fd_val);
	if (ret != SUCCESS)
		return FAILURE;
	ret = adpd410x_reg_read(dev->adpd4100_handler, sd_reg_addr, &sd_val);
	if (ret != SUCCESS)
		return FAILURE;
	sd_val &= ~sd_mask;
	sd_val |= (sd_cnt << sd_bitp) & sd_mask;
	ret = adpd410x_reg_write(dev->adpd4100_handler, sd_reg_addr, sd_val);
	if (ret != SUCCESS)
		return FAILURE;

	for(i = 0; i < dev->active_slots; i++) {
		if(dev->two_channel_slots & (1 << i))
			no_data_samples += 2 * dev->data_sizes[i];
		else
			no_data_samples += dev->data_sizes[i];
	}

	ret = adpd410x_set_opmode(dev->adpd4100_handler,
				  ADPD410X_GOMODE);
	if(ret != SUCCESS)
		return FAILURE;

	while (1) {
		ret = adpd410x_get_fifo_bytecount(dev->adpd4100_handler,
						  &reg_data);
		if(ret != SUCCESS)
			return FAILURE;

		if(reg_data < no_data_samples)
			continue;
		ret = adpd410x_get_data(dev->adpd4100_handler, data_buff);
		if(ret != SUCCESS)
			return FAILURE;
		readback = max(data_buff[(chan * 2)],
			       data_buff[(chan * 2 + 1)]);
		comp = (((float)readback / (float)number_avrg) -
			(float)(zero_adj_flag * 2048)) /
		       (float)(pulse_cnt * adc_sat);
		comp *= 100.0;
		if ((comp > target) || ((((float)sd_cnt * ADPD410X_LED_CURR_LSB) +
					 ((float)fd_cnt * ADPD410X_LED_CURR_LSB)) > max_curr)) {
			sd_cnt--;
			fd_cnt--;
			ret = adpd410x_reg_read(dev->adpd4100_handler, fd_reg_addr, &fd_val);
			if (ret != SUCCESS)
				return FAILURE;
			fd_val &= ~fd_mask;
			fd_val |= (fd_cnt << fd_bitp) & fd_mask;
			ret = adpd410x_reg_write(dev->adpd4100_handler, fd_reg_addr, fd_val);
			if (ret != SUCCESS)
				return FAILURE;
			ret = adpd410x_reg_read(dev->adpd4100_handler, sd_reg_addr, &sd_val);
			if (ret != SUCCESS)
				return FAILURE;
			sd_val &= ~sd_mask;
			sd_val |= (sd_cnt << sd_bitp) & sd_mask;
			ret = adpd410x_reg_write(dev->adpd4100_handler, sd_reg_addr, sd_val);
			if (ret != SUCCESS)
				return FAILURE;
			break;
		} else if ((comp == target) ||
			   ((((float)sd_val * ADPD410X_LED_CURR_LSB) +
			     ((float)fd_val * ADPD410X_LED_CURR_LSB)) ==
			    max_curr)) {
			*landing = comp;
			break;
		}
		fd_cnt++;
		sd_cnt++;
		ret = adpd410x_reg_read(dev->adpd4100_handler, fd_reg_addr,
					&fd_val);
		if (ret != SUCCESS)
			return FAILURE;
		fd_val &= ~fd_mask;
		fd_val |= (fd_cnt << fd_bitp) & fd_mask;
		ret = adpd410x_reg_write(dev->adpd4100_handler, fd_reg_addr,
					 fd_val);
		if (ret != SUCCESS)
			return FAILURE;
		ret = adpd410x_reg_read(dev->adpd4100_handler, sd_reg_addr,
					&sd_val);
		if (ret != SUCCESS)
			return FAILURE;
		sd_val &= ~sd_mask;
		sd_val |= (sd_cnt << sd_bitp) & sd_mask;
		ret = adpd410x_reg_write(dev->adpd4100_handler, sd_reg_addr,
					 sd_val);
		if (ret != SUCCESS)
			return FAILURE;
		*landing = comp;
	}

	ret = adpd410x_set_opmode(dev->adpd4100_handler,
				  ADPD410X_STANDBY);
	if(ret != SUCCESS)
		return FAILURE;

	sprintf((char *)buff, "RESP: REG 0x%.4X = 0x%.4X\n\r", fd_reg_addr,
		fd_val);
	cli_write_string(dev->cli_handler, buff);
	sprintf((char *)buff, "RESP: REG 0x%.4X = 0x%.4X\n\r", sd_reg_addr,
		sd_val);
	cli_write_string(dev->cli_handler, buff);

	ret = adpd410x_reg_read(dev->adpd4100_handler,
				ADPD410X_REG_FIFO_STATUS, &reg_data);
	if (ret != SUCCESS)
		return FAILURE;
	reg_data |= BITM_INT_STATUS_FIFO_CLEAR_FIFO;

	return adpd410x_reg_write(dev->adpd4100_handler,
				  ADPD410X_REG_FIFO_STATUS, reg_data);
}

/**
 * @brief CLI command to calibrate LED current with respect to percent of ADC
 *        full-scale.
 * @param [in] dev - The device structure.
 * @param [in] arg - LED ID number, target percent and, optionally, the maximum
 *                   LED current allowed.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_led(struct cn0503_dev *dev, uint8_t *arg)
{
	uint8_t i = 0, *err_ptr, led_no, exists = 0;
	uint16_t reg_data, led_msk;
	float target, max_curr, landing;
	int32_t ret;
	uint8_t buff[100];
	extern uint8_t uart_current_line[100];

	if (dev->idle_state != 1)
		return SUCCESS;

	if (!arg) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR invalid arguments.\n");
		return FAILURE;
	}

	while(arg[i] != 0) {
		arg[i] = toupper(arg[i]);
		i++;
	}

	led_no = (uart_current_line[7] - 0x30) - 1;
	led_msk = 0xf << (led_no * 4);
	target = strtod((char *)arg, (char **)&err_ptr);
	if (err_ptr == arg) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR invalid arguments.\n");
		return FAILURE;
	}
	max_curr = strtod((char *)err_ptr, (char **)&arg);
	if (err_ptr == arg)
		max_curr = 338;

	i = 0;
	while (i < ADPD410X_MAX_SLOT_NUMBER) {
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_INPUTS(i), &reg_data);
		if (reg_data & led_msk) {
			ret = cn0503_led_cal(dev, i, led_no, target, max_curr,
					     &landing);
			if (ret != SUCCESS)
				return FAILURE;
			exists = 1;
		}
		i++;
	}

	cli_write_string(dev->cli_handler,
			 (uint8_t*)"RESP: PCB-LED");
	led_no += 0x31;
	uart_write(dev->cli_handler->uart_device, &led_no, 1);
	if (exists) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)" ");
		cli_write_string(dev->cli_handler, arg);
		sprintf((char *)buff, "%.2f%%\n", landing);
		cli_write_string(dev->cli_handler, buff);
	} else {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)" NOT USED");
	}

	return SUCCESS;
}

/**
 * @brief Set an ARAT expression for a preset function.
 * @param [in] dev - The device structure.
 * @param [in] opt - Preset option of the optical path.
 * @param [in] chann_no - ID of the optical path.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_channel_preset_arat(struct cn0503_dev *dev, uint8_t opt,
		uint8_t chann_no)
{
	uint8_t *errcheck, buff[50];
	int8_t char_tab[] = {'A', 'B', 'C', 'D'};

	if (opt == TURBIDITY) {
		errcheck = (uint8_t *)realloc(dev->arat[chann_no], 18);
		if(!errcheck) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR memory.\n");
			return FAILURE;
		}
		sprintf((char *)buff, "%c2#2048-%c1#2048-/", char_tab[chann_no],
			char_tab[chann_no]);
		dev->arat[chann_no] = errcheck;
		memcpy(dev->arat[chann_no], buff, 18);
		sprintf((char *)buff, "RESP: DEF%d? ARAT=%c2#2048-%c1#2048-/\n\r",
			chann_no, char_tab[chann_no],
			char_tab[chann_no]);
		cli_write_string(dev->cli_handler, buff);
	} else {
		errcheck = (uint8_t *)realloc(dev->arat[chann_no], 18);
		if(!errcheck) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR memory.\n");
			return FAILURE;
		}
		sprintf((char *)buff, "%c1#2048-%c2#2048-/", char_tab[chann_no],
			char_tab[chann_no]);
		dev->arat[chann_no] = errcheck;
		memcpy(dev->arat[chann_no], buff, 18);
		sprintf((char *)buff, "RESP: DEF%d? ARAT=%c1#2048-%c2#2048-/\n\r",
			chann_no, char_tab[chann_no],
			char_tab[chann_no]);
		cli_write_string(dev->cli_handler, buff);
	}

	return SUCCESS;
}

/**
 * @brief Set LED current for a specific optical platform preset.
 * @param [in] dev - The device structure.
 * @param [in] opt - Preset option of the optical path.
 * @param [in] chann_no - ID of the optical path.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_channel_preset_led_pow(struct cn0503_dev *dev,
		uint8_t chann_no, uint8_t opt)
{
	uint16_t led_reg_addr, reg_val;
	int32_t ret;
	uint8_t buff[80];

	led_reg_addr = !(chann_no % 2) ?
		       ADPD410X_REG_LED_POW12(chann_no) :
		       ADPD410X_REG_LED_POW34(chann_no) ;
	ret = adpd410x_reg_read(dev->adpd4100_handler, led_reg_addr,
				&reg_val);
	if(ret != SUCCESS)
		return FAILURE;

	reg_val &= ~(BITM_LED_POW12_A_LED_CURRENT1 |
		     BITM_LED_POW12_A_LED_CURRENT2);
	if (opt == FLUORESCENCE)
		reg_val |= 0x7070;
	else
		reg_val |= 0x30;

	ret = adpd410x_reg_write(dev->adpd4100_handler, led_reg_addr,
				 reg_val);
	if(ret != SUCCESS)
		return FAILURE;

	sprintf((char *)buff, "RESP: REG 0x%.4X=0x%.4X\n\r", led_reg_addr, reg_val);
	return cli_write_string(dev->cli_handler, buff);
}

/**
 * @brief CLI command to preset a channel for a specific function.
 * @param [in] dev - The device structure.
 * @param [in] arg - Option in ASCII form.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_channel_preset(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;
	uint8_t chann_no, opt, buff[50];
	int8_t *opt_tab[] = {
		(int8_t *)"COLO",
		(int8_t *)"FLUO",
		(int8_t *)"TURB",
		(int8_t *)""
	};
	int8_t *resp_tab[] = {
		(int8_t *)"COLORIMETRY/ABSORBANCE",
		(int8_t *)"FLUORESCENCE",
		(int8_t *)"TURBIDITY",
		(int8_t *)""
	};
	extern uint8_t uart_current_line[100];

	chann_no = *(arg - 2) - 0x30;

	if ((chann_no > 4) || (chann_no == 0)) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: CHANNEL NUMBER");
		return FAILURE;
	}

	opt = 0;
	while (arg[opt] != 0) {
		arg[opt] = toupper(arg[opt]);
		opt++;
	}

	opt = 0;
	while (opt_tab[opt][0] != 0) {
		if (strncmp((char *)arg, (char *)opt_tab[opt], 4) == 0)
			break;
		opt++;
	}
	if (opt > 2) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: OPTION");
		return FAILURE;
	}
	if ((opt > COLORIMETRY) && ((chann_no == 2) || (chann_no == 3))) {
		cli_write_string(dev->cli_handler,
				 (uint8_t*)"ERROR: CHANNEL NUMBER AND OPTION COMBINATION IMPOSSIBLE");
		return FAILURE;
	}

	chann_no--;
	switch(opt) {
	case COLORIMETRY:
		dev->rrat_sube[chann_no] = 1;
		break;
	case FLUORESCENCE:
	case TURBIDITY:
		dev->rrat_sube[chann_no] = 0;
		break;
	default:
		return FAILURE;
	}
	sprintf((char *)buff, "RESP: DEF%d? SUBE=%d\n\r", chann_no,
		dev->rrat_sube[chann_no]);
	cli_write_string(dev->cli_handler, buff);
	dev->ratb[chann_no] = 1;
	sprintf((char *)buff, "RESP: DEF%d? RATB=%.5E\n\r", chann_no,
		dev->ratb[chann_no]);
	cli_write_string(dev->cli_handler, buff);
	ret = cn0503_channel_preset_arat(dev, opt, chann_no);
	if (ret != SUCCESS)
		return FAILURE;
	ret = cn0503_channel_preset_led_pow(dev, chann_no, opt);
	if(ret != SUCCESS)
		return FAILURE;

	cli_write_string(dev->cli_handler,
			 (uint8_t*)"RESP: ");
	sprintf((char *)buff, "CHANN%c %s\n", uart_current_line[5],
		resp_tab[opt]);
	cli_write_string(dev->cli_handler, buff);

	return SUCCESS;
}

/**
 * @brief Dump the DEF parameters in the flash software buffer.
 * @param [in] dev - The device structure.
 * @param [in] rat_no - Optical path ID to dump.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_flash_swbuf_dump_def(struct cn0503_dev *dev,
		uint8_t rat_no)
{
	uint8_t j, temp_data;
	uint16_t sw_buff_index;
	uint8_t buff[50];
	float temp;

	sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28;
	if ((dev->sw_flash_buffer[sw_buff_index] & 0xFF000000) != 0xFF000000) {
		sprintf((char *)buff, "RESP: FL_DUMP DEF%d ARAT ", rat_no);
		cli_write_string(dev->cli_handler, buff);
	}

	for (j = 0; j < 8; j++) {
		sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + j;
		if ((dev->sw_flash_buffer[sw_buff_index] & 0xFF000000) == 0xFF000000)
			break;
		temp_data = (dev->sw_flash_buffer[sw_buff_index] & 0xFF000000) >> 24;
		uart_write(dev->cli_handler->uart_device, &temp_data, 1);
		if ((dev->sw_flash_buffer[sw_buff_index] & 0xFF0000) == 0xFF0000)
			break;
		temp_data = (dev->sw_flash_buffer[sw_buff_index] & 0xFF0000) >> 16;
		uart_write(dev->cli_handler->uart_device, &temp_data, 1);
		if ((dev->sw_flash_buffer[sw_buff_index] & 0xFF00) == 0xFF00)
			break;
		temp_data = (dev->sw_flash_buffer[sw_buff_index] & 0xFF00) >> 8;
		uart_write(dev->cli_handler->uart_device, &temp_data, 1);
		if ((dev->sw_flash_buffer[sw_buff_index] & 0xFF) == 0xFF)
			break;
		temp_data = dev->sw_flash_buffer[sw_buff_index] & 0xFF;
		uart_write(dev->cli_handler->uart_device, &temp_data, 1);
	}
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 8;
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "RESP: FL_DUMP DEF%d RFLT %.2f\n", rat_no, temp);
		cli_write_string(dev->cli_handler, buff);
	}
	sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 9;
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "RESP: FL_DUMP DEF%d ALRM %f ", rat_no, temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%f\n", temp);
		cli_write_string(dev->cli_handler, buff);
	}
	sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 11;
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "RESP: FL_DUMP DEF%d RATB %.5f\n", rat_no, temp);
		cli_write_string(dev->cli_handler, buff);
	}
	sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 12;
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		sprintf((char *)buff, "RESP: FL_DUMP DEF%d SUBE %d\n", rat_no,
			(int)dev->sw_flash_buffer[sw_buff_index]);
		cli_write_string(dev->cli_handler, buff);
	}
	sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 16;
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "RESP: FL_DUMP DEF%d INS1 %.5E ", rat_no, temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
	}
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
	}
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
	}
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
	}
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
	}
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E", temp);
		cli_write_string(dev->cli_handler, buff);
	}
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");
	sw_buff_index = CN0503_FLASH_REG_SIZE + rat_no * 28 + 22;
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "RESP: FL_DUMP DEF%d INS2 %.5E ", rat_no, temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
	}
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
	}
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
	}
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
	}
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E ", temp);
		cli_write_string(dev->cli_handler, buff);
		sw_buff_index++;
	}
	if (dev->sw_flash_buffer[sw_buff_index] != 0xFFFFFFFF) {
		memcpy((void *)&temp,
		       (void *)&dev->sw_flash_buffer[sw_buff_index],
		       4);
		sprintf((char *)buff, "%.5E", temp);
		cli_write_string(dev->cli_handler, buff);
	}
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief CLI command to dump flash software buffer data.
 * @param [in] dev - The device structure.
 * @param [in] arg - Parameter kept to comply with the CLI commands form, but
 *                   with no actual use in this function.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_flash_swbuf_dump(struct cn0503_dev *dev, uint8_t *arg)
{
	int16_t i = 0;
	uint8_t buff[50];

	while((dev->sw_flash_buffer[i] != 0xFFFFFFFF) &&
	      (i < CN0503_FLASH_REG_SIZE)) {
		sprintf((char *)buff, "RESP: FL_DUMP %x %x\n",
			(uint16_t)(dev->sw_flash_buffer[i] >> 16),
			(uint16_t)(dev->sw_flash_buffer[i] & 0xFFFF));
		cli_write_string(dev->cli_handler, buff);
		i++;
	}

	for (i = 0; i < CN0503_RAT_NO; i++)
		cn0503_flash_swbuf_dump_def(dev, i);

	if (dev->sw_flash_buffer[CN0503_FLASH_MODE_IDX] != 0xFFFFFFFF) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"RESP: FL_DUMP MODE ");
		cli_write_string(dev->cli_handler,
				 cn0503_ascii_modes[dev->sw_flash_buffer[CN0503_FLASH_MODE_IDX]]);
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"\n");
	}

	if (dev->sw_flash_buffer[CN0503_FLASH_RATM_IDX] != 0xFFFFFFFF) {
		sprintf((char *)buff, "RESP: FL_DUMP RATMASK %x\n",
			(unsigned int)dev->sw_flash_buffer[CN0503_FLASH_RATM_IDX]);
		cli_write_string(dev->cli_handler, buff);
	}

	return SUCCESS;
}

/**
 * @brief Application CLI prompt at the beginning of the program.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_prompt(struct cn0503_dev *dev)
{
	uint8_t buff[20];
	int32_t ret;
	uint16_t data;

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_CHIP_ID,
				&data);
	if(ret != SUCCESS)
		return FAILURE;

	data &= BITM_CHIP_ID;
	itoa(data, (char *)buff, 16);

	return cli_cmd_prompt(dev->cli_handler, buff);
}

/**
 * @brief Rollback configuration changes made during impulse response measurement.
 *        Free any malloc'd memory.
 *        Helper function for cn0503_impulse_response.
 * @param [in] dev - The device structure.
 */
void impulse_response_rollback(struct cn0503_dev *dev)
{
	int8_t i;
	struct cn0503_impulse_response *impresp = dev->impulse_response;

	if (impresp == NULL)
		return;
	if (impresp->timer != NULL)
		timer_remove(impresp->timer);
	adpd410x_set_opmode(dev->adpd4100_handler, ADPD410X_STANDBY);
	/** Rollback register values */
	struct reg_config *registers = impresp->registers;

	/** Unregister callback */
	if (impresp->interrupt_set) {
		irq_register_callback(dev->irq_handler, ADUCM_GPIO_A_INT_ID, NULL);
	}
	if (impresp->irq_cb != NULL)
		free(impresp->irq_cb->config);
	free(impresp->irq_cb);

	if (registers != NULL) {
		for (i = impresp->nb_reg_writes-1; i >= 0; i--) {
			// Write to rollback value register
			adpd410x_reg_write(dev->adpd4100_handler, registers[i].addr,
					   registers[i].rollback_value);
		}
	}

	/** Rollback output data rate */
	adpd410x_set_sampling_freq(dev->adpd4100_handler, CN0503_CODE_ODR_DEFAULT);

	/** Rollback timeslot number */
	adpd410x_set_last_timeslot(dev->adpd4100_handler, impresp->prev_active_slots);

	/** Free malloc'd structures */
	free(impresp->data);
	free(impresp->averaged_data);
	free(registers);
	free(impresp);
	dev->impulse_response = NULL;
}

/**
 * @brief Set up and start a cycle of impulse response data collection.
 *        Set integration offset, if necessary.
 *        Clear FIFO, if necessary. Start data collection.
 *        Helper function for cn0503_impulse_response.
 * @param [in] dev - The device structure.
 */
int32_t impulse_response_start_collection(struct cn0503_dev *dev)
{
	struct cn0503_impulse_response *impresp = dev->impulse_response;
	uint16_t integ_offset_lower, integ_offset_upper;
	int32_t ret;

	if (impresp->method != IMP) {
		/** Set integration offset */
		integ_offset_lower = impresp->data_cycle *
				     (impresp->sample_period_lower +
				      impresp->sample_period_upper * 32);

		integ_offset_upper = integ_offset_lower / 32 + impresp->led_width +
				     impresp->led_offset + impresp->first_sample_offset;
		// Make sure an offset of 0 corresponds to the empirically-determined
		// end of the LED pulse
		integ_offset_upper += fluo_start_time_offset_per_method[impresp->method];
		integ_offset_lower = integ_offset_lower % 32;

		ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_INTEG_OFFSET(0),
					 integ_offset_upper << BITP_INTEG_OFFSET_A_INTEG_OFFSET_UPPER |
					 integ_offset_lower);
		if (ret != SUCCESS)
			goto rollback;
	}

	/** Clear FIFO */
	ret = adpd410x_reg_write_mask(dev->adpd4100_handler, ADPD410X_REG_FIFO_STATUS,
				      1, BITM_INT_STATUS_FIFO_CLEAR_FIFO);
	if (ret != SUCCESS)
		goto rollback;

	/* Start data collection */
	ret = adpd410x_set_opmode(dev->adpd4100_handler, ADPD410X_GOMODE);
	if (ret != SUCCESS)
		goto rollback;
	return SUCCESS;

rollback:
	impulse_response_rollback(dev);
	return ret;
}

/**
 * @brief Read data from the FIFO.
 *        Called by impulse_response_fifo_callback
 * @param [in] dev - The device structure.
 */
int32_t impulse_response_read_data(struct cn0503_dev *dev)
{
	struct cn0503_impulse_response *impresp = dev->impulse_response;
	if (impresp == NULL)
		return FAILURE;

	uint16_t i;
	int32_t ret;
	float datapoint;
	uint8_t buff[64];
	ret = adpd410x_set_opmode(dev->adpd4100_handler, ADPD410X_STANDBY);
	if (ret != SUCCESS)
		goto rollback;

	/** Read FIFO */
	ret = adpd410x_read_fifo(dev->adpd4100_handler, impresp->data,
				 impresp->nb_fifo_samples, impresp->data_size);
	if (ret != SUCCESS)
		goto rollback;

	/** Average FIFO data */
	for (i = 0; i < impresp->nb_fifo_samples; i++) {
		datapoint = (float) impresp->data[i] / impresp->average_length;
		if (impresp->method == IMP) {
			impresp->averaged_data[i] += datapoint;
		} else {
			impresp->averaged_data[impresp->data_cycle] += datapoint;
		}
	}

	if (++impresp->data_cycle >= impresp->nb_data_cycles) {
		/** Data output */
		timer_stop(impresp->timer);
		snprintf((char *)buff, 64, "IMPULSE RESPONSE COLLECTION TIME (us) %ld\n",
			 impresp->timer->load_value);
		cli_write_string(dev->cli_handler, buff);
		timer_counter_set(impresp->timer, 0);

		snprintf((char *)buff, 64, "IMPULSE RESPONSE SAMPLE PERIOD %0.5f",
			 impresp->sample_period);
		cli_write_string(dev->cli_handler, buff);

		for (i = 0; i < impresp->nb_samples; i++) {
			snprintf((char *)buff, 64, "%0.4f", impresp->averaged_data[i]);

			if (i % 10 == 0) {
				cli_write_string(dev->cli_handler, (uint8_t *)"\nIMPULSE RESPONSE DATA ");
			} else if (i != 0) {
				cli_write_string(dev->cli_handler, (uint8_t *)" ");
			}
			cli_write_string(dev->cli_handler, buff);
		}

		cli_write_string(dev->cli_handler, (uint8_t *)"\n");
		cli_write_string(dev->cli_handler, (uint8_t *)"IMPULSE RESPONSE DONE\n");

		if (impresp->success_callback == NULL) {
			impulse_response_rollback(dev);
		} else {
			impresp->success_callback(dev);
			if (impresp != NULL)
				impulse_response_rollback(dev);
		}
		return SUCCESS;
	} else {
		return impulse_response_start_collection(dev);
	}
rollback:
	impulse_response_rollback(dev);
	return ret;
}

/**
 * @brief Called whenever a FIFO_TH interrupt is triggered.
 *        Sets a flag in dev->impulse_response to read data.
 *        (Data reads within the interrupt callback itself result in deadlock)
 * @param [in] param - The device structure.
 * @param [in] port - The GPIO port that triggered the interrupt.
 * @param [in] pin_int_data - Pin data from the interrupt.
 */
void impulse_response_fifo_callback(void* param, uint32_t port,
				    void* pin_int_data)
{
	struct cn0503_impulse_response *impresp = ((struct cn0503_dev *)
			param)->impulse_response;
	if (impresp != NULL)
		impresp->data_ready = true;
}

/**
 * @brief Parse a single argument of the impulse response or fluorescence decay command
 *        Helper function for cn0503_impulse_response, cn0503_fluo_decay_calibrate/measure
 * @param [in] impresp - The impulse response struct
 * @param [in] arg - Argument, in ASCII
 * @param [out] argval - The address at which the argument value will be stored
 * @param [in] arg_type - Which argument we are parsing, from enum cn0503_impulse_response_args
 */
void impulse_response_parse_arg(struct cn0503_impulse_response *impresp,
				uint8_t *arg, void *argval, uint8_t arg_type)
{
	uint8_t i;
	if (arg_type == NB_SAMPLES) { // uint16_t
		*((uint16_t *) argval) = atoi((char*) arg);
	} else if (arg_type == FIRST_SAMPLE_OFFSET) { // int8_t
		*((int8_t *) argval) = atoi((char*) arg);
	} else if (arg_type == SAMPLE_PERIOD
		   || arg_type == ACQUISITION_WIDTH) { // float
		*((float *) argval) = strtof((char *) arg, NULL);
	} else if (arg_type == METHOD) {
		// Parse sampling method
		i = 0;
		while (arg[i] != 0) { // Make method name uppercase
			arg[i] = toupper(arg[i]);
			i++;
		}
		impresp->method = 0;
		while (impresp_method_tab[impresp->method][0] != 0) {
			if (strncmp((char *) arg,
				    (char *) impresp_method_tab[impresp->method], 3) == 0) {
				break;
			}
			impresp->method++;
		}
	} else {
		*((uint8_t *) argval) = atoi((char*) arg);
	}
}

/**
 * @brief Parse a list of impulse response arguments
 *        Helper function for cn0503_impulse_response, cn0503_fluo_decay_calibrate/measure
 * @param [in] impresp - The impulse response struct
 * @param [in] arg - Arguments, in ASCII, separated by spaces
 * @param [out] argvals - The addresses at which the argument value will be stored
 * @param [in] arg_types - Which arguments we are parsing, from enum cn0503_impulse_response_args
 * @param [in] nb_args - number of arguments
 * @param [in] nb_required_args - nb_args minus number of optional arguments
 * @param [out] nb_args_parsed - number of args parsed. NULL to ignore.
 * @return SUCCESS if success, FAILURE or an error code otherwise.
 */
int32_t impulse_response_parse_arg_list(struct cn0503_impulse_response *impresp,
					uint8_t *arg, void **argvals, uint8_t *arg_types, uint8_t nb_args,
					uint8_t nb_required_args, uint8_t *nb_args_parsed)
{
	uint8_t i, *space_ptr;
	if (nb_args_parsed != NULL)
		*nb_args_parsed = 0;
	for (i = 0; i < nb_args; i++) {
		space_ptr = (uint8_t *)strchr((char *)arg, ' ');
		if (space_ptr)
			*space_ptr = 0;

		// If reading sampling method, must have at least three characters to read
		if (arg_types[i] == METHOD && strnlen((char *) arg, 4) < 3)
			return FAILURE;
		impulse_response_parse_arg(impresp, arg, argvals[i], arg_types[i]);
		if (nb_args_parsed != NULL)
			*nb_args_parsed++;

		if (space_ptr == NULL) {
			if (i < nb_required_args - 1) // Required arguments
				return FAILURE;
			else
				break;
		}
		arg = space_ptr + 1;
	}
	i++;
	return SUCCESS;
}

/**
 * @brief Validate impulse response commands, and modify their values if needed
 *        Helper function for cn0503_impulse_response, cn0503_fluo_decay_calibrate/measure
 * @param [in] dev - The device struct
 * @return SUCCESS if success, FAILURE or an error code otherwise.
 */
int32_t impulse_response_validate_parameters(struct cn0503_dev * dev)
{
	// IMP, TIA, SSI
	float min_sample_periods[] = {2, 1, 0.03125};
	struct cn0503_impulse_response *impresp = dev->impulse_response;

	if (impresp->method > 2) // Invalid method
		return FAILURE;

	// Validate channel number
	if (impresp->chann_no > 3) { // Invalid optical path
		cli_write_string(dev->cli_handler,
				 (uint8_t *) "ERROR: Invalid optical path. Must be 1-4.\n");
		return FAILURE;
	}

	if (impresp->nb_samples > CN0503_IMPRESP_MAX_SAMPLES) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *) "ERROR: Number of samples must at most 1950.\n");
		return FAILURE;
	}

	// Must start sampling after the LED has turned on
	if (impresp->first_sample_offset < -impresp->led_width) {
		impresp->first_sample_offset = -impresp->led_width;
	}
	/**
	 * Ensure sample period lower bound, and
	 * parse sample period to sample_period_upper, in one-microsecond steps
	 * and sample_period_lower, in 32.25-nanosecond steps.
	 * sample_period_lower is only used for single-sided mode.
	 */
	if (impresp->sample_period < min_sample_periods[impresp->method])
		impresp->sample_period = min_sample_periods[impresp->method];
	impresp->sample_period_upper = (uint16_t) impresp->sample_period;
	impresp->sample_period_lower =
		(uint16_t) round(impresp->sample_period * 32) % 32;
	// Only SSI uses the lower sample period
	if (impresp->method != SSI) {
		impresp->sample_period_lower = 0;
		impresp->sample_period = impresp->sample_period_upper;
	}

	if (impresp->method == IMP) {
		// Make sure we don't go past the FIFO depth
		if (impresp->nb_samples * impresp->data_size > ADPD410X_FIFO_DEPTH) {
			impresp->nb_samples = ADPD410X_FIFO_DEPTH / impresp->data_size;
		}
		impresp->nb_fifo_samples = impresp->nb_samples;
		impresp->nb_data_cycles = impresp->average_length;

	} else {
		// Make sure we don't go past the FIFO depth
		if (impresp->average_length * impresp->data_size > ADPD410X_FIFO_DEPTH) {
			impresp->average_length = ADPD410X_FIFO_DEPTH / impresp->data_size;
		}
		// Make sure the last sample time is not greater than the max integration offset
		if (impresp->nb_samples * impresp->sample_period + impresp->led_offset +
		    impresp->led_width + impresp->first_sample_offset > ADPD410X_MAX_INTEG_OS) {
			impresp->nb_samples =
				(uint16_t) ((ADPD410X_MAX_INTEG_OS - impresp->led_offset -
					     impresp->led_width - impresp->first_sample_offset) / impresp->sample_period);
		}
		impresp->nb_fifo_samples = impresp->average_length;
		impresp->nb_data_cycles = impresp->nb_samples;
	}
	return SUCCESS;
}

/**
 * @brief Set up the device configuration, timeslot settings, and sampling frequency
 *        Helper function for cn0503_impulse_response, cn0503_fluo_decay_calibrate/measure
 * @param [in] dev - The device structure
 * @return SUCCESS if success, FAILURE or an error code otherwise.
 */
int32_t impulse_response_setup(struct cn0503_dev *dev)
{
	int32_t ret;
	uint16_t temp_data;
	uint8_t i, j, nb_reg_writes;
	struct gpio_irq_config *gpio_irq;

	struct cn0503_impulse_response *impresp = dev->impulse_response;
	if (impresp == NULL)
		return FAILURE;
	impresp->prev_active_slots = dev->active_slots;

	struct reg_config *registers = calloc(24, sizeof(struct reg_config));
	if (registers == NULL) {
		ret = -ENOMEM;
		return ret;
	}
	impresp->registers = registers;

	/** Set device idle */
	ret = adpd410x_set_opmode(dev->adpd4100_handler, ADPD410X_STANDBY);
	if (ret != SUCCESS)
		return ret;

	ret = cn0503_update_ts_setting(dev);
	if(ret != SUCCESS)
		return ret;

	// Setup timeslots: just A
	ret = adpd410x_set_last_timeslot(dev->adpd4100_handler, ADPD410X_TS_A);
	if (ret != SUCCESS)
		return ret;

	// Sampling frequency: 2 kHz
	ret = adpd410x_set_sampling_freq(dev->adpd4100_handler, 2000);
	if(ret != SUCCESS)
		return ret;

	// Move configuration of desired optical path to timeslot A
	i = 0;
	uint16_t ts_setup_registers[] = {
		ADPD410X_REG_TS_CTRL(0), ADPD410X_REG_CATHODE(0),
		ADPD410X_REG_AFE_TRIM(0), ADPD410X_REG_PATTERN(0), ADPD410X_REG_LED_POW12(0),
		ADPD410X_REG_LED_POW34(0)
	};
	for (j = 0; j < sizeof(ts_setup_registers) / sizeof(ts_setup_registers[0]);
	     j++) {
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ts_setup_registers[j] + 0x20 * impresp->chann_no,
					&temp_data);
		if (ret != SUCCESS)
			return ret;

		impresp->registers[i].addr = ts_setup_registers[j];
		impresp->registers[i].value = temp_data;
		impresp->registers[i++].bitmask = 0xffff;
	}

	/** Clear FIFO before setting up interrupt */
	ret = adpd410x_reg_write_mask(dev->adpd4100_handler, ADPD410X_REG_FIFO_STATUS,
				      1, BITM_INT_STATUS_FIFO_CLEAR_FIFO);
	if (ret != SUCCESS)
		return ret;

	/** Setup interrupts */
	// Set interrupt Y to GPIO 0, triggered by FIFO threshold
	impresp->registers[i].addr = ADPD410X_REG_INT_ENABLE_YD;
	impresp->registers[i].value = 1 << BITP_INT_ENABLE_YD_INTY_EN_FIFO_TH;
	impresp->registers[i++].bitmask = 0xffff;

	impresp->registers[i].addr = ADPD410X_REG_GPIO01;
	impresp->registers[i].value = 0x3 << BITP_GPIO01_GPIOOUT0;
	impresp->registers[i++].bitmask = BITM_GPIO01_GPIOOUT0;

	impresp->registers[i].addr = ADPD410X_REG_GPIO_CFG;
	impresp->registers[i].value = 0x2 << BITP_GPIO_CFG_GPIO_PIN_CFG0;
	impresp->registers[i++].bitmask = BITM_GPIO_CFG_GPIO_PIN_CFG0;

	// Set FIFO threshold
	impresp->registers[i].addr = ADPD410X_REG_FIFO_TH;
	impresp->registers[i].value = impresp->data_size * impresp->nb_fifo_samples - 1;
	impresp->registers[i++].bitmask = BITM_FIFO_CTL_FIFO_TH;

	// Interrupt parameters
	impresp->irq_cb = (struct callback_desc *)
			  calloc(1, sizeof(*(impresp->irq_cb)));
	gpio_irq = (struct gpio_irq_config *) calloc(1, sizeof(*gpio_irq));
	gpio_irq->gpio_handler = dev->adpd4100_handler->gpio0;
	gpio_irq->mode = GPIO_GROUP_POSITIVE_EDGE;
	impresp->irq_cb->callback = impulse_response_fifo_callback;
	impresp->irq_cb->ctx = (void *) dev;
	impresp->irq_cb->config = (void *) gpio_irq;

	ret = gpio_direction_input(gpio_irq->gpio_handler);
	if (ret != SUCCESS)
		return ret;
	ret = irq_register_callback(dev->irq_handler, ADUCM_GPIO_A_INT_ID,
				    impresp->irq_cb);
	if (ret != SUCCESS)
		return ret;
	ret = irq_enable(dev->irq_handler, ADUCM_GPIO_A_INT_ID);
	if (ret != SUCCESS)
		return ret;

	impresp->interrupt_set = true;
	/** End setup interrupts */

	// Do not zero-adjust ADC
	impresp->registers[i].addr = ADPD410X_REG_ADC_OFF2(0);
	impresp->registers[i].value = 0;
	impresp->registers[i++].bitmask = BITM_ADC_OFF2_A_ZERO_ADJUST;
	// Timeslot path: TIA INT ADC
	impresp->registers[i].addr = ADPD410X_REG_TS_PATH(0);
	impresp->registers[i].value = 0x0E6;
	impresp->registers[i++].bitmask = BITM_TS_PATH_A_AFE_PATH_CFG;
	// LED pulse width: user set, LED offset: 16 us
	impresp->registers[i].addr = ADPD410X_REG_LED_PULSE(0);
	impresp->registers[i].value = impresp->led_width << BITP_LED_PULSE_A_LED_WIDTH |
				      impresp->led_offset;
	impresp->registers[i++].bitmask = 0xffff;
	// Data1: dark size 0, update signal size
	impresp->registers[i].addr = ADPD410X_REG_DATA1(0);
	impresp->registers[i].value = impresp->data_size;
	impresp->registers[i++].bitmask = 0xffff;
	// Data2: lit size 0
	impresp->registers[i].addr = ADPD410X_REG_DATA2(0);
	impresp->registers[i].value = 0;
	impresp->registers[i++].bitmask = BITM_DATA2_A_LIT_SIZE;

	if (impresp->method != SSI) {
		// Integ setup: setup integrator as a buffer, disable channel 2 power
		impresp->registers[i].addr = ADPD410X_REG_INTEG_WIDTH(0);
		impresp->registers[i].value = (0x1 << BITP_INTEG_WIDTH_A_AFE_INT_C_BUF) |
					      (0x1 << BITP_INTEG_WIDTH_A_SINGLE_INTEG) | (0x7 <<
							      BITP_INTEG_WIDTH_A_CH2_AMP_DISABLE);
		impresp->registers[i++].bitmask = BITM_INTEG_WIDTH_A_AFE_INT_C_BUF |
						  BITM_INTEG_WIDTH_A_SINGLE_INTEG | BITM_INTEG_WIDTH_A_CH2_AMP_DISABLE;
	}
	if (impresp->method == IMP) {
		// Timeslot control: impulse response, disable channel 2
		impresp->registers[i].addr = ADPD410X_REG_TS_CTRL(0);
		impresp->registers[i].value = 0x3 << BITP_TS_CTRL_A_SAMPLE_TYPE;
		impresp->registers[i++].bitmask = BITM_TS_CTRL_A_SAMPLE_TYPE |
						  BITM_TS_CTRL_A_CH2_EN;
		// Counts: acquisition width
		impresp->registers[i].addr = ADPD410X_REG_COUNTS(0);
		impresp->registers[i].value = 0x1 | impresp->nb_samples <<
					      BITP_COUNTS_A_NUM_INT;
		impresp->registers[i++].bitmask = 0xffff;
		// Lit offset: Start of impulse response sampling
		impresp->registers[i].addr = ADPD410X_REG_DIGINT_LIT(0);
		impresp->registers[i].value = impresp->led_width + impresp->led_offset +
					      impresp->first_sample_offset + fluo_start_time_offset_per_method[0];
		impresp->registers[i++].bitmask = BITM_DIGINT_LIT_A_LIT_OFFSET;
		// Integ offset: spacing between samples user set
		impresp->registers[i].addr = ADPD410X_REG_INTEG_OFFSET(0);
		impresp->registers[i].value = impresp->sample_period_upper <<
					      BITP_INTEG_OFFSET_A_INTEG_OFFSET_UPPER;
		impresp->registers[i++].bitmask = BITM_INTEG_OFFSET_A_INTEG_OFFSET_UPPER |
						  BITM_INTEG_OFFSET_A_INTEG_OFFSET;
	} else {
		// Timeslot control: normal mode, disable channel 2
		impresp->registers[i].addr = ADPD410X_REG_TS_CTRL(0);
		impresp->registers[i].value = 0;
		impresp->registers[i++].bitmask = BITM_TS_CTRL_A_SAMPLE_TYPE |
						  BITM_TS_CTRL_A_CH2_EN;
		// Counts: 1 ADC, 1 pulse
		impresp->registers[i].addr = ADPD410X_REG_COUNTS(0);
		impresp->registers[i].value = 0x1 | (0x1 << BITP_COUNTS_A_NUM_INT);
		impresp->registers[i++].bitmask = 0xffff;
		// Integ offset: will be overwritten, but adding to registers array
		// so that this register value is automatically rolled back at the end
		impresp->registers[i].addr = ADPD410X_REG_INTEG_OFFSET(0);
		impresp->registers[i].value = 0;
		impresp->registers[i++].bitmask = 0;
		if (impresp->method == SSI) {
			// Integ setup: Single integration pulse, disable channel 2 power
			impresp->registers[i].addr = ADPD410X_REG_INTEG_WIDTH(0);
			impresp->registers[i].value = (1 << BITP_INTEG_WIDTH_A_SINGLE_INTEG) |
						      (0x7 << BITP_INTEG_WIDTH_A_CH2_AMP_DISABLE) | 0x8;
			impresp->registers[i++].bitmask = BITM_INTEG_WIDTH_A_AFE_INT_C_BUF |
							  BITM_INTEG_WIDTH_A_SINGLE_INTEG | BITM_INTEG_WIDTH_A_CH2_AMP_DISABLE |
							  BITM_INTEG_WIDTH_A_INTEG_WIDTH;
			// Pattern: Invert integral to match the configuration of the integrator
			// in IMP and TIA as an inverting buffer amplifier
			impresp->registers[i].addr = ADPD410X_REG_PATTERN(0);
			impresp->registers[i].value = (1 << BITP_PATTERN_A_REVERSE_INTEG);
			impresp->registers[i++].bitmask = BITM_PATTERN_A_REVERSE_INTEG;
		}
	}
	nb_reg_writes = i;

	for (i = 0; i < nb_reg_writes; i++) {
		// Store previous reg value for cleanup
		ret = adpd410x_reg_read(dev->adpd4100_handler, impresp->registers[i].addr,
					&(impresp->registers[i].rollback_value));
		if (ret != SUCCESS)
			return ret;

		impresp->registers[i].value = (impresp->registers[i].rollback_value &
					       ~impresp->registers[i].bitmask) |
					      (impresp->registers[i].value & impresp->registers[i].bitmask);
		// Write to register
		ret = adpd410x_reg_write(dev->adpd4100_handler, impresp->registers[i].addr,
					 impresp->registers[i].value);
		if (ret != SUCCESS)
			return ret;

		impresp->nb_reg_writes++;
	}

	/** Data collection */
	// Array for reading samples from the FIFO
	impresp->data = (uint32_t *) calloc(impresp->nb_fifo_samples, sizeof(uint32_t));
	if (impresp->data == NULL)
		return -ENOMEM;

	// Impulse response, averaged over average_length trials
	impresp->averaged_data = (float *) calloc(impresp->nb_samples, sizeof(float));
	if (impresp->averaged_data == NULL)
		return -ENOMEM;

	return SUCCESS;
}

/**
 * @brief Compute the impulse response of a channel
 * @param [in] dev - The device structure
 * @param [in] arg - Channel of interest
 *                   Length of initial LED pulse, in us
 *                   Acquisition region, in us
 *                   Sample spacing, in us
 *                   Method: IMP for regular impulse response mode,
 *							 TIA for TIA-ADC mode,
 *                           SSI for single-sided integration
 *                   [optional] Average length (default 40)
 *                   [optional] Integration start offset from end of LED pulse
 *                   	(default 0)
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_impulse_response(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;
	uint8_t buff[64];
	float acquisition_width;
	struct cn0503_impulse_response *impresp = calloc(1, sizeof(*impresp));
	if (impresp == NULL)
		return -ENOMEM;

	struct timer_init_param timer_param;
	timer_param.id = 0;
	timer_param.freq_hz = 1000000u;
	timer_param.load_value = 0;
	timer_param.extra = NULL;
	timer_init(&impresp->timer, &timer_param);
	timer_start(impresp->timer);

	dev->impulse_response = impresp;

	impresp->data_size = 2;
	impresp->led_offset = CN0503_FLUO_DEFAULT_LED_OFF;
	impresp->success_callback = impulse_response_rollback;
	impresp->average_length = 40;

	/** Parse arguments */
	void *argvals[] = {&impresp->chann_no, &impresp->led_width, &acquisition_width,
			   &impresp->sample_period, &impresp->method, &impresp->average_length,
			   &impresp->first_sample_offset
			  };
	uint8_t arg_types[] = {CHANN_NO, LED_WIDTH, ACQUISITION_WIDTH, SAMPLE_PERIOD, METHOD,
			       AVERAGE_LENGTH, FIRST_SAMPLE_OFFSET
			      };

	ret = impulse_response_parse_arg_list(impresp, arg, argvals, arg_types, 7, 5,
					      NULL);
	if (ret != SUCCESS)
		goto rollback_return;

	// Convert acquisiton width to number of samples
	if (acquisition_width < 0)
		acquisition_width = 0;
	impresp->nb_samples = ceil(acquisition_width / impresp->sample_period) + 1;
	impresp->chann_no--;

	ret = impulse_response_validate_parameters(dev);
	if (ret != SUCCESS)
		goto rollback_return;

	ret = impulse_response_setup(dev);
	if (ret != SUCCESS)
		goto rollback_return;

	timer_stop(impresp->timer);
	snprintf((char *)buff, 64, "IMPULSE RESPONSE SETUP TIME (us) %ld\n",
		 impresp->timer->load_value);
	cli_write_string(dev->cli_handler, buff);
	timer_counter_set(impresp->timer, 0);
	timer_start(impresp->timer);

	return impulse_response_start_collection(dev);

rollback_return:
	impulse_response_rollback(dev);
	return ret;
}

/**
 * @brief Solve Ax = b, where A is a 2x2 matrix and
 *        b is a 2x1 vector.
 * @param [in] A
 * @param [in] b
 * @param [out] x
 */
int32_t matsolve_2x2(float *A, float *b, float *x)
{
	float det = A[0] * A[3] - A[1] * A[2];
	if (det == 0) // Singular matrix
		return FAILURE;
	x[0] = (A[3] * b[0] - A[1] * b[1]) / det;
	x[1] = (A[0] * b[1] - A[2] * b[0]) / det;
	return SUCCESS;
}

/**
 * @brief Given a fluorescence decay impulse response measurement,
 *        subtract out the calibration reference response and
 *        compute the decay time constant.
 *        Helper function for cn0503_fluo_decay_measure.
 * @param [in] dev - the device structure
 */
void compute_decay_constant(struct cn0503_dev *dev)
{
	struct cn0503_impulse_response *impresp = dev->impulse_response;
	float *data = impresp->averaged_data,
	       t0 = (float) impresp->first_sample_offset,
	       ref_response_i, t, t_ref, avg_error = 0,
					 dc_offset, scaling_factor, tau, tau_inv, S_i = 0;
	uint8_t buff[64], nb_flash_pages = 0;
	uint16_t i, j, flash_idx, ref_idx_offset, last_complete_idx = 0;
	uint32_t flash_addr;

	// 2x2 matrices, flattened
	float A[] = {0, 0, 0, 0};
	float C[] = {0, 0, 0, 0};

	// 2x1 matrices, flattened
	float B[] = {0, 0};
	float AinvB[] = {0, 0};
	float D[] = {0, 0};
	float CinvD[] = {0, 0};

	timer_start(impresp->timer);

	if (!impresp->skip_calib) {
		// Read reference response from flash and subtract from decay response
		flash_addr = dev->fluo_calib_flash_page_addr -
			     impresp->calib_slot * 0x800 * CN0503_FLASH_FLUO_MAX_PAGES;

		ref_idx_offset = (uint16_t) ((t0 - impresp->ref_start_time) /
					     impresp->ref_sample_period);
		t_ref = impresp->ref_start_time + ref_idx_offset * impresp->ref_sample_period;
		flash_idx = CN0503_FLASH_FLUO_PARAM_SIZE + ref_idx_offset;

		i = 0;
		while (i < impresp->nb_samples ||
		       last_complete_idx < impresp->nb_samples - 1) {
			while (flash_idx >= CN0503_FLASH_BUFF_SIZE) {
				flash_addr -= 0x800;
				flash_idx -= CN0503_FLASH_BUFF_SIZE;
				nb_flash_pages++;
			}
			flash_read(dev->flash_handler, flash_addr + flash_idx * 4,
				   (uint32_t *) &ref_response_i, 1);

			// Perform second half of linear interpolation on any indices where it is needed
			for (j = last_complete_idx + 1; j < i; j++) {
				t = t0 + j *  impresp->sample_period;
				if (t_ref <= t)
					break;
				data[j] -= ref_response_i * (t - (t_ref-impresp->ref_sample_period)) /
					   impresp->ref_sample_period;
				last_complete_idx = j;
			}
			if (i == impresp->nb_samples)
				break;

			t = t0 + i *  impresp->sample_period;

			if (abs(t_ref - t) < 0.015625) {
				// Equal within half of the minimum timestep
				data[i] -= ref_response_i;
				last_complete_idx = i;
			} else {
				// Computation of ref response index rounds down, so t_ref < t
				// Perform first half of linear interpolation on reference response
				data[i] -= ref_response_i * (t_ref+impresp->ref_sample_period - t) /
					   impresp->ref_sample_period;
			}

			// Compute next round of times and indices for the decay and reference responses
			if (i == impresp->nb_samples - 1)
				ref_idx_offset++;
			else
				ref_idx_offset = (uint16_t) ((t +  impresp->sample_period -
							      impresp->ref_start_time) /
							     impresp->ref_sample_period);

			flash_idx = CN0503_FLASH_FLUO_PARAM_SIZE + ref_idx_offset - nb_flash_pages *
				    CN0503_FLASH_BUFF_SIZE;
			t_ref = impresp->ref_start_time + ref_idx_offset * impresp->ref_sample_period;
			i++;
		}
	}
	// Compute initial exponential fit
	// Based on formulae from:
	// https://fr.scribd.com/doc/14674814/Regressions-et-equations-integrales
	for (i = 1; i < impresp->nb_samples; i++) {
		S_i += (data[i] + data[i - 1]) / 2 *  impresp->sample_period;
		A[0] += pow(i *  impresp->sample_period, 2);
		A[1] += S_i * (i *  impresp->sample_period);
		A[3] += pow(S_i, 2);

		B[0] += (data[i] - data[0]) * i *  impresp->sample_period;
		B[1] += (data[i] - data[0]) * S_i;
	}
	A[2] = A[1];

	matsolve_2x2(A, B, AinvB);
	tau_inv = -AinvB[1];

	for (i = 0; i < impresp->nb_samples; i++) {
		t = t0 + i *  impresp->sample_period;
		C[1] += pow(M_E, -tau_inv * t);
		C[3] += pow(M_E, -2 * tau_inv * t);
		D[0] += data[i];
		D[1] += data[i] * pow(M_E, -tau_inv * t);
	}
	C[0] = impresp->nb_samples;
	C[2] = C[1];

	matsolve_2x2(C, D, CinvD);

	dc_offset = CinvD[0];
	scaling_factor = CinvD[1];
	tau = 1 / tau_inv;

	// Compute average squared error of fit
	for (i = 0; i < impresp->nb_samples; i++) {
		t = t0 + i *  impresp->sample_period;
		avg_error += abs(data[i] - (dc_offset + scaling_factor * pow(M_E,
					    -t * tau_inv)));
	}
	avg_error /= impresp->nb_samples;

	timer_stop(impresp->timer);
	snprintf((char *)buff, 64, "FLUO DECAY PROCESSING TIME (us) %ld\n",
		 impresp->timer->load_value);
	cli_write_string(dev->cli_handler, buff);
	timer_counter_set(impresp->timer, 0);

	snprintf((char *)buff, 64, "FLUO DECAY START TIME %0.1f\n", t0);
	cli_write_string(dev->cli_handler, buff);
	cli_write_string(dev->cli_handler,
			 (uint8_t *)"\nFLUO DECAY FIT (Code LSB vs. microseconds): A + B exp(-t/tau)\n");
	snprintf((char *)buff, 64, "FLUO DECAY A %0.5f\n", dc_offset);
	cli_write_string(dev->cli_handler, buff);
	snprintf((char *)buff, 64, "FLUO DECAY B %0.5f\n", scaling_factor);
	cli_write_string(dev->cli_handler, buff);
	snprintf((char *)buff, 64, "FLUO DECAY TAU %0.5f\n", tau);
	cli_write_string(dev->cli_handler, buff);
	snprintf((char *)buff, 64, "FLUO DECAY AVG PER-SAMPLE ERROR %0.5f\n",
		 avg_error);
	cli_write_string(dev->cli_handler, buff);

	for (i = 0; i < impresp->nb_samples; i++) {
		snprintf((char *)buff, 64, "%0.4f", data[i]);

		if (i % 10 == 0) {
			cli_write_string(dev->cli_handler, (uint8_t *)"\nFLUO DECAY DATA ");
		} else if (i != 0) {
			cli_write_string(dev->cli_handler, (uint8_t *)" ");
		}
		cli_write_string(dev->cli_handler, buff);
	}
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");
	cli_write_string(dev->cli_handler, (uint8_t *)"FLUO DECAY DONE\n");

	impulse_response_rollback(dev);
}

/**
 * @brief Write fluorescence decay calibration data to flash.
 *        Helper function for cn0503_fluo_decay_calibrate.
 * @param [in] dev - the device structure
 */
void process_fluo_decay_calibration(struct cn0503_dev *dev)
{
	struct cn0503_impulse_response *impresp = dev->impulse_response;
	uint16_t flash_idx = 0, i;
	float end_time, *data = impresp->averaged_data;
	uint32_t flash_data[64], flash_addr, flash_addr_offset = 0;
	uint8_t buff[64];

	flash_addr = dev->fluo_calib_flash_page_addr -
		     impresp->calib_slot * 0x800 * CN0503_FLASH_FLUO_MAX_PAGES;
	timer_start(impresp->timer);

	flash_data[flash_idx] = impresp->chann_no & 0xFF;
	flash_data[flash_idx] |= (impresp->led_width & 0xFF) << 8;
	flash_data[flash_idx] |= (impresp->first_sample_offset & 0xFF) << 16;
	flash_data[flash_idx++] |= impresp->method << 24;
	end_time = impresp->first_sample_offset + impresp->sample_period *
		   (impresp->nb_samples - 1);
	memcpy((void *) (flash_data + flash_idx++), (void *) &end_time, 4);
	memcpy((void *) (flash_data + flash_idx++),
	       (void *) &impresp->sample_period,4);

	for (i = 0; i < impresp->nb_samples; i++) {
		if (flash_idx == 64) {
			// Write back full flash page
			if (flash_write(dev->flash_handler, flash_addr + flash_addr_offset * 4,
					flash_data, 64) != SUCCESS)
				goto rollback;
			flash_addr_offset += 64;
			flash_idx = 0;
			if (flash_addr_offset == CN0503_FLASH_BUFF_SIZE) {
				flash_addr -= 0x800;
				flash_addr_offset = 0;
			}
		}
		memcpy((void *) (flash_data + flash_idx++), (void *) (data + i), 4);
	}
	// Write back last flash page
	flash_write(dev->flash_handler, flash_addr + flash_addr_offset * 4,
		    flash_data, 64);

	timer_stop(impresp->timer);
	snprintf((char *)buff, 64, "FLUO CALIB PROCESSING TIME (us) %ld\n",
		 impresp->timer->load_value);
	cli_write_string(dev->cli_handler, buff);
	timer_counter_set(impresp->timer, 0);

rollback:
	impulse_response_rollback(dev);
	cli_write_string(dev->cli_handler, (uint8_t *)"FLUO CALIB DONE\n");
}

/**
 * @brief Set preset parameters for fluorescence decay.
 *        Helper function for cn0503_fluo_decay_measure/calibrate.
 * @param [in] impresp - the impulse response structure
 * @param [in] preset - which preset we are using, from enum cn0503_fluo_presets
 */
void cn0503_fluo_set_presets(struct cn0503_impulse_response *impresp,
			     uint8_t preset)
{
	if (preset == IMPPRESET) {
		impresp->sample_period = 2;
		impresp->method = IMP;
	} else if (preset == TIAPRESET) {
		impresp->sample_period = 1;
		impresp->method = TIA;
	} else if (preset == SSIPRESET) {
		impresp->sample_period = 0.25;
		impresp->method = SSI;
	} else if (preset == MAXRESPRESET) {
		impresp->sample_period = 0.03125;
		impresp->average_length = 25;
		impresp->method = SSI;
	}
	impresp->first_sample_offset = 0;
}

/**
 * @brief Perform calibration for the fluorescence decay measurement.
 * @param [in] dev - The device structure
 * @param [in] arg -
 *		If using a preset:
 *                   {IMPPRESET, TIAPRESET, SSIPRESET, MAXRESPRESET}
 *                   Channel of interest
 *                   Length of acquisition region
 *                   [optional] Average length (default 60)
 *		Otherwise:
 *                   Calibration slot in flash to write to (1 or 2)
 *			         Channel of interest (1 or 4),
 *                   Length of initial LED pulse, in us
 *                   Acquisition width
 *                   Sample period, in us
 *                   Method (IMP, TIA, SSI)
 *                   [optional] Average length (default 60)
 *                   [optional] Integration start offset from end of LED pulse
 *                       (default 0)
 * @return SUCCESS if success, FAILURE or error code otherwise.
 */
int32_t cn0503_fluo_decay_calibrate(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;
	uint8_t i, preset;
	float acquisition_width;
	uint8_t buff[64];

	struct cn0503_impulse_response *impresp = calloc(1, sizeof(*impresp));
	if (impresp == NULL)
		return -ENOMEM;

	struct timer_init_param timer_param;
	timer_param.id = 0;
	timer_param.freq_hz = 1000000u;
	timer_param.load_value = 0;
	timer_param.extra = NULL;
	timer_init(&impresp->timer, &timer_param);
	timer_start(impresp->timer);

	dev->impulse_response = impresp;

	// Default values of parameters
	impresp->data_size = 2;
	impresp->led_offset = CN0503_FLUO_DEFAULT_LED_OFF;
	impresp->average_length = 60;
	impresp->led_width = 50;
	impresp->success_callback = process_fluo_decay_calibration;

	// Read first argument to see if we're using a preset
	uint8_t *space_ptr;
	space_ptr = (uint8_t *)strchr((char *)arg, ' ');
	if (space_ptr == NULL) {
		ret = FAILURE;
		goto rollback_return;
	}
	if (space_ptr - arg >= 9) { // Length of smallest preset keyword
		*space_ptr = 0;
		i = 0;
		while (arg[i] != 0) { // Make keyword name uppercase
			arg[i] = toupper(arg[i]);
			i++;
		}
		preset = 0;
		while (fluo_preset_tab[preset][0] != 0) {
			if (strncmp((char *) arg, (char *) fluo_preset_tab[preset], 13) == 0) {
				break;
			}
			preset++;
		}
		if (preset > 3) { // Invalid method
			ret = FAILURE;
			goto rollback_return;
		}

		cn0503_fluo_set_presets(impresp, preset);
		void *argvals[] = {&impresp->calib_slot, &impresp->chann_no, &acquisition_width,
				   &impresp->average_length
				  };
		uint8_t arg_types[] = {CALIB_SLOT, CHANN_NO, ACQUISITION_WIDTH, AVERAGE_LENGTH};
		ret = impulse_response_parse_arg_list(impresp, space_ptr + 1, argvals,
						      arg_types, 4, 3, NULL);
		if (ret != SUCCESS)
			goto rollback_return;
	} else {
		void *argvals[] = {&impresp->calib_slot, &impresp->chann_no, &impresp->led_width,
				   &acquisition_width, &impresp->sample_period, &impresp->method,
				   &impresp->average_length, &impresp->first_sample_offset
				  };
		uint8_t arg_types[] = {CALIB_SLOT, CHANN_NO, LED_WIDTH, ACQUISITION_WIDTH, SAMPLE_PERIOD,
				       METHOD, AVERAGE_LENGTH, FIRST_SAMPLE_OFFSET
				      };
		ret = impulse_response_parse_arg_list(impresp, arg, argvals, arg_types, 8, 6,
						      NULL);
		if (ret != SUCCESS)
			goto rollback_return;
	}
	// Convert acquisiton width to number of samples
	if (acquisition_width < 0)
		acquisition_width = 0;
	impresp->nb_samples = ceil(acquisition_width / impresp->sample_period) + 1;
	impresp->chann_no--;
	ret = impulse_response_validate_parameters(dev);
	if (ret != SUCCESS)
		goto rollback_return;

	if (impresp->calib_slot != 1 && impresp->calib_slot != 2) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)
				 "FLUO DECAY ERROR: Invalid calibration slot. Must be 1 or 2.\n");
		goto rollback_return;
	}
	impresp->calib_slot--;

	if (impresp->chann_no != 0 && impresp->chann_no != 3) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)
				 "ERROR: Can only use optical paths 1 or 4 for fluorescence decay.\n");
		goto rollback_return;
	}

	// Check that all of the datapoints can fit into flash
	if (impresp->nb_samples > CN0503_FLASH_BUFF_SIZE *
	    CN0503_FLASH_FLUO_MAX_PAGES - CN0503_FLASH_FLUO_PARAM_SIZE) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *) "ERROR: Too many samples!\n");
		goto rollback_return;
	}

	ret = impulse_response_setup(dev);
	if (ret != SUCCESS)
		goto rollback_return;

	timer_stop(impresp->timer);
	snprintf((char *)buff, 64, "FLUO CALIB SETUP TIME (us) %ld\n",
		 impresp->timer->load_value);
	cli_write_string(dev->cli_handler, buff);
	timer_counter_set(impresp->timer, 0);
	timer_start(impresp->timer);

	return impulse_response_start_collection(dev);

rollback_return:
	impulse_response_rollback(dev);
	return ret;
}

/**
 * @brief Measure a fluorescence decay response.
 * @param [in] dev - The device structure
 * @param [in] arg -
 *		If skipping calibration:
 *                   NOCALIB
 *                   Channel of interest (1 or 4)
 *                   Length of LED pulse
 *                   Acquisition width
 *                   Sample period, in us
 *                   Method (IMP, TIA, SSI)
 *                   [optional] Average length (default 40)
 *                   [optional] Integration start offset from end of LED pulse (default 0us)
 *		If using a preset:
 *                   {IMPPRESET, TIAPRESET, SSIPRESET, MAXRESPRESET}
 *                   Channel of interest
 *                   Length of acquisition region
 *                   [optional] Average length (default 40)
 *      Otherwise:
 *                   Channel of interest
 *                   Acquisition width
 *                   Sample period, in us
 *                   [optional] Average length (default 40)
 *                   [optional] Integration start offset from end of LED pulse
 *						(defaults to value from reference response)
 * 					 @note: LED width and method will be read from calibration data in flash
 * @return SUCCESS if success, FAILURE or an error code otherwise
 */
int32_t cn0503_fluo_decay_measure(struct cn0503_dev *dev, uint8_t *arg)
{
	int32_t ret;
	uint32_t flash_data[4], flash_addr;
	uint8_t i, preset;
	float acquisition_width;
	uint16_t flash_idx;
	float end_time;
	uint8_t buff[64], nb_args_parsed;
	bool using_preset = false;

	struct cn0503_impulse_response *impresp = calloc(1, sizeof(*impresp));
	if (impresp == NULL) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *) "FLUO DECAY ERROR: Not enough memory.\n");
		ret = -ENOMEM;
		goto rollback_return;
	}
	struct timer_init_param timer_param;
	timer_param.id = 0;
	timer_param.freq_hz = 1000000u;
	timer_param.load_value = 0;
	timer_param.extra = NULL;
	timer_init(&impresp->timer, &timer_param);
	timer_start(impresp->timer);

	dev->impulse_response = impresp;

	impresp->data_size = 2;
	impresp->led_offset = CN0503_FLUO_DEFAULT_LED_OFF;
	impresp->success_callback = compute_decay_constant;
	impresp->average_length = 40;

	// Read first argument to see if we're skipping calibration or using a preset
	uint8_t *space_ptr;
	space_ptr = (uint8_t *)strchr((char *)arg, ' ');
	if (space_ptr == NULL) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *) "FLUO DECAY ERROR: Invalid argument string.\n");
		ret = FAILURE;
		goto rollback_return;
	}

	if (space_ptr - arg >= 7) { // Length of smallest keyword
		*space_ptr = 0;
		i = 0;
		while (arg[i] != 0) { // Make keyword name uppercase
			arg[i] = toupper(arg[i]);
			i++;
		}
		if (strncmp((char *) arg, "NOCALIB", 7) == 0) { // Skipping calibration
			impresp->skip_calib = true;
			void *argvals[] = {&impresp->chann_no, &impresp->led_width, &acquisition_width,
					   &impresp->sample_period, &impresp->method, &impresp->average_length,
					   &impresp->first_sample_offset
					  };
			uint8_t arg_types[] = {CHANN_NO, LED_WIDTH, ACQUISITION_WIDTH, SAMPLE_PERIOD, METHOD,
					       AVERAGE_LENGTH, FIRST_SAMPLE_OFFSET
					      };
			ret = impulse_response_parse_arg_list(impresp, space_ptr + 1, argvals,
							      arg_types, 7, 5, &nb_args_parsed);
			if (ret != SUCCESS) {
				cli_write_string(dev->cli_handler,
						 (uint8_t *) "FLUO DECAY ERROR: Invalid argument string.\n");
				goto rollback_return;
			}

		} else { // Preset parameters
			preset = 0;
			while (fluo_preset_tab[preset][0] != 0) {
				if (strncmp((char *) arg, (char *) fluo_preset_tab[preset], 13) == 0) {
					break;
				}
				preset++;
			}
			if (preset > 3) { // Invalid preset
				ret = FAILURE;
				cli_write_string(dev->cli_handler,
						 (uint8_t *) "FLUO DECAY ERROR: Invalid preset string.\n");
				goto rollback_return;
			}
			using_preset = true;
			cn0503_fluo_set_presets(impresp, preset);
			void *argvals[] = {&impresp->calib_slot, &impresp->chann_no, &acquisition_width,
					   &impresp->average_length
					  };
			uint8_t arg_types[] = {CALIB_SLOT, CHANN_NO, ACQUISITION_WIDTH, AVERAGE_LENGTH};
			ret = impulse_response_parse_arg_list(impresp, space_ptr + 1, argvals,
							      arg_types, 4, 3, &nb_args_parsed);
			if (ret != SUCCESS) {
				cli_write_string(dev->cli_handler,
						 (uint8_t *) "FLUO DECAY ERROR: Invalid argument string.\n");
				goto rollback_return;
			}
			// Convert acquisiton width to number of samples
			impresp->nb_samples = ceil(acquisition_width / impresp->sample_period) + 1;
		}
	} else {
		void *argvals[] = {&impresp->calib_slot, &impresp->chann_no, &acquisition_width,
				   &impresp->sample_period, &impresp->average_length,
				   &impresp->first_sample_offset
				  };
		uint8_t arg_types[] = {CALIB_SLOT, CHANN_NO, ACQUISITION_WIDTH, SAMPLE_PERIOD,
				       AVERAGE_LENGTH, FIRST_SAMPLE_OFFSET
				      };
		ret = impulse_response_parse_arg_list(impresp, arg, argvals, arg_types, 6, 4,
						      &nb_args_parsed);
		if (ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *) "FLUO DECAY ERROR: Invalid argument string.\n");
			goto rollback_return;
		}
	}
	// Convert acquisiton width to number of samples
	if (acquisition_width < 0)
		acquisition_width = 0;
	impresp->nb_samples = ceil(acquisition_width / impresp->sample_period) + 1;
	impresp->chann_no--;
	if (!impresp->skip_calib) {
		if (impresp->calib_slot != 1 && impresp->calib_slot != 2) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)
					 "FLUO DECAY ERROR: Invalid calibration slot. Must be 1 or 2.\n");
			goto rollback_return;
		}
		impresp->calib_slot--;
		// Read from flash
		flash_idx = 0;
		flash_addr = dev->fluo_calib_flash_page_addr -
			     impresp->calib_slot * 0x800 * CN0503_FLASH_FLUO_MAX_PAGES;
		flash_read(dev->flash_handler, flash_addr, flash_data, 4);
		impresp->ref_chann_no = flash_data[flash_idx] & 0xFF;
		impresp->ref_led_width = (flash_data[flash_idx] >> 8) & 0xFF;
		impresp->ref_start_time = (flash_data[flash_idx] >> 16) & 0xFF;
		impresp->ref_method = flash_data[flash_idx++] >> 24;
		memcpy((void *) &impresp->ref_end_time, (void *) (flash_data + flash_idx++), 4);
		memcpy((void *) &impresp->ref_sample_period,
		       (void *) (flash_data + flash_idx), 4);

		impresp->led_width = impresp->ref_led_width;
		if ((!using_preset && nb_args_parsed < 5) || !using_preset)
			impresp->first_sample_offset = impresp->ref_start_time;
		if (!using_preset)
			impresp->method = impresp->ref_method;

		if (flash_data[flash_idx] == 0xFFFFFFFF) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)
					 "FLUO DECAY ERROR: No calibration data found. Run FLUO_CALIB to calibrate.\n");
			goto rollback_return;
		}
		if (impresp->ref_chann_no != impresp->chann_no) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)
					 "FLUO DECAY ERROR: Calibration performed different channel than decay measurement. "
					 "Run FLUO_CALIB to calibrate.\n");
			goto rollback_return;
		}
		if (impresp->ref_method != impresp->method) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)
					 "FLUO DECAY ERROR: Calibration performed with different sampling method than decay measurement. "
					 "Run FLUO_CALIB to calibrate.\n");
			goto rollback_return;
		}
		end_time = (float) impresp->first_sample_offset + impresp->sample_period *
			   (impresp->nb_samples - 1);
		if (impresp->ref_start_time > impresp->first_sample_offset ||
		    impresp->ref_end_time < end_time) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)
					 "FLUO DECAY ERROR: Calibration region of support does not include full decay acquisition region. "
					 "Run FLUO_CALIB to calibrate.\n");
			goto rollback_return;
		}
	}
	ret = impulse_response_validate_parameters(dev);
	if (ret != SUCCESS)
		goto rollback_return;

	if (impresp->chann_no != 0 && impresp->chann_no != 3) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)
				 "FLUO DECAY ERROR: Can only use optical paths 1 or 4 for fluorescence decay.\n");
		goto rollback_return;
	}

	ret = impulse_response_setup(dev);
	if (ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *) "FLUO DECAY ERROR: Setup.\n");
		goto rollback_return;
	}

	timer_stop(impresp->timer);
	snprintf((char *)buff, 64, "FLUO DECAY SETUP TIME (us) %ld\n",
		 impresp->timer->load_value);
	cli_write_string(dev->cli_handler, buff);
	timer_counter_set(impresp->timer, 0);
	timer_start(impresp->timer);
	return impulse_response_start_collection(dev);

rollback_return:
	impulse_response_rollback(dev);
	return ret;
}

/**
 * @brief Dump fluorescence decay calibration data to the CLI.
 * @param [in] dev - The device structure
 * @param [in] arg - Which calibration slot to output (1 or 2)
 * @return SUCCESS if success, error code otherwise
 */
int32_t cn0503_fluo_dump_calib(struct cn0503_dev *dev, uint8_t *arg)
{
	uint32_t *flash_data, flash_addr;
	uint16_t flash_idx, nb_samples, i;
	uint8_t buff[64];
	int8_t start_time;
	float end_time, sample_period, ref_response_i;
	uint8_t calib_slot = atoi((char *) arg) - 1;

	flash_data = calloc(CN0503_FLASH_BUFF_SIZE, sizeof(uint32_t));
	if (flash_data == NULL)
		return -ENOMEM;

	flash_idx = 0;
	flash_addr = dev->fluo_calib_flash_page_addr -
		     calib_slot * 0x800 * CN0503_FLASH_FLUO_MAX_PAGES;
	flash_read(dev->flash_handler, flash_addr, flash_data, CN0503_FLASH_BUFF_SIZE);

	if (flash_data[flash_idx] == 0xFFFFFFFF) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *) "FLUO DUMP CALIB ERROR: NO CALIBRATION DATA FOUND\n");
		free(flash_data);
		return SUCCESS;
	}

	snprintf((char *) buff, 64, "FLUO DUMP CALIB CHANNEL %d\n",
		 (uint8_t) (flash_data[flash_idx] & 0xFF));
	cli_write_string(dev->cli_handler, buff);

	snprintf((char *)buff, 64, "FLUO DUMP CALIB LED WIDTH %d\n",
		 (uint8_t) ((flash_data[flash_idx] >> 8) & 0xFF));
	cli_write_string(dev->cli_handler, buff);

	start_time = flash_data[flash_idx] >> 16;
	snprintf((char *)buff, 64, "FLUO DUMP CALIB START TIME %d\n", start_time);
	cli_write_string(dev->cli_handler, buff);

	snprintf((char *)buff, 64, "FLUO DUMP CALIB METHOD %s\n",
		 impresp_method_tab[flash_data[flash_idx++] >> 24]);
	cli_write_string(dev->cli_handler, buff);

	memcpy((void *) &end_time, (void *) (flash_data + flash_idx++), 4);
	snprintf((char *)buff, 64, "FLUO DUMP CALIB END TIME %0.5f\n", end_time);
	cli_write_string(dev->cli_handler, buff);

	memcpy((void *) &sample_period, (void *) (flash_data + flash_idx++), 4);
	snprintf((char *)buff, 64, "FLUO DUMP CALIB SAMPLE PERIOD %0.5f",
		 sample_period);
	cli_write_string(dev->cli_handler, buff);

	nb_samples = (uint16_t) (end_time - start_time) / sample_period + 1;
	for (i = 0; i < nb_samples; i++) {
		if (flash_idx >= CN0503_FLASH_BUFF_SIZE) {
			flash_addr -= 0x800;
			flash_idx -= CN0503_FLASH_BUFF_SIZE;
			// Read in next flash page
			flash_read(dev->flash_handler, flash_addr, flash_data, CN0503_FLASH_BUFF_SIZE);
		}
		memcpy((void *) &ref_response_i, (void *) (flash_data + flash_idx++), 4);

		snprintf((char *)buff, 64, "%0.4f", ref_response_i);
		if (i % 10 == 0) {
			cli_write_string(dev->cli_handler, (uint8_t *)"\nFLUO DUMP CALIB DATA ");
		} else if (i != 0) {
			cli_write_string(dev->cli_handler, (uint8_t *)" ");
		}
		cli_write_string(dev->cli_handler, buff);
	}
	cli_write_string(dev->cli_handler, (uint8_t *)"\n");
	cli_write_string(dev->cli_handler, (uint8_t *)"FLUO DUMP CALIB DONE\n");

	free(flash_data);
	return SUCCESS;
}

/**
 * @brief Try to read a data sample for every channel.
 * @param [in] dev - The device structure.
 * @param [out] code_rdy - false if new data is not ready;
 *                         true if data is ready.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_update_code(struct cn0503_dev *dev, bool *code_rdy)
{
	int32_t ret;
	uint16_t byte_count;
	uint8_t no_data_samples = 0;
	int8_t i;

	*code_rdy = false;

	ret = adpd410x_get_fifo_bytecount(dev->adpd4100_handler, &byte_count);
	if(ret != SUCCESS)
		return FAILURE;
	for(i = 0; i < dev->active_slots; i++) {
		if(dev->two_channel_slots & (1 << i))
			no_data_samples += 2 * dev->data_sizes[i];
		else
			no_data_samples += dev->data_sizes[i];
	}
	if(byte_count < no_data_samples)
		return SUCCESS;

	*code_rdy = true;

	return adpd410x_get_data(dev->adpd4100_handler, dev->data_channel);
}

/**
 * @brief Calculate ARAT equation if new data is available.
 * @param [in] dev - The device structure.
 * @param [in] op_ptr - Pointer to the ASCII RPN equation.
 * @param [out] result - Result of the equation.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_calc_ratio(struct cn0503_dev *dev, uint8_t **op_ptr,
				 float *result)
{
	float a, b;
	int8_t i = 0, op_id;
	int32_t ret;

	while(cn0503_ascii_op[i] != 0) {
		if(**op_ptr == cn0503_ascii_op[i]) {
			op_id = i;
			break;
		}
		i++;
	}
	if(i == 4) {
		*result = 0;
		return SUCCESS;
	}

	/* Get second element */
	(*op_ptr)--;
	i = 0;
	while(cn0503_ascii_op[i] != 0) {
		if(**op_ptr == cn0503_ascii_op[i])
			break;
		i++;
	}
	if(i <= 3) {
		ret = cn0503_calc_ratio(dev, op_ptr, &b);
		if(ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR 1\n");
			return FAILURE;
		}
	} else {
		i = 0;
		(*op_ptr)--;
		while(cn0503_ascii_code_id[i][0] != 0) {
			if(!strncmp((char *)(*op_ptr),
				    (char *)cn0503_ascii_code_id[i], 2)) {
				b = (float)dev->data_channel[i];
				break;
			}
			i++;
		}
		if(i > CN0503_L2) {
			if (**op_ptr == '#') {
				b = strtol((char *)((*op_ptr) + 1), NULL, 10);
			} else {
				i = 0;
				while (i < 5) {
					(*op_ptr)--;
					if (**op_ptr == '#')
						break;
					i++;
				}
				if (i == 5) {
					cli_write_string(dev->cli_handler,
							 (uint8_t *)"ERROR 2\n");
					return FAILURE;
				}
				b = strtol((char *)((*op_ptr) + 1), NULL, 10);
			}
		}
	}

	/* Get first element */
	(*op_ptr)--;
	i = 0;
	while(cn0503_ascii_op[i] != 0) {
		if(**op_ptr == cn0503_ascii_op[i])
			break;
		i++;
	}
	if(i <= 3) {
		ret = cn0503_calc_ratio(dev, op_ptr, &a);
		if(ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR 3\n");
			return FAILURE;
		}
	} else {
		i = 0;
		(*op_ptr)--;
		while(cn0503_ascii_code_id[i][0] != 0) {
			if(!strncmp((char *)*op_ptr,
				    (char *)cn0503_ascii_code_id[i], 2)) {
				a = (float)dev->data_channel[i];
				break;
			}
			i++;
		}
		if(i > CN0503_L2) {
			if (**op_ptr == '#') {
				a = strtol((char *)((*op_ptr) + 1), NULL, 10);
			} else {
				i = 0;
				while (i < 5) {
					(*op_ptr)--;
					if (**op_ptr == '#')
						break;
					i++;
				}
				if (i == 5) {
					cli_write_string(dev->cli_handler,
							 (uint8_t *)"ERROR 2\n");
					return FAILURE;
				}
				a = strtol((char *)((*op_ptr) + 1), NULL, 10);
			}
		}
	}

	/* Calculate result */
	switch(op_id) {
	case CN0503_PLUS:
		*result = a + b;
		break;
	case CN0503_MINUS:
		*result = a - b;
		break;
	case CN0503_MULT:
		*result = a * b;
		break;
	case CN0503_DIV:
		if(b == 0) {
			*result = 0;
			return SUCCESS;
		}
		*result = a / b;
		break;
	default:
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR 6\n");
		return FAILURE;
	}

	return SUCCESS;
}

/**
 * @brief Update all present ARAT ratios.
 * @param [in] dev - The device structure.
 * @param [out] rdy - false if new data is not ready;
 *                    true if new data is ready.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_update_arat(struct cn0503_dev *dev, bool *rdy)
{
	int8_t i, j, k;
	int32_t ret;
	uint8_t *last_op_ptr;
	float flt_val;

	*rdy = false;

	for(i = 0; i < CN0503_RAT_NO; i++) {
		if(dev->arat[i][0] == 0) {
			dev->ch_data_filt[i][dev->data_channel_indx] = 0;
			continue;
		}

		last_op_ptr = (uint8_t *)strchr((char *)dev->arat[i], 0) - 1;
		ret = cn0503_calc_ratio(dev, &last_op_ptr,
					&dev->ch_data_filt[i][dev->data_channel_indx]);
		if(ret != SUCCESS)
			return FAILURE;
	}
	dev->data_channel_indx++;

	if(dev->data_channel_indx < CN0503_BLOCK_FILT_SIZE)
		return SUCCESS;

	for(i = 0; i < CN0503_RAT_NO; i++) {
		for(j = (dev->lpf_window[i] - 1); j >= 1; --j)
			dev->arat_flt_data[i][j] = dev->arat_flt_data[i][j - 1];
		dev->arat_flt_data[i][j] = 0;
		for(k = 0; k < CN0503_BLOCK_FILT_SIZE; k++)
			dev->arat_flt_data[i][j] += dev->ch_data_filt[i][k];
		dev->arat_flt_data[i][j] /= CN0503_BLOCK_FILT_SIZE;
		flt_val = 0;
		for(j = 0; j < dev->lpf_window[i]; j++)
			flt_val += dev->arat_flt_data[i][j];
		flt_val /= dev->lpf_window[i];
		dev->arat_data[i] = flt_val;
	}
	*rdy = true;
	dev->data_channel_indx = 0;

	return SUCCESS;
}

/**
 * @brief Update all RRAT ratios.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_update_rrat(struct cn0503_dev *dev)
{
	int8_t i;

	for(i = 0; i < CN0503_RAT_NO; i++) {
		if(dev->ratb[i] == 0)
			continue;
		if(dev->rrat_sube[i] == 1)
			dev->rrat_data[i] = 1 - (dev->arat_data[i] /
						 dev->ratb[i]);
		else if(dev->rrat_sube[i] == 0)
			dev->rrat_data[i] = dev->arat_data[i] / dev->ratb[i];
		else
			dev->rrat_data[i] = 1 - (dev->arat_data[i] /
						 dev->ratb[i]);
	}

	return SUCCESS;
}

/**
 * @brief Update all INS1 ratios.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_update_ins1(struct cn0503_dev *dev)
{
	int8_t i;

	for(i = 0; i < CN0503_RAT_NO; i++)
		dev->ins1_data[i] = dev->ins1_p0[i] +
				    dev->ins1_p1[i] * dev->rrat_data[i] +
				    dev->ins1_p2[i] * pow(dev->rrat_data[i], 2) +
				    dev->ins1_p3[i] * pow(dev->rrat_data[i], 3) +
				    dev->ins1_p4[i] * pow(dev->rrat_data[i], 4) +
				    dev->ins1_p5[i] * pow(dev->rrat_data[i], 5);

	return SUCCESS;
}

/**
 * @brief Update all INS2 ratios.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_update_ins2(struct cn0503_dev *dev)
{
	int8_t i;

	for (i = 0; i < CN0503_RAT_NO; i++) {
		dev->ins2_data[i] = dev->ins2_p0[i] +
				    dev->ins2_p1[i] * dev->ins1_data[i] +
				    dev->ins2_p2[i] * pow(dev->ins1_data[i], 2) +
				    dev->ins2_p3[i] * pow(dev->ins1_data[i], 3) +
				    dev->ins2_p4[i] * pow(dev->ins1_data[i], 4) +
				    dev->ins2_p5[i] * pow(dev->ins1_data[i], 5);

		if (dev->ins2_data[i] < dev->alrm_low[i])
			dev->alrm_state[i] |= 1;
		else if (dev->alrm_state[i] & 1)
			dev->alrm_state[i] &= ~1;

		if (dev->ins2_data[i] > dev->alrm_high[i])
			dev->alrm_state[i] |= 2;
		else if (dev->alrm_state[i] & 2)
			dev->alrm_state[i] &= ~2;
	}

	return SUCCESS;
}

/**
 * @brief Update data and code values wrapper. Signal if code or expressions are
 *        ready since they have different output rates.
 * @param [in] dev - The device structure.
 * @param [out] data_rdy - true if data is ready; false if not.
 * @param [out] code_rdy - true if code is ready; false if not.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_update_data(struct cn0503_dev *dev, bool *data_rdy,
				  bool *code_rdy)
{
	int32_t ret;

	ret = cn0503_update_code(dev, code_rdy);
	if(ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR data code update.\n");
		return FAILURE;
	}
	if(!(*code_rdy))
		return SUCCESS;

	ret = cn0503_update_arat(dev, data_rdy);
	if(ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR data ARAT update.\n");
		return FAILURE;
	}
	if(!(*data_rdy))
		return SUCCESS;

	ret = cn0503_update_rrat(dev);
	if(ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR data RRAT update.\n");
		return FAILURE;
	}

	ret = cn0503_update_ins1(dev);
	if(ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR data INS1 update.\n");
		return FAILURE;
	}

	ret = cn0503_update_ins2(dev);
	if (dev->mode == CN0503_CODE)
		*data_rdy = false;

	return ret;
}

/**
 * @brief Display the current code sample for every active ratio.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_show_line_code(struct cn0503_dev *dev)
{
	uint8_t buff[256];
	int8_t i = 0, j = 0;

	if(dev->mode == CN0503_CODE) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"DATI: ");
		do {
			if(dev->two_channel_slots & (1 << i)) {
				sprintf((char *)buff, "%lX %lX ",
					dev->data_channel[j],
					dev->data_channel[(j + 1)]);
				j += 2;
			} else {
				sprintf((char *)buff, "%lX ",
					dev->data_channel[j]);
				j++;
			}
			cli_write_string(dev->cli_handler,
					 buff);
			i++;
		} while(i < dev->active_slots);
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"\n");
	}

	return SUCCESS;
}

/**
 * @brief Display the current data or code sample for every active ratio.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_show_line(struct cn0503_dev *dev)
{
	uint8_t	buff[256];
	int8_t	j;
	float	*data;

	switch (dev->mode) {
	case CN0503_ARAT:
		data = dev->arat_data;
		break;
	case CN0503_RRAT:
		data = dev->rrat_data;
		break;
	case CN0503_INS1:
		data = dev->ins1_data;
		break;
	case CN0503_INS2:
		data = dev->ins2_data;
		break;
	default:
		return FAILURE;
	}

	cli_write_string(dev->cli_handler,
			 (uint8_t *)"DATF: ");
	for(j = 0; j < CN0503_RAT_NO; j++) {
		if(!(dev->ratmask & (1 << j)))
			continue;
		sprintf((char *)buff, "%.4E ", data[j]);
		cli_write_string(dev->cli_handler,
				 buff);
	}
	cli_write_string(dev->cli_handler,
			 (uint8_t *)"\n");

	return SUCCESS;
}

/**
 * @brief Application main process.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_process(struct cn0503_dev *dev)
{
	int32_t ret;
	bool data_rdy = false, code_rdy = false;
	uint32_t odr_thresh;

	if (dev->impulse_response != NULL) {
		// Wait until an ongoing impulse response measurement has completed
		// before proceeding
		if (dev->impulse_response->data_ready) {
			// New data has been written to the FIFO
			dev->impulse_response->data_ready = false;
			return impulse_response_read_data(dev);
		}
		return SUCCESS;
	}

	ret = cli_process(dev->cli_handler);
	if(ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR CLI process.\n");
		return FAILURE;
	}

	if (!dev->idle_state) {
		ret = cn0503_update_data(dev, &data_rdy, &code_rdy);
		if(ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR data update.\n");
			return FAILURE;
		}
	}

	if(data_rdy)
		dev->odr_indx++;

	odr_thresh = (float)(CN0503_CODE_ODR_DEFAULT / CN0503_BLOCK_FILT_SIZE) /
		     dev->odr;

	if((dev->stream_count != 0) && code_rdy) {
		ret = cn0503_show_line_code(dev);
		if(ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR data show.\n");
			return FAILURE;
		}
		if((dev->stream_cnt_en) && (dev->mode == CN0503_CODE))
			dev->stream_count--;
	}

	if((dev->stream_count != 0) && data_rdy &&
	    (dev->odr_indx >= odr_thresh)) {
		ret = cn0503_show_line(dev);
		if(ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR data show.\n");
			return FAILURE;
		}
		if(dev->stream_cnt_en)
			dev->stream_count--;
		dev->odr_indx = 0;
	}

	return SUCCESS;
}

/**
 * @brief Load CLI tables.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_cli_load_init(struct cn0503_dev *dev)
{
	dev->app_cmd_func_tab = (cmd_func *)calloc((CN0503_CLI_CMD_NO + 1),
				sizeof *dev->app_cmd_func_tab);
	if(!dev->app_cmd_func_tab)
		return FAILURE;
	dev->app_cmd_func_tab[0] = (cmd_func)cn0503_reg_read;
	dev->app_cmd_func_tab[1] = (cmd_func)cn0503_reg_write;
	dev->app_cmd_func_tab[2] = (cmd_func)cn0503_mode_get;
	dev->app_cmd_func_tab[3] = (cmd_func)cn0503_mode_set;
	dev->app_cmd_func_tab[4] = (cmd_func)cn0503_stream;
	dev->app_cmd_func_tab[5] = (cmd_func)cn0503_idle_get;
	dev->app_cmd_func_tab[6] = (cmd_func)cn0503_idle_set;
	dev->app_cmd_func_tab[7] = (cmd_func)cn0503_defparam_route;
	dev->app_cmd_func_tab[8] = (cmd_func)cn0503_help;
	dev->app_cmd_func_tab[9] = (cmd_func)cn0503_alarm_get;
	dev->app_cmd_func_tab[10] = (cmd_func)cn0503_odr_get;
	dev->app_cmd_func_tab[11] = (cmd_func)cn0503_odr_set;
	dev->app_cmd_func_tab[12] = (cmd_func)cn0503_ratmask_get;
	dev->app_cmd_func_tab[13] = (cmd_func)cn0503_ratmask_set;
	dev->app_cmd_func_tab[14] = (cmd_func)cn0503_reboot;
	dev->app_cmd_func_tab[15] = (cmd_func)cn0503_flash_swbuff_clear;
	dev->app_cmd_func_tab[16] = (cmd_func)cn0503_flash_swbuff_load;
	dev->app_cmd_func_tab[17] = (cmd_func)cn0503_flash_program;
	dev->app_cmd_func_tab[18] = (cmd_func)cn0503_flash_erase;
	dev->app_cmd_func_tab[19] = (cmd_func)cn0503_flash_apply;
	dev->app_cmd_func_tab[20] = (cmd_func)cn0503_flash_write;
	dev->app_cmd_func_tab[21] = (cmd_func)cn0503_flash_read;
	dev->app_cmd_func_tab[22] = (cmd_func)cn0503_help_flash;
	dev->app_cmd_func_tab[23] = (cmd_func)cn0503_led;
	dev->app_cmd_func_tab[24] = (cmd_func)cn0503_channel_preset;
	dev->app_cmd_func_tab[25] = (cmd_func)cn0503_flash_swbuf_dump;
	dev->app_cmd_func_tab[26] = (cmd_func)cn0503_impulse_response;
	dev->app_cmd_func_tab[27] = (cmd_func)cn0503_fluo_decay_calibrate;
	dev->app_cmd_func_tab[28] = (cmd_func)cn0503_fluo_decay_measure;
	dev->app_cmd_func_tab[29] = (cmd_func)cn0503_fluo_dump_calib;

	dev->app_cmd_calls = (uint8_t **)calloc((CN0503_CLI_CMD_NO + 1),
						sizeof *dev->app_cmd_calls);
	if(!dev->app_cmd_calls)
		goto error_func;
	dev->app_cmd_calls[0] = (uint8_t *)"reg? ";
	dev->app_cmd_calls[1] = (uint8_t *)"reg ";
	dev->app_cmd_calls[2] = (uint8_t *)"mode?";
	dev->app_cmd_calls[3] = (uint8_t *)"mode ";
	dev->app_cmd_calls[4] = (uint8_t *)"stream";
	dev->app_cmd_calls[5] = (uint8_t *)"idle?";
	dev->app_cmd_calls[6] = (uint8_t *)"idle ";
	dev->app_cmd_calls[7] = (uint8_t *)"def";
	dev->app_cmd_calls[8] = (uint8_t *)"help";
	dev->app_cmd_calls[9] = (uint8_t *)"alrm?";
	dev->app_cmd_calls[10] = (uint8_t *)"odr?";
	dev->app_cmd_calls[11] = (uint8_t *)"odr ";
	dev->app_cmd_calls[12] = (uint8_t *)"ratmask?";
	dev->app_cmd_calls[13] = (uint8_t *)"ratmask ";
	dev->app_cmd_calls[14] = (uint8_t *)"boot";
	dev->app_cmd_calls[15] = (uint8_t *)"fl_clearbuf";
	dev->app_cmd_calls[16] = (uint8_t *)"fl_load ";
	dev->app_cmd_calls[17] = (uint8_t *)"fl_program ";
	dev->app_cmd_calls[18] = (uint8_t *)"fl_erase ";
	dev->app_cmd_calls[19] = (uint8_t *)"fl_apply";
	dev->app_cmd_calls[20] = (uint8_t *)"fl_write ";
	dev->app_cmd_calls[21] = (uint8_t *)"fl_read ";
	dev->app_cmd_calls[22] = (uint8_t *)"fl_help";
	dev->app_cmd_calls[23] = (uint8_t *)"pcb-led";
	dev->app_cmd_calls[24] = (uint8_t *)"chann";
	dev->app_cmd_calls[25] = (uint8_t *)"fl_dump";
	dev->app_cmd_calls[26] = (uint8_t *)"impresp";
	dev->app_cmd_calls[27] = (uint8_t *)"fluo_calib";
	dev->app_cmd_calls[28] = (uint8_t *)"fluo_decay";
	dev->app_cmd_calls[29] = (uint8_t *)"fluo_dump_calib";

	dev->app_cmd_size = (uint8_t *)calloc((CN0503_CLI_CMD_NO + 1),
					      sizeof *dev->app_cmd_size);
	if(!dev->app_cmd_size)
		goto error_calls;
	dev->app_cmd_size[0] = 5;
	dev->app_cmd_size[1] = 4;
	dev->app_cmd_size[2] = 6;
	dev->app_cmd_size[3] = 5;
	dev->app_cmd_size[4] = 6;
	dev->app_cmd_size[5] = 6;
	dev->app_cmd_size[6] = 5;
	dev->app_cmd_size[7] = 3;
	dev->app_cmd_size[8] = 5;
	dev->app_cmd_size[9] = 6;
	dev->app_cmd_size[10] = 5;
	dev->app_cmd_size[11] = 4;
	dev->app_cmd_size[12] = 9;
	dev->app_cmd_size[13] = 8;
	dev->app_cmd_size[14] = 5;
	dev->app_cmd_size[15] = 12;
	dev->app_cmd_size[16] = 8;
	dev->app_cmd_size[17] = 11;
	dev->app_cmd_size[18] = 9;
	dev->app_cmd_size[19] = 9;
	dev->app_cmd_size[20] = 9;
	dev->app_cmd_size[21] = 8;
	dev->app_cmd_size[22] = 8;
	dev->app_cmd_size[23] = 7;
	dev->app_cmd_size[24] = 5;
	dev->app_cmd_size[25] = 8;
	dev->app_cmd_size[26] = 7;
	dev->app_cmd_size[27] = 10;
	dev->app_cmd_size[28] = 10;
	dev->app_cmd_size[29] = 15;

	return SUCCESS;

error_calls:
	free(dev->app_cmd_calls);
error_func:
	free(dev->app_cmd_func_tab);

	return FAILURE;
}

/**
 * @brief Free memory for the CLI tables.
 * @param [in] dev - The device structure.
 * @return void
 */
static void cn0503_cli_load_remove(struct cn0503_dev *dev)
{
	if(dev->app_cmd_func_tab)
		free(dev->app_cmd_func_tab);
	if(dev->app_cmd_calls)
		free(dev->app_cmd_calls);
	if(dev->app_cmd_size)
		free(dev->app_cmd_size);
}

/**
 * @brief Set values for the program initialization structure.
 * @param [out] init_param - Pointer to the initialization structure.
 * @return void
 */
void cn0503_get_config(struct cn0503_init_param *init_param)
{
	struct aducm_uart_init_param *aducm_uart_ini;

#ifdef ADPD4100_SUPPORT
	struct aducm_spi_init_param aducm_spi_init = {
		.continuous_mode = true,
		.dma = false,
		.half_duplex = false,
		.master_mode = MASTER,
		.spi_channel = SPI0
	};
#endif

	aducm_uart_ini = init_param->cli_param.uart_init.extra;
	aducm_uart_ini->parity = UART_NO_PARITY;
	aducm_uart_ini->stop_bits = UART_ONE_STOPBIT;
	aducm_uart_ini->word_length = UART_WORDLEN_8BITS;

#ifdef ADPD4100_SUPPORT
	init_param->adpd4100_param.dev_ops_init.spi_phy_init.extra = &aducm_spi_init;
	init_param->adpd4100_param.dev_ops_init.spi_phy_init.max_speed_hz = 1000000;
	init_param->adpd4100_param.dev_ops_init.spi_phy_init.chip_select = 1;
	init_param->adpd4100_param.dev_ops_init.spi_phy_init.mode = SPI_MODE_0;
	init_param->adpd4100_param.dev_type = ADPD4100;
#else
	init_param->adpd4100_param.dev_ops_init.i2c_phy_init.extra = NULL;
	init_param->adpd4100_param.dev_ops_init.i2c_phy_init.max_speed_hz = 400000;
	init_param->adpd4100_param.dev_ops_init.i2c_phy_init.slave_address = 0x24;
	init_param->adpd4100_param.dev_type = ADPD4101;
#endif

	init_param->adpd4100_param.clk_opt = ADPD410X_INTLFO_INTHFO;
	init_param->adpd4100_param.ext_lfo_freq = 0;
#ifdef ADPD4100_SUPPORT
	init_param->adpd4100_param.gpio0.number = 0x08;
#else
	init_param->adpd4100_param.gpio0.number = 0x0F;
#endif
	init_param->adpd4100_param.gpio0.extra = NULL;
	init_param->adpd4100_param.gpio1.number = 0x0D;
	init_param->adpd4100_param.gpio1.extra = NULL;
	init_param->adpd4100_param.gpio2.number = 0x09;
	init_param->adpd4100_param.gpio2.extra = NULL;
	init_param->adpd4100_param.gpio3.number = 0x21;
	init_param->adpd4100_param.gpio3.extra = NULL;
	init_param->cli_param.uart_init.baud_rate = BD_115200;
	init_param->cli_param.uart_init.device_id = 0;

	init_param->flash_param.id = 0;
	init_param->md_flash_page_addr = CN0503_FLASH_MD_ADDR;
	init_param->uu_flash_page_addr = CN0503_FLASH_UU_ADDR;
	init_param->fluo_calib_flash_page_addr = CN0503_FLASH_FLUO_CALIB_ADDR;

	init_param->irq_param.irq_ctrl_id = 0;
	init_param->irq_param.extra = NULL;
}

/**
 * @brief Set device to get timestamp for low frequency oscillator calibration.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_calibrate_lfo_set_ts(struct cn0503_dev *dev)
{
	int32_t ret;
	uint16_t reg_data;

	/** Enable clock calibration circuitry */
	if(dev->chip_id == 0x02c2) {
		ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
					&reg_data);
		if(ret != SUCCESS)
			return FAILURE;
		reg_data |= BITM_OSC1M_OSC_CLK_CAL_ENA;
		ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
					 reg_data);
	}

	/** Enable GPIO0 input */
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_GPIO_CFG,
				&reg_data);
	if(ret != SUCCESS)
		return FAILURE;
	reg_data |= (1 & BITM_GPIO_CFG_GPIO_PIN_CFG0);
	ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_GPIO_CFG,
				 reg_data);
	if(ret != SUCCESS)
		return FAILURE;

	/** Enable GPIO0 as time stamp input */
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_GPIO_EXT,
				&reg_data);
	if(ret != SUCCESS)
		return FAILURE;
	reg_data |= ((0 << BITP_GPIO_EXT_TIMESTAMP_GPIO) &
		     BITM_GPIO_EXT_TIMESTAMP_GPIO);
	return adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_GPIO_EXT,
				  reg_data);
}

/**
 * @brief Get time stamp for low frequency oscillator calibration.
 * @param [in] dev - The device structure.
 * @param [out] ts_val - Pointer to the timestamp value container.
 * @param [in] ts_gpio - Descriptor for the counter start/stop GPIO.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_calibrate_lfo_get_timestamp(struct cn0503_dev *dev,
		uint32_t *ts_val, struct gpio_desc *ts_gpio)
{
	int32_t ret;
	uint16_t reg_data;

	/** Start time stamp calibration */
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC32K,
				&reg_data);
	if(ret != SUCCESS)
		return FAILURE;
	reg_data |= BITM_OSC32K_CAPTURE_TIMESTAMP;
	ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_OSC32K,
				 reg_data);
	if(ret != SUCCESS)
		return FAILURE;

	/** Give first time stamp trigger */
	ret = gpio_set_value(ts_gpio, GPIO_HIGH);
	if(ret != SUCCESS)
		return FAILURE;
	mdelay(1);
	ret = gpio_set_value(ts_gpio, GPIO_LOW);
	if(ret != SUCCESS)
		return FAILURE;

	/** Start time stamp calibration */
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC32K,
				&reg_data);
	if(ret != SUCCESS)
		return FAILURE;
	reg_data |= BITM_OSC32K_CAPTURE_TIMESTAMP;
	ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_OSC32K,
				 reg_data);
	if(ret != SUCCESS)
		return FAILURE;

	mdelay(10);
	ret = gpio_set_value(ts_gpio, GPIO_HIGH);
	if(ret != SUCCESS)
		return FAILURE;
	mdelay(1);
	ret = gpio_set_value(ts_gpio, GPIO_LOW);
	if(ret != SUCCESS)
		return FAILURE;

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC32K,
				&reg_data);
	if(ret != SUCCESS)
		return FAILURE;
	if(reg_data & BITM_OSC32K_CAPTURE_TIMESTAMP)
		return FAILURE;

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_STAMP_H,
				&reg_data);
	if(ret != SUCCESS)
		return FAILURE;
	*ts_val = (reg_data << 16) & 0xFFFF0000;
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_STAMP_L,
				&reg_data);
	if(ret != SUCCESS)
		return FAILURE;
	*ts_val |= reg_data;

	return SUCCESS;
}

/**
 * @brief Do low frequency oscillator calibration with respect to an external
 *        reference.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_calibrate_lfo(struct cn0503_dev *dev)
{
	int32_t ret;
	uint32_t ts_val_current, ts_val_last = 0, ts_val;
	uint16_t reg_data, cal_value;
	int8_t rdy = 0;
	struct gpio_desc *ts_gpio;
	struct gpio_init_param ts_param;

	cli_write_string(dev->cli_handler,
			 (uint8_t *)"Starting clock calibration.\n");

	ret = cn0503_calibrate_lfo_set_ts(dev);
	if(ret != SUCCESS)
		return FAILURE;

	/** Setup platform GPIO for time stamp trigger */
	ts_param.number = 15;
	ts_param.extra = NULL;
	ret = gpio_get(&ts_gpio, &ts_param);
	if(ret != SUCCESS)
		return FAILURE;
	ret = gpio_direction_output(ts_gpio, GPIO_LOW);
	if(ret != SUCCESS)
		return FAILURE;

	/** Delay to correctly initialize GPIO circuitry in device. */
	mdelay(1);

	while (1) {
		ret = cn0503_calibrate_lfo_get_timestamp(dev, &ts_val_current,
				ts_gpio);
		if(ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR ts get.\n");
			return FAILURE;
		}

		if(ts_val_current < ts_val_last) {
			ts_val_last = 0;
			continue;
		}
		ts_val = ts_val_current - ts_val_last;
		ts_val_last = ts_val_current;

		ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
					&reg_data);
		if(ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR adjust get.\n");
			return FAILURE;
		}
		cal_value = reg_data & BITM_OSC1M_OSC_1M_FREQ_ADJ;
		if(ts_val < (10000 - (10000 * 0.005)))
			cal_value++;
		else if(ts_val > (10000 + (10000 * 0.005)))
			cal_value--;
		else
			rdy = 1;
		if(rdy == 1)
			break;
		if((cal_value == 0) || (cal_value == BITM_OSC1M_OSC_1M_FREQ_ADJ))
			break;
		reg_data &= ~BITM_OSC1M_OSC_1M_FREQ_ADJ;
		reg_data |= cal_value & BITM_OSC1M_OSC_1M_FREQ_ADJ;
		ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
					 reg_data);
		if(ret != SUCCESS) {
			cli_write_string(dev->cli_handler,
					 (uint8_t *)"ERROR adjust set.\n");
			return FAILURE;
		}
	};

	ret = gpio_remove(ts_gpio);
	if(ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR GPIO free.\n");
		return FAILURE;
	}

	if(rdy == 1)
		return SUCCESS;
	else
		return FAILURE;
}

/**
 * @brief Do high frequency oscillator calibration with respect to the low
 *        frequency oscillator.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_calibrate_hfo(struct cn0503_dev *dev)
{
	int32_t ret;
	uint16_t reg_data;

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC32M_CAL,
				&reg_data);
	if(ret != SUCCESS)
		return FAILURE;
	reg_data |= BITM_OSC32M_CAL_OSC_32M_CAL_START;

	do {
		ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC32M_CAL,
					&reg_data);
		if(ret != SUCCESS)
			return FAILURE;
	} while(reg_data & BITM_OSC32M_CAL_OSC_32M_CAL_START);

	/** Disable clock calibration circuitry */
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
				&reg_data);
	if(ret != SUCCESS)
		return FAILURE;
	reg_data &= ~BITM_OSC1M_OSC_CLK_CAL_ENA;
	ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
				 reg_data);

	cli_write_string(dev->cli_handler,
			 (uint8_t *)"Calibration done.\n");

	return SUCCESS;
}

/**
 * @brief Check and update the Manufacturer Defaults flash page.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0503_init_flash_md_check(struct cn0503_dev *dev)
{
	int32_t ret;
	uint16_t i, sw_buff_loc;
	const char *arat_exp[] = {
		"A1#2048-A2#2048-/",
		"B1#2048-B2#2048-/",
		"C1#2048-C2#2048-/",
		"D2#2048-D1#2048-/",
		"",
		"",
		"",
		""
	};
	const float odr = 1;
	const uint32_t mode = CN0503_ARAT;
	const uint32_t ratmask = 0xf;
	const uint32_t sube[] = {0, 1, 1, 0, 1, 1, 1, 1};
	const float def0_ins1_0 = -0.028, def0_ins1_1 = 2017.3, def0_ins2_0 = 0,
		    def0_ins2_1 = 1.2048;

	flash_read(dev->flash_handler, dev->md_flash_page_addr,
		   dev->sw_flash_buffer, 1);
	if ((dev->sw_flash_buffer[0] != 0) &&
	    (dev->sw_flash_buffer[0] != 0xFFFFFFFF))
		return SUCCESS;

	/** General configuration */
	dev->sw_flash_buffer[0]   = 0x00f8000;
	dev->sw_flash_buffer[1]   = 0x00f0006;
	dev->sw_flash_buffer[2]   = 0x0000048;
	dev->sw_flash_buffer[3]   = 0x001000f;
	dev->sw_flash_buffer[4]   = 0x00d4e20;
	dev->sw_flash_buffer[5]   = 0x00e0000;
	dev->sw_flash_buffer[6]   = 0x0100300;
	/** Optical path 1 */
	dev->sw_flash_buffer[7]   = 0x1020005;
	dev->sw_flash_buffer[8]   = 0x1057070;
	dev->sw_flash_buffer[9]   = 0x1060000;
	/** Optical path 2 */
	dev->sw_flash_buffer[10]   = 0x1220050;
	dev->sw_flash_buffer[11]   = 0x1250000;
	dev->sw_flash_buffer[12]   = 0x1260030;
	/** Optical path 3 */
	dev->sw_flash_buffer[13]   = 0x1420500;
	dev->sw_flash_buffer[14]   = 0x14580B0;
	dev->sw_flash_buffer[15]   = 0x1460000;
	/** Optical path 4 */
	dev->sw_flash_buffer[16]   = 0x1625000;
	dev->sw_flash_buffer[17]   = 0x1650000;
	dev->sw_flash_buffer[18]   = 0x16680B0;
	/** AFE Path */
	dev->sw_flash_buffer[19]   = 0x10140DA;
	dev->sw_flash_buffer[20]   = 0x12140DA;
	dev->sw_flash_buffer[21]   = 0x14140DA;
	dev->sw_flash_buffer[22]   = 0x16140DA;
	/** CH2 enable */
	dev->sw_flash_buffer[23]   = 0x1004000;
	dev->sw_flash_buffer[24]   = 0x1204000;
	dev->sw_flash_buffer[25]   = 0x1404000;
	dev->sw_flash_buffer[26]   = 0x1604000;
	/** Precondition PDs to TIA_VREF */
	dev->sw_flash_buffer[27]   = 0x1031000;
	dev->sw_flash_buffer[28]   = 0x1231000;
	dev->sw_flash_buffer[29]   = 0x1431000;
	dev->sw_flash_buffer[30]   = 0x1631000;
	/** AFE gain */
	dev->sw_flash_buffer[31]   = 0x1042A92;
	dev->sw_flash_buffer[32]   = 0x1242A92;
	dev->sw_flash_buffer[33]   = 0x1442A92;
	dev->sw_flash_buffer[34]   = 0x1642A92;
	/** 1 integrate for every 32 pulses */
	dev->sw_flash_buffer[35]   = 0x1070120;
	dev->sw_flash_buffer[36]   = 0x1270120;
	dev->sw_flash_buffer[37]   = 0x1470120;
	dev->sw_flash_buffer[38]   = 0x1670120;
	/** 2us LED pulse with 32us offset */
	dev->sw_flash_buffer[39]   = 0x1090220;
	dev->sw_flash_buffer[40]   = 0x1290220;
	dev->sw_flash_buffer[41]   = 0x1490220;
	dev->sw_flash_buffer[42]   = 0x1690220;
	/** 3us AFE width double sided */
	dev->sw_flash_buffer[43]   = 0x10A0003;
	dev->sw_flash_buffer[44]   = 0x12A0003;
	dev->sw_flash_buffer[45]   = 0x14A0003;
	dev->sw_flash_buffer[46]   = 0x16A0003;
	/** ~32us integrator offset to line up zero crossing of BPF */
	dev->sw_flash_buffer[47]   = 0x10B03FC;
	dev->sw_flash_buffer[48]   = 0x12B03FC;
	dev->sw_flash_buffer[49]   = 0x14B03FC;
	dev->sw_flash_buffer[50]   = 0x16B03FC;
	/** 4 pulse chop */
	dev->sw_flash_buffer[51]   = 0x10D00AA;
	dev->sw_flash_buffer[52]   = 0x12D00AA;
	dev->sw_flash_buffer[53]   = 0x14D00AA;
	dev->sw_flash_buffer[54]   = 0x16D00AA;
	/** 2048 digital offset */
	dev->sw_flash_buffer[55]   = 0x10f8000;
	dev->sw_flash_buffer[56]   = 0x12f8000;
	dev->sw_flash_buffer[57]   = 0x14f8000;
	dev->sw_flash_buffer[58]   = 0x16f8000;
	/** 4 byte wide data */
	dev->sw_flash_buffer[59]   = 0x1100004;
	dev->sw_flash_buffer[60]   = 0x1300004;
	dev->sw_flash_buffer[61]   = 0x1500004;
	dev->sw_flash_buffer[62]   = 0x1700004;

	for (i = 0; i < CN0503_RAT_NO; i++) {
		cn0503_flash_write_def_arat(dev, (uint8_t *)arat_exp[i], i);

		sw_buff_loc = CN0503_FLASH_REG_SIZE + i * 28 + 12;
		dev->sw_flash_buffer[sw_buff_loc] = sube[i];
	}

	sw_buff_loc = CN0503_FLASH_ODR_IDX;
	memcpy((void *)&(dev->sw_flash_buffer[sw_buff_loc]),
	       (void *)&odr,
	       4);

	sw_buff_loc = CN0503_FLASH_MODE_IDX;
	dev->sw_flash_buffer[sw_buff_loc] = mode;

	sw_buff_loc = CN0503_FLASH_RATM_IDX;
	dev->sw_flash_buffer[sw_buff_loc] = ratmask;

	sw_buff_loc = CN0503_FLASH_REG_SIZE + 16;
	memcpy((void *)(&dev->sw_flash_buffer[sw_buff_loc]),
	       (void *)&def0_ins1_0, 4);
	sw_buff_loc++;
	memcpy((void *)(&dev->sw_flash_buffer[sw_buff_loc]),
	       (void *)&def0_ins1_1, 4);
	sw_buff_loc = CN0503_FLASH_REG_SIZE + 22;
	memcpy((void *)(&dev->sw_flash_buffer[sw_buff_loc]),
	       (void *)&def0_ins2_0, 4);
	sw_buff_loc++;
	memcpy((void *)(&dev->sw_flash_buffer[sw_buff_loc]),
	       (void *)&def0_ins2_1, 4);

	ret = flash_write(dev->flash_handler, dev->md_flash_page_addr,
			  dev->sw_flash_buffer, CN0503_FLASH_BUFF_SIZE);
	if (ret != SUCCESS)
		return FAILURE;

	return SUCCESS;
}

/**
 * @brief Initial process of the application.
 * @param [out] device - Pointer to the application handler.
 * @param [in] init_param - Pointer to the application initialization structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_init(struct cn0503_dev **device,
		    struct cn0503_init_param *init_param)
{
	int32_t ret;
	struct cn0503_dev *dev;
	int8_t i, j;
	struct adpd410x_timeslot_init ts_init_tab[4];
	uint16_t data;
	struct callback_desc cli_cb;

	ret = pwr_setup();
	if(ret != SUCCESS)
		return FAILURE;

	dev = (struct cn0503_dev *)calloc(1, sizeof *dev);
	if(!dev)
		return FAILURE;

	dev->md_flash_page_addr = init_param->md_flash_page_addr;
	dev->uu_flash_page_addr = init_param->uu_flash_page_addr;
	dev->fluo_calib_flash_page_addr = init_param->fluo_calib_flash_page_addr;

	cn0503_handler_init(dev, true);

	ret = cn0503_cli_load_init(dev);
	if(ret != SUCCESS)
		goto error_cn;

	ret = cli_setup(&dev->cli_handler, &init_param->cli_param);
	if(ret != SUCCESS)
		goto error_cn_vector;
	cli_load_command_vector(dev->cli_handler, dev->app_cmd_func_tab);
	cli_load_command_calls(dev->cli_handler, dev->app_cmd_calls);
	cli_load_command_sizes(dev->cli_handler, dev->app_cmd_size);
	cli_load_descriptor_pointer(dev->cli_handler, dev);

	ret = irq_ctrl_init(&dev->irq_handler, &init_param->irq_param);
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

	ret = adpd410x_setup(&dev->adpd4100_handler,
			     &init_param->adpd4100_param);
	if(ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR device initialization.\n");
		goto error_irq;
	}

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_CHIP_ID,
				&dev->chip_id);
	if(ret != SUCCESS) {
		cli_write_string(dev->cli_handler,
				 (uint8_t *)"ERROR get chip ID.\n");
		goto error_irq;
	}

	ret = adpd410x_set_sampling_freq(dev->adpd4100_handler,
					 CN0503_CODE_ODR_DEFAULT);
	if(ret != SUCCESS)
		goto error_irq;

	ret = adpd410x_set_last_timeslot(dev->adpd4100_handler, ADPD410X_TS_D);
	if(ret != SUCCESS)
		goto error_irq;
	dev->active_slots = 4;
	dev->two_channel_slots = 0xf;

	ts_init_tab[ADPD410X_TS_A].ts_inputs.option = ADPD410X_INaCH1_INbCH2;
	ts_init_tab[ADPD410X_TS_A].ts_inputs.pair = ADPD410X_INP12;
	ts_init_tab[ADPD410X_TS_A].led1.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_A].led1.fields.let_current_select = 0x70;
	ts_init_tab[ADPD410X_TS_A].led2.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_A].led2.fields.let_current_select = 0x70;
	ts_init_tab[ADPD410X_TS_A].led3.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_A].led3.fields.let_current_select = 0;
	ts_init_tab[ADPD410X_TS_A].led4.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_A].led4.fields.let_current_select = 0;

	ts_init_tab[ADPD410X_TS_B].ts_inputs.option = ADPD410X_INaCH1_INbCH2;
	ts_init_tab[ADPD410X_TS_B].ts_inputs.pair = ADPD410X_INP34;
	ts_init_tab[ADPD410X_TS_B].led1.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_B].led1.fields.let_current_select = 0;
	ts_init_tab[ADPD410X_TS_B].led2.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_B].led2.fields.let_current_select = 0;
	ts_init_tab[ADPD410X_TS_B].led3.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_B].led3.fields.let_current_select = 0x30;
	ts_init_tab[ADPD410X_TS_B].led4.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_B].led4.fields.let_current_select = 0x00;

	ts_init_tab[ADPD410X_TS_C].ts_inputs.option = ADPD410X_INaCH1_INbCH2;
	ts_init_tab[ADPD410X_TS_C].ts_inputs.pair = ADPD410X_INP56;
	ts_init_tab[ADPD410X_TS_C].led1.fields.led_output_select = ADPD410X_OUTPUT_B;
	ts_init_tab[ADPD410X_TS_C].led1.fields.let_current_select = 0x30;
	ts_init_tab[ADPD410X_TS_C].led2.fields.led_output_select = ADPD410X_OUTPUT_B;
	ts_init_tab[ADPD410X_TS_C].led2.fields.let_current_select = 0x00;
	ts_init_tab[ADPD410X_TS_C].led3.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_C].led3.fields.let_current_select = 0;
	ts_init_tab[ADPD410X_TS_C].led4.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_C].led4.fields.let_current_select = 0;

	ts_init_tab[ADPD410X_TS_D].ts_inputs.option = ADPD410X_INaCH1_INbCH2;
	ts_init_tab[ADPD410X_TS_D].ts_inputs.pair = ADPD410X_INP78;
	ts_init_tab[ADPD410X_TS_D].led1.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_D].led1.fields.let_current_select = 0;
	ts_init_tab[ADPD410X_TS_D].led2.fields.led_output_select = ADPD410X_OUTPUT_A;
	ts_init_tab[ADPD410X_TS_D].led2.fields.let_current_select = 0;
	ts_init_tab[ADPD410X_TS_D].led3.fields.led_output_select = ADPD410X_OUTPUT_B;
	ts_init_tab[ADPD410X_TS_D].led3.fields.let_current_select = 0x30;
	ts_init_tab[ADPD410X_TS_D].led4.fields.led_output_select = ADPD410X_OUTPUT_B;
	ts_init_tab[ADPD410X_TS_D].led4.fields.let_current_select = 0x00;

	for (i = 0; i < 4; i++) {
		ts_init_tab[i].enable_ch2 = true;
		ts_init_tab[i].precon_option = ADPD410X_TIA_VREF;
		ts_init_tab[i].afe_trim_opt = ADPD410X_TIA_VREF_1V256;
		ts_init_tab[i].vref_pulse_opt = ADPD410X_TIA_VREF_1V256;
		ts_init_tab[i].chan2 = ADPD410X_TIA_VREF_50K;
		ts_init_tab[i].chan1 = ADPD410X_TIA_VREF_100K;
		ts_init_tab[i].pulse4_subtract = 0xA;
		ts_init_tab[i].pulse4_reverse = 0xA;
		ts_init_tab[i].byte_no = 3;
		ts_init_tab[i].dec_factor = 3;
		ts_init_tab[i].repeats_no = 32;
		ts_init_tab[i].adc_cycles = 1;
		ret = adpd410x_timeslot_setup(dev->adpd4100_handler, i,
					      ts_init_tab + i);
		if(ret != SUCCESS)
			goto error_irq;

		/** Precondition VC1 and VC2 to TIA_VREF+250mV */
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_CATHODE(i), &data);
		if(ret != SUCCESS)
			goto error_irq;
		data &= ~(BITM_CATHODE_A_VC2_SEL | BITM_CATHODE_A_VC1_SEL);
		data |= (2 << BITP_CATHODE_A_VC2_SEL) & BITM_CATHODE_A_VC2_SEL;
		data |= (2 << BITP_CATHODE_A_VC1_SEL) & BITM_CATHODE_A_VC1_SEL;
		ret = adpd410x_reg_write(dev->adpd4100_handler,
					 ADPD410X_REG_CATHODE(i), data);
		if(ret != SUCCESS)
			goto error_irq;

		/** Set the two channels trim option */
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_AFE_TRIM(i), &data);
		if(ret != SUCCESS)
			goto error_irq;
		data |= (1 << BITP_AFE_TRIM_A_CH1_TRIM_INT |
			 1 << BITP_AFE_TRIM_A_CH2_TRIM_INT);
		ret = adpd410x_reg_write(dev->adpd4100_handler,
					 ADPD410X_REG_AFE_TRIM(i), data);
		if(ret != SUCCESS)
			goto error_irq;

		/**
		 * Set to ~32us integrator offset to line up zero crossing of
		 * BPF
		 */
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_INTEG_OFFSET(i),
					&data);
		if(ret != SUCCESS)
			goto error_irq;
		data = 0x03FC & BITM_INTEG_OFFSET_A_INTEG_OFFSET;
		ret = adpd410x_reg_write(dev->adpd4100_handler,
					 ADPD410X_REG_INTEG_OFFSET(i), data);
		if(ret != SUCCESS)
			goto error_irq;

		/** Set to ~32us LED offset */
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_LED_PULSE(i), &data);
		if(ret != SUCCESS)
			goto error_irq;
		data = 0x0220;
		ret = adpd410x_reg_write(dev->adpd4100_handler,
					 ADPD410X_REG_LED_PULSE(i), data);
		if(ret != SUCCESS)
			goto error_irq;
	}

	ret = cn0503_calibrate_lfo(dev);
	if(ret != SUCCESS)
		goto error_irq;

	ret = cn0503_calibrate_hfo(dev);
	if(ret != SUCCESS)
		goto error_irq;

	for(i = 0; i < CN0503_RAT_NO; i++) {
		dev->arat_flt_data[i] = (float *)calloc(dev->lpf_window[i],
							sizeof (float));
		if(!dev->arat_flt_data[i])
			goto error_dlpf;
	}

	ret = flash_init(&dev->flash_handler, &init_param->flash_param);
	if(ret != SUCCESS)
		goto error_dlpf;

	ret = cn0503_init_flash_md_check(dev);
	if(ret != SUCCESS)
		goto error_dlpf;

	ret = cn0503_flash_swbuff_load(dev, (uint8_t *)"1");
	if(ret != SUCCESS)
		goto error_dlpf;
	ret = cn0503_flash_apply(dev, NULL);
	if(ret != SUCCESS)
		goto error_dlpf;
	ret = cn0503_flash_swbuff_load(dev, (uint8_t *)"0");
	if(ret != SUCCESS)
		goto error_dlpf;
	ret = cn0503_flash_apply(dev, NULL);
	if(ret != SUCCESS)
		goto error_dlpf;
	ret = cn0503_flash_swbuff_clear(dev, NULL);
	if(ret != SUCCESS)
		goto error_dlpf;

	*device = dev;

	return SUCCESS;

error_dlpf:
	for(j = 0; j < i; j++)
		free(dev->arat_flt_data[j]);
error_irq:
	irq_ctrl_remove(dev->irq_handler);
error_cli:
	cli_remove(dev->cli_handler);
error_cn_vector:
	cn0503_cli_load_remove(dev);
error_cn:
	free(dev);

	return FAILURE;
}

/**
 * @brief Free memory allocated by cn0503_init().
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0503_remove(struct cn0503_dev *dev)
{
	int32_t ret;
	int8_t i;

	if(!dev)
		return FAILURE;

	cn0503_cli_load_remove(dev);

	ret = cli_remove(dev->cli_handler);
	if(ret != SUCCESS)
		return FAILURE;

	ret = adpd410x_remove(dev->adpd4100_handler);
	if(ret != SUCCESS)
		return FAILURE;

	for(i = 0; i < CN0503_RAT_NO; i++)
		free(dev->arat_flt_data[i]);

	free(dev);

	return SUCCESS;
}
