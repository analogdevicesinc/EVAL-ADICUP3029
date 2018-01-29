/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define xstr(s) str(s)
#define str(s) #s

//#define __IBM__
//#define __AZURE__
#define DEV_ENV
#define DEBUG

#define SDK_VERSION 1.3.1
//#define DEV_ENV

#if !defined(_KEYS_)
#include "private.h"
#endif

#if !defined(DEFAULT_API_KEY)
#error "Add the DEFAULT_API_KEY key into acn-sdk-c/private.h file"
#endif
#if !defined(DEFAULT_SECRET_KEY)
#error "ADD the DEFAULT_SECRET_KEY key into acn-sdk-c/private.h file"
#endif

#if defined(__IBM__)
//#define HTTP_CIPHER
//#define MQTT_CIPHER
#elif defined(__AZURE__)
//  #define HTTP_CIPHER
#  define MQTT_CIPHER
#else // for IoT
#  if !defined(DEV_ENV)
//#   define HTTP_CIPHER
//#   define MQTT_CIPHER
#  endif
//#define MQTT_CIPHER
#endif

#if defined(__linux__) \
    || defined(_ARIS_) \
    || defined(__MBED__) \
    || defined(__XCC__) \
    || defined(__senseability__) \
    || defined(__stm32l475iot__) \
    || defined(__semiconductor__) \
    || defined(__ADUCM3029__)
#else
# error "platform doesn't support"
#endif


#if defined(__XCC__)
# define __NO_STD__
#else
# define __USE_STD__
#endif

#if !defined(SSP_PARAMETER_NOT_USED)
# define SSP_PARAMETER_NOT_USED(x) (void)((x))
#endif

#define NTP_DEFAULT_SERVER "0.pool.ntp.org"
#define NTP_DEFAULT_PORT 123
#define NTP_DEFAULT_TIMEOUT 4000

/* Initialize AP mode parameters structure with SSID, channel and OPEN security type. */
#if !defined(MAIN_WLAN_SSID)
# if defined(_ARIS_)
#  define MAIN_WLAN_SSID      "ARIS_WIFI"
# elif defined(__MBED__)
#  define MAIN_WLAN_SSID      "NUCLEO_WIFI"
# elif defined(__XCC__)
#  define MAIN_WLAN_SSID      "QCA4010_WIFI"
# endif
#endif

#define MAIN_WLAN_CHANNEL   5
#define MAIN_WLAN_AUTH      1

/* cloud connectivity */
#if defined(HTTP_CIPHER)
# define ARROW_SCH "https"
# define ARROW_PORT 443
#else
# define ARROW_SCH "http"
# define ARROW_PORT 12001
#endif
#if defined(DEV_ENV)
# define ARROW_ADDR "pgsdev01.arrowconnect.io"
#else
# define ARROW_ADDR "api.arrowconnect.io"
#endif

#if defined(MQTT_CIPHER)
#  define MQTT_SCH "tls"
# define MQTT_PORT 8883
#else
# define MQTT_SCH "tcp"
# define MQTT_PORT 1883
#endif

#if defined(__IBM__)
#  define MQTT_ADDR ".messaging.internetofthings.ibmcloud.com"
#elif defined(__AZURE__)
#  define MQTT_ADDR "pgshubdev01.azure-devices.net"
#  define VHOST "iothubowner"
#else
# if defined(DEV_ENV)
#  define MQTT_ADDR "pgsdev01.arrowconnect.io"
#  define VHOST "/themis.dev:"
# else
#  define MQTT_ADDR "mqtt-a01.arrowconnect.io"
#  define VHOST "/pegasus:"
# endif
#endif

#define ARROW_API_BASE_URL                  ARROW_SCH "://" ARROW_ADDR ":" xstr(ARROW_PORT)
#define ARROW_API_GATEWAY_ENDPOINT          ARROW_API_BASE_URL "/api/v1/kronos/gateways"
#define ARROW_API_DEVICE_ENDPOINT           ARROW_API_BASE_URL "/api/v1/kronos/devices"
#define ARROW_API_TELEMETRY_ENDPOINT        ARROW_API_BASE_URL "/api/v1/kronos/telemetries"
#define ARROW_API_EVENTS_ENDPOINT           ARROW_API_BASE_URL "/api/v1/core/events"
#define ARROW_API_ACCOUNT_ENDPOINT          ARROW_API_BASE_URL "/api/v1/kronos/accounts"
#define ARROW_API_NODE_ENDPOINT             ARROW_API_BASE_URL "/api/v1/kronos/nodes"
#define ARROW_API_NODE_TYPE_ENDPOINT        ARROW_API_BASE_URL "/api/v1/kronos/nodes/types"
#define ARROW_API_TESTSUITE_ENDPOINT        ARROW_API_BASE_URL "/api/v1/kronos/testsuite"
#define ARROW_API_SOFTWARE_RELEASE_ENDPOINT ARROW_API_BASE_URL "/api/v1/kronos/software/releases/transactions"
#define ARROW_MQTT_URL                      MQTT_SCH "://" MQTT_ADDR ":" xstr(MQTT_PORT)

/* gateway and device configuration */

#if defined(_ARIS_)
/* gateway configuration */
# define GATEWAY_UID_PREFIX          "aris"
# define GATEWAY_NAME                "aris-gateway-demo"
# define GATEWAY_OS                  "ThreadX"
/* device configuration */
# define DEVICE_NAME         "aris-device-demo"
# define DEVICE_TYPE         "aris-device"
# define DEVICE_UID_SUFFIX   "board"
#elif defined(TARGET_NUCLEO_F401RE)
    // gateway
# define GATEWAY_UID_PREFIX          "nucleo"
# define GATEWAY_NAME                "my-test-gateway-123"
# define GATEWAY_OS                  "mbed"
    // device
# if defined(SENSOR_TILE)
#  define DEVICE_UID_SUFFIX   "sensortile"
#  define DEVICE_NAME         "nucleo-sensortile"
#  define DEVICE_TYPE         "st-sensortile"
# else
#  define DEVICE_UID_SUFFIX   "iks01a1"
#  define DEVICE_NAME         "nucleo iks01a1"
#  define DEVICE_TYPE         "x-nucleo-iks01a1"
# endif    
#elif defined(__linux__)
    // gateway
# define GATEWAY_UID_PREFIX          "probook"
# define GATEWAY_NAME                "probook-gateway-demo"
# define GATEWAY_OS                  "linux"
    // device
# define DEVICE_NAME         "probook-4540s"
# define DEVICE_TYPE         "hp-probook-4540s"
# define DEVICE_UID_SUFFIX   "notebook"

#elif defined(__XCC__)
    // gateway
# define GATEWAY_UID_PREFIX          "QCA"
# define GATEWAY_NAME                "QCA-gateway-demo"
# define GATEWAY_OS                  "ThreadX"
    // device
#define DEVICE_NAME         "ULPGN"
#define DEVICE_TYPE         "SX_ULPGN"
#define DEVICE_UID_SUFFIX   "devkit"

#elif defined(__senseability__)
    // gateway
# define GATEWAY_UID_PREFIX          "Cypress"
# define GATEWAY_NAME                "Cypress-gate-demo"
# define GATEWAY_OS                  "none"
    // device
#define DEVICE_NAME         "SenseAbility20"
#define DEVICE_TYPE         "SenseAbility"
#define DEVICE_UID_SUFFIX   "devkit"

#elif defined(__stm32l475iot__)
    // gateway
# define GATEWAY_UID_PREFIX          "STM32"
# define GATEWAY_NAME                "STM32-gate-demo"
# define GATEWAY_OS                  "rtos"
    // device
#define DEVICE_NAME         "B-L475E-IOT01"
#define DEVICE_TYPE         "B-L475E-Type"
#define DEVICE_UID_SUFFIX   "devkit"

#elif defined(__semiconductor__)
    // gateway
# define GATEWAY_UID_PREFIX          "semiconductor"
# define GATEWAY_NAME                "semiconductor-demo"
# define GATEWAY_OS                  "mbed"
    // device
#define DEVICE_NAME         "BB-GEVK"
#define DEVICE_TYPE         "BB-GEVK-IOT"
#define DEVICE_UID_SUFFIX   "devkit"

#elif defined(__ADUCM3029__)
// gateway
# define GATEWAY_UID_PREFIX          "ADUCM3029"
# define GATEWAY_NAME                "semiconductor-demo"
# define GATEWAY_OS                  "crosscore"
// device
#define DEVICE_NAME         "ADICUP3029"
#define DEVICE_TYPE         "ADICUP3029-IOT"
#define DEVICE_UID_SUFFIX   "devkit"

#else
# error "Not supported platform"
#endif
#define GATEWAY_TYPE                "Local"
#define GATEWAY_SOFTWARE_NAME       "eos"
#define GATEWAY_SOFTWARE_VERSION    "0.1"

/* telemetry configuration */

#define TELEMETRY_DEVICE_HID          "_|deviceHid"
#define TELEMETRY_TEMPERATURE         "f|temperature"
#define TELEMETRY_HUMIDITY            "f|humidity"
#define TELEMETRY_PH                  "f|ph"
#define TELEMETRY_RED_LIGHT           "f|red_intensity"
#define TELEMETRY_GREEN_LIGHT         "f|green_intensity"
#define TELEMETRY_BLUE_LIGHT          "f|blue_intensity"
#define TELEMETRY_RED_CONCENTRATION   "f|red_concentration"
#define TELEMETRY_GREEN_CONCENTRATION "f|green_concentration"
#define TELEMETRY_BLUE_CONCENTRATION  "f|blue_concentration"


#if !defined(TELEMETRY_DELAY)
#define TELEMETRY_DELAY             8000
#endif
#define TO_FAHRENHEIT(x) ((x)*1.8 + 32)

#endif /* CONFIG_H_ */
