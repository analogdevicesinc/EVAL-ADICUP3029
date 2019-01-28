/***************************************************************************//**
 *   @file   pid_controller.h
 *   @brief  ADICUP3029 PID controller driver header.
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

#ifndef PID_CONTROLLER_H_
#define PID_CONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

/* Output limits for PWM */
#define PWM_OUT_MAX 10000
#define PWM_OUT_MIN -10000

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
struct pid_controller_init {
	uint16_t f_loop;
	float kp, ki, kd;
	float setpoint;
	uint8_t pid_timer_id;
};

struct pid_controller {
	uint16_t f_loop;
	float kp, ki, kd;
	float error_sum, last_feedback;
	float setpoint;
	uint8_t pid_timer_id;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initializes the PID controller device descriptor. */
int32_t pid_setup(struct pid_controller **device,
		  struct pid_controller_init init_param);

/* Free the resources allocated by pid_setup(). */
void pid_remove(struct pid_controller *dev);

/* Activate/deactivate PID controller. */
int32_t pid_activate(struct pid_controller *dev, bool status);

/* PID controller for PWM duty cycle. */
void pid_controller_output_eval(struct pid_controller *dev, float input,
				int16_t *output);

/* Set constants Kp, Ki and Kd for the PID controller. */
void pid_set_constants(struct pid_controller *dev, float kp, float ki,
		       float kd);

/* Set sample frequency for the PID controller. */
void pid_set_sample_frequency(struct pid_controller *dev,
			      uint16_t new_frequency);

/* Set set-point for the PID controller. */
void pid_set_setpoint(struct pid_controller *dev, float new_setpoint);

#endif /* PID_CONTROLLER_H_ */
