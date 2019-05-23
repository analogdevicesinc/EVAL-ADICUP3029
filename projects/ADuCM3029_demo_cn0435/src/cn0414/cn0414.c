/***************************************************************************//**
*   @file   cn0414.c
*   @brief  CN0414 driver application source.
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
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "cn0414.h"
#if defined(CLI_INTEFACE)
#include "cli.h"
#elif defined(MODBUS_INTERFACE)
#include <mb_slave_data_link.h>
#endif

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
extern volatile uint8_t adc_channel_flag;
extern volatile uint8_t adc_sw_prescaler;
extern float vref;
extern bool modem_rec_flag;
extern uint32_t unique_id;
uint8_t low_sample_rate_flag = 0;

/* ADC channel registers to which the inputs are connected */
uint8_t channels_registers[] = {
	AD717X_CHMAP1_REG,  /* vin1 */
	AD717X_CHMAP5_REG,  /* vin2 */
	AD717X_CHMAP9_REG,  /* vin3 */
	AD717X_CHMAP13_REG, /* vin4 */
	AD717X_CHMAP0_REG,  /* iin1 */
	AD717X_CHMAP3_REG,  /* iin2 */
	AD717X_CHMAP4_REG,  /* iin3 */
	AD717X_CHMAP7_REG,  /* iin4 */
	AD717X_CHMAP2_REG,  /* vin1 second channel for Open-Wire Detection */
	AD717X_CHMAP6_REG,  /* vin2 second channel for Open-Wire Detection */
	AD717X_CHMAP10_REG, /* vin3 second channel for Open-Wire Detection */
	AD717X_CHMAP14_REG  /* vin4 second channel for Open-Wire Detection */
};

/* ADC channel names */
char *adc_chan_names[] = {
	"vin1",
	"vin2",
	"vin3",
	"vin4",
	"iin1",
	"iin2",
	"iin3",
	"iin4",
	"allc",
	""
};

/* HART channel names */
char *cn0414_hart_chan_names[] = {
	"ch1",
	"ch2",
	"ch3",
	"ch4",
	""
};

/* Output data rates options */
char *odr_options[] = {
	"31250_sps",
	"31250_sps",
	"31250_sps",
	"31250_sps",
	"31250_sps",
	"31250_sps",
	"15625_sps",
	"10417_sps",
	"5208_sps",
	"2597_sps",
	"1007_sps",
	"503.8_sps",
	"381_sps",
	"200.3_sps",
	"100.5_sps",
	"59.52_sps",
	"49.68_sps",
	"20.01_sps",
	"16.63_sps",
	"10_sps",
	"5_sps",
	"2.5_sps",
	"1.25_sps",
	""
};

/* Output data rates option sizes */
uint8_t odr_option_size[] = {
	9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 9, 7, 9, 9, 9, 9, 9, 9, 6, 5, 7, 8, 1
};

/* Value of the Output Data Rate of the ADC */
uint16_t odr_actual_rate_values[] = {
	31250, 31250, 31250, 31250, 31250, 31250, 15625, 10417, 5208, 2597,
	1007, 503, 381, 200, 100, 59, 49, 20, 16, 10, 5, 2, 1
};

/* Filter options */
char *fiter_options[] = {
	"s5+s1",
	"s3",
	""
};

/* Filter options sizes */
uint8_t fiter_options_size[] = {5, 2, 1};

/* Output data rates options */
char *output_coding_options[] = {
	"bipolar",
	"bi",
	"unipolar",
	"uni",
	""
};

/* Output data rates option sizes */
uint8_t output_coding_options_size[] = {8, 3, 9, 3, 1};

/* Postfilter options */
char *postfiter_options[] = {
	"opt1", /* 27 SPS, 47 dB rejection, 36.7 ms settling */
	"opt2", /* 25 SPS, 62 dB rejection, 40 ms settling */
	"opt3", /* 20 SPS, 86 dB rejection, 50 ms settling */
	"opt4", /* 16.67 SPS, 92 dB rejection, 60 ms settling */
	""
};
#if defined(CLI_INTEFACE)
/* Available CLI commands */
char *cn0414_cmd_commands[] = {
	"help",
	"h",
	"read ",
	"r ",
	"hart_enable",
	"he",
	"hart_disable",
	"hd",
	"hart_change_channel ",
	"hcc ",
	"hart_transmit ",
	"ht ",
	"hart_get_rec",
	"hg",
	"adc_read_reg ",
	"arr ",
	"adc_write_reg ",
	"awr ",
	"adc_get_samples ",
	"ags ",
	"adc_set_odr ",
	"aso ",
	"adc_set_filt ",
	"asf ",
	"set_update_rate ",
	"sur ",
	"status",
	"stts",
	"adc_set_postfilt ",
	"asp ",
	"adc_en_postfilt",
	"aep",
	"adc_dis_postfilt",
	"adp",
	"adc_set_out_coding ",
	"asoc ",
	"discover_eeprom",
	"de",
	"hart_command_zero ",
	"hcz ",
	"adc_open_wire_enable",
	"aowe",
	"adc_open_wire_disable",
	"aowd",
	"hart_phy_test ",
	"hpt ",
	""
};

/* Functions for available CLI commands */
cmd_func cn0414_v_cmd_fun[] = {
	(cmd_func)cn0414_help,
	(cmd_func)cn0414_channel_display,
	(cmd_func)cn0414_hart_enable,
	(cmd_func)cn0414_hart_disable,
	(cmd_func)cn0414_hart_change_channel,
	(cmd_func)cn0414_hart_transmit,
	(cmd_func)cn0414_hart_get_rec,
	(cmd_func)cn0414_adc_read_reg,
	(cmd_func)cn0414_adc_write_reg,
	(cmd_func)cn0414_adc_get_samples,
	(cmd_func)cn0414_adc_set_odr,
	(cmd_func)cn0414_adc_set_filter,
	(cmd_func)cn0414_channel_set_update_rate,
	(cmd_func)cn0414_status,
	(cmd_func)cn0414_adc_set_postfilt,
	(cmd_func)cn0414_adc_en_postfilt,
	(cmd_func)cn0414_adc_dis_postfilt,
	(cmd_func)cn0414_adc_set_output_coding,
	(cmd_func)cn0414_mem_display_addr,
	(cmd_func)cn0414_hart_send_command_zero,
	(cmd_func)cn0414_adc_open_wire_enable,
	(cmd_func)cn0414_adc_open_wire_disable,
	(cmd_func)cn0414_hart_phy_test,
	NULL
};

/* CLI command sizes */
uint8_t cn0414_command_size[] = {
	5, 2, 5, 2, 12, 3, 13, 3, 20, 4, 14, 3, 13, 3, 13, 4, 14, 4, 16, 4, 12,
	4, 13, 4, 16, 4, 7, 5, 17, 4, 16, 4, 17, 4, 19, 5, 15, 3, 18, 4, 21, 5,
	22, 5, 14, 4, 1
};
#endif
/* HART Command zero */
uint8_t cn0414_hart_command_zero[] = {0x02, 0x80, 0x00, 0x00, 0x82};

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
#if defined(CLI_INTEFACE)
/**
 * Check the ADC ID register and display error if the correct value is not
 * found.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_setup_adc_verify_id(struct cn0414_dev *dev)
{
	ad717x_st_reg *preg;
	int32_t ret;
	uint8_t buffer[20];

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_ID_REG);
	if(ret < 0)
		return ret;
	preg = AD717X_GetReg(dev->ad4111_device, AD717X_ID_REG);

	if((preg->value & AD717X_ID_REG_MASK) != AD4111_ID_REG_VALUE) {
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"Error. ADC not found.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"Expected ID: 0x30dx\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"Received ID: 0x");
		if(ret < 0)
			return ret;
		itoa(preg->value, (char *)buffer, 16);
		ret = usr_uart_write_string(dev->uart_descriptor, buffer);
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"\n");
		if(ret < 0)
			return ret;
	}

	return 0;
}
#endif
/**
 * Setup GPIOs for the CN0414.
 *
 * @param [in] dev        - Pointer to the device structure.
 * @param [in] init_param - Pointer to the structure that contains the device
 *                          initial parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_setup_gpio_setup(struct cn0414_dev *dev,
				       struct cn0414_ini_param *init_param)
{
	int32_t ret;

	ret = gpio_get(&dev->gpio_hart_chan0, init_param->gpio_hart_chan0);
	if(ret < 0)
		return ret;
	ret = gpio_get(&dev->gpio_hart_chan1, init_param->gpio_hart_chan1);
	if(ret < 0)
		return ret;
	ret = gpio_direction_output(dev->gpio_hart_chan0, GPIO_LOW);
	if(ret < 0)
		return ret;
	return gpio_direction_output(dev->gpio_hart_chan1, GPIO_LOW);
}

/**
 * Write protected memory with ID values.
 *
 * @param [in] dev - Pointer to the device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_setup_memory_content_setup(struct cn0414_dev *dev)
{
#ifdef WRITE_PROTECTED_MEMORY
	int32_t ret;

	return memory_write_within_page(dev->memory_device, 0x0000,
					(uint8_t *)"Coming soon...", 14);
#else
	return 0;
#endif
}

/**
 * Partially initializes the a CN0414 device.
 *
 * Partial initialization is used for making the driver compatible with multiple
 * devices on the same board node. The partial initialization initializes only
 * the drivers specific to a single instance of the driver and leaves open the
 * other components to be loaded later.
 *
 * @param [out] device    - The device structure.
 * @param [in] init_param - Pointer to the structure that contains the device
 *                          initial parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_setup_minimum(struct cn0414_dev **device,
			     struct cn0414_ini_param *init_param)
{
	int32_t ret;
	struct cn0414_dev *dev;
	uint32_t i;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	for(i = 0; i < ADC_CHANNEL_NO; i++)
		dev->channel_output[i] = 0;
	dev->channel_index = 0;
	for(i = 0; i < ADC_VOLTAGE_CHAN_NO; i++)
		dev->chan_voltage_status[i] = 0;
	for(i = 1; i < HART_BUFF_SIZE; i++)
		dev->hart_buffer[i] = 0x00;
	dev->hart_buffer[0] = 0;
	dev->hart_rec_size = 0;
	dev->open_wire_detect_enable = OPEN_WIRE_DETECT_DISABLED;
	dev->open_wire_first_done = 0;
#if defined(CLI_INTEFACE)
	ret = adc_update_setup(&dev->adc_update_desc, &init_param->adc_update_init);
	if(ret < 0)
		goto error;
#elif defined(MODBUS_INTERFACE)
	dev->adc_update_desc = NULL;
#endif
	ret = AD717X_Init(&dev->ad4111_device, init_param->ad4111_ini);
	if(ret < 0)
		goto error;

	ret = cn0414_setup_gpio_setup(dev, init_param);
	if(ret < 0)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Initializes the cn0414 device.
 *
 * @param [out] device    - The device structure.
 * @param [in] init_param - Pointer to the structure that contains the device
 *                          initial parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_setup(struct cn0414_dev **device,
		     struct cn0414_ini_param *init_param)
{
	int32_t ret;
	struct cn0414_dev *dev;
	uint32_t i;

	dev = calloc(1, sizeof *dev);
	if (!dev)
		return -1;

	for(i = 0; i < ADC_CHANNEL_NO; i++)
		dev->channel_output[i] = 0;
	dev->channel_index = 0;
	for(i = 0; i < ADC_VOLTAGE_CHAN_NO; i++)
		dev->chan_voltage_status[i] = 0;
	for(i = 1; i < HART_BUFF_SIZE; i++)
		dev->hart_buffer[i] = 0x00;
	dev->hart_buffer[0] = 0;
	dev->hart_rec_size = 0;
	dev->open_wire_detect_enable = OPEN_WIRE_DETECT_DISABLED;
	dev->open_wire_first_done = 0;

	ret = usr_uart_init(&dev->uart_descriptor, init_param->uart_ini);
	if(ret < 0)
		goto error;

	ret = ad5700_setup(&dev->ad5700_device, &init_param->ad5700_init);
	if(ret < 0)
		goto error;
	NVIC_DisableIRQ(HART_CD_INT);

	ret = AD717X_Init(&dev->ad4111_device, init_param->ad4111_ini);
	if(ret < 0)
		goto error;

	ret = cn0414_setup_adc_verify_id(dev);
	if(ret < 0)
		goto error;

	ret = adc_update_setup(&dev->adc_update_desc, &init_param->adc_update_init);
	if(ret < 0)
		goto error;

	ret = adc_update_activate(dev->adc_update_desc, true);
	if(ret < 0)
		goto error;

	ret = memory_setup(&dev->memory_device, &init_param->memory_init);
	if(ret < 0)
		goto error;

	ret = cn0414_setup_gpio_setup(dev, init_param);
	if(ret < 0)
		goto error;

	ret = cn0414_setup_memory_content_setup(dev);
	if(ret < 0)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free the resources allocated by cn0414_setup_minimum().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_remove_minimum(struct cn0414_dev *dev)
{
	int32_t ret;

	if (dev->ad4111_device != NULL) {
		ret = AD717X_remove(dev->ad4111_device);
		if(ret < 0)
			return ret;
	}
	if (dev->adc_update_desc != NULL) {
		ret = adc_update_remove(dev->adc_update_desc);
		if(ret < 0)
			return ret;
	}
	free(dev);

	return 0;
}

/**
 * Free the resources allocated by cn0414_setup().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_remove(struct cn0414_dev *dev)
{
	int32_t ret;

	if(!dev)
		return -1;

	ret = usr_uart_remove(dev->uart_descriptor);
	if(ret < 0)
		return ret;
	ret = ad5700_remove(dev->ad5700_device);
	if(ret < 0)
		return ret;
	ret = AD717X_remove(dev->ad4111_device);
	if(ret < 0)
		return ret;
	ret = adc_update_remove(dev->adc_update_desc);
	if(ret < 0)
		return ret;
	ret = memory_remove(dev->memory_device);
	if(ret < 0)
		return ret;
	free(dev);

	return ret;
}
#if defined(CLI_INTEFACE)
/**
 * Help command helper function. Display help function prompt.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_help_prompt(struct cn0414_dev *dev, bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "For commands with options as arguments typing the command and 'space' without arguments\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"will show the list of options.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"Available verbose commands:\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
	} else {
		ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"Available short commands:\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
	}
}

/**
 * Help command helper function. Display commands specific to the status and
 * usage of the application.
 *
 * @param [in] dev           - The device structure.
 * @param [in] short_command - True to display the long version of commands,
 *                             false to display the short version.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_help_general_commands(struct cn0414_dev *dev,
		bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" help                        - Display available commands\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)
					     " status                      - Display parameters of the application.\n");
	} else {
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" h               - Display available commands\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)" stts            - Display parameters of the application.\n");
	}
}

/**
 * Help command helper function. Display commands used to read channels and
 * update appropriate registers.
 *
 * @param [in] dev           - The device structure.
 * @param [in] short_command - True to display the long version of commands,
 *                             false to display the short version.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_help_read_commands(struct cn0414_dev *dev,
		bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " read <chan>                 - Display voltage or current on the selected channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                               <chan> = channel to be shown\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                               Example: read vin1 - for voltage channel 1.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                                        read iin4 - for voltage channel 4.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                                        read allc - diplay all channels.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" set_update_rate <rate>      - Change channel update rate.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                               <rate> = new channel update rate in Hz.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                               If it is bigger than output data rate divided by 80 can cause unpredictable behaviour.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)
					     "                               Example: set_update_rate 1.4 - individual channel update rate is set to 1.4 Hz.\n");
	} else {
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " r <chan>        - Display voltage or current on the selected channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <chan> = channel to be shown\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   Example: r vin1 - for voltage channel 1.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                            r iin4 - for voltage channel 4.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                            r allc - diplay all channels.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" sur <rate>      - Change channel update rate.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <rate> = new channel update rate in Hz.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   If it is bigger than output data rate divided by 80 can cause unpredictable behaviour.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)
					     "                   Example: set_update_rate 1.4 - individual channel update rate is set to 1.4 Hz.\n");
	}
}

/**
 * Help command helper function. Display commands specific to the AD5700 and
 * HART communication.
 *
 * @param [in] dev           - The device structure.
 * @param [in] short_command - True to display the long version of commands,
 *                             false to display the short version.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_help_hart_commands(struct cn0414_dev *dev,
		bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" hart_enable                - Enable HART channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" hart_disable               - Disable HART channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" hart_change_channel <chan> - Transmit string through HART.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                              <chan> = channel to be selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              Example: hart_change_channel ch1 - for HART channel 1.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" hart_transmit <string>     - Select wanted channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              <string> = string to be transmitted.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " hart_get_rec               - Send the received buffer through UART connection.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " hart_command_zero <pbsize> - Send command zero with the specified number of FFs in the preambule.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              <pbsize> = size of the preambule (no. of 0xFFs in the beginning).\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " hart_phy_test <byte>       - Send command zero with the specified number of FFs in the preambule.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"                              <byte> = byte to send in loop.\n");
	} else {
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" he              - Enable HART channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" hd              - Disable HART channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" hcc <chan>      - Select wanted channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <chan> = channel to be selected.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   Example: hart_change_channel ch1 - for HART channel 1.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" ht <string>     - Transmit string through HART.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <string> = string to be transmitted.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " hg              - Send the received buffer through UART connection.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " hcz <pbsize>    - Send command zero with the specified number of FFs in the preambule.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   <pbsize> = size of the preambule (no. of 0xFFs in the beginning).\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " hpt <byte>      - Send command zero with the specified number of FFs in the preambule.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"                   <byte> = byte to send in loop.\n");
	}
}

/**
 * Help command helper function. Display commands specific to the AD4111.
 *
 * @param [in] dev           - The device structure.
 * @param [in] short_command - True to display the long version of commands,
 *                             false to display the short version.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_help_adc_commands(struct cn0414_dev *dev,
					bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " adc_read_reg <reg>         - Display value of ADC register of the given address.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              <reg> = address of the register in hex.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              Example: adc_read_reg 20 - view value or register 0x20\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " adc_write_reg <reg> <val>  - Change value of the ADC register of the given address.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              <reg> = address of the register in hex.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              <val> = new value of the register in hex.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              Example: adc_write_reg 20 1320 - set register with address 0x20 to value 0x1320.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " adc_get_samples <ch> <nr>  - Get a specific number of samples from the given channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                              <ch> = selected channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              <nr> = number of channels; cannot exceed 2048.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" adc_set_odr <sps>          - Set sample rate.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              <sps> = selected sample rate option.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                               If it is smaller than channel update rate multiplied by 80 can cause unpredictable behaviour.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              Example: adc_set_odr 503.8_sps to set sample rate to 503.8 sps.\n");
		if(ret < 0)
			return ret;
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" adc_set_filt <filter>      - Set filter option.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                              <filter> = selected filter option.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              Example: adc_set_filt s5+s1 to set filter to sinc5 + sinc1\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" adc_en_postfilt            - Enable postfilter.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" adc_dis_postfilt           - Disable postfilter.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" adc_set_postfilt <opt>     - Disable postfilter.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              <opt> = selected postfilter option.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              Use command with space and no argument to get list of options and description.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " adc_set_out_coding <opt>   - Set ADC output to unipolar/bipolar.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                              <opt> = output coding option.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                              Use command with space and no argument to get list of options.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" adc_open_wire_enable       - Enable open wire detection.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)" adc_open_wire_disable      - Disable open wire detection.\n");
	} else {
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " arr <reg>       - Display value of ADC register of the given address.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <reg> = address of the register.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   Example: adc_read_reg 20 - view value or register 0x20\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " awr <reg> <val> - Change value of the ADC register of the given address.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <reg> = address of the register.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   Example: adc_write_reg 20 1320 - set register with address 0x20 to value 0x1320.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <val> = new value of the register.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    " ags <ch> <nr>   - Get a specific number of samples from the given channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <ch> = selected channel.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   <nr> = number of channels; cannot exceed 2048.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" aso <sps>       - Set sample rate.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <sps> = selected sample rate option.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   If it is smaller than channel update rate multiplied by 80 can cause unpredictable behaviour.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   Example: aso 503.8_sps to set sample rate to 503.8 sps.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" asf <filter>    - Set filter option.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <filter> = selected filter option.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   Example: asf s5+s1 to set filter to sinc5 + sinc1\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" aep             - Enable postfilter.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" adp             - Disable postfilter.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" asp <opt>       - Disable postfilter.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <opt> = selected postfilter option.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   Use command with space and no argument to get list of options and description.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" asoc <opt>      - Set ADC output to unipolar/bipolar.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"                   <opt> = output coding option.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)
					    "                   Use command with space and no argument to get list of options.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)" aowe            - Enable open wire detection.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)" awod            - Disable open wire detection.\n");
	}
}

/**
 * Help command helper function. Display commands specific to the 24LC32A and
 * I2C interfaced memory.
 *
 * @param [in] dev           - The device structure.
 * @param [in] short_command - True to display the long version of commands,
 *                             false to display the short version.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_help_memory_commands(struct cn0414_dev *dev,
		bool short_command)
{
	if (!short_command)
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)
					     " discover_eeprom            - Discover EEPROM I2C addresses if there are any.\n");
	else
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)
					     " de              - Discover EEPROM I2C addresses if there are any.\n");
}

/**
 * Display help options in the CLI.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_help(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret;

	ret = cn0414_help_prompt(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0414_help_general_commands(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0414_help_read_commands(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0414_help_hart_commands(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0414_help_adc_commands(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0414_help_memory_commands(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0414_help_prompt(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0414_help_general_commands(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0414_help_read_commands(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0414_help_hart_commands(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0414_help_adc_commands(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	return cn0414_help_memory_commands(dev, HELP_SHORT_COMMAND);
}

/**
 * Status command helper function. Display the update rate of the ADC registers.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_status_update_rate(struct cn0414_dev *dev)
{
	int32_t ret;
	uint8_t buff[20];

	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"Channel update rate: ");
	if(ret < 0)
		return ret;
	cn0414_ftoa(buff, dev->adc_update_desc->f_update);
	ret = usr_uart_write_string(dev->uart_descriptor, buff);
	if(ret < 0)
		return ret;

	return usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"Hz\n");
}

/**
 * Status command helper function. Display the output coding of the ADC.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_status_output_coding(struct cn0414_dev *dev)
{
	int32_t ret;
	ad717x_st_reg *temp_ptr;

	AD717X_ReadRegister(dev->ad4111_device, AD717X_SETUPCON0_REG);
	temp_ptr = AD717X_GetReg(dev->ad4111_device, AD717X_SETUPCON0_REG);
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"ADC output coding: ");
	if(ret < 0)
		return ret;

	if((temp_ptr->value & AD717X_SETUP_CONF_REG_BI_UNIPOLAR) == 0)
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"UNIPOLAR\n");
	else
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"BIPOLAR\n");
}

/**
 * Status command helper function. Display the output data rate of the ADC.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_status_odr(struct cn0414_dev *dev)
{
	int32_t ret;
	ad717x_st_reg *temp_ptr;

	AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	temp_ptr = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON0_REG);
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"ADC output data rate: ");
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t *)odr_options[temp_ptr->value &
								    AD717X_FILT_CONF_REG_ODR(0x1f)]);
	if(ret < 0)
		return ret;

	return usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
}

/**
 * Status command helper function. Display the filter and postfilter status of
 * the ADC.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_status_filter(struct cn0414_dev *dev)
{
	int32_t ret;
	ad717x_st_reg *temp_ptr;

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		return ret;
	temp_ptr = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON0_REG);
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"ADC filter configuration: ");
	if(ret < 0)
		return ret;

	if((temp_ptr->value & AD717X_FILT_CONF_REG_ORDER(3)) == 0)
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t *)fiter_options[0]);
	else
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t *)fiter_options[1]);
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
	if(ret < 0)
		return ret;

	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"ADC postfilter: ");
	if(ret < 0)
		return ret;
	if((temp_ptr->value & AD717X_FILT_CONF_REG_ENHFILTEN) == 0)
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"DEACTIVATED\n");
	else
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"ACTIVATED\n");
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"ADC postfilter configuration: ");
	if(ret < 0)
		return ret;
	switch((temp_ptr->value & AD717X_FILT_CONF_REG_ENHFILT(0x7)) >> 8) {
	case 2:
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"opt1 = 27 SPS, 47 dB rejection, 36.7 ms settling\n");
	case 3:
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"opt2 = 25 SPS, 62 dB rejection, 40 ms settling\n");
	case 5:
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"opt3 = 20 SPS, 86 dB rejection, 50 ms settling\n");
	case 6:
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"opt4 = 16.67 SPS, 92 dB rejection, 60 ms settling\n");
	default:
		return usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
	}
}

/**
 * Status command helper function. Display the status of the voltage inputs
 * (CONNECTED or FLOATING).
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_status_open_wire(struct cn0414_dev *dev)
{
	int32_t ret;
	uint8_t i;

	if (dev->open_wire_detect_enable == OPEN_WIRE_DETECT_ENABLED) {
		for(i = 0; i < ADC_VOLTAGE_CHAN_NO; i++) {
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t*)"Channel ");
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t *)adc_chan_names[i]);
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)": ");
			if(ret < 0)
				return ret;
			if(dev->chan_voltage_status[i] == 0) {
				ret = usr_uart_write_string(dev->uart_descriptor,
							    (uint8_t*)"CONNECTED\n");
			} else {
				ret = usr_uart_write_string(dev->uart_descriptor,
							    (uint8_t*)"FLOATING\n");
			}
			if(ret < 0)
				return ret;
		}
		return ret;
	} else {
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)
					     "To get information on voltage channel status enable open-wire detection.\n");
	}
}

/**
 * Display the status of the application.
 *
 * @param [in] dev  - The device structure.
 * @param [in] arg  - Not used in this case. It exists to keep the function
 *                    prototype compatible with the other functions that can be
 *                    called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_status(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret = 0;

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"Status of the application:\n");
	if(ret < 0)
		goto finish;
	ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
	if(ret < 0)
		goto finish;

	ret = cn0414_status_update_rate(dev);
	if(ret < 0)
		goto finish;

	ret = cn0414_status_output_coding(dev);
	if(ret < 0)
		goto finish;

	ret = cn0414_status_odr(dev);
	if(ret < 0)
		goto finish;

	ret = cn0414_status_filter(dev);
	if(ret < 0)
		goto finish;

	ret = cn0414_status_open_wire(dev);
	if(ret < 0)
		goto finish;

finish:
	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	return ret;
}

/**
 * Helper function for the cn0414_channel_display(). Used to display a single
 * channel.
 *
 * @param [in] dev           - The device structure.
 * @param [in] diplay_index  - Channel to be displayed.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_channel_display_helper(struct cn0414_dev *dev,
		uint8_t diplay_index)
{
	int32_t ret = 0;
	uint8_t buffer[20];
	float value;
	bool ncurrent_or_voltage;

	/* Display code of the channel */
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"ADC code for channel ");
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)adc_chan_names[diplay_index]);
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)": 0x");
	if(ret < 0)
		return ret;
	itoa(dev->channel_output[diplay_index], (char *)buffer, 16);
	ret = usr_uart_write_string(dev->uart_descriptor, buffer);
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n\n");
	if(ret < 0)
		return ret;

	if(diplay_index < 4)
		ncurrent_or_voltage = true;
	else
		ncurrent_or_voltage = false;

	/* Compute value of current or voltage */
	ret = cn0414_compute_adc_value(dev, dev->channel_output[diplay_index],
				       ncurrent_or_voltage, &value);
	if(ret < 0)
		return ret;

	/* Display computed value */
	if(diplay_index < 4)
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"Voltage on channel ");
	else
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"Current on channel ");
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)adc_chan_names[diplay_index]);
	if(ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)" is: ");
	if(ret < 0)
		return ret;
	cn0414_ftoa(buffer, value);
	ret = usr_uart_write_string(dev->uart_descriptor, buffer);
	if(ret < 0)
		return ret;
	if(diplay_index < 4) {
		ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"V\n");
		if(ret < 0)
			return ret;
		if(dev->chan_voltage_status[diplay_index] == 2 &&
		    dev->open_wire_detect_enable == OPEN_WIRE_DETECT_ENABLED) {
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t*)"This channel is FLOATING.\n");
			if(ret < 0)
				return ret;
		}
	} else {
		ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"A\n");
		if(ret < 0)
			return ret;
	}

	return usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
}

/**
 * Display the value of the selected channel.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - The selected channel.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_channel_display(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint8_t i = 0;

	/* Identify channel */
	while (adc_chan_names[i][0] != '\0') {
		if(strncmp((char *)arg, (char *)adc_chan_names[i], 5) == 0)
			break;
		i++;
	}

	/* If not a correct channel exit */
	if(i > 8) {
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"Valid channels are: \n");
		if(ret < 0)
			return ret;
		for(i = 0; i <= 8; i++) {
			ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)" - ");
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t*)adc_chan_names[i]);
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
			if(ret < 0)
				return ret;
		}
		return ret;
	}

	/* If all channels are selected */
	if(i == 8) {
		for(i = 0; i < 8; i++) {
			ret = cn0414_channel_display_helper(dev, i);
			if(ret < 0)
				return ret;
		}
		return ret;
	}

	return cn0414_channel_display_helper(dev, i);
}
#endif
/**
 * If the new channel update rate is bigger than the ADC output data rate give
 * warning message and adjust to a safe value.
 *
 * @param [in] dev          - The device structure.
 * @param [in/out] new_rate - Pointer to the new channel update rate. If it's
 *                            too big adjust it to be lower than the ADC ODR.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0414_channel_sur_check(struct cn0414_dev *dev, float *new_rate)
{
	ad717x_st_reg *temp_ptr;
	uint16_t odr_index;

	AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	temp_ptr = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON0_REG);
	odr_index = temp_ptr->value & AD717X_FILT_CONF_REG_ODR(0x1f);
	if(odr_actual_rate_values[odr_index] < (uint32_t)*new_rate) {
#if defined(CLI_INTEFACE)
		cn0414_status_odr(dev);
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)
				      "ADC output data rate must be bigger or at least equal to the channel update rate.\n");
#endif
		if(odr_index >= 13) {
			*new_rate = 100;
#if defined(CLI_INTEFACE)
			usr_uart_write_string(dev->uart_descriptor,
					      (uint8_t*)"Channel update rate is adjusted to 100Hz.\n");
#endif
		} else {
			*new_rate = 1;
#if defined(CLI_INTEFACE)
			usr_uart_write_string(dev->uart_descriptor,
					      (uint8_t*)"Channel update rate is adjusted to 1Hz.\n");
#endif
		}
	}
}

/**
 * Set update rate of the channel value registers. It is recommended that this
 * rate be at most 80 times lower than the output data rate of the ADC.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - The desired rate.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_channel_set_update_rate(struct cn0414_dev *dev, uint8_t* arg)
{
	float new_rate;

	/* Clear low sample rate warning flag to display a new warning if
	 * necessary */
	low_sample_rate_flag = 0;

	new_rate = atof((char *)arg);

	cn0414_channel_sur_check(dev, &new_rate);

	return adc_update_set_rate(dev->adc_update_desc, new_rate);
}

/**
 * Enable HART reception by enabling the demultiplexer that routes the AD5700 to
 * the circuit inputs. To enable these multipliers the AD4111 GPIO0 must be
 * driven high.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_hart_enable(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret;
	ad717x_st_reg *preg;

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	AD717X_ReadRegister(dev->ad4111_device, AD717X_GPIOCON_REG);
	preg = AD717X_GetReg(dev->ad4111_device, AD717X_GPIOCON_REG);
	preg->value |= AD4111_GPIOCON_REG_DATA0;
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_GPIOCON_REG);

	/* Avoid false interrupts */
	mdelay(5);
	modem_rec_flag = false;

	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	return ret;
}

/**
 * Disable HART reception by disabling the demultiplexer that routes the AD5700
 * to the circuit inputs. To enable these multipliers the AD4111 GPIO0 must be
 * driven low.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_hart_disable(struct cn0414_dev *dev, uint8_t* arg)
{
	ad717x_st_reg *preg;

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	AD717X_ReadRegister(dev->ad4111_device, AD717X_GPIOCON_REG);
	preg = AD717X_GetReg(dev->ad4111_device, AD717X_GPIOCON_REG);
	preg->value &= ~AD4111_GPIOCON_REG_DATA0;
	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	return AD717X_WriteRegister(dev->ad4111_device, AD717X_GPIOCON_REG);
}

/**
 * cn0414_hart_change_channel helper function.
 *
 * @param [in] dev     - The device structure.
 * @param [in] channel - ID of the channel.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_hart_change_chan_helper(struct cn0414_dev *dev,
				       enum hart_channels_cn0414 channel)
{
	int32_t ret;

	ret = gpio_set_value(dev->gpio_hart_chan0, (channel & 0x2) >> 1);
	if(ret < 0)
		return ret;

	return gpio_set_value(dev->gpio_hart_chan1, (channel & 0x1) >> 0);
}

/**
 * Change the active HART channel by selecting the corresponding demultiplexer
 * channel using the GPIOs from the controller.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - New desired channel.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_hart_change_channel(struct cn0414_dev *dev, uint8_t* arg)
{
#if defined(CLI_INTEFACE)
	int32_t ret;
#endif
	uint8_t i = 0;

	/* Identify channel */
	while (cn0414_hart_chan_names[i][0] != '\0') {
		if(strncmp((char *)arg, (char *)cn0414_hart_chan_names[i], 4) == 0)
			break;
		i++;
	}

	if(i >= 4) {
#if defined(CLI_INTEFACE)
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"Error. Incorrect channel. Available channels are:\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"    - ch1\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"    - ch2\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor,
					    (uint8_t*)"    - ch3\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"    - ch4\n");
#elif defined(MODBUS_INTERFACE)
		return 0;
#endif
	}

	return cn0414_hart_change_chan_helper(dev, i);
}

/**
 * Enable or disable HART modulator of the AD5700 chip on the board using the
 * attached AD4111 GPIO pins.
 *
 * @param [in] dev	  - The device structure.
 * @param [in] enable - True to enable modulator, false to disable modulator.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_hart_enable_modulator(struct cn0414_dev *dev, bool enable)
{
	ad717x_st_reg *preg;

	AD717X_ReadRegister(dev->ad4111_device, AD717X_GPIOCON_REG);
	preg = AD717X_GetReg(dev->ad4111_device, AD717X_GPIOCON_REG);

	if(enable) {
		preg->value &= ~AD4111_GPIOCON_REG_DATA0;
		preg->value |= AD4111_GPIOCON_REG_DATA1;
	} else {
		preg->value |= AD4111_GPIOCON_REG_DATA0;
		preg->value &= ~AD4111_GPIOCON_REG_DATA1;
	}

	return AD717X_WriteRegister(dev->ad4111_device, AD717X_GPIOCON_REG);
}

/**
 * Make a HART transmission. The HART output channels must be routed to the
 * boards output headers by enabling the output demultiplexer and disabling the
 * input demultiplexer.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - The string to be transmitted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_hart_transmit(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint32_t size;

	ret = cn0414_hart_enable_modulator(dev, true);
	if(ret < 0)
		return ret;

	size = strlen((char *)arg);

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	ret = ad5700_transmit(dev->ad5700_device, arg, size);
#if defined(CLI_INTEFACE)
	if(ret < 0)
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"Transmission failed!\n");
#endif
	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	return cn0414_hart_enable_modulator(dev, false);
}
#if defined(CLI_INTEFACE)
/**
 * Dump the HART receive buffer content on the serial CLI terminal.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_hart_get_rec(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint32_t i;
	uint8_t buff[20];

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	usr_uart_write_string(dev->uart_descriptor,
			      (uint8_t*)"Bytes received: ");
	itoa(dev->hart_rec_size - 1, (char *)buff, 10);
	ret = usr_uart_write_string(dev->uart_descriptor, buff);
	usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");

	for(i = 1; i < dev->hart_rec_size; i++) {
		itoa(dev->hart_buffer[i], (char *)buff, 16);
		ret = usr_uart_write_string(dev->uart_descriptor, buff);
		ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)" ");
		if(ret < 0) {
			goto finish;
		}
	}
	usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");

	dev->hart_buffer[0] = 0;

finish:
	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	return 0;
}
#endif
/**
 * Process function helper. Receive HART transmission on CD interrupt.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_process_hart_int_rec(struct cn0414_dev *dev)
{
	uint8_t i = 1;
	int32_t ret;
	uint16_t cd_val;

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	do {
		ret = swuart_read_char(dev->ad5700_device->swuart_desc,
				       &dev->hart_buffer[i]);
		if(ret < 0)
			return ret;
		gpio_get_value(dev->ad5700_device->gpio_cd, &cd_val);
		if(ret == 0)
			i++;
	} while((i < 256) && (cd_val != 0));
	dev->hart_rec_size = i;
#if defined(CLI_INTEFACE)
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"\nReceived HART transmission.\n");
	if(ret < 0)
		return ret;
#endif
	dev->hart_buffer[0] = 1;

	modem_rec_flag = false;

	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	return ret;
}

/**
 * Receive and parse the response of a command zero.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_hart_receive_and_parse(struct cn0414_dev *dev,
		uint8_t **message_ptr)
{
	int32_t ret;
	uint8_t i = 1;
	uint32_t timeout = 2000;

	dev->hart_buffer[0] = 0;

	while(!modem_rec_flag && (timeout != 0)) {
		timeout--;
		mdelay(1);
	}

	if(timeout == 0)
		return 0;

	ret = cn0414_process_hart_int_rec(dev);
	if(ret < 0)
		return ret;

	while(dev->hart_buffer[i] == HART_PREAMBLE_CHAR)
		i++;

	if(dev->hart_buffer[i] == HART_SHORT_ADDR_RESPONSE)
		i += 6;
	else if(dev->hart_buffer[i] == HART_LONG_ADDR_RESPONSE)
		i += 10;

	*message_ptr = &dev->hart_buffer[i];
	dev->hart_rec_size -= i;

	return ret;
}
#if defined(CLI_INTEFACE)
/**
 * Receive and parse the response of a command zero.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_hart_display_cmd_zero_response(struct cn0414_dev *dev,
		uint8_t *response)
{
	uint8_t buff[20];
	uint32_t long_address = 0;
	uint8_t id_lsb = 0;

	if(dev->hart_buffer[0] != 1)
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"No response.\n");

	usr_uart_write_string(dev->uart_descriptor,
			      (uint8_t*)"\nProtocol version: ");
	itoa(response[4], (char *)buff, 10);
	usr_uart_write_string(dev->uart_descriptor, buff);
	usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");

	usr_uart_write_string(dev->uart_descriptor,
			      (uint8_t*)"Preamble size: ");
	itoa(response[3], (char *)buff, 10);
	usr_uart_write_string(dev->uart_descriptor, buff);
	usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");

	usr_uart_write_string(dev->uart_descriptor,
			      (uint8_t*)"Manufacturer ID code: 0x");
	itoa(response[1], (char *)buff, 16);
	if(strlen((char *)buff) < 2)
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"0");
	usr_uart_write_string(dev->uart_descriptor, buff);
	usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
	id_lsb = response[1] & 0x3F;

	usr_uart_write_string(dev->uart_descriptor,
			      (uint8_t*)"Manufacturer device type: ");
	itoa(response[2], (char *)buff, 10);
	usr_uart_write_string(dev->uart_descriptor, buff);
	usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");

	long_address = id_lsb;
	usr_uart_write_string(dev->uart_descriptor,
			      (uint8_t*)"Long address: 0x");
	itoa(long_address, (char *)buff, 16);
	if(strlen((char *)buff) < 2)
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"0");
	usr_uart_write_string(dev->uart_descriptor, buff);
	long_address = 0;
	long_address |= (response[1] << (3 * 8));
	long_address |= (response[9] << (2 * 8));
	long_address |= (response[10] << (1 * 8));
	long_address |= (response[11] << (0 * 8));
	itoa(long_address, (char *)buff, 16);
	if(strlen((char *)buff) < 8)
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"0");
	usr_uart_write_string(dev->uart_descriptor, buff);
	usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");

	return 0;
}
#endif
/**
 * Send HART command zero.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Size of the preamble.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_hart_send_command_zero(struct cn0414_dev *dev, uint8_t* arg)
{
	uint8_t preamb_size;
	uint8_t *telegram;
	int32_t ret;
	uint8_t i;
	uint8_t size;

	preamb_size = atoi((char *)arg);

	if((preamb_size < 3) || (preamb_size > 20)) {
#if defined(CLI_INTEFACE)
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"Preambule must be within 3 and 20 bytes.\n");
#elif defined(MODBUS_INTERFACE)
		return 0;
#endif
	}

	size = preamb_size + HART_COMMAND_ZERO_SIZE;

	ret = cn0414_hart_enable_modulator(dev, true);
	if(ret < 0)
		return ret;

	telegram = calloc(size, sizeof *telegram);
	if (!telegram)
		return -1;

	for(i = 0; i < preamb_size; i++)
		telegram[i] = 0xFF;
	for(i = 0; i < HART_COMMAND_ZERO_SIZE; i++)
		telegram[i + preamb_size] = cn0414_hart_command_zero[i];

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	/* Transmit without the "\n" character */
	ret = ad5700_transmit(dev->ad5700_device, telegram, size);
	if(ret < 0)
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"Transmission failed!\n");
#elif defined(MODBUS_INTERFACE)
		return ret;
#endif
	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	free(telegram);

	ret = cn0414_hart_enable_modulator(dev, false);
	if(ret < 0)
		return ret;

	ret = cn0414_hart_receive_and_parse(dev, &telegram);
	if(ret < 0)
		return ret;
#if defined(CLI_INTEFACE)
	ret = cn0414_hart_display_cmd_zero_response(dev, telegram);
	if(ret < 0)
		return ret;

	dev->hart_buffer[0] = 0;
#endif

	return ret;
}
#if defined(CLI_INTEFACE)
/**
 * This method sends the provided test byte through the HART link continuously
 * until stopped by pressing q. Function used to test the HART physical layer.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_hart_phy_test(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint8_t c = 0, rdy;
	uint8_t byte;
	struct gpio_desc *tx_pin, *nrts_pin;

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	ret = gpio_get(&tx_pin, 0x09);
	ret = gpio_get(&nrts_pin, 0x1B);

	gpio_direction_output(nrts_pin, GPIO_LOW);
	gpio_direction_output(tx_pin, GPIO_HIGH);

	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"Press '0' or '1' to change frequency.\nPress 'q' to exit.\n");
	if(ret < 0)
		goto finish;

	byte = strtol((const char *)arg, NULL, 16);

	if(byte > 0xFF)
		goto finish;

	ret = cn0414_hart_enable_modulator(dev, true);
	if(ret < 0)
		goto finish;

	do {
		if(c == '1')
			gpio_set_value(tx_pin, GPIO_HIGH);
		if(c == '0')
			gpio_set_value(tx_pin, GPIO_LOW);
		usr_uart_read_nb(dev->uart_descriptor, &c, 1, &rdy);
	} while (c != 'q');

	gpio_set_value(nrts_pin, GPIO_HIGH);
	gpio_set_value(tx_pin, GPIO_HIGH);

	ret = cn0414_hart_enable_modulator(dev, false);
	if(ret < 0)
		goto finish;
	ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");

finish:
	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);
	gpio_remove(tx_pin);
	gpio_remove(nrts_pin);
	return ret;
}

/**
 * Read a register of the ADC.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Register address.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_read_reg(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint8_t address;
	ad717x_st_reg *p_reg;
	uint8_t buff[20];

	address = strtol((const char *)arg, NULL, 16);
	ret = AD717X_ReadRegister(dev->ad4111_device, address);
	if(ret < 0)
		return ret;

	ret = AD717X_ReadRegister(dev->ad4111_device, address);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, address);
	if (!p_reg)
		return ret;
	itoa(p_reg->value, (char *)buff, 16);
	ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
	if (ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"Value of register 0x");
	if (ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor, arg);
	if (ret < 0)
		return ret;
	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)" is: 0x");
	if (ret < 0)
		return ret;

	return usr_uart_write_string(dev->uart_descriptor, buff);
}

/**
 * Write a register of the ADC.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Register address and new register value separated by space.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_write_reg(struct cn0414_dev *dev, uint8_t* arg)
{
	uint8_t *reg_addr, *reg_data;
	ad717x_st_reg *p_reg;
	uint8_t address;
	int32_t value;

	/* Separate arguments in the argument list */
	reg_data = (uint8_t*)strchr((char*)arg, ' ') + 1;
	reg_addr = arg;
	reg_addr[strcspn((char*)reg_addr, " ")] = '\0';

	address = strtol((const char *)reg_addr, NULL, 16);
	value = strtol((char *)reg_data, NULL, 16);

	AD717X_ReadRegister(dev->ad4111_device, address);
	p_reg = AD717X_GetReg(dev->ad4111_device, address);
	if (!p_reg)
		return -1;
	p_reg->value = value;

	return AD717X_WriteRegister(dev->ad4111_device, address);
}
#endif
/**
 * Separate data from status in case they are read together.
 *
 * @param [in] dev	 	    - The device structure.
 * @param [in] data_status  - Data and status information.
 * @param [out] data_only   - Data separated from the status.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_filter_status(struct cn0414_dev *dev,
				    uint32_t data_status, uint32_t *data_only)
{
	ad717x_st_reg *reg;

	AD717X_ReadRegister(dev->ad4111_device, AD717X_IFMODE_REG);
	reg = AD717X_GetReg(dev->ad4111_device, AD717X_IFMODE_REG);
	if(!reg)
		return -1;

	*data_only = data_status >> 8;
	if(reg->value & AD717X_IFMODE_REG_DATA_WL16)
		*data_only &= 0xffff;
	else
		*data_only &= 0xffffff;

	return 0;
}

/**
 * Activate ADC channel to start conversion. If the input is a voltage input and
 * open wire detection is enabled activate the second channel.
 *
 * @param [in] dev	   - The device structure.
 * @param [in] channel - Number of the channel needed to be enabled.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_adc_activate_channel(struct cn0414_dev *dev,
		uint8_t channel, bool activate)
{
	ad717x_st_reg *reg;
	int32_t ret;

	AD717X_ReadRegister(dev->ad4111_device, channels_registers[channel]);
	reg = AD717X_GetReg(dev->ad4111_device, channels_registers[channel]);

	if(activate)
		reg->value |= AD717X_CHMAP_REG_CH_EN;
	else
		reg->value &= ~AD717X_CHMAP_REG_CH_EN;

	ret = AD717X_WriteRegister(dev->ad4111_device,
				   channels_registers[channel]);
	if(ret < 0)
		return ret;

	if((dev->open_wire_detect_enable == OPEN_WIRE_DETECT_ENABLED) &&
	    (channel < IIN1)) {
		/* Get corresponding register */
		AD717X_ReadRegister(dev->ad4111_device,
				    channels_registers[ADC_CHANNEL_NO + channel]);
		reg = AD717X_GetReg(dev->ad4111_device,
				    channels_registers[ADC_CHANNEL_NO + channel]);
		if(ret < 0)
			return ret;

		if(activate)
			reg->value |= AD717X_CHMAP_REG_CH_EN;
		else
			reg->value &= ~AD717X_CHMAP_REG_CH_EN;

		return AD717X_WriteRegister(dev->ad4111_device,
					    channels_registers[ADC_CHANNEL_NO + channel]);
	}

	return ret;
}

/**
 * Get a number of samples from the selected channel.
 *
 * Get an initial sample and if the open wire detection(OWD) is enabled and it
 * is a voltage input, get a sample from the second associated channel. If OWD
 * is enabled, but the input is current, get only one sample, but separate
 * status information from data. If OWD is disabled only the initial sample is
 * kept.
 *
 * @param [in] dev	 	    - The device structure.
 * @param [in] channel      - Channel to be read.
 * @param [in] sample_no    - Number of samples to retrieve from the channel.
 * @param [out] sample_buff - Sample buffer; output buffer.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_adc_get_samples_helper(struct cn0414_dev *dev,
		uint8_t channel, uint32_t sample_no, uint32_t *sample_buff)
{
	int32_t ret;
	uint32_t data;
	uint8_t floating;

	ret = AD717X_WaitForReady(dev->ad4111_device, 2);
	if(ret < 0)
		return 1;

	if((dev->open_wire_detect_enable == OPEN_WIRE_DETECT_ENABLED) &&
	    (channel < IIN1)) {
		if(dev->open_wire_first_done == 0) {
			AD717X_ReadData(dev->ad4111_device, (int32_t*)&data);
			dev->open_wire_temp_buffer = data;
			dev->open_wire_first_done = 1;

			return 1;
		} else if(dev->open_wire_first_done == 1) {
			AD717X_ReadData(dev->ad4111_device, (int32_t*)&data);
			if(AD717X_STATUS_REG_CH(dev->open_wire_temp_buffer) !=
			    AD717X_STATUS_REG_CH(data)) {
				cn0414_filter_status(dev, dev->open_wire_temp_buffer,
						     &dev->open_wire_temp_buffer);
				cn0414_filter_status(dev, data, &data);
				cn0414_open_wire_detect(dev, dev->open_wire_temp_buffer, data,
							&floating);
				if((floating == 1) && (dev->chan_voltage_status[channel] == 0))
					dev->chan_voltage_status[channel] = 1;
#if defined(CLI_INTEFACE)
				if((floating == 0) && (dev->chan_voltage_status[channel] == 2))
					dev->chan_voltage_status[channel] = 0;
#elif defined(MODBUS_INTERFACE)
				if((floating == 0) && (dev->chan_voltage_status[channel] == 1))
					dev->chan_voltage_status[channel] = 0;
#endif
				*sample_buff = (dev->open_wire_temp_buffer + data) / 2;
				dev->open_wire_first_done = 0;
			} else {
				dev->open_wire_first_done = 0;

				return 1;
			}
		}
	} else if((dev->open_wire_detect_enable == OPEN_WIRE_DETECT_ENABLED) &&
		  (channel >= IIN1)) {
		AD717X_ReadData(dev->ad4111_device, (int32_t*)&data);
		cn0414_filter_status(dev, data, &data);
		*sample_buff = data;
	} else {
		AD717X_ReadData(dev->ad4111_device, (int32_t*)&data);
		*sample_buff = data;
	}

	return 0;
}
#if defined(CLI_INTEFACE)
/**
 * Dump the samples in the serial terminal CLI in code value and actual volts or
 * amperes values separated by a comma to be compatible with the .csv format.
 *
 * @param [in] dev	 	   - The device structure.
 * @param [in] channel     - Channel to be read.
 * @param [in] sample_no   - Number of samples to retrieve from the channel.
 * @param [in] sample_buff - Sample buffer to be transmitted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_adc_display_samples(struct cn0414_dev *dev,
		uint8_t channel, uint32_t sample_no, uint32_t *sample_buff)
{
	uint32_t i;
	uint8_t buff[20];
	int32_t ret;
	float calc_sample;

	for (i = 0 ; i < sample_no; i++) {
		itoa(sample_buff[i], (char *)buff, 10);
		ret = usr_uart_write_string(dev->uart_descriptor, buff);
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)", ");
		if(ret < 0)
			return ret;
		if(channel < 4)
			ret = cn0414_compute_adc_value(dev, sample_buff[i], true,
						       &calc_sample);
		else
			ret = cn0414_compute_adc_value(dev, sample_buff[i], false,
						       &calc_sample);
		if(ret < 0)
			return ret;
		cn0414_ftoa(buff, calc_sample);
		ret = usr_uart_write_string(dev->uart_descriptor, buff);
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
		if(ret < 0)
			return ret;
	}

	return 0;
}

/**
 * Dump the samples in the serial terminal CLI in code value and actual volts or
 * amperes values separated by a comma to be compatible with the .csv format.
 *
 * @param [in] dev	 	   - The device structure.
 * @param [in] channel     - Channel to be read.
 * @param [in] sample_no   - Number of samples to retrieve from the channel.
 * @param [in] sample_buff - Sample buffer to be transmitted.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0414_ags_deactivate_channels(struct cn0414_dev *dev)
{
	ad717x_st_reg *reg;

	AD717X_ReadRegister(dev->ad4111_device,
			    channels_registers[dev->channel_index]);
	reg = AD717X_GetReg(dev->ad4111_device,
			    channels_registers[dev->channel_index]);
	if((reg->value & 0x8000) != 0)
		cn0414_adc_activate_channel(dev, dev->channel_index, false);
}

/**
 * Get a number of samples from a specified channel. The number of samples
 * cannot be more than 2048. If open wire detection is enabled and an open wire
 * is detected mid function the method exits and the application will display
 * the floating channel.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Channel and number of samples separated by space.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_get_samples(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint32_t i, sample_no, *sample_buff, temp_sampl;
	uint8_t channel = 0, *chan_ptr, *n_samples_ptr;

	/* Disable interrupts to avoid skipping channels */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	n_samples_ptr = (uint8_t*)strchr((char*)arg, ' ') + 1;
	chan_ptr = arg;
	chan_ptr[strcspn((char*)chan_ptr, " ")] = '\0';


	sample_no = atoi((char *)n_samples_ptr);

	if(sample_no > 2048) {
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"Sample number must be smaller than 2048.\n");
		return -1;
	}

	/* Identify channel */
	while (adc_chan_names[channel][0] != '\0') {
		if(strncmp((char *)chan_ptr, (char *)adc_chan_names[channel], 5) == 0)
			break;
		channel++;
	}
	if(channel > 8) {
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"Valid channels are: \n");
		for(i = 0; i <= 8; i++) {
			usr_uart_write_string(dev->uart_descriptor, (uint8_t*)" - ");
			usr_uart_write_string(dev->uart_descriptor,
					      (uint8_t *)adc_chan_names[i]);
			usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
		}
		return 0;
	}

	cn0414_ags_deactivate_channels(dev);

	sample_buff = calloc(sample_no, sizeof(uint32_t));
	if (!sample_buff)
		return -1;

	ret = cn0414_adc_activate_channel(dev, channel, true);
	if(ret < 0)
		goto finish;

	/* Get samples */
	i = 0;
	do {
		ret = cn0414_adc_get_samples_helper(dev, channel, 1, &temp_sampl);
		if(ret == 0) {
			*(sample_buff + i) = temp_sampl;
			i++;
		}
	} while(i != sample_no);

	/* Transmit samples */
	cn0414_adc_display_samples(dev, channel, sample_no, sample_buff);

finish:
	/* Get corresponding register */
	ret = cn0414_adc_activate_channel(dev, channel, false);

	free(sample_buff);

	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	return ret;
}

/**
 * If the new channel update rate is bigger than the ADC output data rate give
 * warning message and adjust to a safe value.
 *
 * @param [in] dev            - The device structure.
 * @param [in/out] rate_index - Pointer to the new ADC ODR.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0414_adc_odr_check(struct cn0414_dev *dev, uint32_t *rate_index)
{
	if(odr_actual_rate_values[*rate_index] <
	    (uint32_t)dev->adc_update_desc->f_update) {
		cn0414_status_update_rate(dev);
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)
				      "ADC output data rate cannot be lower than the channel update rate.\n");
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"ODR has been adjusted to 31250 samples per second.\n");
		*rate_index = 0;
	}
}

/**
 * If the new channel update rate is lower than the average speed of human
 * typing give out a warning that the CLI might not get all the letters if the
 * typing is too fast.
 *
 * @param [in] dev          - The device structure.
 * @param [in/out] new_rate - New channel update rate.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0414_adc_odr_cli_warning(struct cn0414_dev *dev,
				       uint32_t rate_index)
{
	if(rate_index >= 19) {
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"The CLI will be slower at this rate and lower.\n");
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"To see what the CLI managed to get so far press <TAB>.\n");
	}
}
#endif
/**
 * Change ADC output data rate. It is recommended to be at least 80 times
 * greater than the channel register update rate.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - New output data rate.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_set_odr(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint32_t i = 0;
	ad717x_st_reg *p_reg;

	/* Identify value */
	while (odr_options[i][0] != '\0') {
		if(strncmp((char *)arg, (char *)odr_options[i],
			   odr_option_size[i]) == 0)
			break;
		i++;
	}
	if(i >= 23) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"Sample rates are:\n");
		i = 5;
		while(i < 23) {
			usr_uart_write_string(dev->uart_descriptor, (uint8_t*)" - ");
			usr_uart_write_string(dev->uart_descriptor,
					      (uint8_t *)odr_options[i]);
			usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
			i++;
		}
#endif
		return 0;
	}
#if defined(CLI_INTEFACE)
	cn0414_adc_odr_check(dev, &i);

	cn0414_adc_odr_cli_warning(dev, i);
#endif
	/* Disable interrupts to avoid skipping channels */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		goto finish;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON0_REG);
	p_reg->value &= ~AD717X_FILT_CONF_REG_ODR(0x1ff);
	p_reg->value |= AD717X_FILT_CONF_REG_ODR(i);
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		goto finish;
	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON1_REG);
	if(ret < 0)
		goto finish;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON1_REG);
	p_reg->value &= ~AD717X_FILT_CONF_REG_ODR(0x1ff);
	p_reg->value |= AD717X_FILT_CONF_REG_ODR(i);
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_FILTCON1_REG);
	if(ret < 0)
		goto finish;

	/* Clear low sample rate warning flag to display a new warning if
	 * necessary */
	low_sample_rate_flag = 0;

finish:
	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	return ret;
}

/**
 * Set filter options of the ADC.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - New filter option.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_set_filter(struct cn0414_dev *dev, uint8_t* arg)
{
	ad717x_st_reg *p_reg;
	uint32_t i = 0;
	int32_t ret;

	/* Identify value */
	while (fiter_options[i][0] != '\0') {
		if(strncmp((char *)arg, (char *)fiter_options[i],
			   fiter_options_size[i]) == 0)
			break;
		i++;
	}
	if(i > 1) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"Unavailable Filter. Available filter options are:\n");
		i = 0;
		while(i < 2) {
			usr_uart_write_string(dev->uart_descriptor, (uint8_t*)" - ");
			usr_uart_write_string(dev->uart_descriptor,
					      (uint8_t *)fiter_options[i]);
			usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
			i++;
		}
#endif
		return 0;
	}

	if(i == 1)
		i = 3;

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON0_REG);
	p_reg->value &= ~AD717X_FILT_CONF_REG_ORDER(0b11);
	p_reg->value |= AD717X_FILT_CONF_REG_ORDER(i);
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		return ret;
	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON1_REG);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON1_REG);
	p_reg->value &= ~AD717X_FILT_CONF_REG_ORDER(0b11);
	p_reg->value |= AD717X_FILT_CONF_REG_ORDER(i);

	return AD717X_WriteRegister(dev->ad4111_device, AD717X_FILTCON1_REG);
}

/**
 * Enable ADC postfilter option.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_en_postfilt(struct cn0414_dev *dev, uint8_t* arg)
{
	ad717x_st_reg *p_reg;
	int32_t ret;

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON0_REG);
	p_reg->value |= AD717X_FILT_CONF_REG_ENHFILTEN;
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		return ret;
	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON1_REG);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON1_REG);
	p_reg->value |= AD717X_FILT_CONF_REG_ENHFILTEN;

	return AD717X_WriteRegister(dev->ad4111_device, AD717X_FILTCON1_REG);
}

/**
 * Disable ADC postfilter option.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_dis_postfilt(struct cn0414_dev *dev, uint8_t* arg)
{
	ad717x_st_reg *p_reg;
	int32_t ret;

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON0_REG);
	p_reg->value &= ~AD717X_FILT_CONF_REG_ENHFILTEN;
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		return ret;
	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON1_REG);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON1_REG);
	p_reg->value &= ~AD717X_FILT_CONF_REG_ENHFILTEN;

	return AD717X_WriteRegister(dev->ad4111_device, AD717X_FILTCON1_REG);
}

/**
 * Change the ADC postfilter option.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - New postfilter option.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_set_postfilt(struct cn0414_dev *dev, uint8_t* arg)
{
	ad717x_st_reg *p_reg;
	uint32_t i = 0;
	int32_t ret;

	/* Identify option */
	while (postfiter_options[i][0] != '\0') {
		if(strncmp((char *)arg, (char *)postfiter_options[i], 4) == 0)
			break;
		i++;
	}
	if(i > 3) {
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"Unavailable Filter. Available postfilter options are:\n");
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)" - opt1 = 27 SPS, 47 dB rejection, 36.7 ms settling\n");
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)" - opt2 = 25 SPS, 62 dB rejection, 40 ms settling\n");
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)" - opt3 = 20 SPS, 86 dB rejection, 50 ms settling\n");
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)" - opt4 = 16.67 SPS, 92 dB rejection, 60 ms settling\n");
#endif
		return 0;
	}

	/* Get value to put in register */
	switch(i) {
	case 0:
		i = 2;
		break;
	case 1:
		i = 3;
		break;
	case 2:
		i = 5;
		break;
	case 3:
		i = 6;
		break;
	default:
		i = 2;
	}

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON0_REG);
	p_reg->value &= ~AD717X_FILT_CONF_REG_ENHFILT(0b111);
	p_reg->value |= AD717X_FILT_CONF_REG_ENHFILT(i);
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		return ret;
	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON1_REG);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON1_REG);
	p_reg->value &= ~AD717X_FILT_CONF_REG_ENHFILT(0b111);
	p_reg->value |= AD717X_FILT_CONF_REG_ENHFILT(i);

	return AD717X_WriteRegister(dev->ad4111_device, AD717X_FILTCON1_REG);
}

/**
 * Set ADC output coding (unipolar/bipolar).
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Output coding style choice.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_set_output_coding(struct cn0414_dev *dev, uint8_t* arg)
{
	uint8_t i = 0;
	ad717x_st_reg *p_reg;
	int32_t ret;

	/* Identify option */
	while (output_coding_options[i][0] != '\0') {
		if(strncmp((char *)arg, (char *)output_coding_options[i],
			   output_coding_options_size[i]) == 0)
			break;
		i++;
	}
	if(i > 3) {
#if defined(CLI_INTEFACE)
		/* Verbose options */
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"Unavailable option. Available output coding verbose options are:\n");
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)" - ");
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t *)output_coding_options[0]);
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)" - ");
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t *)output_coding_options[2]);
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");

		/* Short options */
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t*)"Available output coding short options are:\n");
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)" - ");
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t *)output_coding_options[1]);
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)" - ");
		usr_uart_write_string(dev->uart_descriptor,
				      (uint8_t *)output_coding_options[3]);
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
#endif
		return 0;
	}

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_SETUPCON0_REG);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_SETUPCON0_REG);
	switch(i) {
	case 0:
	case 1:
		p_reg->value |= AD717X_SETUP_CONF_REG_BI_UNIPOLAR;
		break;
	case 2:
	case 3:
		p_reg->value &= ~AD717X_SETUP_CONF_REG_BI_UNIPOLAR;
		break;
	}
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_SETUPCON0_REG);
	if(ret < 0)
		return ret;
	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_SETUPCON1_REG);
	if(ret < 0)
		return ret;
	p_reg = AD717X_GetReg(dev->ad4111_device, AD717X_SETUPCON1_REG);
	switch(i) {
	case 0:
	case 1:
		p_reg->value |= AD717X_SETUP_CONF_REG_BI_UNIPOLAR;
		break;
	case 2:
	case 3:
		p_reg->value &= ~AD717X_SETUP_CONF_REG_BI_UNIPOLAR;
		break;
	}

	return AD717X_WriteRegister(dev->ad4111_device, AD717X_SETUPCON1_REG);
}

/**
 * Enables open-wire detection. To enable Open Wire Detection the GPIOCON
 * register and the IFMODE register of the ADC must be written.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_open_wire_enable(struct cn0414_dev *dev, uint8_t* arg)
{
	ad717x_st_reg *temp_reg;
	int32_t ret;

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_GPIOCON_REG);
	if(ret < 0)
		return ret;
	temp_reg = AD717X_GetReg(dev->ad4111_device, AD717X_GPIOCON_REG);
	temp_reg->value |= AD4111_GPIOCON_REG_OW_EN;
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_GPIOCON_REG);
	if(ret < 0)
		return ret;

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_IFMODE_REG);
	if(ret < 0)
		return ret;
	temp_reg = AD717X_GetReg(dev->ad4111_device, AD717X_IFMODE_REG);
	temp_reg->value |= AD717X_IFMODE_REG_DATA_STAT;
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_IFMODE_REG);
	if(ret < 0)
		return ret;

	dev->open_wire_detect_enable = OPEN_WIRE_DETECT_ENABLED;

	return ret;
}

/**
 * Disables open-wire detection.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_adc_open_wire_disable(struct cn0414_dev *dev, uint8_t* arg)
{
	ad717x_st_reg *temp_reg;
	int32_t ret;
	uint8_t i;

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_GPIOCON_REG);
	if(ret < 0)
		return ret;
	temp_reg = AD717X_GetReg(dev->ad4111_device, AD717X_GPIOCON_REG);
	temp_reg->value &= ~AD4111_GPIOCON_REG_OW_EN;
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_GPIOCON_REG);
	if(ret < 0)
		return ret;

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_IFMODE_REG);
	if(ret < 0)
		return ret;
	temp_reg = AD717X_GetReg(dev->ad4111_device, AD717X_IFMODE_REG);
	temp_reg->value &= ~AD717X_IFMODE_REG_DATA_STAT;
	ret = AD717X_WriteRegister(dev->ad4111_device, AD717X_IFMODE_REG);
	if(ret < 0)
		return ret;

	dev->open_wire_detect_enable = OPEN_WIRE_DETECT_DISABLED;
	for(i = 0; i < ADC_VOLTAGE_CHAN_NO; i++)
		dev->chan_voltage_status[i] = 0;

	return ret;
}
#if defined(CLI_INTEFACE)
/**
 * Display EEPROM address.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_mem_display_addr(struct cn0414_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint8_t addr;
	uint8_t buffer[5];
	uint8_t i = 0;
	uint8_t found_flag = 0;

	while(1) {
		/* Get address */
		ret = cn0414_mem_discover(dev, i, &addr);
		if(ret < 0)
			return ret;

		if(addr <= (A0_VDD | A1_VDD | A2_VDD)) {
			/* Display I2C address */
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t*)"EEPROM discovered at address: 0x5");
			if(ret < 0)
				return ret;
			itoa(addr, (char *)buffer, 16);
			ret = usr_uart_write_string(dev->uart_descriptor, buffer);
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
			if(ret < 0)
				return ret;

			/* Display status of adress pins */
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t*)" - P10 = ");
			if(ret < 0)
				return ret;
			itoa(A0_MASK(addr), (char *)buffer, 10);
			ret = usr_uart_write_string(dev->uart_descriptor, buffer);
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t*)" - P11 = ");
			if(ret < 0)
				return ret;
			itoa(A1_MASK(addr), (char *)buffer, 10);
			ret = usr_uart_write_string(dev->uart_descriptor, buffer);
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t*)" - P12 = ");
			if(ret < 0)
				return ret;
			itoa(A2_MASK(addr), (char *)buffer, 10);
			ret = usr_uart_write_string(dev->uart_descriptor, buffer);
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)"\n");
			if(ret < 0)
				return ret;

			i = addr + 1;
			found_flag = 1;
		} else {
			break;
		}
	}

	if(found_flag != 1) {
		return usr_uart_write_string(dev->uart_descriptor,
					     (uint8_t*)"No EEPROM found.\n");
	}

	return ret;
}
#endif
/**
 * Process function helper. Updates the ADC read register on the update timer
 * interrupt.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_process_update_adc(struct cn0414_dev *dev)
{
	int32_t ret;

	/* Disable interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(HART_CD_INT);

	ret = cn0414_read_channel(dev, dev->channel_index);
	if((ret < 0) || (ret == 1))
		goto finish;

	if(dev->channel_index == IIN4)
		dev->channel_index = VIN1;
	else
		dev->channel_index++;

	adc_channel_flag = 0;
	adc_sw_prescaler = 0;

finish:
	/* Enable interrupts */
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(HART_CD_INT);

	return ret;
}

/**
 * Detect HART command zero from a freshly received buffer.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_process_hart_detect_command_zero(struct cn0414_dev *dev)
{
	uint8_t i = 1;
	uint8_t k = 0;

	i = 1;
	while(i <= 20) {
		if(dev->hart_buffer[i] == 0xFF) {
			i++;
			continue;
		} else {
			break;
		}
	}

	if(i <= 20)
		for(k = 0; k < HART_COMMAND_ZERO_SIZE; k++)
			if(dev->hart_buffer[i + k] != cn0414_hart_command_zero[k])
				break;

	if(k == HART_COMMAND_ZERO_SIZE)
		if(dev->hart_buffer[i + k] == 0xFF)
#if defined(CLI_INTEFACE)
			return usr_uart_write_string(dev->uart_descriptor,
						     (uint8_t*)"Command zero detected.\n");
#elif defined(MODBUS_INTERFACE)
			return 0;
#endif

	return 0;
}
#if defined(CLI_INTEFACE)
/**
 * Process function helper. Displays floating channels based on the channel
 * status register in the device structure.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_process_disp_floating_channels(struct cn0414_dev *dev)
{
	int32_t ret;
	uint8_t i;

	for(i = 0; i < ADC_VOLTAGE_CHAN_NO; i++) {
		if(dev->chan_voltage_status[i] == 1) {
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t*)"\nChannel ");
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t *)adc_chan_names[i]);
			if(ret < 0)
				return ret;
			ret = usr_uart_write_string(dev->uart_descriptor,
						    (uint8_t*)" FLOATING\n");
			if(ret < 0)
				return ret;
			dev->chan_voltage_status[i] = 2;
		}
	}
	return ret;
}

/**
 * Process function helper. Displays low data rate warning if the ADC output
 * data rate is smaller than 80 times the register update rate. If this is the
 * case the application may stop the CLI capability and freeze.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0414_process_low_odr_warning(struct cn0414_dev *dev)
{
	int32_t ret;

	ret = usr_uart_write_string(dev->uart_descriptor,
				    (uint8_t*)"\nCARE! SAMPLE RATE CRITICALLY LOW!\n");
	if(ret < 0)
		goto finish;
	ret = usr_uart_write_string(dev->uart_descriptor, (uint8_t*)">");
	if(ret < 0)
		goto finish;

finish:
	low_sample_rate_flag = 2;

	return ret;
}
#endif
/**
 * Implements the CN0414 main process.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_process(struct cn0414_dev *dev)
{
	int32_t ret;
	int32_t cheat;

	if((adc_channel_flag == 1) &&
	    (adc_sw_prescaler >= dev->adc_update_desc->sw_prescaler)) {
		cheat = cn0414_process_update_adc(dev);
		if(cheat < 0)
#if defined(CLI_INTEFACE)
			usr_uart_write_string(dev->uart_descriptor,
					      (uint8_t*)"\nADC update error.\n");
#elif defined(MODBUS_INTERFACE)
			return cheat;
#endif
	}
	if(modem_rec_flag) {
		ret = cn0414_process_hart_int_rec(dev);
		if(ret < 0)
#if defined(CLI_INTEFACE)
			usr_uart_write_string(dev->uart_descriptor,
					      (uint8_t*)"\nHART receive error.\n");
#elif defined(MODBUS_INTERFACE)
			return ret;
#endif
		cn0414_process_hart_detect_command_zero(dev);
#if defined(CLI_INTEFACE)
		usr_uart_write_string(dev->uart_descriptor, (uint8_t*)">");
#endif
	}
#if defined(CLI_INTEFACE)
	if (dev->open_wire_detect_enable == 1) {
		ret = cn0414_process_disp_floating_channels(dev);
		if(ret < 0)
			usr_uart_write_string(dev->uart_descriptor,
					      (uint8_t*)"\nFloating channels display error.\n");
	}

	if(low_sample_rate_flag == 1) {
		ret = cn0414_process_low_odr_warning(dev);
		if(ret < 0)
			usr_uart_write_string(dev->uart_descriptor,
					      (uint8_t*)"\nODR warning error.\n");
	}
#endif

	return 0;
}

/**
 * Read one of the channels on the board.
 *
 * @param [in] dev     - The device structure.
 * @param [in] channel - Channel to be read.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_read_channel(struct cn0414_dev *dev, enum adc_channels channel)
{
	int32_t ret;
	ad717x_st_reg *reg;
	uint32_t current_sample_rate;

	/* Get filter configuration register to calculate sample rate */
	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_FILTCON0_REG);
	if(ret < 0)
		return ret;
	reg = AD717X_GetReg(dev->ad4111_device, AD717X_FILTCON0_REG);

	/* Get sample rate */
	current_sample_rate = odr_actual_rate_values[reg->value &
					 AD717X_FILT_CONF_REG_ODR(0x1ff)];

	/* If the sample rate is less than 80 the update rate  assert flag to give
	 * warning */
	if((current_sample_rate < (8 * dev->adc_update_desc->f_update)) &&
	    (low_sample_rate_flag == 0))
		low_sample_rate_flag = 1;

	ret = AD717X_ReadRegister(dev->ad4111_device, channels_registers[channel]);
	if(ret < 0)
		return ret;
	reg = AD717X_GetReg(dev->ad4111_device, channels_registers[channel]);
	if((reg->value & 0x8000) == 0) {
		ret = cn0414_adc_activate_channel(dev, channel, true);
		if(ret < 0)
			return ret;
	}

	/* Disable UART interrupt to avoid hanging */
	NVIC_DisableIRQ(UART_INT);

	ret = cn0414_adc_get_samples_helper(dev, channel, 1,
					    &dev->channel_output[channel]);
	if((ret < 0) || (ret == 1)) {
		/* Enable UART interrupts */
		NVIC_EnableIRQ(UART_INT);

		return ret;
	}

	/* Enable UART interrupts */
	NVIC_EnableIRQ(UART_INT);

	return cn0414_adc_activate_channel(dev, channel, false);
}

/**
 * Compute value in volts or amperes from an ADC sample.
 *
 * @param [in] dev           - The device structure.
 * @param [in] code          - The raw ADC sample.
 * @param [in] ncurr_or_volt - Boolean value that decides if the conversion is
 *                             to volts or to amperes.
 * @param [out] result       - Result of the conversion in floating precision.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_compute_adc_value(struct cn0414_dev *dev, uint32_t code,
				 bool ncurr_or_volt, float *result)
{
	int32_t ret;
	ad717x_st_reg *p_conf_reg;
	double bit_rep_nr;

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_IFMODE_REG);
	if(ret < 0)
		return ret;
	p_conf_reg = AD717X_GetReg(dev->ad4111_device, AD717X_IFMODE_REG);

	if((p_conf_reg->value & AD717X_IFMODE_REG_DATA_WL16) == 0)
		bit_rep_nr = 24;
	else
		bit_rep_nr = 16;

	ret = AD717X_ReadRegister(dev->ad4111_device, AD717X_SETUPCON0_REG);
	if(ret < 0)
		return ret;
	p_conf_reg = AD717X_GetReg(dev->ad4111_device, AD717X_SETUPCON0_REG);

	if ((p_conf_reg->value & AD717X_SETUP_CONF_REG_BI_UNIPOLAR) == 0) {
		if(!ncurr_or_volt)
			*result = ((float)code * vref) / (pow(2,bit_rep_nr) * 50);
		else
			*result = ((float)code * vref) / (pow(2,bit_rep_nr) * 0.1);
	} else {
		if(!ncurr_or_volt)
			*result = (((float)code / pow(2,bit_rep_nr - 1) - 1) * vref) / 50;
		else
			*result = (((float)code / pow(2,bit_rep_nr - 1) - 1) * vref) / 0.1;
	}

	return ret;
}
#if defined(CLI_INTEFACE)
/**
 * Convert floating point value to ASCII. Maximum 4 decimals.
 *
 * @param [in] value   - The floating point value to be converted.
 * @param [out] buffer - ASCII buffer; output buffer.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
void cn0414_ftoa(uint8_t *buffer, float value)
{
	int32_t fraction;
	uint8_t local_buffer[20];
	float subunit;
	uint8_t i;

	/* Initialize buffer */
	strcpy((char *)buffer, "");

	/* If between -1 and 0 place the '-' in front manually */
	if((value < 0.0) && (value > -1.0))
		strcat((char *)buffer, "-");

	/* Convert integral part */
	itoa((int32_t)value, (char*)local_buffer, 10);
	strcat((char *)buffer, (char *)local_buffer);
	strcat((char *)buffer, ".");
	/* Display zeros after the decimal point. Else they would be eluded. */
	subunit = value - (uint32_t)value;
	i = 0;
	while(fabs(subunit) < 0.1 && i < 7) {
		strcat((char*)buffer, "0");
		subunit *= 10;
		i++;
	}
	/* Convert decimal part */
	fraction = (int32_t)(subunit * 100000000);
	itoa(fabs(fraction), (char*)local_buffer, 10);
	strcat((char*)buffer, (char*)local_buffer);
}
#endif
/**
 * Discover the first EEPROM present on the board.
 *
 * @param [in] dev		  - The device structure.
 * @param [in] start_addr - I2C address from where to start searching (not
 *                          bigger than 7).
 * @param [out] address   - I2C address of the memory encountered.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_mem_discover(struct cn0414_dev *dev, uint8_t start_addr,
			    uint8_t *address)
{
	int32_t ret;
	uint8_t i;
	uint8_t backup, temp, test = 0xAA;

	if(start_addr > 7)
		return -1;

	/* Check every available memory address */
	for (i = start_addr; i < (A0_VDD | A1_VDD | A2_VDD) + 1; ++i) {
		/* Change address */
		ret = memory_change_i2c_address(dev->memory_device, i);
		if(ret < 0)
			return ret;

		/* If there is a memory there, make backup */
		memory_read(dev->memory_device, 0x0000, &backup, 1);
		/* Test write */
		memory_write(dev->memory_device, 0x0000, &test, 1);
		mdelay(5);
		/* Test read */
		memory_read(dev->memory_device, 0x0000, &temp, 1);
		if(temp == test) {
			/* Rewrite memory with backup data */
			ret = memory_write(dev->memory_device, 0x0000, &backup, 1);
			if(ret < 0)
				return ret;
			break;
		}
	}

	*address = i;

	return 0;
}

/**
 * Take two samples from the ADC and determine if the input is floating.
 *
 * @param [in] dev           - The device structure.
 * @param [in] voltage_chan1 - First channel data.
 * @param [in] voltage_chan2 - Second channel data.
 * @param [out] floating     - is 1 if floating and 0 if connected.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0414_open_wire_detect(struct cn0414_dev *dev, uint32_t voltage_chan1,
				uint32_t voltage_chan2, uint8_t *floating)
{
	int32_t ret;
	float fdata1, fdata2;

	/* Calculate voltage levels */
	ret = cn0414_compute_adc_value(dev, voltage_chan1, ADC_VOLTAGE_CHANNEL,
				       &fdata1);
	if(ret < 0)
		return ret;
	ret = cn0414_compute_adc_value(dev, voltage_chan2, ADC_VOLTAGE_CHANNEL,
				       &fdata2);
	if(ret < 0)
		return ret;

	if(((fdata1 - fdata2) < -0.3) || ((fdata1 - fdata2) > 0.3))
		*floating = 1;
	else
		*floating = 0;

	return ret;
}
