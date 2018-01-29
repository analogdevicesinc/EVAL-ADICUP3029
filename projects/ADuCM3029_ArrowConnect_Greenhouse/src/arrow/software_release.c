#include "arrow/software_release.h"
#include <http/routine.h>
#include <debug.h>
#include <time/watchdog.h>
#include <arrow/sys.h>

#define URI_LEN sizeof(ARROW_API_SOFTWARE_RELEASE_ENDPOINT) + 200

static __release_cb  __release = NULL;
static __download_payload_cb  __payload = NULL;
static __download_complete_cb __download = NULL;

static char *serialize_software_trans(const char *hid, release_sched_t *rs) {
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "objectHid", json_mkstring(hid));
  json_append_member(_main, "softwareReleaseScheduleHid", json_mkstring(rs->schedule_hid));
  json_append_member(_main, "toSoftwareReleaseHid", json_mkstring(rs->release_hid));
  return json_encode(_main);
}

typedef struct _gateway_software_sched_ {
  arrow_gateway_t *gate;
  release_sched_t *rs;
} gateway_software_sched_t;

static void _gateway_software_releases_trans_init(http_request_t *request, void *arg) {
  gateway_software_sched_t *gs = (gateway_software_sched_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  strcpy(uri, ARROW_API_SOFTWARE_RELEASE_ENDPOINT);
  strcat(uri, "/gateways/upgrade");
  http_request_init(request, POST, uri);
  FREE_CHUNK(uri);
  char *payload = serialize_software_trans(P_VALUE(gs->gate->hid), gs->rs);
  http_request_set_payload(request, p_heap(payload));
}

static int _gateway_software_releases_trans_proc(http_response_t *response, void *arg) {
  release_sched_t *rs = (release_sched_t *)arg;
  if ( IS_EMPTY(response->payload.buf) )  return -1;
  JsonNode *_main = json_decode(P_VALUE(response->payload.buf));
  JsonNode *hid = json_find_member(_main, "hid");
  if ( !hid ) return -1;
  property_copy(&rs->trans_hid, p_stack(hid->string_));
  return 0;
}

int arrow_gateway_software_releases_trans(arrow_gateway_t *gate, release_sched_t *rs) {
  gateway_software_sched_t sch = {gate, rs};
  P_CLEAR(rs->trans_hid);
  STD_ROUTINE(_gateway_software_releases_trans_init, &sch, _gateway_software_releases_trans_proc, NULL, "Software Trans fail");
}

typedef struct _device_software_sched_ {
  arrow_device_t *gate;
  release_sched_t *rs;
} device_software_sched_t;

static void _device_software_releases_trans_init(http_request_t *request, void *arg) {
  device_software_sched_t *gs = (device_software_sched_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  strcpy(uri, ARROW_API_SOFTWARE_RELEASE_ENDPOINT);
  strcat(uri, "/devices/upgrade");
  http_request_init(request, POST, uri);
  FREE_CHUNK(uri);
  char *payload = serialize_software_trans(P_VALUE(gs->gate->hid), gs->rs);
  http_request_set_payload(request, p_heap(payload));
}

static int _device_software_releases_trans_proc(http_response_t *response, void *arg) {
  release_sched_t *rs = (release_sched_t *)arg;
  if ( IS_EMPTY(response->payload.buf) )  return -1;
  JsonNode *_main = json_decode(P_VALUE(response->payload.buf));
  JsonNode *hid = json_find_member(_main, "hid");
  if ( !hid ) return -1;
  property_copy(&rs->trans_hid, p_stack(hid->string_));
  return 0;
}


int arrow_device_software_releases_trans(arrow_device_t *dev, release_sched_t *rs) {
  device_software_sched_t sch = {dev, rs};
  P_CLEAR(rs->trans_hid);
  STD_ROUTINE(_device_software_releases_trans_init, &sch, _device_software_releases_trans_proc, NULL, "Software Trans fail");
}

typedef enum {
  received,
  success,
  fail
} state_enums;

typedef struct _ans_ {
  state_enums state;
  const char *hid;
  const char *error;
} ans_t;

static void _software_releases_ans_init(http_request_t *request, void *arg) {
  ans_t *ans = (ans_t *)arg;
  int n = 0;
  CREATE_CHUNK(uri, URI_LEN);
  switch(ans->state) {
    case received:
      n = snprintf(uri, URI_LEN, "%s/%s/received", ARROW_API_SOFTWARE_RELEASE_ENDPOINT, ans->hid);
    break;
    case success:
      n = snprintf(uri, URI_LEN, "%s/%s/succeeded", ARROW_API_SOFTWARE_RELEASE_ENDPOINT, ans->hid);
    break;
    case fail:
      n = snprintf(uri, URI_LEN, "%s/%s/failed", ARROW_API_SOFTWARE_RELEASE_ENDPOINT, ans->hid);
    break;
  }
  uri[n] = 0x0;
  DBG("uri %s", uri);
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
  if ( ans->state == fail && ans->error ) {
    JsonNode *_error = json_mkobject();
    json_append_member(_error, "error", json_mkstring(ans->error));
    http_request_set_payload(request, p_heap(json_encode(_error)));
  }
}

int arrow_software_releases_trans_fail(const char *hid, const char *error) {
  ans_t ans = {fail, hid, error};
  STD_ROUTINE(_software_releases_ans_init, &ans, NULL, NULL, "Software Trans fail");
}

int arrow_software_releases_trans_received(const char *hid) {
  ans_t ans = {received, hid, NULL};
  STD_ROUTINE(_software_releases_ans_init, &ans, NULL, NULL, "Software Trans fail");
}

int arrow_software_releases_trans_success(const char *hid) {
  ans_t ans = {success, hid, NULL};
  STD_ROUTINE(_software_releases_ans_init, &ans, NULL, NULL, "Software Trans fail");
}

static void _software_releases_start_init(http_request_t *request, void *arg) {
  const char *hid = (const char *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int n = snprintf(uri, URI_LEN, "%s/%s/start", ARROW_API_SOFTWARE_RELEASE_ENDPOINT, hid);
  uri[n] = 0x0;
  http_request_init(request, POST, uri);
  FREE_CHUNK(uri);
}

int arrow_software_releases_trans_start(const char *hid) {
  STD_ROUTINE(_software_releases_start_init, (void*)hid, NULL, NULL, "Software Trans start fail");
}

int ev_DeviceSoftwareRelease(void *_ev, JsonNode *_parameters) {
  SSP_PARAMETER_NOT_USED(_ev);
  JsonNode *tmp = json_find_member(_parameters, "softwareReleaseTransHid");
  if ( !tmp || tmp->tag != JSON_STRING ) return -1;
  char *trans_hid = tmp->string_;
  arrow_software_releases_trans_received(trans_hid);
  tmp = json_find_member(_parameters, "tempToken");
  if ( !tmp || tmp->tag != JSON_STRING ) return -1;
  char *_token = tmp->string_;
  DBG("FW TOKEN: %s", tmp->string_);
  DBG("FW HID: %s", trans_hid);
//  msleep(100000);
  tmp = json_find_member(_parameters, "fromSoftwareVersion");
  if ( !tmp || tmp->tag != JSON_STRING ) return -1;
  char *_from = tmp->string_;
  tmp = json_find_member(_parameters, "toSoftwareVersion");
  if ( !tmp || tmp->tag != JSON_STRING ) return -1;
  char *_to = tmp->string_;
  tmp = json_find_member(_parameters, "md5checksum");
  if ( !tmp || tmp->tag != JSON_STRING ) return -1;
  char *_checksum = tmp->string_;
  wdt_feed();
  int ret = arrow_software_release_download(_token, trans_hid);
  wdt_feed();
  SSP_PARAMETER_NOT_USED(_checksum);
  SSP_PARAMETER_NOT_USED(_to);
  SSP_PARAMETER_NOT_USED(_from);
//  int ret = arrow_software_release(_token, _checksum, _from, _to);
  if ( ret < 0 ) {
    arrow_software_releases_trans_fail(trans_hid, "failed");
  } else {
    arrow_software_releases_trans_success(trans_hid);
    reboot();
  }
  return ret;
}

void create_release_schedule(release_sched_t *rs, const char *shed_hid, const char *rel_hid) {
  P_CLEAR(rs->trans_hid);
  rs->schedule_hid = shed_hid;
  rs->release_hid = rel_hid;
}

void free_release_schedule(release_sched_t *rs) {
  property_free(&rs->trans_hid);
}

int __attribute__((weak)) arrow_software_release(const char *token,
                                                 const char *chsum,
                                                 const char *from,
                                                 const char *to) {
  if ( __release ) return __release(token, chsum, from, to);
  return -1;
}

int arrow_software_release_set_cb(__release_cb cb) {
  __release = cb;
  return 0;
}

typedef struct _token_hid_ {
  const char *token;
  const char *hid;
} token_hid_t;


// set the callback for update file processing
int arrow_software_release_dowload_set_cb(
    __download_payload_cb pcb,
    __download_complete_cb ccb) {
  __payload = pcb;
  __download = ccb;
  return 0;
}

// this is a special payload handler for the OTA
int arrow_software_release_payload_handler(void *r,
                                           property_t payload,
                                           int size) {
  http_response_t *res = (http_response_t *)r;
  property_t *response_buffer = &res->payload.buf;
  return __payload(response_buffer, payload.value, size);
}

static void _software_releases_download_init(http_request_t *request, void *arg) {

  token_hid_t *th = (token_hid_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  SSP_PARAMETER_NOT_USED(th);
  int n = snprintf(uri, URI_LEN, "%s/%s/%s/file", ARROW_API_SOFTWARE_RELEASE_ENDPOINT, th->hid, th->token);
  uri[n] = 0x0;
//  strcpy(uri, "http://mirror.tochlab.net:80/pub/gnu/hello/hello-1.3.tar.gz");
  http_request_init(request, GET, uri);
  request->_response_payload_meth._p_add_handler = arrow_software_release_payload_handler;
  FREE_CHUNK(uri);
  wdt_feed();
}

static int _software_releases_download_proc(http_response_t *response, void *arg) {
//  release_sched_t *rs = (release_sched_t *)arg;
  SSP_PARAMETER_NOT_USED(arg);
  wdt_feed();
//  if ( IS_EMPTY(response->payload.buf) )  return -1;
  DBG("file size : %d", response->payload.size);
  if ( __download ) return __download(&response->payload.buf);
  return 0;
}

int arrow_software_release_download(const char *token, const char *tr_hid) {
  token_hid_t th = { token, tr_hid };
  STD_ROUTINE(_software_releases_download_init, &th, _software_releases_download_proc, NULL, "File download fail");
}
