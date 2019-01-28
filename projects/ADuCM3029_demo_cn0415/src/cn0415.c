/***************************************************************************//**
 *   @file   cn0415.c
 *   @brief  ADICUP3029 CN0415 application source.
 *   @author Andrei Drimbarean (Andrei.Drimbarean@analog.com)
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "config.h"
#include "cn0415.h"
#include "math.h"

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
extern float dith_ampl;
extern uint8_t uart_current_line[256];
extern uint8_t uart_previous_line[256];
extern uint8_t uart_line_index;
extern uint8_t uart_cmd;
extern uint8_t pid_flag;
extern bool dither_period;

/* Available commands */
char *cmd_commands[] = {
	"help",
	"h",
	"frequency ",
	"f ",
	"duty_cycle ",
	"d ",
	"read_adc",
	"r",
	"activate_dither",
	"ad",
	"remove_dither",
	"rd",
	"dither_frequency ",
	"df ",
	"dither_amplitude ",
	"da ",
	"set_pid_freq ",
	"spf ",
	"set_pid_sp ",
	"sps ",
	"add_pid",
	"ap",
	"remove_pid",
	"rp",
	"set_pid_kp ",
	"kp ",
	"set_pid_ki ",
	"ki ",
	"set_pid_kd ",
	"kd ",
	"set_pulse_value ",
	"spuv ",
	"set_pulse_period ",
	"spup ",
	"dig_sol_on",
	"dson",
	"dig_sol_off",
	"dsof",
	"prop_sol_set ",
	"pss ",
	"reset",
	"rst",
	"status",
	"stts",
	"calibrate",
	"clb",
	""
};

/* Functions for available commands */
cmd_func v_cmd_fun[] = {
	cn0415_help,
	cn0415_pwm_set_freq,
	cn0415_pwm_set_duty,
	cn0415_read_voltage,
	cn0415_dither_activate,
	cn0415_dither_remove,
	cn0415_dither_set_freq,
	cn0415_dither_set_ampl,
	cn0415_pid_set_sample_frequency,
	cn0415_pid_set_setpoint,
	cn0415_pid_add,
	cn0415_pid_remove,
	cn0415_pid_set_kp,
	cn0415_pid_set_ki,
	cn0415_pid_set_kd,
	cn0415_pulse_set_val,
	cn0415_pulse_set_period,
	cn0415_dig_sol_on,
	cn0415_dig_sol_off,
	cn0415_prop_sol_set,
	cn0415_reset,
	cn0415_status,
	cn0415_calibration,
	NULL
};

uint8_t command_size[] = {5, 2, 10, 2, 11, 2, 9, 2, 16, 3, 14, 3, 17, 3, 17, 3,
			  13, 4, 11, 4, 8, 3, 11, 3, 11, 3, 11, 3, 11, 3, 16, 5, 17, 5, 11,
			  5, 12, 5, 13, 4, 6, 4, 7, 5, 10, 4, 1};

float offset_err, gain_err = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Initializes the cn0415 device.
 *
 * @param [out] device     - The device structure.
 * @param [in]  init_param - The structure that contains the device initial
 *                           parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_setup(struct cn0415_dev **device,
		     struct cn0415_init_param init_param)
{
	struct cn0415_dev *dev;
	int32_t ret;

	/* Allocate memory for board descriptor */
	dev = (struct cn0415_dev *)malloc(sizeof(*dev));
	if(!dev)
		return -1;

	/* Initialize Systick timer */
	ret = timer_start();
	if(ret != 0)
		goto error;
#ifdef FLASH_INIT_PARAMETERS
	/* Get flash parameters */
	ret = flash_read(dev->param_array, FLASH_PARAM_SIZE);
	if(ret != 0)
		goto error;
#endif

	/* Initialize UART controller */
	ret = usr_uart_init(&dev->aducm3029_uart_desc,
			    init_param.aducm3029_uart_init_param);
	if(ret != 0)
		goto error;
#ifdef FLASH_INIT_PARAMETERS
	/* Get PWM frequency from flash if necessary */
	init_param.aducm3029_pwm_init_param.frequency = dev->param_array[F_PWM];
#endif

	/* Initialize PWM timer */
	ret = pwm_setup(&dev->aducm3029_pwm_desc,
			init_param.aducm3029_pwm_init_param);
	if(ret != 0)
		goto error;
#ifdef FLASH_INIT_PARAMETERS
	/* Get dither frequency and amplitude from flash if necessary */
	init_param.aducm3029_pwm_init_param.dither_amplitude =
		dev->param_array[DITH_AMPL_INT];
	init_param.aducm3029_pwm_init_param.dither_amplitude +=
		dev->param_array[DITH_AMPL_DEC] / 10000;
	init_param.aducm3029_pwm_init_param.dither_frequency =
		dev->param_array[DITH_FREQ];
#endif

	/* Initialize PWM dither timer */
	ret = pwm_dither_setup(dev->aducm3029_pwm_desc,
			       init_param.aducm3029_pwm_init_param);
	if(ret != 0)
		goto error;

	/* Initialize setup the LTC2312 device and SPI controller */
	ret = ltc2312_setup(&dev->ltc2312_device, &init_param.ltc2312_init);
	if(ret != 0)
		goto error;

	/* Initialize Flash controller */
	ret = flash_init();
	if(ret != 0)
		goto error;

	/* Initialize GPIOs */
	ret = gpio_get(&dev->gpio_fault_detect, GPIO_FAULT_DETECT);
	if(ret != 0)
		goto error;
	ret = gpio_get(&dev->gpio_oc_reset, GPIO_OC_RESET);
	if(ret != 0)
		goto error;
	ret = gpio_get(&dev->gpio_lt4367_fault, GPIO_LT4367_FAULT);
	if(ret != 0)
		goto error;
	ret = gpio_direction_input(dev->gpio_fault_detect);
	if(ret != 0)
		goto error;
	ret = gpio_direction_input(dev->gpio_lt4367_fault);
	if(ret != 0)
		goto error;
	ret = gpio_direction_output(dev->gpio_oc_reset, 0);
	if(ret != 0)
		goto error;

#ifdef FLASH_INIT_PARAMETERS
	/* Get PID controller parameters from flash if necessary */
	init_param.controller_init.f_loop = dev->param_array[F_LOOP];
	init_param.controller_init.kp = dev->param_array[PID_KP_INT];
	init_param.controller_init.kp +=
		(float)dev->param_array[PID_KP_DEC] / 10000;
	init_param.controller_init.ki = dev->param_array[PID_KI_INT];
	init_param.controller_init.ki +=
		(float)dev->param_array[PID_KI_DEC] / 10000;
	init_param.controller_init.kd = dev->param_array[PID_KD_INT];
	init_param.controller_init.kd +=
		(float)dev->param_array[PID_KD_DEC] / 10000;
	init_param.controller_init.setpoint = dev->param_array[HOLD_VAL_INT];
	init_param.controller_init.setpoint +=
		(float)dev->param_array[HOLD_VAL_DEC] / 10000;

	/* Get pulse parameters */
	init_param.pulse_val = dev->param_array[PULSE_VAL_INT];
	init_param.pulse_val +=
		(float)dev->param_array[PULSE_VAL_DEC] / 10000;
	init_param.t_pulse = dev->param_array[T_PULSE];
#endif

	/* Initialize PID controller */
	ret = pid_setup(&dev->controller, init_param.controller_init);
	if(ret != 0)
		goto error;

#ifndef FLASH_INIT_PARAMETERS
	dev->param_array[F_PWM] = dev->aducm3029_pwm_desc->frequency;
	dev->param_array[F_LOOP] = dev->controller->f_loop;
	dev->param_array[PID_KP_INT] = dev->controller->kp;
	dev->param_array[PID_KP_DEC] =
		(uint32_t)(dev->controller->kp * 10000) % 10000;
	dev->param_array[PID_KI_INT] = dev->controller->ki;
	dev->param_array[PID_KI_DEC] =
		(uint32_t)(dev->controller->ki * 10000) % 10000;
	dev->param_array[PID_KD_INT] = dev->controller->kd;
	dev->param_array[PID_KD_DEC] =
		(uint32_t)(dev->controller->kp * 10000) % 10000;
	dev->param_array[PULSE_VAL_INT] = dev->pulse_val;
	dev->param_array[PULSE_VAL_DEC] =
		(uint32_t)(dev->pulse_val * 10000) % 10000;
	dev->param_array[T_PULSE] = dev->pulse_t;
	dev->param_array[HOLD_VAL_INT] = dev->controller->setpoint;
	dev->param_array[HOLD_VAL_DEC] =
		(uint32_t)(dev->controller->setpoint * 10000) % 10000;
	dev->param_array[DITH_AMPL_INT] = dev->aducm3029_pwm_desc->dither_amplitude;
	dev->param_array[DITH_AMPL_DEC] =
		(uint32_t)(dev->aducm3029_pwm_desc->dither_amplitude * 10000) %
		10000;
	dev->param_array[DITH_FREQ] = dev->aducm3029_pwm_desc->dither_frequency;

	ret = flash_write(dev->param_array, FLASH_PARAM_SIZE);
	if(ret != 0)
		goto error;
#endif

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free the resources allocated by cn0415_setup().
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_remove(struct cn0415_dev *dev)
{
	int32_t ret = 0;

	/* Free UART driver memory */
	ret = usr_uart_remove(dev->aducm3029_uart_desc);
	if(ret != 0)
		return ret;

	/* Free PWM timer memory */
	ret = pwm_remove(dev->aducm3029_pwm_desc);
	if(ret != 0)
		return ret;

	/* Free LTC2312 driver memory */
	ret = ltc2312_remove(dev->ltc2312_device);
	if(ret != 0)
		return ret;

	/* GPIO remove and free memory */
	ret = gpio_remove(dev->gpio_fault_detect);
	if(ret != 0)
		return ret;
	ret = gpio_remove(dev->gpio_lt4367_fault);
	if(ret != 0)
		return ret;
	ret = gpio_remove(dev->gpio_oc_reset);
	if(ret != 0)
		return ret;

	/* Free PID controller memory */
	pid_remove(dev->controller);

	/* Free device driver memory */
	free(dev);

	return ret;
}

/**
 * Set pulse value for when the solenoid needs to pulse before reaching hold
 * value.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - New pulse value in A or 0.01% duty cycle.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pulse_set_val(struct cn0415_dev *dev, uint8_t *args)
{
	/* Set pulse value */
	dev->pulse_val = atof((char *)args);

	/* Save in flash */
	dev->param_array[PULSE_VAL_INT] = dev->pulse_val;
	dev->param_array[PULSE_VAL_DEC] =
		(uint32_t)(dev->pulse_val * 10000) % 10000;

	return flash_write(dev->param_array, FLASH_PARAM_SIZE);
}

/**
 * Set pulse period for when the solenoid needs to pulse before reaching hold
 * value.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - New pulse period in ms.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pulse_set_period(struct cn0415_dev *dev, uint8_t *args)
{
	/* Set pulse period */
	dev->pulse_t = atoi((char *)args);

	/* Save in flash */
	dev->param_array[T_PULSE] = dev->pulse_t;

	return flash_write(dev->param_array, FLASH_PARAM_SIZE);
}

/**
 * Set solenoid to zero value.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_dig_sol_off(struct cn0415_dev *dev, uint8_t *args)
{
	int32_t ret;

	/* Change to hold value */
	if(dev->controller->setpoint > 1) {
		return pwm_set_duty_cycle(dev->aducm3029_pwm_desc, 0);
	} else {
		ret = pid_activate(dev->controller, false);
		if(ret != 0)
			return ret;
		dev->pid = 0;

		return pwm_set_duty_cycle(dev->aducm3029_pwm_desc, 0);
	}
}

/**
 * Set solenoid hold value.
 *
 * If a pulse value and time is present the actuator will first pulse with the
 * value for the corresponding time then go back to hold value.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_dig_sol_on(struct cn0415_dev *dev, uint8_t *args)
{
	int32_t ret;
	int16_t pid_ctrl_adjust; /* Container for the PID adjustment value. */
	uint16_t command = 0;
	float temp;

	/* Pulse if necessary */
	if(dev->pulse_val != 0 && dev->pulse_t != 0) {
		/* If pulse value is in duty cycle */
		if(dev->pulse_val > 1) {
			ret = pwm_set_duty_cycle(dev->aducm3029_pwm_desc, dev->pulse_val);
			if(ret != 0)
				return ret;
			mdelay(dev->pulse_t);
			/* If pulse value is in Amperes */
		} else {
			temp = dev->controller->setpoint;
			dev->controller->setpoint = dev->pulse_val;
			do {
				ret = cn0415_pid_controller(dev, &pid_ctrl_adjust);
				if(ret != 0)
					return ret;
				if((0 - pid_ctrl_adjust) > dev->aducm3029_pwm_desc->duty_cycle)
					command = 0;
				else
					command = dev->aducm3029_pwm_desc->duty_cycle +
						  pid_ctrl_adjust;
				if(command > 10000)
					command = 10000;

				/* Update duty cycle */
				ret = pwm_set_duty_cycle(dev->aducm3029_pwm_desc, command);
				if(ret != 0)
					return ret;
			} while(pid_ctrl_adjust != 0);
			dev->controller->setpoint = temp;
			mdelay(dev->pulse_t);
		}
	}

	/* Change to hold value */
	if(dev->controller->setpoint > 1) {
		return pwm_set_duty_cycle(dev->aducm3029_pwm_desc,
					  dev->controller->setpoint);
	} else {
		dev->pid = 1;

		return pid_activate(dev->controller, true);
	}
}

/**
 * Set proportional solenoid on.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Proportional solenoid setpoint.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_prop_sol_set(struct cn0415_dev *dev, uint8_t *setpoint)
{
	int32_t ret = 0;
	float fsetpoint;

	/* If dither parameters are set activate dither */
	if(dev->aducm3029_pwm_desc->dither_amplitude != 0 &&
	    dev->aducm3029_pwm_desc->dither_frequency != 0) {
		ret = pwm_dither_activate(dev->aducm3029_pwm_desc);
		if(ret != 0)
			return ret;
	}

	/* Activate proportional solenoid */
	fsetpoint = atof((char *)setpoint);
	if(fsetpoint == 0) {
		ret = pwm_set_duty_cycle(dev->aducm3029_pwm_desc, (uint16_t)fsetpoint);
		if(ret != 0)
			return ret;

		return cn0415_pid_remove(dev, NULL);
	}
	if(fsetpoint <= 1) {
		ret = cn0415_pid_set_setpoint(dev, setpoint);
		if(ret != 0)
			return ret;

		return cn0415_pid_add(dev, NULL);
	}

	return ret;
}

/**
 * Helper function for the cn0415_reset(). Resets the PWM parameters.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0415_reset_pwm_param(struct cn0415_dev *dev)
{
	int32_t ret;

	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Reset PWM parameters...\n");
	ret = cn0415_pwm_set_freq(dev, (uint8_t*)"1000");
	if(ret != 0)
		return ret;
	ret = cn0415_pwm_set_duty(dev, (uint8_t*)"0");
	if(ret != 0)
		return ret;

	return usr_uart_write_string(dev->aducm3029_uart_desc,
				     (uint8_t*)"PWM frequency set to 1000Hz.\n");
}

/**
 * Helper function for the cn0415_reset(). Resets the PID parameters.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0415_reset_pid_param(struct cn0415_dev *dev)
{
	int32_t ret;

	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Reset PID parameters...\n");
	ret = cn0415_pid_remove(dev, NULL);
	if(ret != 0)
		return ret;
	ret = cn0415_pid_set_kp(dev, (uint8_t*)"0");
	if(ret != 0)
		return ret;
	ret = cn0415_pid_set_ki(dev, (uint8_t*)"0");
	if(ret != 0)
		return ret;
	ret = cn0415_pid_set_kd(dev, (uint8_t*)"0");
	if(ret != 0)
		return ret;
	ret = cn0415_pid_set_sample_frequency(dev, (uint8_t*)"100");
	if(ret != 0)
		return ret;
	ret = cn0415_pid_set_setpoint(dev, (uint8_t*)"0");
	if(ret != 0)
		return ret;

	return usr_uart_write_string(dev->aducm3029_uart_desc,
				     (uint8_t*)"PID controller frequency set to 100Hz.\n");
}

/**
 * Helper function for the cn0415_reset(). Resets the dither parameters.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0415_reset_dither_param(struct cn0415_dev *dev)
{
	int32_t ret;

	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Reset dither parameters...\n");
	ret = cn0415_dither_remove(dev, NULL);
	if(ret != 0)
		return ret;
	ret = cn0415_dither_set_freq(dev, (uint8_t*)"50");
	if(ret != 0)
		return ret;
	ret = cn0415_dither_set_ampl(dev, (uint8_t*)"-c 0");
	if(ret != 0)
		return ret;

	return usr_uart_write_string(dev->aducm3029_uart_desc,
				     (uint8_t*)"Dither frequency set to 50Hz.\n");
}

/**
 * Helper function for the cn0415_reset(). Resets the pulse parameters.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0415_reset_pulse_param(struct cn0415_dev *dev)
{
	int32_t ret;

	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Reset pulse parameters...\n");
	ret = cn0415_pulse_set_val(dev, (uint8_t*)"0");
	if(ret != 0)
		return ret;

	return cn0415_pulse_set_period(dev, (uint8_t*)"0");
}

/**
 * Helper function for the cn0415_reset(). Resets the faults parameters.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0415_reset_fault_param(struct cn0415_dev *dev)
{
	int32_t ret;

	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Faults reset...\n");
	ret = gpio_set_value(dev->gpio_oc_reset, 1);
	if(ret != 0)
		return ret;
	mdelay(1);

	return gpio_set_value(dev->gpio_oc_reset, 0);
}

/**
 * Reset controller, parameter and faults.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_reset(struct cn0415_dev *dev, uint8_t *args)
{
	int32_t ret;

	/* Reset PWM parameters */
	ret = cn0415_reset_pwm_param(dev);
	if(ret != 0)
		return ret;

	/* Reset PID parameters */
	ret = cn0415_reset_pid_param(dev);
	if(ret != 0)
		return ret;

	/* Reset dither parameters */
	ret = cn0415_reset_dither_param(dev);
	if(ret != 0)
		return ret;

	/* Reset pulse parameters */
	ret = cn0415_reset_pulse_param(dev);
	if(ret != 0)
		return ret;

	/* Faults reset */
	ret = cn0415_reset_fault_param(dev);
	if(ret != 0)
		return ret;

	/* Reset complete */
	return usr_uart_write_string(dev->aducm3029_uart_desc,
				     (uint8_t*)"Reset complete!\n");
}

/**
 * Helper function for the cn0415_status(). Display the parameters related to
 * the PWM controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0415_status_pwm(struct cn0415_dev *dev)
{
	uint8_t buffer[20];

	/* PWM frequency */
	itoa(dev->aducm3029_pwm_desc->frequency, (char *)buffer, 10);
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    PWM frequency: ");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t *)buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"Hz\n");
	/* PWM duty cycle */
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    PWM duty cycle: ");
	itoa((dev->aducm3029_pwm_desc->duty_cycle / 100), (char *)buffer, 10);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t *)buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t *)".");
	itoa((dev->aducm3029_pwm_desc->duty_cycle % 100), (char *)buffer, 10);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t *)buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t *)"%\n");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t *)"\n");
}

/**
 * Helper function for the cn0415_status(). Display the parameters related to
 * the dither controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0415_status_dither(struct cn0415_dev *dev)
{
	uint8_t buffer[20];

	/* Dither frequency */
	itoa(dev->aducm3029_pwm_desc->dither_frequency, (char *)buffer, 10);
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    Dither frequency: ");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"Hz\n");
	/* Dither amplitude */
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    Dither amplitude: ");
	if(dev->aducm3029_pwm_desc->dither_amplitude > 1) {
		itoa((uint32_t)(dev->aducm3029_pwm_desc->dither_amplitude / 100),
		     (char *)buffer, 10);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)".");
		itoa(((uint32_t)dev->aducm3029_pwm_desc->dither_amplitude % 100),
		     (char *)buffer, 10);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"%\n");
	} else {
		cn0415_ftoa((uint8_t*)buffer,
			    dev->aducm3029_pwm_desc->dither_amplitude);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"A\n");
	}
}

/**
 * Helper function for the cn0415_status_pid(). Display the frequency of the
 * update for the PID controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0415_status_pid_frequency(struct cn0415_dev *dev)
{
	uint8_t buffer[20];

	itoa(dev->controller->f_loop, (char *)buffer, 10);
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    PID loop frequency: ");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"Hz\n");
}

/**
 * Helper function for the cn0415_status_pid(). Display the Kp constant for the
 * PID controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0415_status_pid_kp(struct cn0415_dev *dev)
{
	uint8_t buffer[20];

	cn0415_ftoa((uint8_t*)buffer, dev->controller->kp);
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    PID Kp: ");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
}

/**
 * Helper function for the cn0415_status_pid(). Display the Ki constant for the
 * PID controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0415_status_pid_ki(struct cn0415_dev *dev)
{
	uint8_t buffer[20];

	cn0415_ftoa((uint8_t*)buffer, dev->controller->ki);
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    PID Ki: ");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
}

/**
 * Helper function for the cn0415_status_pid(). Display the Kd constant for the
 * PID controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0415_status_pid_kd(struct cn0415_dev *dev)
{
	uint8_t buffer[20];

	cn0415_ftoa((uint8_t*)buffer, dev->controller->kd);
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    PID Kd: ");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
}

/**
 * Helper function for the cn0415_status_pid(). Display the set-point for the
 * PID controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0415_status_pid_setpoint(struct cn0415_dev *dev)
{
	uint8_t buffer[20];

	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    PID setpoint: ");
	if(dev->controller->setpoint > 1) {
		itoa((uint32_t)(dev->controller->setpoint / 100), (char *)buffer, 10);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)".");
		itoa(((uint32_t)dev->controller->setpoint % 100), (char *)buffer, 10);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"%\n");
	} else {
		cn0415_ftoa((uint8_t*)buffer, dev->controller->setpoint);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"A\n");
	}
}

/**
 * Helper function for the cn0415_status(). Display the parameters related to
 * the PID controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0415_status_pid(struct cn0415_dev *dev)
{
	/* PID frequency */
	cn0415_status_pid_frequency(dev);
	/* PID Kp */
	cn0415_status_pid_kp(dev);
	/* PID Ki */
	cn0415_status_pid_ki(dev);
	/* PID Kd */
	cn0415_status_pid_kd(dev);
	/* PID setpoint */
	cn0415_status_pid_setpoint(dev);
}

/**
 * Helper function for the cn0415_status(). Display the parameters related to
 * the pulse controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static void cn0415_status_pulse(struct cn0415_dev *dev)
{
	uint8_t buffer[20];

	/* Pulse time */
	itoa(dev->pulse_t, (char *)buffer, 10);
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    Pulse time: ");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"ms\n");
	/* Pulse amplitude */
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"    Pulse amplitude: ");
	if(dev->pulse_val > 1) {
		itoa((uint32_t)(dev->pulse_val / 100), (char *)buffer, 10);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)".");
		itoa(((uint32_t)dev->pulse_val % 100), (char *)buffer, 10);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"%\n");
	} else {
		cn0415_ftoa((uint8_t*)buffer, dev->pulse_val);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)buffer);
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"A\n");
	}
}

/**
 * Helper function for the cn0415_status(). Display the parameters related to
 * the system faults.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0415_status_faults(struct cn0415_dev *dev)
{
	int32_t ret;
	uint16_t gpio_temp; /* Container for the GPIO value. */

	/* Over current fault */
	ret = gpio_get_value(dev->gpio_fault_detect, &gpio_temp);
	if(ret != 0)
		return ret;
	if(gpio_temp == 0)
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"    OVER CURRENT ERROR! RESET NEEDED!");
	/* Over or under voltage fault */
	ret = gpio_get_value(dev->gpio_lt4367_fault, &gpio_temp);
	if(ret != 0)
		return ret;
	if(gpio_temp == 0)
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"    SUPPLY VOLTAGE ERROR!");

	return ret;
}

/**
 * Display status of the application.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_status(struct cn0415_dev *dev, uint8_t *args)
{
	int32_t ret;

	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Status report!\n");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");

	/* PWM status */
	cn0415_status_pwm(dev);

	/* Dither status */
	cn0415_status_dither(dev);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t *)"\n");

	/* PID status */
	cn0415_status_pid(dev);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");

	/* Pulse status */
	cn0415_status_pulse(dev);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");

	/* Faults status */
	ret = cn0415_status_faults(dev);
	if(ret != 0)
		return ret;

	return usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
}

/**
 * PID controller for PWM duty cycle.
 *
 * @param [in]  dev    - The device structure.
 * @param [out] output - The calculated output in duty cycle percentage.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pid_controller(struct cn0415_dev *dev, int16_t *output)
{
	int32_t ret;
	uint16_t feedback_d;
	float feedback_f;

	/* Read feedback and transform it to current value */
	ret = ltc2312_read(dev->ltc2312_device, &feedback_d);
	if(ret != 0)
		return ret;
	ltc2312_code_to_voltage(dev->ltc2312_device, (uint16_t)feedback_d, vref,
				&feedback_f);
	feedback_f = feedback_f / (INPUT_AMPLIFIER_GAIN * SENSE_RESISTOR);

	/* Run controller evaluation */
	pid_controller_output_eval(dev->controller, feedback_f, output);

	return ret;
}

/**
 * Set proportional constant, Kp, for the PID controller.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - New value of the proportional constant.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pid_set_kp(struct cn0415_dev *dev, uint8_t *args)
{
	float kp, ki, kd;

	kp = atof((char *)args);
	ki = dev->controller->ki * dev->controller->f_loop;
	kd = dev->controller->kd / dev->controller->f_loop;

	/* Change current PID constants */
	pid_set_constants(dev->controller, kp, ki, kd);

	/* Save PID constants in non-volatile memory */
	dev->param_array[PID_KP_INT] = dev->controller->kp;
	dev->param_array[PID_KP_DEC] =
		(uint32_t)(dev->controller->kp * 10000) % 10000;

	return flash_write(dev->param_array, FLASH_PARAM_SIZE);
}

/**
 * Set integral constant, Ki, for the PID controller.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - New value of the integral constant.
 *
 * @return CN415_SUCCESS in case of success, negative error code otherwise.
 */
int32_t cn0415_pid_set_ki(struct cn0415_dev *dev, uint8_t *args)
{
	float kp, ki, kd;

	ki = atof((char *)args);
	kp = dev->controller->kp;
	kd = dev->controller->kd / dev->controller->f_loop;

	/* Change current PID constants */
	pid_set_constants(dev->controller, kp, ki, kd);

	/* Save PID constants in non-volatile memory */
	dev->param_array[PID_KI_INT] = dev->controller->ki;
	dev->param_array[PID_KI_DEC] =
		(uint32_t)(dev->controller->ki * 10000) % 10000;

	return flash_write(dev->param_array, FLASH_PARAM_SIZE);
}

/**
 * Set derivative constant, Kd, for the PID controller.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - New value of the derivative constant.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pid_set_kd(struct cn0415_dev *dev, uint8_t *args)
{
	float kp, ki, kd;

	kd = atof((char *)args);
	ki = dev->controller->ki * dev->controller->f_loop;
	kp = dev->controller->kp;

	/* Change current PID constants */
	pid_set_constants(dev->controller, kp, ki, kd);

	/* Save PID constants in non-volatile memory */
	dev->param_array[PID_KD_INT] = dev->controller->kd;
	dev->param_array[PID_KD_DEC] =
		(uint32_t)(dev->controller->kd * 10000) % 10000;

	return flash_write(dev->param_array, FLASH_PARAM_SIZE);
}

/**
 * Set sample frequency for the PID controller.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Buffer with frequency value in ASCII.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pid_set_sample_frequency(struct cn0415_dev *dev, uint8_t *args)
{
	uint16_t freq;

	freq = atoi((char *)args);

	/* Update sampling frequency */
	pid_set_sample_frequency(dev->controller, freq);

	/* Save PID constants in non-volatile memory */
	dev->param_array[F_LOOP] = dev->controller->f_loop;
	dev->param_array[PID_KI_INT] = dev->controller->ki;
	dev->param_array[PID_KI_DEC] =
		(uint32_t)(dev->controller->ki * 10000) % 10000;
	dev->param_array[PID_KD_INT] = dev->controller->kd;
	dev->param_array[PID_KD_DEC] =
		(uint32_t)(dev->controller->kd * 10000) % 10000;

	return flash_write(dev->param_array, FLASH_PARAM_SIZE);
}

/**
 * Set set-point for the PID controller.
 *
 * @param [in] dev  - The device structure.
 * @param [in] args - The new set-point.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pid_set_setpoint(struct cn0415_dev *dev, uint8_t *args)
{
	float set_point;

	set_point = atof((char *)args);

	/* Set current set-point */
	pid_set_setpoint(dev->controller, set_point);

	/* Save set-point in non-volatile memory */
	dev->param_array[HOLD_VAL_INT] = dev->controller->setpoint;
	dev->param_array[HOLD_VAL_DEC] =
		(uint32_t)(dev->controller->setpoint * 10000) % 10000;

	return flash_write(dev->param_array, FLASH_PARAM_SIZE);
}

/**
 * Set duty cycle for the PWM.
 *
 * @param [in] dev      - The device structure.
 * @param [in] new_duty - The new duty cycle.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pwm_set_duty(struct cn0415_dev *dev, uint8_t* new_duty)
{
	uint16_t iduty;

	/* Convert new duty cycle value from ASCII to integer representation */
	iduty = (uint16_t)atoi((char*)new_duty);

	/* Change PWM duty cycle */
	return pwm_set_duty_cycle(dev->aducm3029_pwm_desc, iduty);
}

/**
 * Set frequency for the PWM.
 *
 * @param [in] dev  - The device structure.
 * @param [in] freq - The new frequency.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pwm_set_freq(struct cn0415_dev *dev, uint8_t* freq)
{
	int32_t ret;
	uint32_t ifreq;

	/* Convert new frequency value from ASCII to integer representation */
	ifreq = (uint32_t)atoi((char*)freq);

	/* Change PWM frequency */
	ret = pwm_set_frequency(dev->aducm3029_pwm_desc, ifreq);
	if(ret != 0)
		return ret;

	/* Save frequency in flash */
	dev->param_array[F_PWM] = ifreq;

	return flash_write(dev->param_array, FLASH_PARAM_SIZE);
}

/**
 * Use the PID controller for the output.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pid_add(struct cn0415_dev *dev, uint8_t *args)
{
	dev->pid = 1;

	return pid_activate(dev->controller, true);
}

/**
 * Deactivate PID control.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_pid_remove(struct cn0415_dev *dev, uint8_t *args)
{
	dev->pid = 0;

	return pid_activate(dev->controller, false);
}

/**
 * Add dither to the PWM duty cycle.
 *
 * @param [in] dev        - The device structure.
 * @param [in] init_param - The structure that contains the device initial
 *                          parameters.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_add_dither(struct cn0415_dev *dev,
			  struct cn0415_init_param init_param)
{
	dev->aducm3029_pwm_desc->has_dither = 1;

	return pwm_dither_setup(dev->aducm3029_pwm_desc,
				init_param.aducm3029_pwm_init_param);
}

/**
 * Read ADC value and display the current and voltage values.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return CN415_SUCCESS in case of success, negative error code otherwise.
 */
int32_t cn0415_read_voltage(struct cn0415_dev *dev, uint8_t *arg)
{
	int32_t ret = 0;
	uint16_t adc_value;
	float voltage, current;
	float subunit;
	uint8_t buffer[20];
	int32_t fraction;
	uint8_t i;

	/* Read ADC */
	ret = ltc2312_read(dev->ltc2312_device, &adc_value);
	if(ret != 0)
		return ret;

	/* Convert ADC value to voltage */
	ltc2312_code_to_voltage(dev->ltc2312_device, adc_value, vref, &voltage);

	/* Convert floating point to ASCII and display */
	itoa((int32_t)voltage, (char*)buffer, 10);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"ADC voltage is: ");
	usr_uart_write_string(dev->aducm3029_uart_desc, buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)".");
	/* Display zeros after the decimal point. Else they would be eluded. */
	subunit = fmod(voltage, 1.0);
	i = 0;
	while(subunit < 0.1 && i < 3) {
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"0");
		subunit *= 10;
		i++;
	}
	fraction = (int32_t)(voltage * 10000) % 10000;
	itoa(fraction, (char*)buffer, 10);
	usr_uart_write_string(dev->aducm3029_uart_desc, buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"V");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");

	/* Convert to current value */
	current = voltage / (20 * r_sense);

	/* Account for calibration */
	current += gain_err;

	/* Convert floating point to ASCII and display */
	itoa((int32_t)current, (char*)buffer, 10);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"ADC current value is: ");
	usr_uart_write_string(dev->aducm3029_uart_desc, buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)".");
	/* Display zeros after the decimal point. Else they would be eluded. */
	subunit = fmod(current, 1.0);
	i = 0;
	while(subunit < 0.1 && i < 3) {
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"0");
		subunit *= 10;
		i++;
	}
	fraction = (int32_t)(current * 10000) % 10000;
	itoa(fraction, (char*)buffer, 10);
	usr_uart_write_string(dev->aducm3029_uart_desc, buffer);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"A");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");

	return ret;
}

/**
 * Helper function for cn0415_help(). Display the list of general commands for
 * the application.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void cn0415_help_general(struct cn0415_dev *dev, bool long_nshort)
{
	if (long_nshort) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" help                    - Display available commands\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" status             - Show application status and parameters.\n");
	} else {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" h          - Display available commands\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" stts       - Show application status and parameters.\n");
	}
}

/**
 * Helper function for cn0415_help(). Display the list of functions for the PWM
 * controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void cn0415_help_pwm(struct cn0415_dev *dev, bool long_nshort)
{
	if (long_nshort) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" frequency <freq>        - Set frequency to the specified value.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                           <freq> = value of the new frequency.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " duty_cycle <duty>       - Set duty cycle to the specified value.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                           <duty> = value of the new duty cycle.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                           Example: 5126 = 51.26%\n");
	} else {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" f <freq>   - Set frequency to the specified value.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              <freq> = value of the new frequency.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" d <duty>   - Set duty cycle to the specified value.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              <duty> = value of the new duty cycle.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              Example: 5126 = 51.26%\n");
	}
}

/**
 * Helper function for cn0415_help(). Display the list of functions for the ADC
 * controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void cn0415_help_adc(struct cn0415_dev *dev, bool long_nshort)
{
	if (long_nshort) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " read_adc                - Read and display the value of the ADC.\n");
	} else {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" r          - Read and display the value of the ADC.\n");
	}
}

/**
 * Helper function for cn0415_help(). Display the list of functions for the
 * dither controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void cn0415_help_dither(struct cn0415_dev *dev, bool long_nshort)
{
	if (long_nshort) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" activate_dither         - Activate PWM dither functionality.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" remove_dither           - Deactivate PWM dither functionality.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " dither_frequency <freq> - Set dither frequency to the given value.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                           <freq> = value of the new frequency.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" dither_amplitude <duty> - Set dither amplitude.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                           <duty> = value of the amplitude in duty cycle or amps.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                           Use parameter -c for current and -d for duty cycle.\n");
	} else {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" ad         - Activate PWM dither functionality.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" rd         - Deactivate PWM dither functionality.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" df <freq>  - Set dither frequency to the given value.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              <freq> = value of the new frequency.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" da <duty>  - Set dither amplitude.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "              <duty> = value of the amplitude in duty cycle variation.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "              Use parameter -c for current and -d for duty cycle.\n");
	}
}

/**
 * Helper function for cn0415_help(). Display the list of functions for the PID
 * controller.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void cn0415_help_pid(struct cn0415_dev *dev, bool long_nshort)
{
	if (long_nshort) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" add_pid                 - Activate PID controller.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" remove_pid              - Deactivate PID controller.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" set_pid_kp <kp>         - Set PID Kp constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                           <kp> = value of the new Kp constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" set_pid_ki <ki>         - Set PID Ki constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                           <ki> = value of the new Ki constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" set_pid_kd <kd>         - Set PID Kd constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                           <kd> = value of the new Kd constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" set_pid_freq <freq>     - Set PID sampling frequency.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                           <freq> = value of the new frequency.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" set_pid_sp <sp>         - Set PID set point(hold value).\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                           <sp> = new hold value in A or 0.01%(duty cycle).\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                           Example: 0.435 = 0.435 A = 435mA\n");
	} else {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" ap         - Activate PID controller.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" rp         - Deactivate PID controller.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" kp <kp>    - Set PID Kp constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              <kp> = value of the new Kp constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" ki <ki>    - Set PID Ki constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              <ki> = value of the new Ki constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" kd <kd>    - Set PID Kd constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              <kd> = value of the new Kd constant.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" spf <freq> - Set PID sampling frequency.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              <freq> = value of the new frequency.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" sps <sp>   - Set PID set point(hold value).\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              <sp> = new hold value in A or 0.01%(duty cycle).\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              Example: 0.435 = 0.435 A = 435mA\n");
	}
}

/**
 * Helper function for cn0415_help(). Display the list of functions for the
 * application controller. The difference between this function and the one
 * about the general commands is that the general commands provide
 * meta-information about the application, like status and help, and these
 * commands help control the application at a high level of abstraction.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
static void cn0415_help_app(struct cn0415_dev *dev, bool long_nshort)
{
	if (long_nshort) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" dig_sol_on         - Set digital solenoid high.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                      PID set-point must be given before this command.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                      If PID set-point is between 0 and 1 it will be considered to be in amps.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                      For this the PID must be tunned with Kp, Ki and Kd at apropriate values.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                      If set-point is between 2 and 10000 it will be considered to be in 0.01% duty cycle.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                      Example: 5126 = 51.26% duty cycle.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                      If pulse value and period have been set before, the controller will pulse first then return to set-point.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                      To deactivate pulsing set pulse value or period to 0.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" dig_sol_off        - Set digital solenoid low.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                      Goes to 0 duty cycle and deactivates PID controller.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" prop_sol_set <val> - Set and activate proportional solenoid.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                      <val> = value of the current or duty cycle.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "                      Example: 0.123 = 0.123 amperes through solenoid.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" reset              - Reset controller, parameters and faults.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" calibrate          - Run calibration sequence.\n");
	} else {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" dson       - Set digital solenoid high.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              PID set-point must be given before this command.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "              If PID set-point is between 0 and 1 it will be considered to be in amps.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "              For this the PID must be tunned with Kp, Ki and Kd at apropriate values.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "              If set-point is between 2 and 10000 it will be considered to be in 0.01% duty cycle.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              Example: 5126 = 51.26% duty cycle.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "              If pulse value and period have been set before, the controller will pulse first then return to set-point.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "              To deactivate pulsing set pulse value or period to 0.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" dsof       - Set digital solenoid low.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      "              Goes to 0 duty cycle and deactivates PID controller.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" pss <val>  - Set and activate proportional solenoid.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              <val> = value of the current or duty cycle.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"              Example: 0.123 = 0.123 amperes through solenoid.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" rst        - Reset controller, parameters and faults.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)" clb        - Run calibration sequence.\n");
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
 * @return needs to return int32_t to be compatible with the CLI commands
 *         functions but the return value is always 0 (irrelevant)
 */
int32_t cn0415_help(struct cn0415_dev *dev, uint8_t* arg)
{
	/* Display verbose menu */
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Available verbose commands:\n");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");

	/* Help command */
	cn0415_help_general(dev, true);

	/* PWM commands */
	cn0415_help_pwm(dev, true);

	/* ADC commands */
	cn0415_help_adc(dev, true);

	/* Dither commands */
	cn0415_help_dither(dev, true);

	/* PID controller commands */
	cn0415_help_pid(dev, true);

	/* Application commands */
	cn0415_help_app(dev, true);

	/* Display short commend menu */
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Available short commands:\n");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");

	/* Help command */
	cn0415_help_general(dev, false);

	/* PWM commands */
	cn0415_help_pwm(dev, false);

	/* ADC commands */
	cn0415_help_adc(dev, false);

	/* Dither commands */
	cn0415_help_dither(dev, false);

	/* PID controller commands */
	cn0415_help_pid(dev, false);

	/* Application commands */
	cn0415_help_app(dev, false);

	return 0;
}

/**
 * Echoes characters received from CLI. Implements CLI feedback.
 *
 * @param [in] dev  - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_parse(struct cn0415_dev *dev)
{
	int32_t ret;
	uint8_t c = 0, rdy = 1;

	/* Wait and read a character from UART */
	ret = usr_uart_read_nb(dev->aducm3029_uart_desc, &c, &rdy);
	if(ret != 0)
		return ret;

	/* Decide what to do with it */
	if(rdy == 1) {
		switch(c) {
		case _LF:
		case _CR:
			uart_cmd = UART_TRUE;
			break;
		case _BS:
			if(uart_line_index == 0)
				break;
			uart_line_index--;
			break;
		case _NC:
			break;
		default:
			uart_current_line[uart_line_index++] = c;
			if(uart_line_index == 256) {
				uart_line_index--;
			}
		}

		uart_current_line[uart_line_index] = '\0';
	}

	return ret;
}

/**
 * Implements the CLI logic.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_process(struct cn0415_dev *dev)
{
	int32_t  ret = 0;
	cmd_func func = NULL;
	uint8_t i = 0;
	int16_t pid_ctrl_adjust; /* Container for the PID adjustment value. */
	uint16_t command = 0;

	/* Disable timer interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR1_INT);
	NVIC_DisableIRQ(TMR2_INT);

	/* Get character and identify it */
	ret = cn0415_parse(dev);
	if(ret != 0)
		return ret;

	/* Enable timer interrupts */
	NVIC_EnableIRQ(TMR1_INT);
	NVIC_EnableIRQ(TMR2_INT);

	/* Check if <ENTER> key was pressed */
	if (uart_cmd == UART_TRUE) {
		do {
			uart_previous_line[i] = uart_current_line[i];
		} while(uart_current_line[i++] != '\0');
		/* Find needed function based on typed command */
		cn0415_find_command(dev, uart_current_line, &func);

		/* Check if there is a valid command */
		if (func) {
			usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
			/* Call the desired function */
			(*func)(dev, (uint8_t*)strchr((char*)uart_current_line, ' ') + 1);

			/* Check if there is no match for typed command */
		} else if (strlen((char *)uart_current_line) != 0) {
			usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
			/* Display a message for unknown command */
			usr_uart_write_string(dev->aducm3029_uart_desc,
					      (uint8_t*)"Unknown command!");
			usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
		}
		/* Prepare for next <ENTER> */
		uart_cmd = UART_FALSE;
		cn0415_cmd_prompt(dev);
	}

	/* Apply dither signal on the duty cycle */
	ret = pwm_dither_apply(dev->aducm3029_pwm_desc);
	if(ret != 0)
		return ret;
	if(dev->aducm3029_pwm_desc->has_dither == 1) {
		if(dither_period) {
			if(dev->aducm3029_pwm_desc->dither_amplitude <= 1) {
				pid_set_setpoint(dev->controller,
						 ((float)dev->param_array[HOLD_VAL_DEC] / 10000) +
						 dith_ampl);
				dither_period = false;
			}
		}
	}

	/* Add control */
	if (pid_flag == 1) {
		ret = cn0415_pid_controller(dev, &pid_ctrl_adjust);
		if(ret != 0) {
			return ret;
		}
		if((0 - pid_ctrl_adjust) > dev->aducm3029_pwm_desc->duty_cycle)
			command = 0;
		else
			command = dev->aducm3029_pwm_desc->duty_cycle + pid_ctrl_adjust;
		if(command > 10000)
			command = 10000;

		/* Update duty cycle */
		if(dev->pid == 1) {
			ret = pwm_set_duty_cycle(dev->aducm3029_pwm_desc, command);
			if(ret != 0) {
				return ret;
			}
		}

		pid_flag = 0;
	}

	return ret;
}

/**
 * Get the CLI commands and correlate them to functions.
 *
 * @param [in]  dev      - The device structure.
 * @param [in]  command  - Command received from the CLI.
 * @param [out] function - Pointer to the corresponding function.
 *
 * @return void
 */
void cn0415_find_command(struct cn0415_dev *dev, uint8_t *command,
			 cmd_func* function)
{
	uint8_t i = 0;

	while (v_cmd_fun[i/2] != NULL) {
		if(strncmp((char *)command,
			   (char *)cmd_commands[i],
			   command_size[i]) == 0 ||
		    strncmp((char *)command,
			    (char *)cmd_commands[i + 1],
			    command_size[i+1]) == 0) {
			if(command_size == 0) {
				break;
			}
			*function = v_cmd_fun[i / 2];
			break;
		}
		i += 2;
	}
}

/**
 * Display command prompt for the user on the CLI at the beginning of the
 * program.
 *
 * @param [in] dev - The device structure.
 *
 * @return void
 */
void cn0415_cmd_prompt(struct cn0415_dev *dev)
{
	static uint8_t count = 0;

	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");

	/* Check first <ENTER> is pressed after reset */
	if(count == 0) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " ###########################         ###     ###    ###    ###     ###       #######      ####### \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " ###########################        #####    ####   ###    ####    ###      ###    ###  ###    ### \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " ###########################       ##  ##    #####  ###   ##  ##   ###     ###      ##  ##          \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " #####    ##################       ##   ##   ##  ## ###  ##   ##   ###     ##       ## ###   #####  \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " #####       ###############      #########  ##   #####  ########  ###     ###      ##  ##      ##  \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " #####          ############      ##     ##  ##    #### ###    ### ###      ###   ####  ###    ###  \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " #####             #########     ##      ### ##     ### ##      ## ########   ######      ###### #  \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " #####                ######                                                                        \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " #####                ######     # ####      #######   #      ##  #     ####     #######    ####    \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " #####             #########     #########  ######### ###    ### ###  ###  ###  ########  ###  ###  \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " #####          ############     ##     ##  ##         ##    ##  ### ###     ## ##       ###        \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " #####       ###############     ##     ### ########   ###  ###  ### ##         ########  ######    \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " #####    ##################     ##      ## ###         ##  ##   ### ##         ###          ###### \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " ###########################     ##     ### ##           ####    ### ###     ## ##       ##      ## \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " ###########################     #########  #########    ####    ###  ########  ######### ########  \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)
				      " ###########################                                             ###                ####   \n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"\n");

		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"\tWelcome to cn0415 application!\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"Type <help> or <h> to see available commands...\n");
		usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
		count++;
	}

	usr_uart_write_char(dev->aducm3029_uart_desc, '>', UART_WRITE_NO_INT);

	uart_line_index = 0;
}

/**
 * Performs calibration on the ADC measurements.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_calibration(struct cn0415_dev *dev, uint8_t* arg)
{
	int32_t ret;
	uint16_t adc_value;
	uint8_t readings[20];
	uint8_t i = 0;
	float sense_current; /* Current got from ADC measurement. */
	float max_current; /* Maximum current, input by the user in the CLI. */
	float ext_meas_cur; /* Current measured with external device and input by
	                     * the user in the CLI. */

	/* Disable timer interrupts to avoid hanging */
	NVIC_DisableIRQ(TMR1_INT);
	NVIC_DisableIRQ(TMR2_INT);

	/* Prompt for maximum current value */
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Enter the maximum current value(mA):\n");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)">");
	do {
		ret = usr_uart_read_char(dev->aducm3029_uart_desc, &readings[i]);
		if(ret != 0) {
			return ret;
		}
		i++;
	} while(readings[i-1] != _CR && readings[i-1] != _LF);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
	readings[i-1] = '\0';
	/* Transform from ASCII to float */
	max_current = (float)atoi((char*)readings);
	/* Reset i for further use */
	i = 0;

	/* Calculate sensed current in mA */
	ret = pwm_set_duty_cycle(dev->aducm3029_pwm_desc, 10000);
	if(ret != 0)
		return ret;
	timer_sleep(10);
	ret = ltc2312_read(dev->ltc2312_device, &adc_value);
	if(ret != 0)
		return ret;
	ltc2312_code_to_voltage(dev->ltc2312_device, adc_value, vref,
				&sense_current);
	sense_current = (sense_current * 1000) / (20 * r_sense);

	/* If gross error is bigger than 10% fail */
	if((sense_current - max_current) / max_current >= 0.1 ||
	    (sense_current - max_current) / max_current <= -0.1) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"Gross error check failed.\n");

		return -1;
	}

	/* Prompt for measured current value */
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Enter the measured current value(mA):\n");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)">");
	do {
		ret = usr_uart_read_char(dev->aducm3029_uart_desc, &readings[i]);
		if(ret != 0) {
			return ret;
		}
		i++;
	} while(readings[i-1] != _CR && readings[i-1] != _LF);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
	readings[i-1] = '\0';
	/* Transform from ASCII to float */
	ext_meas_cur = (float)atoi((char*)readings);

	/* If measured value is more than 3% off the actual value fail */
	if(((sense_current - ext_meas_cur) / ext_meas_cur >= 0.03) ||
	    ((sense_current - ext_meas_cur) / ext_meas_cur <= -0.03)) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"Error check failed.\n");

		return -1;
	}

	/* Remember error */
	gain_err = (sense_current - ext_meas_cur) / 1000;

	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Calibration complete.\n");

	ret = pwm_set_duty_cycle(dev->aducm3029_pwm_desc, 0);
	if(ret != 0)
		return ret;

	/* Enable timer interrupts */
	NVIC_EnableIRQ(TMR1_INT);
	NVIC_EnableIRQ(TMR2_INT);

	return ret;
}

/**
 * Activate dither funcionality on the PWM signal.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_dither_activate(struct cn0415_dev *dev, uint8_t* arg)
{
	return pwm_dither_activate(dev->aducm3029_pwm_desc);
}

/**
 * Deactivate dither funcionality on the PWM signal.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_dither_remove(struct cn0415_dev *dev, uint8_t* arg)
{
	dev->controller->setpoint = dev->param_array[HOLD_VAL_INT];
	dev->controller->setpoint += (float)dev->param_array[HOLD_VAL_DEC] / 10000;

	return pwm_dither_remove(dev->aducm3029_pwm_desc);
}

/**
 * Set dither frequency.
 *
 * @param [in] dev  - The device structure.
 * @param [in] freq - New frequency of the dither.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_dither_set_freq(struct cn0415_dev *dev, uint8_t* freq)
{
	int32_t ret;
	uint32_t ifreq;

	/* Convert new frequency value from ASCII to integer representation */
	ifreq = (uint32_t)atoi((char*)freq);

	/* Change PWM frequency */
	ret = pwm_dither_change_frequency(dev->aducm3029_pwm_desc, ifreq);
	if(ret != 0)
		return ret;

	/* Save frequency in flash */
	dev->param_array[DITH_FREQ] = ifreq;

	return flash_write(dev->param_array, FLASH_PARAM_SIZE);
}

/**
 * Set dither amplitude (in 0.01% of duty cycle).
 *
 * @param [in] dev  - The device structure.
 * @param [in] duty - Amplitude in percent duty cycle.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0415_dither_set_ampl(struct cn0415_dev *dev, uint8_t* duty)
{
	float fduty;
	uint8_t *dty_ptr;
	uint8_t duty_copy[20];

	strcpy((char *)duty_copy, (char *)duty);

	dty_ptr = (uint8_t *)strchr((char *)duty_copy, ' ');
	if(dty_ptr == NULL) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"da USAGE:\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"    da -c <val> - Set dither amplitude in amps.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                  Ex.: da -c 0.234 = dither amplitude 234mA\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"    da -d <val> - Set dither amplitude in duty cycle.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                  Ex.: da -d 526 = dither amplitude 5.26%\n");

		return 0;
	}
	*dty_ptr = '\0';
	dty_ptr++;

	if(strcmp((char *)duty_copy, "-c") != 0 &&
	    strcmp((char *)duty_copy, "-d") != 0) {
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"da USAGE:\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"    da -c <val> - Set dither amplitude in amps.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                  Ex.: da -c 0.234 = dither amplitude 234mA\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"    da -d <val> - Set dither amplitude in duty cycle.\n");
		usr_uart_write_string(dev->aducm3029_uart_desc,
				      (uint8_t*)"                  Ex.: da -d 526 = dither amplitude 5.26%\n");

		return 0;
	}

	/* Convert new frequency value from ASCII to integer representation */
	fduty = atof((char *)dty_ptr);

	/* Change PWM frequency */
	pwm_dither_change_amplitude(dev->aducm3029_pwm_desc, fduty);

	/* Save frequency in flash */
	dev->param_array[DITH_AMPL_INT] = fduty;
	dev->param_array[DITH_AMPL_DEC] = (uint32_t)(fduty * 10000) % 10000;

	return flash_write(dev->param_array, FLASH_PARAM_SIZE);
}

/**
 * Convert floating point value to ASCII. Maximum 4 decimals.
 *
 * @param [out] buffer - Output ASCII buffer.
 * @param [in]  value  - The floating point value to be converted.
 *
 * @return void
 */
void cn0415_ftoa(uint8_t *buffer, float value)
{
	int32_t fraction;
	uint8_t local_buffer[20];
	float subunit;
	uint8_t i;

	/* Convert integral part */
	itoa((int32_t)value, (char*)local_buffer, 10);
	strcpy((char *)buffer, (char *)local_buffer);
	strcat((char *)buffer, ".");
	/* Display zeros after the decimal point. Else they would be eluded. */
	subunit = fmod(value, 1.0);
	i = 0;
	while(subunit < 0.1 && i < 3) {
		strcat((char*)buffer, "0");
		subunit *= 10;
		i++;
	}
	/* Convert decimal part */
	fraction = (int32_t)(value * 10000) % 10000;
	itoa(fraction, (char*)local_buffer, 10);
	strcat((char*)buffer, (char*)local_buffer);
}
