/*****************************************************************************
 * main.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"
#include "spi_extra.h"
#include "ad7799.h"
#include "iio_ad7799.h"
#include "drivers/uart/adi_uart.h"
#include "parameters.h"
#include "error.h"
#include "iio.h"
#include "iio_app.h"
#include "irq.h"
#include "irq_extra.h"
#include "uart.h"
#include "uart_extra.h"
#include "platform_init.h"
#include "timer.h"
#include "util.h"

int main(int argc, char *argv[])
{

	int32_t ret;

	ret = platform_init();
	if (IS_ERR_VALUE(ret))
		return ret;

	struct aducm_spi_init_param aducm_param = {
			.continuous_mode = true,
			.dma = false,
			.half_duplex = false,
			.master_mode = MASTER,
	};

	struct spi_init_param init_param = {
			.device_id = 0,
			.chip_select = 1,
			.extra = &aducm_param,
			.max_speed_hz = 1000000,
			.mode = SPI_MODE_3,
			.platform_ops = NULL
	};

	struct ad7799_init_param ad7799_param = {
			.spi_init = init_param,
			.chip_type = ID_AD7798,
			.gain = AD7799_GAIN_1,
			.polarity = AD7799_BIPOLAR,
			.vref_mv = 4096
	};

	struct ad7799_dev *ad7799_device;

	ret = ad7799_init(&ad7799_device, &ad7799_param);
	if (IS_ERR_VALUE(ret))
			return ret;

	struct iio_app_device devices[] = {
		IIO_APP_DEVICE("AD7799", ad7799_device,
			       &ad7799_iio_descriptor,
			       NULL, NULL)
	};

	return iio_app_run(devices, ARRAY_SIZE(devices));

	return 0;
}

