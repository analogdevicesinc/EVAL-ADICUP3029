/*****************************************************************************
 * ADuCM3029_IBMWatson.h
 *****************************************************************************/

#ifndef __ADUCM3029_IBMWATSON_H__
#define __ADUCM3029_IBMWATSON_H__

#include <adi_wifi_noos.h>
//#include <framework/noos/adi_wifi_noos.h>
#include "sensors_data.h"
#include "telemetry.h"
extern "C" {
#include "json.h"
#include <drivers/wdt/adi_wdt.h>
}
/* Defined in pinmux.c. */
extern "C" int32_t adi_initpinmux(void);
extern "C" int Leds_Controll(const char *str);

/********************************* Wi-Fi Configuration *********************************/
/* SSID of the access point. */
uint8_t aWifiSSID[] = "AnalogDevices";

/* Password of the access point. */
uint8_t aWifiPassword[] = "Analog123!";

/********************************* MQTT Configuration *********************************/

/* IP address of the broker to publish to. */
uint8_t aMQTTBrokerIp[] = "0qqed7.messaging.internetofthings.ibmcloud.com";

/* Port of the broker to publish to. */
uint8_t aMQTTBrokerPort[] = "1883";

/*! MQTT publisher name. */
uint8_t aMQTTPublisherName[] = "d:0qqed7:C_Client:ADI_GreenHouse";

/*! MQTT Topic name. This size of this string should be ADI_WIFI_MQTT_PACKET_SIZE - sizeof(ADI_DATA_PACKET) - 2u,
 *  If you want a longer topic name, increase the size of ADI_WIFI_MQTT_PACKET_SIZE.
 */
uint8_t aMQTTTopicName[] = "iot-2/evt/ADI_GreenHouse_data/fmt/json";

uint8_t aMQTTSubscribeTopic[] = "iot-2/cmd/ADI_GreenHouse_cmd/fmt/json";
                              // iot-2/type/C_Client/id/ADI_GreenHouse/cmd/ADI_GreenHouse_cmd/fmt/json

/*MQTT username*/
uint8_t aMQTTUsername[] = "use-token-auth";

/*MQTT authentication password*/
uint8_t aMQTTPassword[] = "KObjoe*OOEOcTEgQvx";

/*! MQTT publish packet quality of service. */
#define ADI_WIFI_MQTT_PUBLISER_QOS                 (0u)

/*! MQTT publisher version. */
#define ADI_WIFI_MQTT_PUBLISHER_VERSION            (3u)

/*! MQTT publisher <-> broker connection keep alive timeout. */
#define ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE          (7200u)

/*! Connection link id, valid id range (0-4) for multiple connections. 5 indicates single connection mode. */
#define ADI_WIFI_CONNECTION_ID                     (5u)

/*! MQTT publisher <-> broker PING command timeout. */
#define ADI_WIFI_MQTT_PING_TIMEOUT                 (ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE - 1000u)

#endif /* __ADUCM3029_IBMWATSON_H__ */
