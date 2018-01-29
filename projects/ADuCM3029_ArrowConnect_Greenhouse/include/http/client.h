 /* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef HTTPCLIENT_CLIENT_H_
#define HTTPCLIENT_CLIENT_H_

#include "request.h"
#if !defined(__XCC__)
#else
#define SSL_INBUF_SIZE               6000
#define SSL_OTA_INBUF_SIZE           20000
#define SSL_OUTBUF_SIZE              3500
#endif

#if defined(_ARIS_) && defined(ETH_MODE)
#include "nx_api.h"
#include "nxd_dns.h"
#endif

typedef struct __session_flags {
  int _new   : 16;
  int _close : 16;
} __session_flags_t;

typedef int (*rw_func)(uint8_t *, uint16_t, void *);

typedef struct {
  int sock;
  uint32_t timeout;
  int response_code;
  __session_flags_t flags;
  rw_func         _r_func;
  rw_func         _w_func;
} http_client_t;

void http_session_close_set(http_client_t *cli, bool mode);
bool http_session_close(http_client_t *cli);

void http_client_init(http_client_t *cli);
void http_client_free(http_client_t *cli);

int http_client_do(http_client_t *cli, http_request_t *req, http_response_t *res);

#endif /* HTTPCLIENT_CLIENT_H_ */
