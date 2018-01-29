/*****************************************************************************
 * ADuCM3029_Greenhouse_Local.h
 *****************************************************************************/

#ifndef __ADUCM3029_GREENHOUSE_LOCAL_H__
#define __ADUCM3029_GREENHOUSE_LOCAL_H__

#include "sensors_data.h"
#include "telemetry.h"

extern "C" {
#include "Communication.h"
#include "json.h"
}

/* Defined in pinmux.c. */
extern "C" int32_t adi_initpinmux(void);
extern "C" int Leds_Controll(const char *str);

#endif /* __ADUCM3029_GREENHOUSE_LOCAL_H__ */
