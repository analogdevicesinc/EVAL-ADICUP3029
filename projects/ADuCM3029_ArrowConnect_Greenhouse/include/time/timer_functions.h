/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#if !defined(TIMER_FUNCTIONS)
#define TIMER_FUNCTIONS

#define timerclear(tvp)     ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#define timerisset(tvp)     ((tvp)->tv_sec || (tvp)->tv_usec)
#define timercmp(tvp, uvp, cmp)                 \
        (((tvp)->tv_sec == (uvp)->tv_sec) ?             \
                ((tvp)->tv_usec cmp (uvp)->tv_usec) :           \
                ((tvp)->tv_sec cmp (uvp)->tv_sec))
#define timeradd(tvp, uvp, vvp)                     \
        do {                                \
            (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;      \
            (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;   \
            if ((vvp)->tv_usec >= 1000000) {            \
                (vvp)->tv_sec++;                \
                (vvp)->tv_usec -= 1000000;          \
            }                           \
        } while (0)
#define timersub(tvp, uvp, vvp)                     \
        do {                                \
            (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;      \
            (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;   \
            if ((int)(vvp)->tv_usec < 0) {               \
                (vvp)->tv_sec--;                \
                (vvp)->tv_usec += 1000000;          \
            }                           \
        } while (0)


#endif
