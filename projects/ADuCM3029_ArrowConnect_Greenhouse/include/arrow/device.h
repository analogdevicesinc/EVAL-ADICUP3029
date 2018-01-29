/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_DEVICE_H_
#define ARROW_DEVICE_H_

#include "json/json.h"
#include <arrow/gateway.h>
#include <arrow/mem.h>

typedef struct __attribute_packed__ {
  property_t name;
  property_t type;
  property_t uid;
  property_t gateway_hid;
#if defined(__XCC__)
  struct json_t *info;
  struct json_t *prop;
#else
    JsonNode *info;
    JsonNode *prop;
#endif
    property_t hid;
#if defined(__IBM__)
    property_t eid;
#endif
} arrow_device_t;


void arrow_device_init(arrow_device_t *dev);
void arrow_device_free(arrow_device_t *dev);

char *arrow_device_get_uid(arrow_device_t *dev);
char *arrow_device_get_gateway_hid(arrow_device_t *dev);

void arrow_device_add_gateway_hid(arrow_device_t *dev, const char *name);
void arrow_device_add_uid(arrow_device_t *dev, const char *name);

void arrow_device_add_info(arrow_device_t *dev, const char *key, const char *value);
void arrow_device_add_property(arrow_device_t *dev, const char *key, const char *value);

char *arrow_device_serialize(arrow_device_t *dev);
int arrow_device_parse(arrow_device_t *dev, const char *str);

int arrow_prepare_device(arrow_gateway_t *gateway, arrow_device_t *device);

#endif /* ARROW_DEVICE_H_ */
