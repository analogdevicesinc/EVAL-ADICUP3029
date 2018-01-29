/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#define MODULE_NAME "HTTP_Request"

#include "http/routine.h"
#include <http/client.h>
#include <arrow/request.h>
#include <debug.h>

static http_client_t _cli = { -1, -1, 0, {1, 1}, NULL, NULL };

http_client_t *current_client(void) {
  return &_cli;
}

int __http_routine(response_init_f req_init, void *arg_init,
                   response_proc_f resp_proc, void *arg_proc) {
  int ret = 0;
  http_request_t request;
  http_response_t response;
  http_client_init(&_cli);
  req_init(&request, arg_init);
  sign_request(&request);
  ret = http_client_do(&_cli, &request, &response);
  http_request_close(&request);
  http_client_free(&_cli);
  if ( ret < 0 ) goto http_error;
  if ( resp_proc ) {
    ret = resp_proc(&response, arg_proc);
  } else {
    if ( response.m_httpResponseCode != 200 ) {
      ret = -1;
      goto http_error;
    }
  }
http_error:
  http_response_free(&response);
  return ret;
}
