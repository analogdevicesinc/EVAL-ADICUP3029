/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_GATEWAY_H_
#define ARROW_GATEWAY_H_

#include <arrow/mem.h>
#include <config.h>
#include <json/json.h>

typedef struct __attribute_packed__ {
  property_t name;
  property_t uid;
  property_t os;
  property_t type;
  property_t hid;
  property_t software_name;
  property_t software_version;
  property_t sdkVersion;
} arrow_gateway_t;

enum account_type {
  unknown = 0,
  IoT,
  IBM,
  AWS,
  Azure
};

typedef struct __attribute_packed__ {
  int type;    // enum account_type
#if defined(__IBM__)
  property_t organizationId;
  property_t authMethod;
  property_t authToken;
  property_t gatewayId;
  property_t gatewayType;
#elif defined(__AZURE__)
  property_t host;
  property_t accessKey;
#else
#endif
} arrow_gateway_config_t;

void arrow_gateway_init(arrow_gateway_t *gate);
char *arrow_gateway_serialize(arrow_gateway_t *gate);
int arrow_gateway_parse(arrow_gateway_t *gate, const char *str);
void arrow_gateway_free(arrow_gateway_t *gate);

void arrow_gateway_config_init(arrow_gateway_config_t *config);
void arrow_gateway_config_free(arrow_gateway_config_t *config);
#if defined(__IBM__)
void arrow_gateway_config_add_authMethod(arrow_gateway_config_t *conf, const char*authMeth);
void arrow_gateway_config_add_authToken(arrow_gateway_config_t *conf, const char*authToken);
void arrow_gateway_config_add_gatewayId(arrow_gateway_config_t *conf, const char*gID);
void arrow_gateway_config_add_gatewayType(arrow_gateway_config_t *conf, const char*gT);
void arrow_gateway_config_add_organizationId(arrow_gateway_config_t *conf, const char*oID);
#elif defined(__AZURE__)
void arrow_gateway_config_add_host(arrow_gateway_config_t *conf, const char *host);
void arrow_gateway_config_add_accessKey(arrow_gateway_config_t *conf, const char *accessKey);
#endif

int arrow_prepare_gateway(arrow_gateway_t *gateway);
#endif /* ARROW_GATEWAY_H_ */
