/***************************************************************************//**
 *   @file   ADuCM3029_demo_cn0415.c
 *   @brief  ADICUP3029 CN0415 main file source.
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

#include <sys/platform.h>
#include "adi_initialize.h"

#include <drivers/tmr/adi_tmr.h>
#include "platform_drivers.h"
#include "cn0415.h"
#include "ltc2312.h"

int main(int argc, char *argv[])
{
	adi_initComponents();

	struct cn0415_init_param cn0415_init;
	struct cn0415_dev *dev;
	int32_t ret;
	uint8_t c;

	/* Initialize ADC and SPI module */
	cn0415_init.ltc2312_init.spi_init.chip_select = 0;
	cn0415_init.ltc2312_init.spi_init.id = SPI_ARDUINO;
	cn0415_init.ltc2312_init.spi_init.max_speed_hz = 4000000;
	cn0415_init.ltc2312_init.spi_init.mode = 0;
	cn0415_init.ltc2312_init.type = LTC2312_14;

	/* Initialize UART module */
	cn0415_init.aducm3029_uart_init_param.baudrate = bd115200;
	cn0415_init.aducm3029_uart_init_param.bits_no = 8;

	/* Initialize PWM module */
	cn0415_init.aducm3029_pwm_init_param.duty_cycle = 0;
	cn0415_init.aducm3029_pwm_init_param.frequency = 2500;

	/* Initialize dither module */
	cn0415_init.aducm3029_pwm_init_param.has_dither = 0;
	cn0415_init.aducm3029_pwm_init_param.dither_frequency = 1000;
	cn0415_init.aducm3029_pwm_init_param.dither_amplitude = 500;

	cn0415_init.controller_init.f_loop = 2000;
	cn0415_init.controller_init.kd = 0.5;
	cn0415_init.controller_init.ki = 2;
	cn0415_init.controller_init.kp = 11000;
	cn0415_init.controller_init.pid_timer_id = ADI_TMR_DEVICE_GP2;
	cn0415_init.controller_init.setpoint = 0;
	cn0415_init.has_pid = 0;

	/* Setup device */
	ret = cn0415_setup(&dev, cn0415_init);
	if(ret != CN415_SUCCESS)
		return CN415_FAILURE;

	/* Calibrate device */
	/* Calibration prompt */
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");
	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Do you want to calibrate?(y/n)\n");
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)">");

	/* Read response */
	usr_uart_read_char(dev->aducm3029_uart_desc, &c);
	usr_uart_write_string(dev->aducm3029_uart_desc, (uint8_t*)"\n");

	/* Calibration */
	if(c == 'y' || c == 'Y')
		cn0415_calibration(dev, NULL);

	usr_uart_write_string(dev->aducm3029_uart_desc,
			      (uint8_t*)"Press enter key to continue...\n");

	while(1) {
		/* Application process */
		ret = cn0415_process(dev);
		if(ret != 0) {
			return ret;
		}
	}

	return CN415_SUCCESS;
}

