/*!
 *****************************************************************************
 * @file:    ADuCM3029_IBMWatson.h
 * @brief:
 * @version: $Revision$
 * @date:    $Date$
 *-----------------------------------------------------------------------------
 *
Copyright (c) 2015-2018 Analog Devices, Inc.
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Modified versions of the software must be conspicuously marked as such.
  - This software is licensed solely and exclusively for use with processors
    manufactured by or for Analog Devices, Inc.
  - This software may not be combined or merged with other code in any manner
    that would cause the software to become subject to terms and conditions
    which differ from those listed here.
  - Neither the name of Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.
  - The use of this software may or may not infringe the patent rights of one
    or more patent holders.  This license does not release you from the
    requirement that you obtain separate licenses from these patent holders
    to use this software.
THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
NON-INFRINGEMENT, TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF
CLAIMS OF INTELLECTUAL PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

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
