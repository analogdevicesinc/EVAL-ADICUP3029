/*
** adi_initialize.c source file generated on July 30, 2020 at 11:50:44.
**
** Copyright (C) 2000-2020 Analog Devices Inc., All Rights Reserved.
**
** This file is generated automatically. You should not modify this source file,
** as your changes will be lost if this source file is re-generated.
*/

#include <sys/platform.h>

#include "adi_initialize.h"

extern int32_t adi_initpinmux(void);

int32_t adi_initComponents(void)
{
	int32_t result = 0;


	if (result == 0) {
		result = adi_initpinmux(); /* auto-generated code (order:0) */
	}

	return result;
}

