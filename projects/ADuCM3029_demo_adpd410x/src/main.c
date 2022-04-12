/***************************************************************************//**
 *   @file   main.c
 *   @brief  Main Application.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <sys/platform.h>
#include <stdint.h>
#include "adi_initialize.h"

#include "adpd410x_app.h"
#include "no-os/error.h"
#include "iio_adpd410x.h"
#include "iio_app.h"
#include "no-os/util.h"

#define MAX_SIZE_BASE_ADDR		1000

static uint32_t in_buff[MAX_SIZE_BASE_ADDR];

#define ADPD410X_BASEADDR		((uint32_t)in_buff)

int main(int argc, char *argv[])
{
	int32_t ret;
	struct adpd410x_app_dev *adpd410x_app;
	uint32_t data[8];

	ret = platform_init();
	if (IS_ERR_VALUE(ret))
		return ret;

	ret = adpd410x_app_init(&adpd410x_app);
	if (IS_ERR_VALUE(ret))
		return ret;

	struct iio_data_buffer rd_buf = {
		.buff = (void *)ADPD410X_BASEADDR,
		.size = MAX_SIZE_BASE_ADDR
	};

	struct iio_app_device devices[] = {
		IIO_APP_DEVICE("adpd410x", adpd410x_app->adpd4100_handler,
			       &adpd410x_iio_descriptor,
			       &rd_buf, NULL),
	};

	return iio_app_run(devices, ARRAY_SIZE(devices));

	return 0;
}
