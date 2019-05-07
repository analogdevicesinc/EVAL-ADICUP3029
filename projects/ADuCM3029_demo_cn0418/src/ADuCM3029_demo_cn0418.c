/*****************************************************************************
 * ADuCM3029_demo_cn0418.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "timer.h"
#include "drivers/gpio/adi_gpio.h"
#include "swuart.h"
#include "ad5755.h"
#include "platform_drivers.h"
#include "cn0418.h"
#include <stdlib.h>
#include <math.h>

#define HART_BAUDRATE 1200

int main(int argc, char *argv[])
{
	int32_t ret;
	struct cn0418_init_param cn0418_init;
	struct cn0418_dev *cn0418_dev;

	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();

	timer_start();

	/* CN0418 device initialization */
	cn0418_init.ad5755_init.spi_init.chip_select = 0xFF;
	cn0418_init.ad5755_init.spi_init.id = 0;
	cn0418_init.ad5755_init.spi_init.max_speed_hz = 4000000;
	cn0418_init.ad5755_init.spi_init.mode = SPI_MODE_2;
	cn0418_init.ad5755_init.spi_init.type = SPI_ARDUINO;
	cn0418_init.ad5755_init.gpio_clr = 0x18;
	cn0418_init.ad5755_init.gpio_ldac = 0x16;
	cn0418_init.ad5755_init.gpio_poc = 0x19;
	cn0418_init.ad5755_init.gpio_rst = 0x17;
	cn0418_init.ad5755_init.this_device = ID_AD5755_1;
	cn0418_init.ad5700_init.swuart_init.baudrate = HART_BAUDRATE;
	cn0418_init.ad5700_init.swuart_init.id = 0;
	cn0418_init.ad5700_init.swuart_init.rx_port_pin.port_number = ADI_GPIO_PORT0;
	cn0418_init.ad5700_init.swuart_init.rx_port_pin.pin_number = ADI_GPIO_PIN_13;
	cn0418_init.ad5700_init.swuart_init.tx_port_pin.port_number = ADI_GPIO_PORT0;
	cn0418_init.ad5700_init.swuart_init.tx_port_pin.pin_number = ADI_GPIO_PIN_9;
	cn0418_init.ad5700_init.swuart_init.type = ADICUP3029_SWUART;
	cn0418_init.ad5700_init.swuart_init.parity = UART_ODD_PARITY;
	cn0418_init.ad5700_init.swuart_init.no_of_bits = 8;
	cn0418_init.ad5700_init.swuart_init.delay_timer = 1;
	cn0418_init.ad5700_init.gpio_nrts = 0x1B;
	cn0418_init.ad5700_init.gpio_cd = 0x21;
	cn0418_init.usr_uart_init.baudrate = bd115200;
	cn0418_init.usr_uart_init.bits_no = 8;
	cn0418_init.hart_mult_a0 = 0x1E;
	cn0418_init.hart_mult_a1 = 0x1C;

	/* Memory init */
	cn0418_init.memory_init.i2c_param.id = 0;
	cn0418_init.memory_init.i2c_param.max_speed_hz = 400000;
	cn0418_init.memory_init.i2c_param.slave_address = 0x77;
	cn0418_init.memory_init.i2c_param.type = ADICUP3029_SPI;
	cn0418_init.memory_init.i2c_address = A0_VDD | A1_VDD | A2_VDD;

	ret = cn0418_setup(&cn0418_dev, &cn0418_init);
	if(ret < 0)
		return ret;

	ret = cn0418_cmd_prompt(cn0418_dev);
	if(ret < 0)
		return ret;

	while(1) {
		ret = cn0418_process(cn0418_dev);
		if(ret < 0)
			return ret;
	}

	return 0;
}

