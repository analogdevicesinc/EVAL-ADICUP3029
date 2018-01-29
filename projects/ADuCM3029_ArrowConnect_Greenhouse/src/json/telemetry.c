/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "json/telemetry.h"
#include <config.h>
#include <json/json.h>
#include <json/data.h>
#include <debug.h>
#include <stdio.h>

static char tmpdata[50];

char *telemetry_serialize(arrow_device_t *device, void *d) {
  sensors_data_t *data = (sensors_data_t *)d;
  JsonNode *_node = json_mkobject();
  int ret;
  json_append_member(_node, TELEMETRY_DEVICE_HID, json_mkstring(P_VALUE(device->hid)));
  json_append_member(_node, TELEMETRY_TEMPERATURE, json_mknumber(data->temperature));
  json_append_member(_node, TELEMETRY_HUMIDITY, json_mknumber(data->humidity));
  json_append_member(_node, TELEMETRY_PH, json_mknumber(data->ph));
  json_append_member(_node, TELEMETRY_RED_LIGHT, json_mknumber(data->red_intensity));
  json_append_member(_node, TELEMETRY_GREEN_LIGHT, json_mknumber(data->green_intensity));
  json_append_member(_node, TELEMETRY_BLUE_LIGHT, json_mknumber(data->blue_intensity));
  json_append_member(_node, TELEMETRY_RED_CONCENTRATION, json_mknumber(data->red_concentration));
  json_append_member(_node, TELEMETRY_GREEN_CONCENTRATION, json_mknumber(data->green_concentration));
  json_append_member(_node, TELEMETRY_BLUE_CONCENTRATION, json_mknumber(data->blue_concentration));


  char *tmp = json_encode(_node);
  json_delete(_node);
  return tmp;
}
