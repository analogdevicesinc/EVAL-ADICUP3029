/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/device_command.h"
#include <http/routine.h>
#include <json/json.h>
#include <arrow/mem.h>
#include <arrow/events.h>
#include <debug.h>

static cmd_handler *__handlers = NULL;

static void __create_cmd_handler(cmd_handler *hd, const char *name, fp callback) {
  hd->name = malloc(strlen(name)+1);
  strcpy(hd->name, name);
  hd->callback = callback;
  hd->next = NULL;
}

// handlers
int has_cmd_handler(void) {
	if ( __handlers ) return 0;
	return -1;
}

int add_cmd_handler(const char *name, fp callback) {
  cmd_handler *h = malloc(sizeof(cmd_handler));
  __create_cmd_handler(h, name, callback);
  if ( !__handlers ) {
    __handlers = h;
  } else {
    cmd_handler *last = __handlers;
    while( last->next ) last = last->next;
    last->next = h;
  }
  return 0;
}

void free_cmd_handler(void) {
  cmd_handler *curr = __handlers;
  while( curr ) {
    cmd_handler *rm = curr;
    curr = curr->next;
    free(rm->name);
    free(rm);
  }
}


// events
static char *form_evetns_url(const char *hid, cmd_type ev) {
    char *uri = (char *)malloc(sizeof(ARROW_API_EVENTS_ENDPOINT) + strlen(hid) + 15);
    strcpy(uri, ARROW_API_EVENTS_ENDPOINT);
    strcat(uri, "/");
    strcat(uri, hid);
    switch(ev) {
        case failed:    strcat(uri, "/failed"); break;
        case received:  strcat(uri, "/received"); break;
        case succeeded: strcat(uri, "/succeeded"); break;
    }
    return uri;
}

typedef struct _event_data {
	char *hid;
	cmd_type ev;
  char *payload; // FIXME property
} event_data_t;

static void _event_ans_init(http_request_t *request, void *arg) {
    event_data_t *data = (event_data_t *)arg;
	char *uri = form_evetns_url(data->hid, data->ev);
  http_request_init(request, PUT, uri);
	free(uri);
	if ( data->payload ) {
    http_request_set_payload(request, p_stack(data->payload));
	}
}

int arrow_send_event_ans(const char *hid, cmd_type ev, const char *payload) {
  event_data_t edata = {(char*)hid, ev, (char *)payload};
	int ret = __http_routine(_event_ans_init, &edata, NULL, NULL);
	if ( ret < 0 ) {
		DBG("Arrow Event answer failed...");
	}
	return ret;
}

static int fill_string_from_json(JsonNode *_node, const char *name, char **str) __attribute__((used));
static int fill_string_from_json(JsonNode *_node, const char *name, char **str) {
  JsonNode *tmp = json_find_member(_node, name);
  if ( ! tmp || tmp->tag != JSON_STRING ) return -1;
  *str = malloc(strlen(tmp->string_)+1);
  strcpy(*str, tmp->string_);
  return 0;
}

static fp find_cmd_handler(const char *cmd) {
  if ( __handlers ) {
    cmd_handler *h = __handlers;
    while(h) {
      if ( strcmp(h->name, cmd) == 0 ) return h->callback;
      h = h->next;
    }
  } else {
    DBG("No cmd handlers");
  }
  return NULL;
}

int __attribute__((weak)) command_handler(const char *name,
                    JsonNode *payload,
                    JsonNode **error) {
  int ret = -1;
  fp callback = find_cmd_handler(name);
  if ( callback ) {
    ret = callback(payload->string_);
    if ( ret < 0 ) {
      *error = json_mkobject();
      json_append_member(*error, "error", json_mkstring("Something went wrong"));
    }
  } else {
    *error = json_mkobject();
    json_append_member(*error, "error", json_mkstring("there is no a command handler"));
  }
  return ret;
}

int ev_DeviceCommand(void *_ev, JsonNode *_parameters) {
  int ret = -1;
  JsonNode *_error = NULL;
  mqtt_event_t *ev = (mqtt_event_t *)_ev;
  arrow_send_event_ans(ev->gateway_hid, received, NULL);
  DBG("start device command processing");

  JsonNode *tmp = json_find_member(_parameters, "deviceHid");
  if ( !tmp || tmp->tag != JSON_STRING ) return -1;

  JsonNode *cmd = json_find_member(_parameters, "command");
  if ( !cmd || cmd->tag != JSON_STRING ) return -1;
  DBG("ev cmd: %s", cmd->string_);

  JsonNode *pay = json_find_member(_parameters, "payload");
  if ( !pay || pay->tag != JSON_STRING ) return -1;
  DBG("ev msg: %s", pay->string_);

  ret = command_handler(cmd->string_, pay, &_error);
  if ( ret < 0 ) {
      DBG("command_handler fail %d", ret);
  }

  if ( _error ) {
    arrow_send_event_ans(ev->gateway_hid, failed, json_encode(_error));
    json_delete(_error);
  } else {
    arrow_send_event_ans(ev->gateway_hid, succeeded, NULL);
  }

  return 0;
}
