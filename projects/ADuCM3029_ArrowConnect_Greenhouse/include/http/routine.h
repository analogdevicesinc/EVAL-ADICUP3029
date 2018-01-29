/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef HTTPCLIENT_ROUTINE_H_
#define HTTPCLIENT_ROUTINE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <http/request.h>
#include <http/client.h>

typedef void(*response_init_f)(http_request_t *request, void *arg);
typedef int (*response_proc_f)(http_response_t *response, void *arg);

int __http_routine(response_init_f req_init, void *arg_init,
                   response_proc_f resp_proc, void *arg_proc);

#define STD_ROUTINE(init, i_arg, proc, p_arg, fail_msg) { \
  int ret = __http_routine(init, i_arg, proc, p_arg); \
  if ( ret < 0 ) { \
    if (fail_msg) DBG("Error:" fail_msg); \
  } \
  return ret; \
}

http_client_t *current_client(void);

#if defined(__cplusplus)
}
#endif

#endif /* HTTPCLIENT_ROUTINE_H_ */
