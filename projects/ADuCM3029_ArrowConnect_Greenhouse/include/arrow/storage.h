/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_STORAGE_H_
#define ARROW_STORAGE_H_

#include <arrow/device.h>
#include <arrow/gateway.h>

#if defined(__cplusplus)
extern "C" {
#endif


// default structure for the device information
typedef struct {
  int magic;
  char ssid[64];
  char pass[64];
  int sec;
  int type;
  char padding[116];
  char gateway_hid[64];
  char device_hid[64];
  char device_eid[64];
  char unused[64];
} flash_mem_t;



int init_flash(void);

int read_from_flash(uint32_t *pArray, uint32_t ulStartAddress, uint32_t u32Size);

// restore the gateway information (hid)
int restore_gateway_info(arrow_gateway_t *gateway);

// save the gateway information (hid)
void save_gateway_info(const arrow_gateway_t *gateway);

// restore the device information (hid for all and eid for IBM account)
int restore_device_info(arrow_device_t *device);

// save the device information (hid for all and eid for IBM account)
void save_device_info(arrow_device_t *device);

// restore the wifi settings (SSID, password, secure mode)
int restore_wifi_setting(char *ssid, char *pass, int *sec);

// save the wifi settings (SSID, password, secure mode)
void save_wifi_setting(const char *ssid, const char *pass, int sec);

#if defined(__cplusplus)
}
#endif

#endif


