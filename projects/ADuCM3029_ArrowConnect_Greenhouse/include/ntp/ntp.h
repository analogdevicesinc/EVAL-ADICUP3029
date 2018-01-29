/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_NTP_H_
#define ARROW_NTP_H_

#ifndef NTP_DEFAULT_SERVER
#define NTP_DEFAULT_SERVER "0.pool.ntp.org"
#endif

#ifndef NTP_DEFAULT_PORT
#define NTP_DEFAULT_PORT 123
#endif

#ifndef NTP_DEFAULT_TIMEOUT
#define NTP_DEFAULT_TIMEOUT 4000
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#include <unint.h>
#include <sys/platform.h>

int ntp_set_time_cycle(void);
int ntp_set_time_common(const char *server, uint16_t port, int timeout, int try_times);

#if defined(__cplusplus)
}
#endif

#endif /* ARROW_NTP_H_ */
