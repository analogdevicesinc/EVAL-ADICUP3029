/*
 **
 ** Source file generated on February 12, 2018 at 16:30:12.	
 **
 ** Copyright (C) 2011-2018 Analog Devices Inc., All Rights Reserved.
 **
 ** This file is generated automatically based upon the options selected in 
 ** the Pin Multiplexing configuration editor. Changes to the Pin Multiplexing
 ** configuration should be made by changing the appropriate options rather
 ** than editing this file.
 **
 ** Selected Peripherals
 ** --------------------
 */

#include <sys/platform.h>
#include <stdint.h>

#define SPI0_SCLK_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<0))
#define SPI0_MOSI_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<2))
#define SPI0_MISO_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<4))
#define SPI0_CS_1_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<20))
#define SPI1_SCLK_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<12))
#define SPI1_MISO_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<14))
#define SPI1_MOSI_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<16))
#define SPI1_CS_0_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<18))
#define SPI2_SCLK_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<4))
#define SPI2_MISO_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<6))
#define SPI2_MOSI_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<8))
#define SPI2_CS_0_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<10))
#define I2C0_SCL0_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<8))
#define I2C0_SDA0_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<10))
#define UART0_TX_PORTP0_MUX  ((uint32_t) ((uint32_t) 1<<20))
#define UART0_RX_PORTP0_MUX  ((uint32_t) ((uint32_t) 1<<22))
#define UART0_UART_SOUT_EN_PORTP0_MUX  ((uint32_t) ((uint32_t) 3<<24))
#define SYS_CLK_CLOCK_OUT_PORTP2_MUX  ((uint32_t) ((uint32_t) 2<<22))

int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
	/* Configure pin multiplexers */
	*((volatile uint32_t *)REG_GPIO0_CFG) = SPI0_SCLK_PORTP0_MUX | SPI0_MOSI_PORTP0_MUX
	     | SPI0_MISO_PORTP0_MUX | I2C0_SCL0_PORTP0_MUX | I2C0_SDA0_PORTP0_MUX
	     | UART0_TX_PORTP0_MUX | UART0_RX_PORTP0_MUX | UART0_UART_SOUT_EN_PORTP0_MUX;
	*((volatile uint32_t *)REG_GPIO1_CFG) = SPI0_CS_1_PORTP1_MUX | SPI1_SCLK_PORTP1_MUX
	     | SPI1_MISO_PORTP1_MUX | SPI1_MOSI_PORTP1_MUX | SPI1_CS_0_PORTP1_MUX
	     | SPI2_SCLK_PORTP1_MUX | SPI2_MISO_PORTP1_MUX | SPI2_MOSI_PORTP1_MUX
	     | SPI2_CS_0_PORTP1_MUX;
	*((volatile uint32_t *)REG_GPIO2_CFG) = SYS_CLK_CLOCK_OUT_PORTP2_MUX;
    return 0;
}

