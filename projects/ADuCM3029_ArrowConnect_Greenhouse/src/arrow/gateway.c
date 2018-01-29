/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/gateway.h"
#include <unint.h>
#include <arrow/net.h>
#if defined(__USE_STD__)
#include <stdlib.h>
#endif
#include <debug.h>

#if defined(__IBM__)
P_ADD(arrow_gateway_config, organizationId);
P_ADD(arrow_gateway_config, authMethod);
P_ADD(arrow_gateway_config, authToken);
P_ADD(arrow_gateway_config, gatewayId);
P_ADD(arrow_gateway_config, gatewayType);
#elif defined(__AZURE__)
P_ADD(arrow_gateway_config, host);
P_ADD(arrow_gateway_config, accessKey);
#endif

void arrow_gateway_init(arrow_gateway_t *gate) {
  memset(gate, 0, sizeof(arrow_gateway_t));
}

char *arrow_gateway_serialize(arrow_gateway_t *gate) {
  JsonNode *_main = json_mkobject();
  if ( !IS_EMPTY( gate->name ) )
    json_append_member(_main, "name", json_mkstring( P_VALUE(gate->name) ));
  if ( !IS_EMPTY( gate->uid ) )
    json_append_member(_main, "uid", json_mkstring( P_VALUE(gate->uid) ));
  if ( !IS_EMPTY(gate->os) )
    json_append_member(_main, "osName", json_mkstring( P_VALUE(gate->os) ));
  if ( !IS_EMPTY(gate->type) )
    json_append_member(_main, "type", json_mkstring( P_VALUE(gate->type) ));
  if ( !IS_EMPTY(gate->software_name) )
    json_append_member(_main, "softwareName", json_mkstring( P_VALUE(gate->software_name) ));
  if ( !IS_EMPTY(gate->software_version) )
    json_append_member(_main, "softwareVersion", json_mkstring( P_VALUE(gate->software_version) ));
  if ( !IS_EMPTY(gate->sdkVersion) )
    json_append_member(_main, "sdkVersion", json_mkstring( P_VALUE(gate->sdkVersion) ));
  char *str = json_encode(_main);
  json_minify(str);
  json_delete(_main);
  return str;
}

int arrow_gateway_parse(arrow_gateway_t *gate, const char *str) {
  if (!str) return -1;
  DBG("parse this: %s", str);
  JsonNode *_main = json_decode(str);
  if ( !_main ) return -1;
  JsonNode *hid = json_find_member(_main, "hid");
  if ( !hid ) return -1;
  if ( hid->tag != JSON_STRING ) return -1;
  property_copy( &gate->hid, p_stack(hid->string_));
  json_delete(_main);
  return 0;
}

void arrow_gateway_free(arrow_gateway_t *gate) {
  property_free( &gate->name );
  property_free( &gate->uid );
  property_free( &gate->os );
  property_free( &gate->type );
  property_free( &gate->hid );
  property_free( &gate->software_name );
  property_free( &gate->software_version );
  property_free( &gate->sdkVersion );
}


void arrow_gateway_config_init(arrow_gateway_config_t *config) {
  memset(config, 0x00, sizeof(arrow_gateway_config_t));
  config->type = unknown;
}


void arrow_gateway_config_free(arrow_gateway_config_t *config) {
#if defined(__IBM__)
	property_free ( &config->authMethod );
	property_free ( &config->authToken );
	property_free ( &config->gatewayId );
	property_free ( &config->gatewayType );
	property_free ( &config->organizationId );
#else
  SSP_PARAMETER_NOT_USED(config);
#endif
}

int arrow_prepare_gateway(arrow_gateway_t *gateway) {
  arrow_gateway_init(gateway);
  property_copy( &gateway->name, p_const(GATEWAY_NAME));
  property_copy( &gateway->os, p_const(GATEWAY_OS));
  property_copy( &gateway->software_name, p_const(GATEWAY_SOFTWARE_NAME));
  property_copy( &gateway->software_version, p_const(GATEWAY_SOFTWARE_VERSION));
  property_copy( &gateway->type, p_const(GATEWAY_TYPE));
  property_copy( &gateway->sdkVersion, p_const(xstr(SDK_VERSION)));
  char *uid = (char*)malloc(sizeof(GATEWAY_UID_PREFIX) + 14); // 6*2 for mac + 2
  strcpy(uid, GATEWAY_UID_PREFIX);
  strcat(uid, "-");
  uint32_t uidlen = sizeof(GATEWAY_UID_PREFIX);
  char mac[7];
  get_mac_address(mac);
  int i = 0;
  for(i=0; i<6; i++) sprintf(uid+uidlen+2*i, "%02x", (uint8_t)(mac[i]));
  uidlen += 12;
  uid[uidlen] = '\0';
  DBG("uid: [%s]", uid);
  property_copy( &gateway->uid, p_heap(uid));
  return 0;
}
