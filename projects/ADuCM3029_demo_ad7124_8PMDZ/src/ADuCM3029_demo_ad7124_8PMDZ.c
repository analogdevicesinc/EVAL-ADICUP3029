/*****************************************************************************
 * ADuCM3029_demo_ad7124_8PMDZ.c
 *****************************************************************************/

#include <sys/platform.h>
#include <stdlib.h>
#include "adi_initialize.h"
#include "app.h"
#include "error.h"
#include "ad7124_regs.h"
#include "spi_extra.h"
#include "uart_extra.h"
#include "irq_extra.h"

int main(int argc, char *argv[])
{
	struct ad7124_8pmdz_dev *ad7124_8pmdz_device;
	struct ad7124_8pmdz_init_param ad7124_8pmdz_initial;
	struct aducm_spi_init_param aducm_spi_ini;
	struct aducm_uart_init_param aducm_uart_ini;
	int32_t ret;
	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();

	aducm_spi_ini.continuous_mode = true;
	aducm_spi_ini.dma = false;
	aducm_spi_ini.half_duplex = false;
	aducm_spi_ini.master_mode = MASTER;
	aducm_spi_ini.spi_channel = SPI1;

	aducm_uart_ini.parity = UART_NO_PARITY;
	aducm_uart_ini.stop_bits = UART_ONE_STOPBIT;
	aducm_uart_ini.word_length = UART_WORDLEN_8BITS;

	ad7124_8pmdz_initial.ad7124_initial.regs = ad7124_regs;
	ad7124_8pmdz_initial.ad7124_initial.spi_init.chip_select = 0x00;
	ad7124_8pmdz_initial.ad7124_initial.spi_init.max_speed_hz = 10000000;
	ad7124_8pmdz_initial.ad7124_initial.spi_init.mode = SPI_MODE_3;
	ad7124_8pmdz_initial.ad7124_initial.spi_init.extra = &aducm_spi_ini;
	ad7124_8pmdz_initial.ad7124_initial.spi_rdy_poll_cnt = 25000;
	ad7124_8pmdz_initial.cli_initial.uart_init.baud_rate = BD_115200;
	ad7124_8pmdz_initial.cli_initial.uart_init.device_id = 0;
	ad7124_8pmdz_initial.cli_initial.uart_init.extra = &aducm_uart_ini;
	ad7124_8pmdz_initial.irq_init.irq_ctrl_id = 0;
	ad7124_8pmdz_initial.irq_init.extra = NULL;

	ret = ad7124_8pmdz_init(&ad7124_8pmdz_device, &ad7124_8pmdz_initial);
	if(ret != SUCCESS)
		return FAILURE;

	ret = ad7124_8pmdz_prompt(ad7124_8pmdz_device);
	if(ret != SUCCESS)
		return FAILURE;

	while(1) {
		ret = ad7124_8pmdz_process(ad7124_8pmdz_device);
		if(ret != SUCCESS)
			return FAILURE;
	}

	return 0;
}

