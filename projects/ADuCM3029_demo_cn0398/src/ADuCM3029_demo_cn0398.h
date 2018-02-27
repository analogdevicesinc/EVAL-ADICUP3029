/**
******************************************************************************
*   @file     ADuCM3029_demo_cn0398.h
*   @brief    Main header file for CN0398 demo
*   @version  V0.1
*   @author   ADI
*   @date     February 2018
*
*******************************************************************************
* Copyright 2018(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*  - Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  - Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
*  - Neither the name of Analog Devices, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*  - The use of this software may or may not infringe the patent rights
*    of one or more patent holders.  This license does not release you
*    from the requirement that you obtain separate licenses from these
*    patent holders to use this software.
*  - Use of the software either in source or binary form, must be run
*    on or directly connected to an Analog Devices Inc. component.
*
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*******************************************************************************/

#ifndef __ADUCM3029_DEMO_CN0398_H__
#define __ADUCM3029_DEMO_CN0398_H__

#include <base_sensor/adi_sensor.h>
#include <base_sensor/adi_sensor_packet.h>
#include <base_sensor/adi_sensor_errors.h>
#include "adi_cn0398.h"
#include <framework/noos/adi_wifi_noos.h>

/* Defined in pinmux.c. */
extern "C" int32_t adi_initpinmux(void);

/**************************** Wi-Fi Configuration ****************************/
/* SSID of the access point. */
uint8_t aWifiSSID[] = "****";

/* Password of the access point. */
uint8_t aWifiPassword[] = "****";

/**************************** MQTT Configuration *****************************/

/* IP address of the broker to publish to. */
uint8_t aMQTTBrokerIp[] = "****";

/* Port of the broker to publish to. */
uint8_t aMQTTBrokerPort[] = "1883";

/*! MQTT publisher name. */
uint8_t aMQTTPublisherName[] = "device_publisher";

/*! MQTT Topic name. This size of this string should be
 *  ADI_WIFI_MQTT_PACKET_SIZE - sizeof(ADI_DATA_PACKET) - 2u, If you want a
 *  longer topic name, increase the size of ADI_WIFI_MQTT_PACKET_SIZE.
 */
uint8_t aMQTTTopicName[] = "cn0398";

uint8_t aMQTTTopic[] = "comm_channel";

#define SENSOR_DATA_PAYLOAD_SIZE (14u)

/*! MQTT publish packet quality of service. */
#define ADI_WIFI_MQTT_PUBLISER_QOS (0u)

/*! MQTT publisher version. */
#define ADI_WIFI_MQTT_PUBLISHER_VERSION (3u)

/*! MQTT publisher <-> broker connection keep alive timeout. */
#define ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE (7200u)

/*! Connection link id, valid id range (0-4) for multiple connections.
 *  5 indicates single connection mode. */
#define ADI_WIFI_CONNECTION_ID (5u)

/*! MQTT publisher <-> broker PING command timeout. */
#define ADI_WIFI_MQTT_PING_TIMEOUT (ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE - 1000u)

/*************************** Sensor Configuration ****************************/

/* Accelerometer instance ID. */
#define ADI_ADC_ID (1u)

#endif /* __ADUCM3029_DEMO_CN0398_H__ */
