/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef KRONOS_C_SDK_ARROW_NET_H_
#define KRONOS_C_SDK_ARROW_NET_H_

#if defined(__cplusplus)
extern "C" {
#endif

int get_mac_address(char *mac);

#if defined(_ARIS_)
# include "driver/source/nmasic.h"
# if defined(ETH_MODE)
#  include "nxd_bsd.h"
# endif
#elif defined(__MBED__)
//# include "WiFi.h"
//# include <type>
#elif defined(__linux__)
# include <stdlib.h>
#endif

#if defined(__cplusplus)
}
#endif

#endif  // KRONOS_C_SDK_ARROW_NET_H_
