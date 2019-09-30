/***************************************************************************//**
 *   @file   aio_dio_pdmz.c
 *   @brief  Implementation of the I/O analog/digital PMOD.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "aio_dio_pdmz.h"
#include <stdlib.h>

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

extern const struct ad5592r_rw_ops ad5592r_rw_ops;
extern const struct ad5592r_rw_ops ad5593r_rw_ops;

/* Command vector */
cmd_func aiodio_fnc_ptr[] = {
	(cmd_func)aiodio_help,
	(cmd_func)aiodio_analog_out,
	(cmd_func)aiodio_analog_in,
	(cmd_func)aiodio_digital_out,
	(cmd_func)aiodio_digital_in,
	(cmd_func)aiodio_mirror_mode,
	(cmd_func)aiodio_prod_test_mode,
	(cmd_func)aiodio_status,
	NULL
};

/* Command call vector */
char *aiodio_fnc_calls[] = {
	"help",
	"h",
	"analog_out ",
	"ao ",
	"analog_in ",
	"ai ",
	"digital_out ",
	"do ",
	"digital_in ",
	"di ",
	"mirror",
	"m",
	"test",
	"t",
	"status",
	"stts",
	""
};

/* Command size vector */
uint8_t aiodio_fnc_call_size[] = {
	5, 2, 11, 3, 10, 3, 12, 3, 11, 3, 6, 2, 5, 2, 7, 5, 1
};

static volatile uint8_t mode_timer_flag = 0;
static volatile uint8_t test_blink_count = 0;
static volatile uint32_t test_sw_pescaler = 0;
static volatile uint8_t led_state = 0;
static const uint16_t staircase[] = {812, 1638, 2458, 3277};

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Timer counter interrupt callback function.
 *
 * Implements interrupt delay for the operating modes. This is so that the
 * program is not blocked and the CLI still works when waiting to do a mode
 * cycle.
 *
 * @param [in] cb_param - Pointer to callback parameter list.
 * @param [in] event    - Interrupt source identifier.
 * @param [in/out] arg  - Argument list given by the driver.
 *
 * @return none
 */
static void mode_delay_callback(void *cb_param, uint32_t event, void *arg)
{
	if(mode_timer_flag == 0)
		mode_timer_flag = 1;
}

/**
 * Timer counter interrupt callback function.
 *
 * Implements interrupt delay blinking lights on test mode pass.
 *
 * @param [in] cb_param - Pointer to callback parameter list.
 * @param [in] event    - Interrupt source identifier.
 * @param [in/out] arg  - Argument list given by the driver.
 *
 * @return none
 */
static void blink_delay_callback(void *cb_param, uint32_t event, void *arg)
{
	if(test_sw_pescaler < 5000) {
		test_sw_pescaler++;
		return;
	}
	test_sw_pescaler = 0;
	if(test_blink_count != 0) {
		gpio_set_value(((struct aiodio_dev *)cb_param)->blink_led1, led_state);
		gpio_set_value(((struct aiodio_dev *)cb_param)->blink_led2, !led_state);
		led_state = !led_state;
		test_blink_count--;
	}
	if(test_blink_count == 0) {
		gpio_set_value(((struct aiodio_dev *)cb_param)->blink_led1, GPIO_LOW);
		gpio_set_value(((struct aiodio_dev *)cb_param)->blink_led2, GPIO_LOW);
		led_state = 0;
	}
}

/**
 * Set the type of the channels.
 *
 * Set 4 channels as input and 4 as output. Of every mentioned group 2 are
 * analog and 2 are digital.
 *
 * @param [in] dev - Pointer to the application structure.
 *
 * @return void
 */
static void aiodio_setup_mirror_channels(struct aiodio_dev *dev)
{
	dev->board_device->channel_modes[AIODIO_CH0] = CH_MODE_ADC;
	dev->board_device->channel_modes[AIODIO_CH1] = CH_MODE_ADC;
	dev->board_device->channel_modes[AIODIO_CH2] = CH_MODE_GPIO;
	dev->board_device->channel_modes[AIODIO_CH3] = CH_MODE_GPIO;
	dev->board_device->channel_modes[AIODIO_CH4] = CH_MODE_UNUSED;
	dev->board_device->channel_offstate[AIODIO_CH4] = CH_OFFSTATE_OUT_HIGH;
	dev->board_device->channel_modes[AIODIO_CH5] = CH_MODE_UNUSED;
	dev->board_device->channel_offstate[AIODIO_CH5] = CH_OFFSTATE_OUT_HIGH;
	dev->board_device->channel_modes[AIODIO_CH6] = CH_MODE_DAC;
	dev->board_device->channel_modes[AIODIO_CH7] = CH_MODE_DAC;
	dev->board_device->num_channels = 8;
}

/**
 * Dynamically allocate memory for the app, board and connectivity drivers.
 *
 * This is a helper function for aiodio_setup().
 *
 * @param [out] dev - Reference to the pointer of the application handler.
 *
 * @return 0 in case of success, negative error value otherwise.
 */
static inline int32_t aiodio_setup_memory(struct aiodio_dev **dev)
{
	*dev = calloc(1, sizeof **dev);
	if(!(*dev))
		return -1;

	(*dev)->board_device = calloc(1, sizeof *((*dev)->board_device));
	if(!(*dev)->board_device)
		return -1;

	(*dev)->board_device->ops = calloc(1, sizeof *((*dev)->board_device->ops));
	if(!(*dev)->board_device->ops)
		return -1;

	return 0;
}

/**
 * Test the existence of a device.
 *
 * Save the contents of the ADC enable register in case it is there, then do a
 * write and read of the same register with a control value to test if it can
 * be accessed. If it is there, restore the contents and return. Else return
 * directly.
 *
 * @param [in] dev - Pointer to the application structure.
 *
 * @return 0 in case of no error and the device exists, non-zero error code
 *         otherwise.
 */
static int32_t aiodio_test_connection(struct aiodio_dev *dev)
{
	int32_t ret;
	uint16_t read_back = 0, test_val = 0xab, mem_reg;

	ret = ad5592r_base_reg_read(dev->board_device, AD5592R_REG_ADC_EN,
				    &mem_reg);
	if (ret != 0)
		return ret;

	ret = ad5592r_base_reg_write(dev->board_device, AD5592R_REG_ADC_EN,
				     test_val);
	if (ret != 0)
		return ret;

	ret = ad5592r_base_reg_read(dev->board_device, AD5592R_REG_ADC_EN,
				    &read_back);
	if (!ret && (read_back & 0xff) != test_val)
		return FAILURE;

	return ad5592r_base_reg_write(dev->board_device, AD5592R_REG_ADC_EN,
				      mem_reg);
}

/**
 * Detect which PMOD is present between EVAL-AD5592R-PDMZ and EVAL-AD5593R-PDMZ.
 *
 * Start testing with EVAL-AD5592R-PDMZ and, if not found, test for
 * EVAL-AD5593R-PDMZ. The first device present will be used, which means that if
 * both are present EVAL-AD5592R-PDMZ will be the opne used. If no device is
 * found the application exits.
 * aiodio_setup() helper function.
 *
 * @param [out] dev        - Pointer to the application structure.
 * @param [in]  init_param - Application initialization structure.
 * @param [out] ad5592_n3  - Indicates the device found:
 *                           true if EVAL-AD5592R-PDMZ is found;
 *                           false if EVAL-AD5593R-PDMZ is found.
 *
 * @return 0 in case of success, non-zero error code otherwise.
 */
static inline int32_t aiodio_detect_device(struct aiodio_dev *dev,
		struct aiodio_init_param *init_param, bool *ad5592_n3)
{
	int32_t ret;
	struct ad5592r_rw_ops *temp;

	*ad5592_n3 = false;

	ret = spi_init(&dev->board_spi, &init_param->spi_init);
	if(ret != 0)
		return ret;
	dev->board_device->spi = dev->board_spi;

	ret = i2c_init(&dev->board_i2c, &init_param->i2c_init);
	if(ret != 0)
		return ret;
	dev->board_device->i2c = dev->board_i2c;

	temp = (void *)dev->board_device->ops;

	temp->gpio_read = ad5592r_rw_ops.gpio_read;
	temp->read_adc = ad5592r_rw_ops.read_adc;
	temp->reg_read = ad5592r_rw_ops.reg_read;
	temp->reg_write = ad5592r_rw_ops.reg_write;
	temp->write_dac = ad5592r_rw_ops.write_dac;
	dev->read_adc = &ad5592r_read_adc;
	dev->write_dac = &ad5592r_write_dac;

	ret = aiodio_test_connection(dev);
	if(ret == 0) {
		*ad5592_n3 = true;
		return ret;
	}

	temp->gpio_read = ad5593r_rw_ops.gpio_read;
	temp->read_adc = ad5593r_rw_ops.read_adc;
	temp->reg_read = ad5593r_rw_ops.reg_read;
	temp->reg_write = ad5593r_rw_ops.reg_write;
	temp->write_dac = ad5593r_rw_ops.write_dac;
	dev->read_adc = &ad5593r_read_adc;
	dev->write_dac = &ad5593r_write_dac;

	return aiodio_test_connection(dev);
}

/**
 * Setup the application.
 *
 * Setup the communication interface based on the given type of device and do
 * device initialization.
 *
 * @param [out] device    - Pointer to the application structure.
 * @param [in] init_param - Application initialization structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_setup(struct aiodio_dev **device,
		     struct aiodio_init_param *init_param)
{
	int32_t ret;
	struct aiodio_dev *dev;
	bool ad5592_n3;

	timer_start();

	aiodio_setup_memory(&dev);

	aiodio_setup_mirror_channels(dev);

	ret = cli_setup(&dev->board_cli, &init_param->cli_init);
	if(ret != 0)
		goto error;

	cli_load_command_vector(dev->board_cli, aiodio_fnc_ptr);
	cli_load_command_calls(dev->board_cli, aiodio_fnc_calls);
	cli_load_command_sizes(dev->board_cli, aiodio_fnc_call_size);
	cli_load_descriptor_pointer(dev->board_cli, dev);

	ret = aiodio_detect_device(dev, init_param, &ad5592_n3);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t *)"ERROR! No device found!");
		goto error;
	}

	if(ad5592_n3) {
		ret = ad5592r_init(dev->board_device, &init_param->ad5592_3r_param);
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)" AD5592R device detected.\n");
	} else {
		ret = ad5593r_init(dev->board_device, &init_param->ad5592_3r_param);
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)" AD5593R device detected.\n");
	}
	if(ret != 0)
		goto error;

	ret = ad5592r_gpio_direction_input(dev->board_device, AIODIO_CH2);
	if(ret != 0)
		goto error;
	ret = ad5592r_gpio_direction_input(dev->board_device, AIODIO_CH3);
	if(ret != 0)
		goto error;
	ret = ad5592r_gpio_direction_output(dev->board_device, AIODIO_CH4,
					    DIO_HIGH);
	if(ret != 0)
		goto error;
	ret = ad5592r_gpio_direction_output(dev->board_device, AIODIO_CH5,
					    DIO_HIGH);
	if(ret != 0)
		goto error;

	init_param->mode_delay_timer.callback_func_ptr = mode_delay_callback;
	init_param->mode_delay_timer.callback_param = NULL;
	ret = timer_counter_setup(&dev->mode_delay_timer,
				  &init_param->mode_delay_timer);
	if(ret != 0)
		goto error;
	ret = timer_counter_activate(dev->mode_delay_timer, true);
	if(ret != 0)
		goto error;

	init_param->blink_delay_timer.callback_func_ptr = blink_delay_callback;
	init_param->blink_delay_timer.callback_param = (void *)dev;
	ret = timer_counter_setup(&dev->blink_delay_timer,
				  &init_param->blink_delay_timer);
	if(ret != 0)
		goto error;

	dev->mirror_mode = 1;
	dev->test_mode = 0;
	dev->test_success = 0;

	ret = gpio_get(&dev->blink_led1, 31);
	if(ret != 0)
		goto error;
	ret = gpio_get(&dev->blink_led2, 32);
	if(ret != 0)
		goto error;
	ret = gpio_direction_output(dev->blink_led1, GPIO_LOW);
	if(ret != 0)
		goto error;
	ret = gpio_direction_output(dev->blink_led2, GPIO_LOW);
	if(ret != 0)
		goto error;

	ret = cli_cmd_prompt(dev->board_cli, (uint8_t *)"AD592/3R");
	if(ret < 0)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free resources allocated by aiodio_setup().
 *
 * @param [in] dev - Pointer to the application structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_remove(struct aiodio_dev *dev)
{
	int32_t ret;

	if(!dev)
		return -1;

	ret = spi_remove(dev->board_spi);
	if(ret != 0)
		return ret;

	ret = i2c_remove(dev->board_i2c);
	if(ret != 0)
		return ret;

	ret = cli_remove(dev->board_cli);
	if(ret != 0)
		return ret;

	ret = timer_counter_activate(dev->mode_delay_timer, false);
	if(ret != 0)
		return ret;
	ret = timer_counter_remove(dev->mode_delay_timer);
	if(ret != 0)
		return ret;
	ret = timer_counter_activate(dev->blink_delay_timer, false);
	if(ret != 0)
		return ret;
	ret = timer_counter_remove(dev->blink_delay_timer);
	if(ret != 0)
		return ret;

	ret = gpio_remove(dev->blink_led1);
	if(ret != 0)
		return ret;
	ret = gpio_remove(dev->blink_led2);
	if(ret != 0)
		return ret;

	free(dev);

	return ret;
}

/**
 * Display help function prompt.
 *
 * Help command helper function.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static inline int32_t aiodio_help_prompt(struct aiodio_dev *dev,
		bool short_command)
{
	int32_t ret;

	if(!short_command) {
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)"\tAD5592/3R application.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "For commands with options as arguments typing the command and 'space' without arguments\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)"will show the list of options.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)"Pressing TAB will bring up the last command.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->board_cli->uart_device,
					     (uint8_t*)"Available verbose commands.\n\n");
	} else {
		return usr_uart_write_string(dev->board_cli->uart_device,
					     (uint8_t*)"\nAvailable short commands:\n\n");
	}
}

/**
 * Display general commands in help function.
 *
 * Help command helper function.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static inline int32_t aiodio_help_general_commands(struct aiodio_dev *dev,
		bool short_command)
{
	int32_t ret;

	if(!short_command) {
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)" help                     - Display this help message.\n");
		if(ret != 0)
			return ret;
		return usr_uart_write_string(dev->board_cli->uart_device,
					     (uint8_t*)
					     " status                   - Display the status of the application.\n");
	} else {
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)" h               - Display this help message.\n");
		if(ret != 0)
			return ret;
		return usr_uart_write_string(dev->board_cli->uart_device,
					     (uint8_t*)" stts            - Display the status of the application.\n");
	}
}

/**
 * Display channel specific commands in help function.
 *
 * Help command helper function.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static inline int32_t aiodio_help_channel_commands(struct aiodio_dev *dev,
		bool short_command)
{
	int32_t ret;

	if(!short_command) {
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " analog_out <chan> <val>  - Change the channel to analog output and write the code in the analog output register.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            <chan> = channel to be changed (0-7).\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            <val> = DAC code in decimal integer format (0-4095).\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " analog_in <chan>         - Change the channel to analog input and read the ADC channel.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            <chan> = channel to be changed and read. (0-7).\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " digital_out <chan> <val> - Change the channel to digital output and set it's state to HIGH or LOW.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            <chan> = channel to be changed (0-7).\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            <val> = '0' or '1' for LOW or HIGH, respecively.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " digital_in <chan>        - Change the channel to digital input and read it's state.\n");
		if(ret != 0)
			return ret;
		return usr_uart_write_string(dev->board_cli->uart_device,
					     (uint8_t*)
					     "                           <chan> = channel to be changed and read (0-7).\n");
	} else {
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " ao <chan> <val> - Change the channel to analog output and write the code in the analog output register.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)"                   <chan> = channel to be changed (0-7).\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   <val> = DAC code in decimal integer format (0-4095).\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " ai <chan>       - Change the channel to analog input and read the ADC channel.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   <chan> = channel to be changed and read. (0-7).\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " do <chan> <val> - Change the channel to digital output and set it's state to HIGH or LOW.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)"                   <chan> = channel to be changed (0-7).\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   <val> = '0' or '1' for LOW or HIGH, respecively.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " di <chan>       - Change the channel to digital input and read it's state.\n");
		if(ret != 0)
			return ret;
		return usr_uart_write_string(dev->board_cli->uart_device,
					     (uint8_t*)
					     "                   <chan> = channel to be changed and read (0-7).\n");
	}
}

/**
 * Display mode commands in help function.
 *
 * Help command helper function.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static inline int32_t aiodio_help_mode_commands(struct aiodio_dev *dev,
		bool short_command)
{
	int32_t ret;

	if(!short_command) {
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " mirror                   - Start mirror more. In this mode output channels mirror the input channels.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)"                            Channel configuration is:\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            Channel 0 = Analog Input (ADC) -> Channel 7 = Analog Output (DAC)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            Channel 1 = Analog Input (ADC) -> Channel 6 = Analog Output (DAC)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            Channel 2 = Digital Input (GPIO) -> Channel 5 = Digital Output (GPIO)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            Channel 3 = Digital Input (GPIO) -> Channel 4 = Digital Output (GPIO)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " test                     - Start production test mode. In this mode 4 channels are analog output set to a 4 different voltage values and 4 channels are analog input.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            If the input channels read the expected voltage outputs when connected to the output ones, a pass message is printed.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)"                            Channel configuration is:\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            Channel 0 = Analog Output (DAC) -> Channel 7 = Analog Input (ADC)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            Channel 1 = Analog Output (DAC) -> Channel 6 = Analog Input (ADC)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                            Channel 2 = Analog Output (DAC) -> Channel 5 = Analog Input (ADC)\n");
		if(ret != 0)
			return ret;
		return usr_uart_write_string(dev->board_cli->uart_device,
					     (uint8_t*)
					     "                            Channel 3 = Analog Output (DAC) -> Channel 4 = Analog Input (ADC)\n");
	} else {
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " m               - Start mirror more. In this mode output channels mirror the input channels.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)"                   Channel configuration is:\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   Channel 0 = Analog Input (ADC) -> Channel 7 = Analog Output (DAC)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   Channel 1 = Analog Input (ADC) -> Channel 6 = Analog Output (DAC)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   Channel 2 = Digital Input (GPIO) -> Channel 5 = Digital Output (GPIO)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   Channel 3 = Digital Input (GPIO) -> Channel 4 = Digital Output (GPIO)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    " t               - Start production test mode. In this mode 4 channels are analog output set to a 4 different voltage values and 4 channels are analog input.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   If the input channels read the expected voltage outputs when connected to the output ones, a pass message is printed.\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)"                   Channel configuration is:\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   Channel 0 = Analog Output (DAC) -> Channel 7 = Analog Input (ADC)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   Channel 1 = Analog Output (DAC) -> Channel 6 = Analog Input (ADC)\n");
		if(ret != 0)
			return ret;
		ret = usr_uart_write_string(dev->board_cli->uart_device,
					    (uint8_t*)
					    "                   Channel 2 = Analog Output (DAC) -> Channel 5 = Analog Input (ADC)\n");
		if(ret != 0)
			return ret;
		return usr_uart_write_string(dev->board_cli->uart_device,
					     (uint8_t*)
					     "                   Channel 3 = Analog Output (DAC) -> Channel 4 = Analog Input (ADC)\n");

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
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_help(struct aiodio_dev *dev, uint8_t *arg)
{
	int32_t ret;

	ret = aiodio_help_prompt(dev, HELP_LONG_COMMAND);
	if(ret != 0)
		return ret;

	ret = aiodio_help_general_commands(dev, HELP_LONG_COMMAND);
	if(ret != 0)
		return ret;

	ret = aiodio_help_channel_commands(dev, HELP_LONG_COMMAND);
	if(ret != 0)
		return ret;

	ret = aiodio_help_mode_commands(dev, HELP_LONG_COMMAND);
	if(ret != 0)
		return ret;

	ret = aiodio_help_prompt(dev, HELP_SHORT_COMMAND);
	if(ret != 0)
		return ret;

	ret = aiodio_help_general_commands(dev, HELP_SHORT_COMMAND);
	if(ret != 0)
		return ret;

	ret = aiodio_help_channel_commands(dev, HELP_SHORT_COMMAND);
	if(ret != 0)
		return ret;

	return aiodio_help_mode_commands(dev, HELP_SHORT_COMMAND);
}

/**
 * Display the status of a single channel.
 *
 * aiodio_status() helper function.
 *
 * @param [in] dev     - The device structure.
 * @param [in] chan_no - Number of the channel to be displayed.
 *
 * @return void
 */
static inline void aiodio_status_channel_disp(struct aiodio_dev *dev,
		int8_t chan_no)
{
	uint8_t buff[20];
	const uint8_t base = 10;

	itoa(chan_no, (char *)buff, base);
	usr_uart_write_string(dev->board_cli->uart_device, (uint8_t*)"Channel ");
	usr_uart_write_string(dev->board_cli->uart_device, buff);
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)" configuration: ");
	switch(dev->board_device->channel_modes[chan_no]) {
	case CH_MODE_ADC:
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Analog Input (ADC)\n");
		break;
	case CH_MODE_DAC:
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Analog Output (DAC)\n");
		break;
	case CH_MODE_GPIO:
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Digital Input (GPIO)\n");
		break;
	case CH_MODE_UNUSED:
	/* fallthrough */
	default:
		switch(dev->board_device->channel_offstate[chan_no]) {
		case CH_OFFSTATE_OUT_LOW:
		/* fallthrough */
		case CH_OFFSTATE_OUT_HIGH:
			usr_uart_write_string(dev->board_cli->uart_device,
					      (uint8_t*)"Digital Output (GPIO)\n");
			break;
		default:
			usr_uart_write_string(dev->board_cli->uart_device,
					      (uint8_t*)"UNUSED\n");
			break;
		}
		break;
	}
}

/**
 * Display the status of the application.
 *
 * This includes which device is detected, what is the state of the channels and
 * if any of the modes is active.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_status(struct aiodio_dev *dev, uint8_t *arg)
{
	int8_t i;
	if(dev->board_device->ops->reg_read == ad5592r_rw_ops.reg_read)
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"AD5592R device active.\n");
	else if(dev->board_device->ops->reg_read == ad5593r_rw_ops.reg_read)
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"AD5593R device active.\n");

	for(i = 7; i >= 0; --i)
		aiodio_status_channel_disp(dev, 7 - i);

	if(dev->mirror_mode)
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Mirror mode active.\n");
	if(dev->test_mode)
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Production test mode active.\n");

	return 0;
}

/**
 * Reset registers regarding one channel for the driver to be able to change
 * it's configuration.
 *
 * This is a helper function to multiple functions in this module.
 *
 * @param [in] dev     - The device structure.
 * @param [in] chan_no - Number of the channel to be reset.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t aiodio_reset_channel(struct aiodio_dev *dev, uint8_t chan_no)
{
	dev->board_device->channel_modes[chan_no] = CH_MODE_UNUSED;
	dev->board_device->channel_offstate[chan_no] = CH_OFFSTATE_PULLDOWN;
	dev->board_device->gpio_map &= ~BIT(chan_no);
	dev->board_device->gpio_in &= ~BIT(chan_no);
	dev->board_device->gpio_out &= ~BIT(chan_no);
	return ad5592r_set_channel_modes(dev->board_device);
}

/**
 * Display a short tutorial for using the 'ao' command in case of mistake.
 *
 * aiodio_analog_out() helper function.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void aiodio_analog_out_prompt(struct aiodio_dev *dev)
{
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"Analog Output command model:\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"ao x yyyy\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"Where:\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"\tx    = channel number (0-7);\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"\tyyyy = DAC code in decimal integer format (0-4095).\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"The spaces <' '> are mandatory.\n");
}

/**
 * Set a channel as analog output (DAC channel) and set the output code.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - The number of the channel (0-7) and the output code
 *                   (0-4095) divided by space. The code is decimal integer.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_analog_out(struct aiodio_dev *dev, uint8_t *arg)
{
	uint8_t *chan_no_ptr, *dac_code_ptr;
	uint8_t chan_no;
	uint16_t dac_code;
	int32_t ret;

	if(!arg) {
		aiodio_analog_out_prompt(dev);
		return 0;
	}

	if(arg[1] != ' ') {
		aiodio_analog_out_prompt(dev);
		return 0;
	}

	chan_no_ptr = arg;
	chan_no_ptr[1] = '\0';
	chan_no = atoi((char *)chan_no_ptr);
	if(chan_no > 7) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"The channel number needs to be between 0 and 7.\n");
		return 0;
	}

	dac_code_ptr = arg + 2;
	if(*dac_code_ptr == '\0') {
		aiodio_analog_out_prompt(dev);
	}
	dac_code = atoi((char *)dac_code_ptr);
	if(dac_code > 4095) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"The DAC code needs to be between 0 and 4095.\n");
		return 0;
	}

	aiodio_reset_channel(dev, chan_no);

	dev->board_device->channel_modes[chan_no] = CH_MODE_DAC;
	ret = ad5592r_set_channel_modes(dev->board_device);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Channel mode error.\n");
		return ret;
	}
	ret = dev->write_dac(dev->board_device, chan_no, (uint16_t)dac_code);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Channel set error.\n");
		return ret;
	}

	dev->mirror_mode = 0;
	dev->test_mode = 0;

	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"DAC channel ");
	usr_uart_write_string(dev->board_cli->uart_device, chan_no_ptr);
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)" set to ");
	usr_uart_write_string(dev->board_cli->uart_device, dac_code_ptr);
	return usr_uart_write_string(dev->board_cli->uart_device,
				     (uint8_t*)".\n");
}

/**
 * Display a short tutorial for using the 'ai' command in case of mistake.
 *
 * aiodio_analog_in() helper function.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void aiodio_analog_in_prompt(struct aiodio_dev *dev)
{
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"Analog Input command model:\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"ai x\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"Where:\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"\tx = channel number (0-7);\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"The spaces <' '> are mandatory.\n");
}

/**
 * Set a channel as analog input (ADC channel) and read the input.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - The number of the channel (0-7).
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_analog_in(struct aiodio_dev *dev, uint8_t *arg)
{
	uint8_t chan_no, buff[20], base = 10;
	int32_t ret;
	uint16_t adc_val;

	if(!arg) {
		aiodio_analog_in_prompt(dev);
		return 0;
	}

	chan_no = atoi((char *)arg);
	if(chan_no > 7) {
		aiodio_analog_in_prompt(dev);
		return 0;
	}

	aiodio_reset_channel(dev, chan_no);

	dev->board_device->channel_modes[chan_no] = CH_MODE_ADC;
	ret = ad5592r_set_channel_modes(dev->board_device);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Channel mode error.\n");
		return ret;
	}
	ret = dev->read_adc(dev->board_device, chan_no, &adc_val);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Channel set error.\n");
		return ret;
	}
	adc_val &= 0xfff;

	dev->mirror_mode = 0;
	dev->test_mode = 0;

	itoa(adc_val, (char *)buff, base);
	usr_uart_write_string(dev->board_cli->uart_device, (uint8_t*)"Channel ");
	usr_uart_write_string(dev->board_cli->uart_device, arg);
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)" ADC value: ");
	usr_uart_write_string(dev->board_cli->uart_device, buff);
	return usr_uart_write_string(dev->board_cli->uart_device, (uint8_t*)"\n");
}

/**
 * Display a short tutorial for using the 'do' command in case of mistake.
 *
 * aiodio_digital_out() helper function.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void aiodio_digital_out_prompt(struct aiodio_dev *dev)
{
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"Digital Output command model:\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"do x y\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"Where:\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"\tx = channel number (0-7);\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"\ty = '1' or '0' for high and low, respectively.\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"The spaces <' '> are mandatory.\n");
}

/**
 * Set a channel as digital output (GPIO channel) and set the output state.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - The number of the channel (0-7) and the output state (0-1)
 *                   for 'LOW' and 'HIGH', respectively.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_digital_out(struct aiodio_dev *dev, uint8_t *arg)
{
	uint8_t *chan_no_ptr, *level_ptr;
	uint8_t chan_no, level;
	int32_t ret;

	if(!arg) {
		aiodio_digital_out_prompt(dev);
		return 0;
	}

	if(arg[1] != ' ') {
		aiodio_digital_out_prompt(dev);
		return 0;
	}

	chan_no_ptr = arg;
	chan_no_ptr[1] = '\0';
	chan_no = atoi((char *)chan_no_ptr);
	if(chan_no > 7) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"The channel number needs to be between 0 and 7.\n");
		return 0;
	}

	level_ptr = arg + 2;
	if(*level_ptr == '\0') {
		aiodio_digital_out_prompt(dev);
	}
	level = atoi((char *)level_ptr);
	if(level > 1) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"The level can only be either '0' or '1'.\n");
		return 0;
	}

	dev->board_device->channel_modes[chan_no] = CH_MODE_UNUSED;
	dev->board_device->channel_offstate[chan_no] = CH_OFFSTATE_OUT_LOW;
	ret = ad5592r_set_channel_modes(dev->board_device);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Channel mode error.\n");
		return ret;
	}
	ad5592r_gpio_set(dev->board_device, chan_no, level);

	dev->mirror_mode = 0;
	dev->test_mode = 0;

	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"Digital output channel ");
	usr_uart_write_string(dev->board_cli->uart_device, chan_no_ptr);
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)" set to ");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t *)(level ? "HIGH" : "LOW"));
	return usr_uart_write_string(dev->board_cli->uart_device,
				     (uint8_t*)".\n");
}

/**
 * Display a short tutorial for using the 'di' command in case of mistake.
 *
 * aiodio_digital_in() helper function.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void aiodio_digital_in_prompt(struct aiodio_dev *dev)
{
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"Digital Input command model:\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"di x\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"Where:\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"\tx = channel number (0-7);\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)"The spaces <' '> are mandatory.\n");
}

/**
 * Set a channel as digital input (GPIO channel) and read the state.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - The number of the channel (0-7).
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_digital_in(struct aiodio_dev *dev, uint8_t *arg)
{
	uint8_t chan_no;
	int32_t ret, gpio_temp;

	if(!arg) {
		aiodio_digital_in_prompt(dev);
		return 0;
	}

	chan_no = atoi((char *)arg);
	if(chan_no > 7) {
		aiodio_digital_in_prompt(dev);
		return 0;
	}

	aiodio_reset_channel(dev, chan_no);

	dev->board_device->channel_modes[chan_no] = CH_MODE_GPIO;
	ret = ad5592r_set_channel_modes(dev->board_device);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Channel mode error.\n");
		return ret;
	}

	gpio_temp = ad5592r_gpio_get(dev->board_device, chan_no);

	dev->mirror_mode = 0;
	dev->test_mode = 0;

	usr_uart_write_string(dev->board_cli->uart_device, (uint8_t*)"Channel ");
	usr_uart_write_string(dev->board_cli->uart_device, arg);
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t*)" digital value: ");
	return usr_uart_write_string(dev->board_cli->uart_device,
				     (uint8_t*)(gpio_temp ? "HIGH\n" : "LOW\n"));
}

/**
 * Activate mirror mode.
 *
 * In mirror mode, the channels set in a specific way: 4 inputs and 4 outputs.
 * Channels are set up as follows:
 * CH0 as Analog Input linked to CH7 as Analog Output;
 * CH1 as Analog Input linked to CH6 as Analog Output;
 * CH2 as Digital Input linked to CH5 as Digital Output;
 * CH3 as Digital Input linked to CH4 as Digital Output.
 * Once every 100 ms the input channels are read and output channels are
 * updated.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_mirror_mode(struct aiodio_dev *dev, uint8_t *arg)
{
	int32_t ret;
	uint8_t i;
	const float mirror_rate = 10.0;

	for(i = 0; i < 8; i++)
		aiodio_reset_channel(dev, i);

	aiodio_setup_mirror_channels(dev);

	ret = ad5592r_set_channel_modes(dev->board_device);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Channel mode error.\n");
		return ret;
	}
	ret = timer_counter_set_rate(dev->mode_delay_timer, mirror_rate);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Timer set delay error.\n");
		return ret;
	}
	dev->mirror_mode = 1;
	dev->test_mode = 0;
	mode_timer_flag = 0;

	return usr_uart_write_string(dev->board_cli->uart_device,
				     (uint8_t*)"Mirror mode activated!\n");
}

/**
 * Set channels for the production test mode.
 *
 * aiodio_prod_test_mode() helper function.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static inline void aiodio_setup_test_channels(struct aiodio_dev *dev)
{
	dev->board_device->channel_modes[AIODIO_CH0] = CH_MODE_DAC;
	dev->board_device->channel_modes[AIODIO_CH1] = CH_MODE_DAC;
	dev->board_device->channel_modes[AIODIO_CH2] = CH_MODE_DAC;
	dev->board_device->channel_modes[AIODIO_CH3] = CH_MODE_DAC;
	dev->board_device->channel_modes[AIODIO_CH4] = CH_MODE_ADC;
	dev->board_device->channel_modes[AIODIO_CH5] = CH_MODE_ADC;
	dev->board_device->channel_modes[AIODIO_CH6] = CH_MODE_ADC;
	dev->board_device->channel_modes[AIODIO_CH7] = CH_MODE_ADC;
	dev->board_device->num_channels = 8;
}

/**
 * Activate production test mode.
 *
 * This mode is used for production test of the boards. 4 channles are inputs
 * and 4 outputs. The outputs each have a specific value different from 0 or
 * full scale. Each input is assigned an output and when each input reads the
 * correct output twice in succession the board is passed. PASS!! message is
 * written to the CLI in ASCII art and the leds are blinked on the ADICUP3029
 * for a second. The read is done twice a second.
 * Channels are set up as follows:
 * CH0 as Analog Output linked to CH7 as Analog Input;
 * CH1 as Analog Output linked to CH6 as Analog Input;
 * CH2 as Analog Output linked to CH5 as Analog Input;
 * CH3 as Analog Output linked to CH4 as Analog Input.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_prod_test_mode(struct aiodio_dev *dev, uint8_t *arg)
{
	const float test_rate = 2.0;
	int8_t i;
	int32_t ret;

	for(i = 0; i < 8; i++)
		aiodio_reset_channel(dev, i);

	aiodio_setup_test_channels(dev);
	ret = ad5592r_set_channel_modes(dev->board_device);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Channel mode error.\n");
		return ret;
	}
	ret = timer_counter_set_rate(dev->mode_delay_timer, test_rate);
	if(ret != 0) {
		usr_uart_write_string(dev->board_cli->uart_device,
				      (uint8_t*)"Timer set delay error.\n");
		return ret;
	}
	for(i = 3; i >= 0; i--)
		dev->board_device->ops->write_dac(dev->board_device, i, staircase[i]);
	dev->mirror_mode = 0;
	dev->test_mode = 1;
	mode_timer_flag = 0;

	return usr_uart_write_string(dev->board_cli->uart_device,
				     (uint8_t*)"Production test mode activated!\n");
}

/**
 * Implement the mirror mode algorithm.
 *
 * aiodio_process() helper function.
 *
 * @param [in] dev - Pointer to the application structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static inline int32_t aiodio_process_mirror(struct aiodio_dev *dev)
{
	int32_t gpio_temp, ret;
	uint16_t analog_temp;

	gpio_temp = ad5592r_gpio_get(dev->board_device, AIODIO_CH2);
	ret = ad5592r_gpio_set(dev->board_device, AIODIO_CH5, gpio_temp);
	if(ret != 0)
		return ret;

	gpio_temp = ad5592r_gpio_get(dev->board_device, AIODIO_CH3);
	ret = ad5592r_gpio_set(dev->board_device, AIODIO_CH4, gpio_temp);
	if(ret != 0)
		return ret;

	ret = dev->read_adc(dev->board_device, AIODIO_CH0, &analog_temp);
	if(ret != 0)
		return ret;
	analog_temp &= 0xfff;
	ret = dev->write_dac(dev->board_device, AIODIO_CH7, analog_temp);
	if(ret != 0)
		return ret;
	ret = dev->read_adc(dev->board_device, AIODIO_CH1, &analog_temp);
	if(ret != 0)
		return ret;
	analog_temp &= 0xfff;
	ret = dev->write_dac(dev->board_device, AIODIO_CH6, analog_temp);

	mode_timer_flag = 0;

	return ret;
}

/**
 * Print the ASCII art 'PASS!!' message when a production test is passed.
 *
 * @param [in] dev - Pointer to the application structure.
 *
 * @return void
 */
static inline void aiodio_print_pass(struct aiodio_dev *dev)
{
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t *)"\n######     #     #####   #####  ### ###\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t *)"#     #   # #   #     # #     # ### ###\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t *)"#     #  #   #  #       #       ### ###\n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t *)"######  #     #  #####   #####   #   # \n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t *)"#       #######       #       # \n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t *)"#       #     # #     # #     # ### ### \n");
	usr_uart_write_string(dev->board_cli->uart_device,
			      (uint8_t *)"#       #     #  #####   #####  ### ### \n\n\n");

}

/**
 * Implement the production test mode algorithm.
 *
 * aiodio_process() helper function.
 *
 * @param [in] dev - Pointer to the application structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static inline int32_t aiodio_process_test(struct aiodio_dev *dev)
{
	int32_t ret;
	uint16_t analog_temp, min, max;
	const uint16_t test_tol = 82; /* Test tolerance */
	int8_t i;

	ret = timer_counter_activate(dev->blink_delay_timer, false);
	if(ret != 0)
		return ret;

	for(i = 3; i >= 0; --i) {
		ret = dev->read_adc(dev->board_device, (i + 4), &analog_temp);
		if(ret != 0)
			return ret;
		analog_temp &= 0xFFF;
		min = staircase[3 - i] - test_tol;
		max = staircase[3 - i] + test_tol;
		if((analog_temp < min) || (analog_temp > max)) {
			i = 10;
			break;
		}
	}
	if(i == -1)
		dev->test_success++;
	else
		dev->test_success = 0;

	if(dev->test_success == 2) {
		test_blink_count = 4;
		ret = timer_counter_activate(dev->blink_delay_timer, true);
		if(ret != 0)
			return ret;
		aiodio_print_pass(dev);
		dev->test_success = 0;
	}
	mode_timer_flag = 0;

	return ret;
}

/**
 * Application process.
 *
 * Read the 4 input channels and mirror their values on the output channels.
 *
 * @param [in] dev - Pointer to the application structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t aiodio_process(struct aiodio_dev *dev)
{
	int32_t ret;

	NVIC_DisableIRQ(TMR0_INT);
	NVIC_DisableIRQ(TMR1_INT);
	ret = cli_process(dev->board_cli);
	if(ret != 0)
		return ret;
	NVIC_EnableIRQ(TMR0_INT);
	NVIC_EnableIRQ(TMR1_INT);

	if(dev->mirror_mode && mode_timer_flag) {
		ret = aiodio_process_mirror(dev);
		if(ret != 0)
			return ret;
	}

	if(dev->test_mode && mode_timer_flag && !test_blink_count) {
		ret = aiodio_process_test(dev);
		if(ret != 0)
			return ret;
	}

	return ret;
}
