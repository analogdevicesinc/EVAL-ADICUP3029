/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/events.h"
#include <arrow/device_command.h>
#include <arrow/software_release.h>
#include <arrow/state.h>

#if !defined(NO_SOFTWARE_UPDATE)
# include <arrow/software_update.h>
#endif

#if defined(__USE_STD__)
#include <ctype.h>
#endif
#include <debug.h>
#include <http/client.h>
#include <arrow/request.h>
#include <json/json.h>
#include <arrow/mem.h>
#include <arrow/gateway_payload_sign.h>

static void free_mqtt_event(mqtt_event_t *mq) {
  if ( mq->gateway_hid ) free(mq->gateway_hid);
  if ( mq->device_hid ) free(mq->device_hid);
  if ( mq->cmd ) free(mq->cmd);
  if ( mq->payload ) free(mq->payload);
  if ( mq->name ) free(mq->name);
}

static int fill_string_from_json(JsonNode *_node, const char *name, char **str) {
  JsonNode *tmp = json_find_member(_node, name);
  if ( ! tmp || tmp->tag != JSON_STRING ) return -1;
  *str = malloc(strlen(tmp->string_)+1);
  strcpy(*str, tmp->string_);
  return 0;
}

typedef int (*submodule)(void *, JsonNode *);
typedef struct {
  char *name;
  submodule proc;
} sub_t;

sub_t sub_list[] = {
  { "ServerToGateway_DeviceCommand", ev_DeviceCommand },
  { "ServerToGateway_DeviceStateRequest", ev_DeviceStateRequest },
#if !defined(NO_SOFTWARE_UPDATE)
  { "ServerToGateway_GatewaySoftwareUpdate", ev_GatewaySoftwareUpdate },
#endif
  { "ServerToGateway_DeviceSoftwareRelease", ev_DeviceSoftwareRelease },
  { "ServerToGateway_GatewaySoftwareRelease", ev_DeviceSoftwareRelease }
};

// checker

typedef int(*sign_checker)(const char *, mqtt_event_t *, const char *);
struct check_signature_t {
  const char *version;
  sign_checker check;
};

static int check_sign_1(const char *sign, mqtt_event_t *ev, const char *can) {
  char signature[65];
  int err = gateway_payload_sign(signature,
                                 ev->gateway_hid,
                                 ev->name,
                                 ev->encrypted,
                                 can,
                                 "1");
  if ( err ) {
    return -1;
  }
  DBG("cmp { %s, %s }", sign, signature);
  return ( strcmp(sign, signature) == 0 ? 1 : 0 );
}

static struct check_signature_t checker_collection[] = {
  {"1", check_sign_1},
};

static int check_signature(const char *vers, const char *sing, mqtt_event_t *ev, const char *canParamStr) {
  unsigned int i = 0;
  for ( i = 0; i< sizeof(checker_collection) / sizeof(struct check_signature_t); i++ ) {
    if ( strcmp(vers, checker_collection[i].version ) == 0 ) {
      DBG("check version %s", checker_collection[i].version);
      return checker_collection[i].check(sing, ev, canParamStr);
    }
  }
  return -1;
}

static int cmpstringp(const void *p1, const void *p2) {
  return strcmp(* (char * const *) p1, * (char * const *) p2);
}

#if defined(__XCC__)
typedef int (*__compar_fn_t) (const void *, const void *);
extern void qsort(void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar);
#endif

static char *form_canonical_prm(JsonNode *param) {
  JsonNode *child;
  char *canParam = NULL;
  char *can_list[MAX_PARAM_LINE] = {0};
  int count = 0;
  json_foreach(child, param) {
    DBG("get child {%s}", json_key(child));
    can_list[count] = malloc(MAX_PARAM_LINE_SIZE);
    unsigned int i;
    for ( i=0; i<strlen(json_key(child)); i++ ) *(can_list[count]+i) = tolower(json_key(child)[i]);
    *(can_list[count]+i) = '=';
    switch(child->tag) {
      case JSON_STRING: strcpy(can_list[count]+i+1, child->string_); break;
#if defined(__XCC__)
      case json_True: strcpy(can_list[count]+i+1, "true"); break;
      case json_False: strcpy(can_list[count]+i+1, "false"); break;
      default:
        snprintf(can_list[count]+i+1, 50, "%d", child->valueint);
#else
      case JSON_BOOL: strcpy(can_list[count]+i+1, (child->bool_?"true":"false")); break;
      default:
        snprintf(can_list[count]+i+1, 50, "%f", child->number_);
#endif
    }
    count++;
  }

  canParam = malloc(count * MAX_PARAM_LINE_SIZE);
  *canParam = 0;
  qsort(can_list, count, sizeof(char *), cmpstringp);
  int i = 0;
  for (i=0; i<count; i++) {
    strcat(canParam, can_list[i]);
    if ( i < count-1 ) strcat(canParam, "\n");
    free(can_list[i]);
  }

  return canParam;
}


int process_event(const char *str) {
  DBG("ev: %s", str);
  mqtt_event_t mqtt_e;
  int ret = -1;
  memset(&mqtt_e, 0x0, sizeof(mqtt_event_t));
  JsonNode *_main = json_decode(str);
  if ( !_main ) {
      DBG("event payload decode failed %d", strlen(str));
      return -1;
  }

  if ( fill_string_from_json(_main, "hid", &mqtt_e.gateway_hid) < 0 ) {
    DBG("cannot find HID");
    goto error;
  }
  DBG("ev ghid: %s", mqtt_e.gateway_hid);

  if ( fill_string_from_json(_main, "name", &mqtt_e.name) < 0 ) {
    DBG("cannot find name");
    goto error;
  }
  DBG("ev name: %s", mqtt_e.name);

  JsonNode *_encrypted = json_find_member(_main, "encrypted");
  if ( !_encrypted ) goto error;
#if defined(__XCC__)
  mqtt_e.encrypted = _encrypted->type == json_True? 1 : 0;
#else
  mqtt_e.encrypted = _encrypted->bool_;
#endif

  JsonNode *_parameters = json_find_member(_main, "parameters");
  if ( !_parameters ) goto error;

  JsonNode *sign_version = json_find_member(_main, "signatureVersion");
  if ( sign_version ) {
    DBG("signature vertsion: %s", sign_version->string_);
    JsonNode *sign = json_find_member(_main, "signature");
    if ( !sign ) goto error;
    char *can = form_canonical_prm(_parameters);
    if ( !check_signature(sign_version->string_, sign->string_, &mqtt_e, can) ) {
      DBG("Alarm! signature is failed...");
      free(can);
      goto error;
    }
    free(can);
  }

  submodule current_processor = NULL;
  int i = 0;
  for (i=0; i < (int)(sizeof(sub_list)/sizeof(sub_t)); i++) {
    if ( strcmp(sub_list[i].name, mqtt_e.name) == 0 ) {
      current_processor = sub_list[i].proc;
    }
  }

  if ( current_processor ) {
    ret = current_processor(&mqtt_e, _parameters);
  } else {
    DBG("No event processor for %s", mqtt_e.name);
    goto error;
  }

error:
  free_mqtt_event(&mqtt_e);
  if ( _main ) json_delete(_main);
  return ret;
}
