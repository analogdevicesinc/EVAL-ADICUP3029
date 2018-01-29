/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/account.h"
#include <http/routine.h>
#include <json/json.h>
#include <debug.h>

typedef struct _account {
  const char *name;
  const char *email;
  const char *password;
} account_t;

static void _account_request(http_request_t *request, void *arg) {
  http_request_init(request, POST, ARROW_API_ACCOUNT_ENDPOINT);
  account_t *acc = (account_t *)arg;
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "name", json_mkstring(acc->name));
  json_append_member(_main, "email", json_mkstring(acc->email));
  json_append_member(_main, "password", json_mkstring(acc->password));
  char *payload = json_encode(_main);
  json_minify(payload);
  http_request_set_payload(request, p_heap(payload));
  json_delete(_main);
}

static int _account_process(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("account res", response->m_httpResponseCode);
  if ( response->m_httpResponseCode != 200 )
    return -1;
  return 0;
}

int arrow_create_account(const char *name, const char *email, const char *pass) {
  int ret = 0;
  account_t acc = {name, email, pass};
  ret = __http_routine(_account_request, &acc, _account_process, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Account create failed...");
  }
  return ret;
}
