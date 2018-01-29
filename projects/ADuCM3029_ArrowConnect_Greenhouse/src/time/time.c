/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "time/time.h"

time_t build_time(void) {
  static const char *built = __DATE__" "__TIME__;
  struct tm t;
  const char *ret = (const char *)strptime(built, "%b %d %Y %H:%M:%S", &t);
  if ( ret ) {
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    return mktime(&t);
  }

  return 0;
}

#if !defined(TARGET_NUCLEO_F401RE)
void set_time(time_t t) {
    stime(&t);
}
#endif

