#include "arrow/telemetry_api.h"
#include <arrow/find_by.h>
#include <json/telemetry.h>
#include <http/routine.h>
#include <debug.h>

#define URI_LEN sizeof(ARROW_API_TELEMETRY_ENDPOINT) + 50

int telemetry_response_data_list_init(telemetry_response_data_list_t *data, int size, int page, int tsize, int tpage) {
  data->size = size;
  data->page = page;
  data->totalPages = tpage;
  data->totalSize = tsize;
  data->data = NULL;
  return 0;
}

static void add_to_tail(telemetry_data_info_t **info,
                        const char *deviceHid,
                        const char *name,
                        const char *type,
                        time_t timestamp,
                        int flval) {
  if (! *info ) {
    *info = malloc(sizeof(telemetry_data_info_t));
    (*info)->deviceHid = strdup(deviceHid);
    (*info)->name = strdup(name);
    (*info)->type = strdup(type);
    (*info)->timestamp = timestamp;
    (*info)->floatValue = flval;
    (*info)->next = NULL;
  } else {
    add_to_tail(&((*info)->next), deviceHid, name, type, timestamp, flval);
  }
}

static void free_at_last(telemetry_data_info_t *info) {
  if ( info->next ) free_at_last(info->next);
  if ( info->deviceHid ) free(info->deviceHid);
  if ( info->name ) free(info->name);
  if ( info->type ) free(info->type);
  free(info);
}


int telemetry_response_data_list_free(telemetry_response_data_list_t *data) {
  free_at_last(data->data);
  return 0;
}

int add_telemetry_data_info(telemetry_response_data_list_t *data,
                            const char *deviceHid,
                            const char *name,
                            const char *type,
                            time_t timestamp,
                            int flval) {
  add_to_tail(&data->data, deviceHid, name, type, timestamp, flval);
  return 0;
}


typedef struct _device_telemetry {
  arrow_device_t *device;
  void *data;
  int count;
} device_telemetry_t;

static void _telemetry_init(http_request_t *request, void *arg) {
  device_telemetry_t *dt = (device_telemetry_t *)arg;
  http_request_init(request, POST, ARROW_API_TELEMETRY_ENDPOINT);
  request->is_chunked = 1;
  http_request_set_payload(request, p_heap(telemetry_serialize(dt->device, dt->data)));
}

int arrow_send_telemetry(arrow_device_t *device, void *d) {
  int ret = 0;
  device_telemetry_t dt = {device, d, 1};
  ret = __http_routine(_telemetry_init, &dt, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Telemetry send failed...");
  }
  return ret;
}

static void _telemetry_batch_init(http_request_t *request, void *arg) {
  device_telemetry_t *dt = (device_telemetry_t *)arg;
  char *uri = (char*)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/batch", ARROW_API_TELEMETRY_ENDPOINT);
  http_request_init(request, POST, uri);
  free(uri);
  request->is_chunked = 1;
  int i = 0;
  char *_main = NULL;
  for( i = 0; i < dt->count; i++ ) {
    char *tmp = telemetry_serialize(dt->device, (int*)dt->data + i);
    if ( !_main ) {
      _main = (char*)malloc(strlen(tmp) * ( dt->count + 1 ));
      strcpy(_main, "[");
    }
    strcat(_main, tmp);
    if ( i != dt->count - 1 ) strcat(_main, ",");
    else strcat(_main, "]");
    free(tmp);
  }
  DBG("main: %s", _main);
  http_request_set_payload(request, p_heap(_main));
}

int arrow_telemetry_batch_create(arrow_device_t *device, void *data, int size) {
  device_telemetry_t dt = {device, data, size};
  int ret = __http_routine(_telemetry_batch_init, &dt, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Telemetry send failed...");
  }
  return ret;
}

typedef struct _telemetry_hid_ {
  find_by_t *params;
  const char *hid;
} telemetry_hid_t;

static void _telemetry_find_by_application_hid_init(http_request_t *request, void *arg) {
  telemetry_hid_t *appl = (telemetry_hid_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/applications/%s", ARROW_API_TELEMETRY_ENDPOINT, appl->hid);
  http_request_init(request, GET, uri);
  free(uri);
  find_by_t *params = appl->params;
  ADD_FIND_BY_TO_REQ(params, request);
}

static int _telemetry_find_by_application_hid_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("telem appl: %s", P_VALUE(response->payload.buf));
  return 0;
}

int arrow_telemetry_find_by_application_hid(const char *hid, int n, ...) {
  find_by_t *params = NULL;
  COLLECT_FIND_BY(params, n);
  telemetry_hid_t app = {params, hid};
  int ret = __http_routine(_telemetry_find_by_application_hid_init, &app, _telemetry_find_by_application_hid_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Telemetry find by failed...");
  }
  return ret;
}

static void _telemetry_find_by_device_hid_init(http_request_t *request, void *arg) {
  telemetry_hid_t *appl = (telemetry_hid_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/devices/%s", ARROW_API_TELEMETRY_ENDPOINT, appl->hid);
  http_request_init(request, GET, uri);
  free(uri);
  find_by_t *params = appl->params;
  ADD_FIND_BY_TO_REQ(params, request);
}

static int _telemetry_find_by_device_hid_proc(http_response_t *response, void *arg) {
  telemetry_response_data_list_t* t = (telemetry_response_data_list_t*)arg;
  if ( response->m_httpResponseCode != 200 ) return -1;
  DBG("telem appl: %s", P_VALUE(response->payload.buf));
  JsonNode *_main = json_decode(P_VALUE(response->payload.buf));
  JsonNode *size  = json_find_member(_main, "size");
  if ( !size ) return -1;
  JsonNode *page  = json_find_member(_main, "page");
  if ( !page ) return -1;
  JsonNode *totalSize  = json_find_member(_main, "totalSize");
  if ( !totalSize ) return -1;
  JsonNode *totalPages  = json_find_member(_main, "totalPages");
  if ( !totalPages ) return -1;
  telemetry_response_data_list_init(t, json_number(size),
                                    json_number(page),
                                    json_number(totalSize),
                                    json_number(totalPages));
  JsonNode *data  = json_find_member(_main, "data");
  if ( !data ) return 0;
  int i = 0;
  for ( i = 0; i < t->size; i++ ) {
    JsonNode *info = json_find_element(data, i);
    if ( !info ) goto parse_error;

    JsonNode *deviceHid = json_find_member(info, "deviceHid");
    if ( !deviceHid ) goto parse_error;
    JsonNode *name = json_find_member(info, "name");
    if ( !name ) goto parse_error;
    JsonNode *type = json_find_member(info, "type");
    if ( !type ) goto parse_error;
    JsonNode *timestamp = json_find_member(info, "timestamp");
    if ( !timestamp ) goto parse_error;
    JsonNode *floatValue = json_find_member(info, "floatValue");
    if ( !floatValue ) goto parse_error;

    add_telemetry_data_info(t,
                            deviceHid->string_,
                            name->string_,
                            type->string_,
                            json_number(timestamp),
                            json_number(floatValue));
  }
  json_delete(_main);
  return 0;

parse_error:
  DBG("parse error");
  json_delete(_main);
  telemetry_response_data_list_free(t);
  return -1;
}

int arrow_telemetry_find_by_device_hid(const char *hid,
                                       telemetry_response_data_list_t *data,
                                       int n, ...) {
  find_by_t *params = NULL;
  COLLECT_FIND_BY(params, n);
  telemetry_hid_t app = {params, hid};
  int ret = __http_routine(_telemetry_find_by_device_hid_init, &app, _telemetry_find_by_device_hid_proc, data);
  if ( ret < 0 ) {
    DBG("Arrow Telemetry find by failed...");
  }
  return ret;
}

static void _telemetry_find_by_node_hid_init(http_request_t *request, void *arg) {
  telemetry_hid_t *appl = (telemetry_hid_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/nodes/%s", ARROW_API_TELEMETRY_ENDPOINT, appl->hid);
  http_request_init(request, GET, uri);
  free(uri);
  find_by_t *params = appl->params;
  ADD_FIND_BY_TO_REQ(params, request);
}

static int _telemetry_find_by_node_hid_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("telem appl: %s", P_VALUE(response->payload.buf));
  return 0;
}

int arrow_telemetry_find_by_node_hid(const char *hid, int n, ...) {
  find_by_t *params = NULL;
  COLLECT_FIND_BY(params, n);
  telemetry_hid_t app = {params, hid};
  int ret = __http_routine(_telemetry_find_by_node_hid_init, &app, _telemetry_find_by_node_hid_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Telemetry find by failed...");
  }
  return ret;
}
