/*****************************************************************************
 * ADuCM3029_demo_cn0414.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "timer.h"
#include "platform_drivers.h"
#include "swuart.h"
#include "ad5700.h"
#include "ad717x.h"
#include <stdio.h>

#include "ad4111_regs.h"
#include "memory.h"
#include "cn0414.h"

/* Baudrate for software UART used for HART communication */
#define HART_BAUDRATE 1200

int main(int argc, char *argv[])
{
	int32_t ret;
	struct swuart_init dut_init;
	struct uart_init_param uart_init;
	ad717x_init_param ad4111_ini;
	struct ad5700_init_param ad5700_init;

	struct cn0414_dev *cn0414_device;
	struct cn0414_ini_param cn0414_init;
	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();

	dut_init.baudrate = HART_BAUDRATE;
	dut_init.id = 0;
	dut_init.rx_port_pin.port_number = ADI_GPIO_PORT0;
	dut_init.rx_port_pin.pin_number = ADI_GPIO_PIN_13;
	dut_init.tx_port_pin.port_number = ADI_GPIO_PORT0;
	dut_init.tx_port_pin.pin_number = ADI_GPIO_PIN_9;
	dut_init.parity = UART_ODD_PARITY;
	dut_init.no_of_bits = 8;
	dut_init.type = ADICUP3029_SWUART;
	dut_init.delay_timer = 1;

	ad5700_init.swuart_init = dut_init;
	ad5700_init.gpio_nrts = 0x1b;
	ad5700_init.gpio_cd = 0x21;

	ad4111_ini.spi_init.chip_select = 0xFF;
	ad4111_ini.spi_init.id = 0;
	ad4111_ini.spi_init.max_speed_hz = 4000000;
	ad4111_ini.spi_init.mode = SPI_MODE_3;
	ad4111_ini.spi_init.type = SPI_ARDUINO;
	ad4111_ini.regs = ad4111_regs;
	ad4111_ini.num_regs = 55;

	uart_init.baudrate = bd115200;
	uart_init.bits_no = 8;
	uart_init.has_callback = true;

	cn0414_init.memory_init.i2c_param.id = 0;
	cn0414_init.memory_init.i2c_param.max_speed_hz = 400000;
	cn0414_init.memory_init.i2c_param.slave_address = 0x77;
	cn0414_init.memory_init.i2c_param.type = ADICUP3029_SPI;
	cn0414_init.memory_init.i2c_address = A0_VDD | A1_VDD | A2_VDD;

	cn0414_init.ad4111_ini = ad4111_ini;
	cn0414_init.ad5700_init = ad5700_init;
	cn0414_init.uart_ini = uart_init;
	cn0414_init.adc_update_init.f_update = 100;
	cn0414_init.adc_update_init.update_timer = 0;
	cn0414_init.gpio_hart_chan0 = 0x1e;
	cn0414_init.gpio_hart_chan1 = 0x1c;

	ret = cn0414_setup(&cn0414_device, &cn0414_init);
	if(ret != 0)
		return -1;

	ret = cn0414_cmd_prompt(cn0414_device);

	while(1)
		cn0414_process(cn0414_device);

	return 0;
}

