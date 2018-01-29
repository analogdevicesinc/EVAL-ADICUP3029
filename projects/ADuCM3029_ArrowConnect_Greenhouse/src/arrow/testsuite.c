#include "arrow/testsuite.h"
#include <http/routine.h>
#include <debug.h>

#define URI_LEN sizeof(ARROW_API_TESTSUITE_ENDPOINT) + 70

typedef struct _proc_gate_ {
  arrow_gateway_t *gate;
  test_procedure_t *proc;
} proc_gate_t;

static void _test_gateway_init(http_request_t *request, void *arg) {
  proc_gate_t *pg = (proc_gate_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/gateways/test", ARROW_API_TESTSUITE_ENDPOINT);
  http_request_init(request, POST, uri);
  FREE_CHUNK(uri);
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "hid", json_mkstring(P_VALUE(pg->gate->hid)));
  json_append_member(_main, "testProcedureHid", json_mkstring(P_VALUE(pg->proc->hid)));
  http_request_set_payload(request, p_heap(json_encode(_main)));
  json_delete(_main);
}

static int _test_gateway_proc(http_response_t *response, void *arg) {
  test_procedure_t *proc = (test_procedure_t *)arg;
  if ( response->m_httpResponseCode != 200 ) return -1;
  JsonNode *_main = json_decode(P_VALUE(response->payload.buf));
  JsonNode *test_res = json_find_member(_main, "hid");
  if ( !test_res ) {
    json_delete(_main);
    return -1;
  }
  DBG("test result hid %s", test_res->string_);
  P_COPY(proc->result_hid, p_stack(test_res->string_));
  return 0;
}

int arrow_test_gateway(arrow_gateway_t *gateway, test_procedure_t *proc) {
  proc_gate_t pg = { gateway, proc };
  int ret = __http_routine(_test_gateway_init, &pg, _test_gateway_proc, proc);
  if ( ret < 0 ) {
    DBG("Arrow TEST gateway failed...");
  }
  return ret;
}

typedef struct _proc_dev_ {
  arrow_device_t *dev;
  test_procedure_t *proc;
} proc_dev_t;

static void _test_device_init(http_request_t *request, void *arg) {
  proc_dev_t *pd = (proc_dev_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/devices/test", ARROW_API_TESTSUITE_ENDPOINT);
  http_request_init(request, POST, uri);
  FREE_CHUNK(uri);
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "hid", json_mkstring(P_VALUE(pd->dev->hid)));
  json_append_member(_main, "testProcedureHid", json_mkstring(P_VALUE(pd->proc->hid)));
  http_request_set_payload(request, p_heap(json_encode(_main)));
  json_delete(_main);
}

static int _test_device_proc(http_response_t *response, void *arg) {
  test_procedure_t *proc = (test_procedure_t *)arg;
  if ( response->m_httpResponseCode != 200 ) return -1;
  JsonNode *_main = json_decode(P_VALUE(response->payload.buf));
  JsonNode *test_res = json_find_member(_main, "hid");
  if ( !test_res ) {
    json_delete(_main);
    return -1;
  }
  DBG("test result hid %s", test_res->string_);
  P_COPY(proc->result_hid, p_stack(test_res->string_));
  return 0;
}

int arrow_test_device(arrow_device_t *device, test_procedure_t *proc) {
  proc_dev_t pd = {device, proc};
  int ret = __http_routine(_test_device_init, &pd, _test_device_proc, proc);
  if ( ret < 0 ) {
    DBG("Arrow TEST device failed...");
  }
  return ret;
}

static void _test_begin_init(http_request_t *request, void *arg) {
  test_procedure_t *proc = (test_procedure_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/begin", ARROW_API_TESTSUITE_ENDPOINT, P_VALUE(proc->result_hid));
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
}

int arrow_test_begin(test_procedure_t *proc) {
  int ret = __http_routine(_test_begin_init, proc, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST begin failed...");
  }
  return ret;
}

static void _test_end_init(http_request_t *request, void *arg) {
  test_procedure_t *proc = (test_procedure_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/end", ARROW_API_TESTSUITE_ENDPOINT, P_VALUE(proc->result_hid));
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
}

int arrow_test_end(test_procedure_t *hid) {
  int ret = __http_routine(_test_end_init, hid, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST end failed...");
  }
  return ret;
}

typedef struct _test_step_ {
  test_procedure_t *proc;
  int number;
  int status;
  const char *error;
} test_step_t;

enum {
  SUCCESS,
  FAIL
};

static void _test_step_begin_init(http_request_t *request, void *arg) {
  test_step_t *st = (test_step_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/steps/%d/begin",
           ARROW_API_TESTSUITE_ENDPOINT,
           P_VALUE(st->proc->result_hid),
           st->number);
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
}

int arrow_test_step_begin(test_procedure_t *proc, int number) {
  test_step_t st = { proc, number, SUCCESS, NULL };
  int ret = __http_routine(_test_step_begin_init, &st, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST begin step failed...");
  }
  return ret;
}

static void _test_step_end_init(http_request_t *request, void *arg) {
  test_step_t *st = (test_step_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/steps/%d/end",
           ARROW_API_TESTSUITE_ENDPOINT,
           P_VALUE(st->proc->result_hid),
           st->number);
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "error", json_mkstring(st->error));
  switch(st->status) {
    case SUCCESS:
      json_append_member(_main, "status", json_mkstring("SUCCESS"));
    break;
    case FAIL:
      json_append_member(_main, "status", json_mkstring("FAIL"));
    break;
  }
  http_request_set_payload(request, p_heap(json_encode(_main)));
  json_delete(_main);
}

int arrow_test_step_end(test_procedure_t *proc, int number, int status, const char *err) {
  test_step_t st = {proc, number, status, err};
  int ret = __http_routine(_test_step_end_init, &st, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST end step failed...");
  }
  return ret;
}

int arrow_test_step_success(test_procedure_t *proc, int number) {
  return arrow_test_step_end(proc, number, SUCCESS, "");
}

int arrow_test_step_fail(test_procedure_t *proc, int number, const char *error) {
  return arrow_test_step_end(proc, number, FAIL, error);
}

static void _test_step_skip_init(http_request_t *request, void *arg) {
  test_step_t *st = (test_step_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/steps/%d/skip",
           ARROW_API_TESTSUITE_ENDPOINT,
           P_VALUE(st->proc->result_hid),
           st->number);
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
}

int arrow_test_step_skip(test_procedure_t *proc, int number) {
  test_step_t st = {proc, number, SUCCESS, NULL};
  int ret = __http_routine(_test_step_skip_init, &st, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST skip step failed...");
  }
  return ret;
}

