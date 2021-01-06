/*****************************************************************************
 * ADuCM3029_demo_cn0531.c
 *****************************************************************************/

#include <sys/platform.h>
#include <stdlib.h>
#include "adi_initialize.h"
#include "cn0531.h"
#include "error.h"

int main(int argc, char *argv[])
{
	struct cn0531_dev *cn0531_dev;
	struct cn0531_init_param cn0531_param;
	int32_t ret;
	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();

	cn0531_param.ad5791_param.act_device = ID_AD5791;
	cn0531_param.ad5791_param.gpio_clr.number = 0x10;
	cn0531_param.ad5791_param.gpio_clr.extra = NULL;
	cn0531_param.ad5791_param.gpio_ldac.number = 0x0E;
	cn0531_param.ad5791_param.gpio_ldac.extra = NULL;
	cn0531_param.ad5791_param.gpio_reset.number = 0x0C;
	cn0531_param.ad5791_param.gpio_reset.extra = NULL;
	cn0531_param.ad5791_param.spi_init.chip_select = 0xFF;
	cn0531_param.ad5791_param.spi_init.extra = NULL;
	cn0531_param.ad5791_param.spi_init.id = SPI_PMOD;
	cn0531_param.ad5791_param.spi_init.max_speed_hz = 5000000;
	cn0531_param.ad5791_param.spi_init.mode = SPI_MODE_3;
	cn0531_param.ad5791_param.spi_init.type = ADICUP3029_SPI;
	cn0531_param.cli_param.uart_init.baudrate = bd115200;
	cn0531_param.cli_param.uart_init.bits_no = 8;
	cn0531_param.cli_param.uart_init.has_callback = true;
	cn0531_param.platform_adc.adc_id = 0;

	ret = cn0531_init(&cn0531_dev, &cn0531_param);
	if(ret != SUCCESS)
		return FAILURE;

	while(1) {
		ret = cn0531_process(cn0531_dev);
		if(ret != SUCCESS)
			return FAILURE;
	}

	return 0;
}

