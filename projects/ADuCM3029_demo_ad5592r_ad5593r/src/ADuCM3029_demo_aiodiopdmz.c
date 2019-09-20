/*****************************************************************************
 * ADuCM3029_demo_aiodiopdmz.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "aio_dio_pdmz.h"
#include "config.h"
#include "platform_drivers.h"

int main(int argc, char *argv[])
{
	struct aiodio_dev *aiodio_dut;
	struct aiodio_init_param aiodio_init;
	int32_t ret;

	aiodio_init.i2c_init.id = 0;
	aiodio_init.i2c_init.max_speed_hz = 400000;
	aiodio_init.i2c_init.slave_address = 0x10 | AD5593R_A0_STATE;
	aiodio_init.i2c_init.type = ADICUP3029_I2C;
	aiodio_init.spi_init.chip_select = 0xff;
	aiodio_init.spi_init.id = SPI_PMOD;
	aiodio_init.spi_init.max_speed_hz = 1000000;
	aiodio_init.spi_init.mode = SPI_MODE_2;
	aiodio_init.spi_init.type = ADICUP3029_SPI;
	aiodio_init.ad5592_3r_param.int_ref = true;
	aiodio_init.cli_init.uart_init.baudrate = bd115200;
	aiodio_init.cli_init.uart_init.bits_no = 8;
	aiodio_init.cli_init.uart_init.has_callback = true;
	aiodio_init.mode_delay_timer.f_update = 10;
	aiodio_init.mode_delay_timer.update_timer = 0;
	aiodio_init.blink_delay_timer.f_update = 26000;
	aiodio_init.blink_delay_timer.update_timer = 1;

	adi_initComponents();

	ret = aiodio_setup(&aiodio_dut, &aiodio_init);
	if(ret != 0)
		return ret;

	while(1) {
		ret = aiodio_process(aiodio_dut);
		if(ret != 0)
			return ret;
	}

	return 0;
}

