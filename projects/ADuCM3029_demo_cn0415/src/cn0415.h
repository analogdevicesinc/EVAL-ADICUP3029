/***************************************************************************//**
 *   @file   cn0415.h
 *   @brief  ADICUP3029 CN0415 application header.
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

#ifndef CN0415_H_
#define CN0415_H_

#include "pid_controller.h"
#include "ltc2312.h"
#include "pwm.h"

#define INPUT_AMPLIFIER_GAIN (float)20
#define SENSE_RESISTOR (float)0.1
#define _NC 0
#define _BS 8
#define _CR 13
#define _LF 10
#define ESC '\e'
#define UPH '['
#define UPL 'A'
#define FLASH_PARAM_SIZE 16
#define SAMPLE_NO 50

/* GPIOs */
#define GPIO_FAULT_DETECT 0x0F
#define GPIO_OC_RESET 0x09
#define GPIO_LT4367_FAULT 0x0B

#define CN415_SUCCESS		0
#define CN415_FAILURE		-1

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
struct cn0415_pid_controller {
	uint16_t t_loop;
	float kp, ki, kd;
	float error_sum, last_feedback;
	float setpoint;
};

enum param_array_elements {
	F_PWM = 0,
	F_LOOP,
	PID_KP_INT,
	PID_KP_DEC,
	PID_KI_INT,
	PID_KI_DEC,
	PID_KD_INT,
	PID_KD_DEC,
	PULSE_VAL_INT,
	PULSE_VAL_DEC,
	T_PULSE,
	HOLD_VAL_INT,
	HOLD_VAL_DEC,
	DITH_AMPL_INT,
	DITH_AMPL_DEC,
	DITH_FREQ
};

struct cn0415_dev {
	/* Device peripheral */
	struct uart_desc *aducm3029_uart_desc;
	struct pwm_desc *aducm3029_pwm_desc;
	struct gpio_desc *gpio_fault_detect;
	struct gpio_desc *gpio_oc_reset;
	struct gpio_desc *gpio_lt4367_fault;
	/* Board components */
	struct ltc2312_dev *ltc2312_device;
	struct pid_controller *controller;
	/* Flags */
	uint8_t pid;
	uint8_t dither;
	/* Private components */
	uint32_t param_array[16];
	float pulse_val;
	uint32_t pulse_t;
};

struct cn0415_init_param {
	/* Device peripheral */
	struct uart_init_param aducm3029_uart_init_param;
	struct pwm_init_param aducm3029_pwm_init_param;
	/* Board components */
	struct ltc2312_init_param ltc2312_init;
	struct pid_controller_init controller_init;
	/* Flags */
	uint8_t has_pid;
};

typedef  int32_t (*cmd_func)(struct cn0415_dev*, uint8_t*);

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/
/* Initializes the cn0415 device. */
int32_t cn0415_setup(struct cn0415_dev **device,
		     struct cn0415_init_param init_param);

/* Free the resources allocated by cn0415_setup(). */
int32_t cn0415_remove(struct cn0415_dev *dev);

/* Set pulse value for when the solenoid needs to pulse before reaching
 * hold value. */
int32_t cn0415_pulse_set_val(struct cn0415_dev *dev, uint8_t *args);

/* Set pulse period for when the solenoid needs to pulse before reaching
 * hold value. */
int32_t cn0415_pulse_set_period(struct cn0415_dev *dev, uint8_t *args);

/* Set solenoid to zero value. */
int32_t cn0415_dig_sol_off(struct cn0415_dev *dev, uint8_t *args);

/* Set solenoid hold value. If a pulse value and time is present the
 * actuator will first pulse with the value for the corresponding time
 * then go back to hold value. */
int32_t cn0415_dig_sol_on(struct cn0415_dev *dev, uint8_t *args);

/* Set proportional solenoid on. */
int32_t cn0415_prop_sol_set(struct cn0415_dev *dev, uint8_t *setpoint);

/* Reset controller, parameter and faults. */
int32_t cn0415_reset(struct cn0415_dev *dev, uint8_t *args);

/* Display status of the application. */
int32_t cn0415_status(struct cn0415_dev *dev, uint8_t *args);

/* PID controller for PWM duty cycle. */
int32_t cn0415_pid_controller(struct cn0415_dev *dev, int16_t *output);

/* Set proportional constant, Kp, for the PID controller. */
int32_t cn0415_pid_set_kp(struct cn0415_dev *dev, uint8_t *args);

/* Set integral constant, Ki, for the PID controller. */
int32_t cn0415_pid_set_ki(struct cn0415_dev *dev, uint8_t *args);

/* Set derivative constant, Kd, for the PID controller. */
int32_t cn0415_pid_set_kd(struct cn0415_dev *dev, uint8_t *args);

/* Set sample frequency for the PID controller. */
int32_t cn0415_pid_set_sample_frequency(struct cn0415_dev *dev, uint8_t *args);

/* Set set-point for the PID controller. */
int32_t cn0415_pid_set_setpoint(struct cn0415_dev *dev, uint8_t *args);

/* Set duty cycle for the PWM. */
int32_t cn0415_pwm_set_duty(struct cn0415_dev *dev, uint8_t* new_duty);

/* Set frequency for the PWM. */
int32_t cn0415_pwm_set_freq(struct cn0415_dev *dev, uint8_t* freq);

/* Use the PID controller for the output. */
int32_t cn0415_pid_remove(struct cn0415_dev *dev, uint8_t *args);

/* Deactivate PID control. */
int32_t cn0415_pid_add(struct cn0415_dev *dev, uint8_t *args);

/* Add dither to the PWM duty cycle. */
int32_t cn0415_add_dither(struct cn0415_dev *dev,
			  struct cn0415_init_param init_param);

/* Read ADC value and display the current and voltage values. */
int32_t cn0415_read_voltage(struct cn0415_dev *dev, uint8_t *arg);

/* Display help options in the CLI. */
int32_t cn0415_help(struct cn0415_dev *dev, uint8_t* arg);

/* Echoes characters received from CLI. Implements CLI feedback. */
int32_t cn0415_parse(struct cn0415_dev *dev);

/* Implements the CLI logic. */
int32_t cn0415_process(struct cn0415_dev *dev);

/* Get the CLI commands and correlate them to functions. */
void cn0415_find_command(struct cn0415_dev *dev, uint8_t *command,
			 cmd_func* function);

/* Display command prompt for the user on the CLI at the beginning of the
 * program. */
void cn0415_cmd_prompt(struct cn0415_dev *dev);

/* Performs calibration on the ADC measurements. */
int32_t cn0415_calibration(struct cn0415_dev *dev, uint8_t* arg);

/* Activate dither funcionality on the PWM signal. */
int32_t cn0415_dither_activate(struct cn0415_dev *dev, uint8_t* arg);

/* Deactivate dither funcionality on the PWM signal. */
int32_t cn0415_dither_remove(struct cn0415_dev *dev, uint8_t* arg);

/* Set dither frequency. */
int32_t cn0415_dither_set_freq(struct cn0415_dev *dev, uint8_t* freq);

/* Set dither amplitude (in 0.01% of duty cycle). */
int32_t cn0415_dither_set_ampl(struct cn0415_dev *dev, uint8_t* duty);

/* Convert floating point value to ASCII. Maximum 4 decimals. */
void cn0415_ftoa(uint8_t *buffer, float value);

#endif /* CN0415_H_ */
