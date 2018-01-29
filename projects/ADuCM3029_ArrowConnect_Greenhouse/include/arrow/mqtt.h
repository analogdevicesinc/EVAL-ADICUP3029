/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_MQTT_H_
#define ARROW_MQTT_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "gateway.h"
#include "device.h"

// Establishing MQTT connection depends used define:
// __IBM__ or __AZURE__ in private.h file
// if there is no define IoT connection will be used as default.
int mqtt_connect(arrow_gateway_t *gateway,
                 arrow_device_t *device,
                 arrow_gateway_config_t *config);

// Terminate MQTT connection
void mqtt_disconnect(void);

// Subscribe on MQTT events
// In this context this means that device can be controlled by
// command received from the cloud
int mqtt_subscribe(void);

// Wait some event from the cloud
// The user's command or software update command
int mqtt_yield(int timeout_ms);

// Send the telemetry data to the cloud
// there is extremely needed the telemetry_serialize function implementation to serealize 'data' correctly
int mqtt_publish(arrow_device_t *device, void *data);

#if defined(__cplusplus)
}
#endif

#endif /* ARROW_MQTT_H_ */
