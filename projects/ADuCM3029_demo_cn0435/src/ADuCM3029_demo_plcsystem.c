/***************************************************************************//**
*   @file   ADuCM3029_demo_plcsystem.c
*   @brief  Main module of the application; contains main() function.
*   @author Andrei Drimbarean (andrei.drimbarean@analog.com)
********************************************************************************
* Copyright 2019(c) Analog Devices, Inc.
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
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
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
#include "adi_initialize.h"
#include "system_manager.h"
#include "ad4111_regs.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/* Baudrate for software UART used for HART communication */
#define HART_BAUDRATE 1200

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

uint32_t unique_id = 0x00112233;
float vref = 2.5;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Main function of the application.
 */
int main(int argc, char *argv[])
{
	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();

	struct system_manager_init sys_manager_init;
	struct system_manager_dev *system_manager;
	struct swuart_init swuart_init_param;
	struct ad5700_init_param ad5700_init;
	ad717x_init_param ad4111_ini;
	struct memory_init_param memory_init;
#if defined(CLI_INTEFACE)
	struct cli_init_param cli_init;
#elif defined(MODBUS_INTERFACE)
	struct modbus_slave_init_param mb_slave_init;
#endif
	struct ad5755_init_param ad5755_init;
	int32_t ret;

	sys_manager_init.gpio_hart_deco_a0 = 0x23;
	sys_manager_init.gpio_hart_deco_a1 = 0x24;
	sys_manager_init.gpio_cs_deco_a0 = 0x25;
	sys_manager_init.gpio_cs_deco_a1 = 0x26;
	sys_manager_init.gpio_spi_cs = 0x1A;

	memory_init.i2c_address = A0_VDD | A1_VDD | A2_VDD;
	memory_init.i2c_param.id = 0;
	memory_init.i2c_param.max_speed_hz = 400000;
	memory_init.i2c_param.slave_address = 0x77;
	memory_init.i2c_param.type = ADICUP3029_SPI;
	swuart_init_param.baudrate = HART_BAUDRATE;
	swuart_init_param.id = 0;
	swuart_init_param.rx_port_pin.port_number = ADI_GPIO_PORT0;
	swuart_init_param.rx_port_pin.pin_number = ADI_GPIO_PIN_13;
	swuart_init_param.tx_port_pin.port_number = ADI_GPIO_PORT0;
	swuart_init_param.tx_port_pin.pin_number = ADI_GPIO_PIN_9;
	swuart_init_param.parity = UART_ODD_PARITY;
	swuart_init_param.no_of_bits = 8;
	swuart_init_param.type = ADICUP3029_SWUART;
	swuart_init_param.delay_timer = 1;
	ad5700_init.swuart_init = swuart_init_param;
	ad5700_init.gpio_nrts = 0x1b;
	ad5700_init.gpio_cd = 0x21;
	ad4111_ini.spi_init.chip_select = 0x1A;
	ad4111_ini.spi_init.id = 0;
	ad4111_ini.spi_init.max_speed_hz = 4000000;
	ad4111_ini.spi_init.mode = SPI_MODE_3;
	ad4111_ini.spi_init.type = SPI_ARDUINO;
	ad4111_ini.regs = ad4111_regs;
	ad4111_ini.num_regs = 55;
#if defined(CLI_INTEFACE)
	cli_init.uart_init.baudrate = bd115200;
	cli_init.uart_init.bits_no = 8;
#elif defined(MODBUS_INTERFACE)
	mb_slave_init.phy_layer.baudrate = bd19200;
	mb_slave_init.phy_layer.bits_no = 8;
	mb_slave_init.de_nre_no = 0x0f;
	mb_slave_init.slave_id_gpio[0] = 35;
	mb_slave_init.slave_id_gpio[1] = 36;
	mb_slave_init.slave_id_gpio[2] = 37;
	mb_slave_init.slave_id_gpio[3] = 38;
#endif
	sys_manager_init.ad5700_init = ad5700_init;
	sys_manager_init.memory_init = memory_init;
	sys_manager_init.cn0414_type_init.ad4111_ini = ad4111_ini;
	sys_manager_init.cn0414_type_init.adc_update_init.update_timer = 0;
	sys_manager_init.cn0414_type_init.adc_update_init.f_update = 10;
#if defined(CLI_INTEFACE)
	sys_manager_init.cli_init = cli_init;
#elif defined(MODBUS_INTERFACE)
	sys_manager_init.mb_slave_init = mb_slave_init;
#endif
	ad5755_init.spi_init.chip_select = 0x1A;
	ad5755_init.spi_init.id = 0;
	ad5755_init.spi_init.max_speed_hz = 4000000;
	ad5755_init.spi_init.mode = SPI_MODE_2;
	ad5755_init.spi_init.type = SPI_ARDUINO;
	ad5755_init.gpio_clr = 0x18;
	ad5755_init.gpio_ldac = 0x16;
	ad5755_init.gpio_poc = 0x19;
	ad5755_init.gpio_rst = 0x17;
	ad5755_init.this_device = ID_AD5755_1;
	sys_manager_init.cn0418_type_init.ad5755_init = ad5755_init;
	sys_manager_init.cn0414_type_init.gpio_hart_chan0 = 0x1e;
	sys_manager_init.cn0414_type_init.gpio_hart_chan1 = 0x1c;
	sys_manager_init.cn0418_type_init.hart_mult_a0 = 0x1e;
	sys_manager_init.cn0418_type_init.hart_mult_a1 = 0x1c;

	ret = system_setup(&system_manager, &sys_manager_init);
	if(ret < 0)
		return ret;
#if defined(CLI_INTEFACE)
	ret = cli_cmd_prompt(system_manager->cli_descriptor);
	if(ret < 0)
		return ret;
#endif
	while(1) {
		system_process(system_manager);
	}

	return 0;
}

