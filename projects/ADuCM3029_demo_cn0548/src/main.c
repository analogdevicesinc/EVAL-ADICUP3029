/*****************************************************************************
 * main.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include <stdio.h>
#include <stdlib.h>
#include "power.h"
#include "spi.h"
#include "spi_extra.h"
#include "ad7799.h"
#include "ad7799_iio.h"
#include "drivers/uart/adi_uart.h"
#include "parameters.h"
#include "error.h"
#include "iio.h"
#include "irq.h"
#include "irq_extra.h"
#include "uart.h"
#include "uart_extra.h"
#include "timer.h"

int main(int argc, char *argv[])
{
	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	
	pwr_setup();

	int32_t ret;
	uint32_t data_ch;

	uint8_t ascii_buff[50];
	float ch;

	uint8_t uart_word_len = 8;

	/* iio descriptor. */
	struct iio_desc  *iio_desc;

	/* iio init param */
	struct iio_init_param iio_init_param;

	/* Initialization for UART. */
	struct uart_init_param uart_init_par;

	/* IRQ initial configuration. */
	struct irq_init_param irq_init_param;

	/* IRQ instance. */
	struct irq_ctrl_desc *irq_desc;
	int32_t platform_irq_init_par = 0;

	irq_init_param = (struct irq_init_param ) {
			.irq_ctrl_id = INTC_DEVICE_ID,
			.extra = &platform_irq_init_par
		};

	/* Aducm platform dependent initialization for UART. */
	struct aducm_uart_init_param platform_uart_init_par = {
			.parity = UART_NO_PARITY,
			.stop_bits = UART_ONE_STOPBIT,
			.word_length = UART_WORDLEN_8BITS
	};
	uart_init_par = (struct uart_init_param) {
		.device_id = UART_DEVICE_ID,
		.baud_rate = UART_BAUDRATE,
		.extra = &platform_uart_init_par
	};

	iio_init_param.phy_type = USE_UART;
	iio_init_param.uart_init_param = &uart_init_par;

	ret = irq_ctrl_init(&irq_desc, &irq_init_param);
	if(ret < 0)
		return ret;

	ret = irq_global_enable(irq_desc);
	if (ret < 0)
		return ret;


	struct aducm_spi_init_param aducm_param = {
			.continuous_mode = true,
			.dma = false,
			.half_duplex = false,
			.master_mode = MASTER,
			.spi_channel = SPI0
	};

	struct spi_init_param init_param = {
			.chip_select = 1,
			.extra = &aducm_param,
			.max_speed_hz = 4000000,
			.mode = SPI_MODE_3,
			.platform_ops = NULL
	};

	struct ad7799_init_param ad7799_param = {
			.spi_init = init_param,
			.chip_type = ID_AD7798
	};

	struct ad7799_dev *ad7799_device;

	ret = ad7799_init(&ad7799_device, &ad7799_param);
	if (ret)
		return FAILURE;

	/* IIO related */
	ret = iio_init(&iio_desc, &iio_init_param);
		if(ret < 0)
			return ret;

	ret = iio_register(iio_desc, &ad7799_iio_descriptor, "AD7799", ad7799_device);
	if (ret < 0)
		return ret;

	do {
		ret = iio_step(iio_desc);
	} while (true);

	return 0;
}

