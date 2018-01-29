/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef STM32L475IOT_TIME_TIME_H_
#define STM32L475IOT_TIME_TIME_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <config.h>
#include <time.h>
#include <sys/time.h>
#include <platforms/default/time/time.h>

int stime (const time_t *__when);

#if defined(__cplusplus)
}
#endif

#endif // STM32L475IOT_TIME_TIME_H_
