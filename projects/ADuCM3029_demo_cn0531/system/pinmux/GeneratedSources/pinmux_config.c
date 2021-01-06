/*
 **
 ** Source file generated on August 21, 2020 at 11:57:50.	
 **
 ** Copyright (C) 2011-2020 Analog Devices Inc., All Rights Reserved.
 **
 ** This file is generated automatically based upon the options selected in 
 ** the Pin Multiplexing configuration editor. Changes to the Pin Multiplexing
 ** configuration should be made by changing the appropriate options rather
 ** than editing this file.
 **
 ** Selected Peripherals
 ** --------------------
 ** SPI1 (SCLK, MISO, MOSI, CS_0)
 ** UART0 (Tx, Rx)
 ** ADC0_IN (ADC0_IN0, ADC0_IN1, ADC0_IN2, ADC0_IN3, ADC0_IN4, ADC0_IN5)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P0_10, P0_11, P1_06, P1_07, P1_08, P1_09, P2_03, P2_04, P2_05, P2_06, P2_07, P2_08
 */

#include <sys/platform.h>
#include <stdint.h>

#define SPI1_SCLK_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<12))
#define SPI1_MISO_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<14))
#define SPI1_MOSI_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<16))
#define SPI1_CS_0_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<18))
#define UART0_TX_PORTP0_MUX  ((uint32_t) ((uint32_t) 1<<20))
#define UART0_RX_PORTP0_MUX  ((uint32_t) ((uint32_t) 1<<22))
#define ADC0_IN_ADC0_IN0_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<6))
#define ADC0_IN_ADC0_IN1_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<8))
#define ADC0_IN_ADC0_IN2_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<10))
#define ADC0_IN_ADC0_IN3_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<12))
#define ADC0_IN_ADC0_IN4_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<14))
#define ADC0_IN_ADC0_IN5_PORTP2_MUX  ((uint32_t) ((uint32_t) 1<<16))

int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* PORTx_MUX registers */
    *((volatile uint32_t *)REG_GPIO0_CFG) = UART0_TX_PORTP0_MUX | UART0_RX_PORTP0_MUX;
    *((volatile uint32_t *)REG_GPIO1_CFG) = SPI1_SCLK_PORTP1_MUX | SPI1_MISO_PORTP1_MUX
     | SPI1_MOSI_PORTP1_MUX | SPI1_CS_0_PORTP1_MUX;
    *((volatile uint32_t *)REG_GPIO2_CFG) = ADC0_IN_ADC0_IN0_PORTP2_MUX | ADC0_IN_ADC0_IN1_PORTP2_MUX
     | ADC0_IN_ADC0_IN2_PORTP2_MUX | ADC0_IN_ADC0_IN3_PORTP2_MUX | ADC0_IN_ADC0_IN4_PORTP2_MUX
     | ADC0_IN_ADC0_IN5_PORTP2_MUX;

    return 0;
}

