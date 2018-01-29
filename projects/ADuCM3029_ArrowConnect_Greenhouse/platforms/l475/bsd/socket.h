/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef _STM32L475IOT_BSD_SOCKET_H_
#define _STM32L475IOT_BSD_SOCKET_H_

#include <unint.h>
# include <bsd/inet.h>
# include <platforms/default/bsd/sockdef.h>
# include <platforms/default/bsd/sockdecl.h>
# include <bsd/struct_hostent.h>
# define htons _htons
# define htonl _htonl
# define ntohl _ntohl

#endif // _STM32L475IOT_BSD_SOCKET_H_
