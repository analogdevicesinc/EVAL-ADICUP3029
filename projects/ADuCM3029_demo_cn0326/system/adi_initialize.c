/*
** adi_initialize.c source file generated on February 12, 2018 at 16:30:12.
**
** Copyright (C) 2000-2018 Analog Devices Inc., All Rights Reserved.
**
** This file is generated automatically. You should not modify this source file,
** as your changes will be lost if this source file is re-generated.
*/

#include <sys/platform.h>

#include "adi_initialize.h"
#include "drivers/pwr/adi_pwr.h"

#include "AD7793.h"
#include "CN0326.h"
#include "Timer.h"

extern int32_t adi_initpinmux(void);

int32_t adi_initComponents(void)
{
	int32_t result = 0;

	if (result == 0) {
		adi_initpinmux(); /* Initialize pin multiplexers */
		adi_init_power(); /* Initialize power supply and clock source */
		timer_start();    /* Start the System Tick Timer. */
		AD7793_Init();    /* AD7793 converter initialization part */
		CN0326_Init();    /* CN0326 application initialization part */
	}

	return result;
}

uint32_t adi_init_power()
{
	uint32_t status;
	status = adi_pwr_Init();
	if(status != ADI_PWR_SUCCESS)
		return 1;
	/* Enable external 26MHz oscillator as clock source */
	status = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1);
	if(status != ADI_PWR_SUCCESS)
		return 1;
	/* Enable clock to the UART module */
	status = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1);
	if(status != ADI_PWR_SUCCESS)
		return 1;
	return 0;
}

