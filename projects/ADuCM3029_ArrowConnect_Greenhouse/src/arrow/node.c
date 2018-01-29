#include "arrow/node.h"
#include <http/routine.h>
#include <json/json.h>
#include <debug.h>

#define URI_LEN sizeof(ARROW_API_NODE_ENDPOINT) + 50

static void _node_list_init(http_request_t *request, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  http_request_init(request, GET, ARROW_API_NODE_ENDPOINT);
}

static int _node_list_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode != 200 ) return -1;
  DBG("gateway hid: %s", P_VALUE(response->payload.buf));
  return 0;
}

int arrow_node_list(void) {
  int ret = __http_routine(_node_list_init, NULL, _node_list_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Node list failed...");
  }
  return ret;
}

static char *arrow_node_serialize(arrow_node_t *node) {
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "description", json_mkstring(node->description));
  json_append_member(_main, "enabled", json_mkbool(node->enabled));
  json_append_member(_main, "name", json_mkstring(node->name));
  json_append_member(_main, "nodeTypeHid", json_mkstring(node->nodeTypeHid));
  json_append_member(_main, "parentNodeHid", json_mkstring(node->parentNodeHid));
  char *payload = json_encode(_main);
  json_delete(_main);
  return payload;
}

static void _node_create_init(http_request_t *request, void *arg) {
  arrow_node_t *node = (arrow_node_t *) arg;
  http_request_init(request, POST, ARROW_API_NODE_ENDPOINT);
  http_request_set_payload(request, p_heap(arrow_node_serialize(node)));
}

static int _node_create_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode != 200 ) return -1;
  DBG("ans: %s", P_VALUE(response->payload.buf));
  return 0;
}

int arrow_node_create(arrow_node_t *node) {
  int ret = __http_routine(_node_create_init, node, _node_create_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Node create failed...");
  }
  return ret;
}

static void _node_update_init(http_request_t *request, void *arg) {
  arrow_node_t *node = (arrow_node_t *) arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s", ARROW_API_NODE_ENDPOINT, node->hid);
  http_request_init(request, PUT, uri);
  free(uri);
  http_request_set_payload(request, p_heap(arrow_node_serialize(node)));
}

static int _node_update_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode != 200 ) return -1;
  DBG("ans: %s", P_VALUE(response->payload.buf));
  return 0;
}


int arrow_node_update(arrow_node_t *node) {
  int ret = __http_routine(_node_update_init, node, _node_update_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Node updaet failed...");
  }
  return ret;
}
