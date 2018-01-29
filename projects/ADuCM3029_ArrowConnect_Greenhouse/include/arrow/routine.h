/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_ROUTINE_H_
#define ARROW_ROUTINE_H_

#if defined(__cplusplus)
extern "C" {
#endif
#include <arrow/gateway.h>
#include <arrow/device.h>

typedef int (*get_data_cb)(void *);

// There is only one gateway and device for this space
// to get these static objects use the functions:
arrow_device_t *current_device(void);
arrow_gateway_t *current_gateway(void);

// Initialize the gateway object and device object as well
// This function implemented the algorithm to get complete information about a gateway and device.
// The WDT function is used.
int arrow_initialize_routine(void);

// Routine function for terminating current connections with the cloud
// and terminate all gateway/device information.
void arrow_close(void);

// If there is no the stored gateway information
// form the register gateway request and save a taken information
// In other case just form the gateway checkin request
int arrow_connect_gateway(arrow_gateway_t *gateway);

// If there is no any device information form the device register request
// and save the taken information
int arrow_connect_device(arrow_gateway_t *gateway, arrow_device_t *device);

// Routine function for telemetry sending to the cloud
// there is extremely needed the telemetry_serialize function implementation to serealize 'data' correctly
int arrow_send_telemetry_routine(void *data);

// Funtion set the new state for this device
int arrow_update_state(const char *name, const char *value);

// Routine for MQTT connection establishing
// Automatically prepare needed information and send it to the cloud MQTT
int arrow_mqtt_connect_routine(void);

// This routine send the telemetry data every TELEMETRY_DELAY msec
// using the data_cb function for forming current telemetry values
void arrow_mqtt_send_telemetry_routine(get_data_cb data_cb, void *data);

#if defined(__cplusplus)
}
#endif

#endif /* ARROW_ROUTINE_H_ */
