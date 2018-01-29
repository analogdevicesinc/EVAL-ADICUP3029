#include "arrow/gateway_api.h"
#include <http/routine.h>
#include <arrow/sign.h>
#include <debug.h>

#define URI_LEN sizeof(ARROW_API_GATEWAY_ENDPOINT) + 50

static void _gateway_config_init(http_request_t *request, void *arg) {
	arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
	CREATE_CHUNK(uri, URI_LEN);
	strcpy(uri, ARROW_API_GATEWAY_ENDPOINT);
	strcat(uri, "/");
	strcat(uri, P_VALUE(gateway->hid) );
	strcat(uri, "/config");
	http_request_init(request, GET, uri);
	FREE_CHUNK(uri);
}

static int _gateway_config_proc(http_response_t *response, void *arg) {
	arrow_gateway_config_t *config = (arrow_gateway_config_t *)arg;
	DBG("response %d", response->m_httpResponseCode);
	if ( response->m_httpResponseCode != 200 ) {
		return -1;
	}
  DBG("pay: {%s}\r\n", P_VALUE(response->payload.buf));

  JsonNode *_main = json_decode(P_VALUE(response->payload.buf));
	JsonNode *tmp;
	JsonNode *_main_key = json_find_member(_main, "key");
	if ( _main_key ) {
		tmp = json_find_member(_main_key, "apiKey");
		if (tmp) {
			DBG("(%d) api key: %s", strlen(tmp->string_), tmp->string_);
			set_api_key(tmp->string_);
		}
		tmp = json_find_member(_main_key, "secretKey");
		if (tmp) {
			DBG("(%d) secret key: %s", strlen(tmp->string_), tmp->string_);
			set_secret_key(tmp->string_);
		}
	} else return -1;
	arrow_gateway_config_init(config);
#if defined(__IBM__)
	JsonNode *_main_ibm = json_find_member(_main, "ibm");
	if ( _main_ibm ) {
		config->type = 1;
		tmp = json_find_member(_main_ibm, "organizationId");
		if ( tmp ) arrow_gateway_config_add_organizationId(config, tmp->string_);
		tmp = json_find_member(_main_ibm, "gatewayType");
		if ( tmp ) arrow_gateway_config_add_gatewayType(config, tmp->string_);
		tmp = json_find_member(_main_ibm, "gatewayId");
		if ( tmp ) arrow_gateway_config_add_gatewayId(config, tmp->string_);
		tmp = json_find_member(_main_ibm, "authToken");
		if ( tmp ) arrow_gateway_config_add_authToken(config, tmp->string_);
		tmp = json_find_member(_main_ibm, "authMethod");
		if ( tmp ) arrow_gateway_config_add_authMethod(config, tmp->string_);
	}
#elif defined(__AZURE__)
	JsonNode *_main_azure = json_find_member(_main, "azure");
	if ( _main_azure ) {
		config->type = 3;
		tmp = json_find_member(_main_azure, "host");
		if ( tmp ) arrow_gateway_config_add_host(config, tmp->string_);
		tmp = json_find_member(_main_azure, "accessKey");
		if ( tmp ) arrow_gateway_config_add_accessKey(config, tmp->string_);
	}
#endif
	json_delete(_main);
	return 0;
}

int arrow_gateway_config(arrow_gateway_t *gateway, arrow_gateway_config_t *config) {
	int ret = __http_routine(_gateway_config_init, gateway, _gateway_config_proc, config);
	if ( ret < 0 ) {
		DBG("Arrow Gateway config failed...");
	}
	return ret;
}


static void _gateway_register_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  http_request_init(request, POST, ARROW_API_GATEWAY_ENDPOINT);
  char *payload = arrow_gateway_serialize(gateway);
  DBG("payload %s", payload);
  http_request_set_payload(request, p_heap(payload));
}

static int _gateway_register_proc(http_response_t *response, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  DBG("response gate reg %d", response->m_httpResponseCode);
  // convert to null-terminate string
  P_VALUE(response->payload.buf)[response->payload.size] = 0x0;
  if ( arrow_gateway_parse(gateway, P_VALUE(response->payload.buf)) < 0 ) {
      DBG("parse error");
      return -1;
  } else {
      DBG("gateway hid: %s", P_VALUE(gateway->hid) );
  }
  return 0;
}

int arrow_register_gateway(arrow_gateway_t *gateway) {
  int ret = __http_routine(_gateway_register_init, gateway, _gateway_register_proc, gateway);
  if ( ret < 0 ) {
    DBG("Gateway register failed...");
  }
  return ret;
}

static void _gateway_heartbeat_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  strcpy(uri, ARROW_API_GATEWAY_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, P_VALUE(gateway->hid) );
  strcat(uri, "/heartbeat");
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
}

int arrow_gateway_heartbeat(arrow_gateway_t *gateway) {
  int ret = __http_routine(_gateway_heartbeat_init, gateway, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Gateway heartbeat failed...");
  }
  return ret;
}

static void _gateway_checkin_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  strcpy(uri, ARROW_API_GATEWAY_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, P_VALUE(gateway->hid));
  strcat(uri, "/checkin");
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
}

int arrow_gateway_checkin(arrow_gateway_t *gateway) {
  int ret = __http_routine(_gateway_checkin_init, gateway, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Gateway checkin failed...");
  }
  return ret;
}

static void _gateway_find_init(http_request_t *request, void *arg) {
  char *hid = (char *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s", ARROW_API_GATEWAY_ENDPOINT, hid);
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
}

static int _gateway_find_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("find [%s]", P_VALUE(response->payload.buf));
  } else return -1;
  return 0;
}

int arrow_gateway_find(const char *hid) {
  int ret = __http_routine(_gateway_find_init, (void*)hid, _gateway_find_proc, NULL);
  if ( ret < 0 ) {
    DBG("Gateway register failed...");
  }
  return ret;
}

static void _gateway_find_by_init(http_request_t *request, void *arg) {
  find_by_t *params = (find_by_t *)arg;
  http_request_init(request, GET, ARROW_API_GATEWAY_ENDPOINT);
  ADD_FIND_BY_TO_REQ(params, request);
}

static int _gateway_find_by_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("gw find [%s]", P_VALUE(response->payload.buf));
  } else return -1;
  return 0;
}


int arrow_gateway_find_by(int n, ...) {
  find_by_t *params = NULL;
  COLLECT_FIND_BY(params, n);
  int ret = __http_routine(_gateway_find_by_init, params, _gateway_find_by_proc, NULL);
  if ( ret < 0 ) {
    DBG("Gateway find by failed...");
  }
  return ret;
}

typedef struct _gate_param_ {
  arrow_gateway_t *gate;
  find_by_t *params;
} gate_param_t;

static void _gateway_list_logs_init(http_request_t *request, void *arg) {
  gate_param_t *dp = (gate_param_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN,"%s/%s/logs", ARROW_API_GATEWAY_ENDPOINT, P_VALUE(dp->gate->hid) );
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
  find_by_t *params = dp->params;
  ADD_FIND_BY_TO_REQ(params, request);
}

static int _gateway_list_logs_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("gateway list logs: %s", P_VALUE(response->payload.buf));
  return 0;
}

int arrow_gateway_logs_list(arrow_gateway_t *gateway, int n, ...) {
  find_by_t *params = NULL;
  COLLECT_FIND_BY(params, n);
  gate_param_t dp = { gateway, params };
  int ret = __http_routine(_gateway_list_logs_init, &dp, _gateway_list_logs_proc, NULL);
  if ( ret < 0 ) {
    DBG("Gateway logs failed...");
  }
  return ret;
}

static void _gateway_devices_list_init(http_request_t *request, void *arg) {
  char *hid = (char *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s/devices", ARROW_API_GATEWAY_ENDPOINT, hid);
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
}

static int _gateway_devices_list_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("devices [%s]", P_VALUE(response->payload.buf));
  } else return -1;
  return 0;
}

int arrow_gateway_devices_list(const char *hid) {
  int ret = __http_routine(_gateway_devices_list_init, (void*)hid, _gateway_devices_list_proc, NULL);
  if ( ret < 0 ) {
    DBG("Gateway devices list failed...");
  }
  return ret;
}

typedef struct _gate_dev_cmd_ {
  const char *g_hid;
  const char *d_hid;
  const char *cmd;
  const char *payload;
} gate_dev_cmd_t;

static void _gateway_device_cmd_init(http_request_t *request, void *arg) {
  gate_dev_cmd_t *gdc = (gate_dev_cmd_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s/devices/%s/actions/command", ARROW_API_GATEWAY_ENDPOINT,
           gdc->g_hid, gdc->d_hid);
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "command", json_mkstring(gdc->cmd));
  json_append_member(_main, "deviceHid", json_mkstring(gdc->d_hid));
  json_append_member(_main, "payload", json_mkstring(gdc->payload));
  http_request_set_payload(request, p_heap(json_encode(_main)));
  json_delete(_main);
}

static int _gateway_device_cmd_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("devices [%s]", P_VALUE(response->payload.buf));
  } else return -1;
  return 0;
}

int arrow_gateway_device_send_command(const char *gHid, const char *dHid, const char *cmd, const char *payload) {
  gate_dev_cmd_t gdc = {gHid, dHid, cmd, payload};
  int ret = __http_routine(_gateway_device_cmd_init, &gdc, _gateway_device_cmd_proc, NULL);
  if ( ret < 0 ) {
    DBG("Gateway devices list failed...");
  }
  return ret;
}

typedef struct _gateway_error {
  arrow_gateway_t *gateway;
  const char *error;
} gateway_error_t;

static void _gateway_errors_init(http_request_t *request, void *arg) {
  gateway_error_t *de = (gateway_error_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s/errors", ARROW_API_GATEWAY_ENDPOINT, P_VALUE(de->gateway->hid) );
  http_request_init(request, POST, uri);
  FREE_CHUNK(uri);
  JsonNode *error = json_mkobject();
  json_append_member(error, "error", json_mkstring(de->error));
  http_request_set_payload(request, p_heap(json_encode(error)));
  json_delete(error);
}

int arrow_gateway_error(arrow_gateway_t *gateway, const char *error) {
  gateway_error_t de = { gateway, error };
  int ret = __http_routine(_gateway_errors_init, &de, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Gateway error failed...");
  }
  return ret;
}

static void _gateway_update_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gate = (arrow_gateway_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s", ARROW_API_GATEWAY_ENDPOINT, P_VALUE(gate->hid) );
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
  http_request_set_payload(request, p_heap(arrow_gateway_serialize(gate)));
}

static int _gateway_update_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode != 200 ) return -1;
  return 0;
}

int arrow_gateway_update(arrow_gateway_t *gateway) {
  int ret = __http_routine(_gateway_update_init, gateway, _gateway_update_proc, NULL);
  if ( ret < 0 ) {
    DBG("Gateway update failed...");
  }
  return ret;
}
