/*
 **
 ** Source file generated on February 1, 2017 at 12:36:57.
 **
 ** Copyright (C) 2017 Analog Devices Inc., All Rights Reserved.
 **
 ** This file is generated automatically based upon the options selected in
 ** the Pin Multiplexing configuration editor. Changes to the Pin Multiplexing
 ** configuration should be made by changing the appropriate options rather
 ** than editing this file.
 **
 ** Selected Peripherals
 ** --------------------
 ** SPI0 (SCLK, MOSI, MISO, CS_0, CS_1, CS_2, CS_3, RDY)
 ** SPI1 (SCLK, MISO, MOSI, CS_0, CS_1, CS_2, RDY)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P0_00, P0_01, P0_02, P0_03, P0_14, P1_06, P1_07, P1_08, P1_09, P1_10, P1_14, P2_02,
 ** P2_08, P2_09, P2_11
 */

#include <sys/platform.h>
#include <stdint.h>


#define SPI2_SCLK_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<4))
#define SPI2_MISO_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<6))
#define SPI2_MOSI_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<8))
#define SPI0_SCLK_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<0))
#define SPI0_MOSI_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<2))
#define SPI0_MISO_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<4))
#define SPI0_CS_1_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<20))

int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void)
{
	/* PORTx_MUX registers */
	*((volatile uint32_t *)REG_GPIO0_CFG) = SPI0_SCLK_PORTP0_MUX |
						SPI0_MOSI_PORTP0_MUX
						| SPI0_MISO_PORTP0_MUX;
	*((volatile uint32_t *)REG_GPIO1_CFG) = SPI0_CS_1_PORTP1_MUX |
						SPI2_SCLK_PORTP1_MUX | SPI2_MISO_PORTP1_MUX | SPI2_MOSI_PORTP1_MUX;

	return 0;
}
