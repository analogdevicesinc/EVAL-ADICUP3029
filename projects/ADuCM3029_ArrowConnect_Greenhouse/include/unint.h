/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef KRONOS_C_SDK_UNINT_H_
#define KRONOS_C_SDK_UNINT_H_

#include <config.h>

#if defined(_ARIS_)
# include <inttypes.h>
# include <sys/types.h>

#elif defined(__MBED__)
# include <inttypes.h>
# include <sys/types.h>

#elif defined(__XCC__)
# include <qcom/basetypes.h>
# include <qcom/stdint.h>
typedef A_INT64 int64_t;

#elif defined(__linux__)
# include <inttypes.h>

#elif defined(__senseability__)
# include <cytypes.h>
# include <sys/types.h>
typedef unsigned int size_t;

#elif defined(__stm32l475iot__)
# include <stdint.h>
# include <sys/types.h>

#elif defined(__semiconductor__)
# include <inttypes.h>
# include <sys/types.h>
#endif

#if defined(__USE_STD__)
#include <stdbool.h>
#else
typedef int bool;
#define false 0
#define true 1
#endif

#endif // KRONOS_C_SDK_UNINT_H_
