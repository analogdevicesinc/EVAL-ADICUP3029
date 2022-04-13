/***************************************************************************//**
 *   @file   cn0548.c
 *   @brief  CN0548 main application.
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
********************************************************************************
 * Copyright 2021(c) Analog Devices, Inc.
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
#include "no_os_spi.h"
#include "spi_extra.h"
#include "ad7799.h"
#include "iio_ad7799.h"
#include "parameters.h"
#include "no_os_error.h"
#include "iio_app.h"
#include "platform_init.h"
#include "no_os_util.h"
#include "app_config.h"

int main(int argc, char *argv[])
{

	int32_t ret;

	ret = platform_init();
	if (NO_OS_IS_ERR_VALUE(ret))
		return ret;

	struct aducm_spi_init_param aducm_param = {
		.continuous_mode = true,
		.dma = false,
		.half_duplex = false,
		.master_mode = MASTER,
	};

	struct no_os_spi_init_param init_param = {
		.device_id = 0,
		.chip_select = 1,
		.extra = &aducm_param,
		.max_speed_hz = 1000000,
		.mode = NO_OS_SPI_MODE_3,
		.platform_ops = &aducm_spi_ops,
	};

	struct ad7799_init_param ad7799_param = {
		.spi_init = init_param,
		.chip_type = ID_AD7798,
		.gain = AD7799_GAIN_1,
#ifdef CN0548_UNIPOLAR
		.polarity = AD7799_UNIPOLAR,
#else
		.polarity = AD7799_BIPOLAR,
#endif
		.vref_mv = 4096
	};

	struct ad7799_dev *ad7799_device;

	ret = ad7799_init(&ad7799_device, &ad7799_param);
	if (NO_OS_IS_ERR_VALUE(ret))
		return ret;

	struct iio_app_device devices[] = {
		IIO_APP_DEVICE("AD7799", ad7799_device,
			       &ad7799_iio_descriptor,
			       NULL, NULL)
	};

	return iio_app_run(devices, NO_OS_ARRAY_SIZE(devices));

	return 0;
}
