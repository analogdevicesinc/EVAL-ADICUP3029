/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/** \file
NTP Client header file
*/

#ifndef NTPCLIENT_H_
#define NTPCLIENT_H_

#include <arrow/mem.h>
#if defined(_ARIS_)
# if defined(ETH_MODE)
#  include "nx_api.h"
#  include "nx_udp.h"
#  include "nxd_dns.h"
# endif
# include "hal_data.h"
#endif

#include <unint.h>

#if defined (__ICCARM__)
    #pragma pack()
#endif
  struct NTPPacket //See RFC 4330 for Simple NTP
  {
    //WARN: We are in LE! Network is BE!
    //LSb first
    unsigned mode : 3;
    unsigned vn : 3;
    unsigned li : 2;

    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;
    //32 bits header

    uint32_t rootDelay;
    uint32_t rootDispersion;
    uint32_t refId;

    uint32_t refTm_s;
    uint32_t refTm_f;
    uint32_t origTm_s;
    uint32_t origTm_f;
    uint32_t rxTm_s;
    uint32_t rxTm_f;
    uint32_t txTm_s;
    uint32_t txTm_f;
#if defined (__ICCARM__)
  };
#else
  } __attribute_packed__;
#endif

///NTP client results
enum NTPResult
{
  NTP_DNS, ///<Could not resolve name
  NTP_PRTCL, ///<Protocol error
  NTP_TIMEOUT, ///<Connection timeout
  NTP_CONN, ///<Connection error
  NTP_OK = 0, ///<Success
};


/**Get current time (blocking)
  Update the time using the server host
  Blocks until completion
  @param host NTP server IPv4 address or hostname (will be resolved via DNS)
  @param port port to use; defaults to 123
  @param timeout waiting timeout in ms (osWaitForever for blocking function, not recommended)
  @return 0 on success, NTP error code (<0) on failure
 */
int ntp_set_time(const char* host, uint16_t port, uint32_t timeout);

#endif /* NTPCLIENT_H_ */
