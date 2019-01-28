/***************************************************************************//**
 *   @file   pwm.h
 *   @brief  Header file of the PWM module.
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

#ifndef PWM_H_
#define PWM_H_

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/* Duty cycle has a precision of 0.01% by default. To avoid floating point
 * calculations the duty cycle is required to be given in
 * "parts-per-ten-thousand". For example:
 * 		- for 25%    duty cycle duty_cycle := 2500;
 * 		- for 69%    duty cycle duty_cycle := 6900;
 * 		- for 52.13% duty cycle duty_cycle := 5213;
 * 		- etc.
 *
 * The private parameters do not need to be addressed by the user.*/
struct pwm_desc {
	/* Parameters */
	uint32_t frequency;
	uint16_t duty_cycle;
	/* Dither parameters */
	uint8_t  has_dither;
	uint32_t dither_frequency;
	float  dither_amplitude;
	/* Private parameters */
	uint16_t load;
	uint16_t dload;
};

struct pwm_init_param {
	/* Parameters */
	uint32_t frequency;
	uint16_t duty_cycle;
	/* Dither parameters */
	uint8_t  has_dither;
	uint32_t dither_frequency;
	float  dither_amplitude;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/
/* Initialize timer peripheral with PWM functionality. */
int32_t pwm_setup(struct pwm_desc **device, struct pwm_init_param init_param);

/* Free the resources allocated by pwm_setup(). */
int32_t pwm_remove(struct pwm_desc *dev);

/* Changes the duty cycle of the PWM. */
int32_t pwm_set_duty_cycle(struct pwm_desc *dev, uint16_t duty_cycle);

/* Changes the frequency of the PWM. */
int32_t pwm_set_frequency(struct pwm_desc *dev, uint32_t new_frequency);

/* Initialize timer peripheral with dither functionality on PWM signal. */
int32_t pwm_dither_setup(struct pwm_desc *device,
			 struct pwm_init_param init_param);

/* Activate dither functionality. */
int32_t pwm_dither_activate(struct pwm_desc *dev);

/* Deactivate dither functionality. */
int32_t pwm_dither_remove(struct pwm_desc *dev);

/* Change dither frequency. */
int32_t pwm_dither_change_frequency(struct pwm_desc *dev,
				    uint32_t new_frequency);

/* Change dither amplitude. */
void pwm_dither_change_amplitude(struct pwm_desc *dev,
				 float new_amplitude);

/* Internal function used to apply dither on the PWM signal. */
int32_t pwm_dither_apply(struct pwm_desc *dev);

#endif /* PWM_H_ */
