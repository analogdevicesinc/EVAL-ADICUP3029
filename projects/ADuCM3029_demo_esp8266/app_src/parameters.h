/***************************************************************************//**
 *   @file   parameters.h
 *   @brief  Parameters Definitions to configure project.
 *   @author Mihail Chindris (mihail.chindris@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "uart_extra.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/* Configure to which mqtt server to connect */

/* Connect to local mqtt broker */
#define LOCAL			0
/* Connect to online broker */
#define TEST_MOSQUITTO		1
#define MQTT_ECLIPSE		2

#define SERVER_TO_USE		TEST_MOSQUITTO

#if SERVER_TO_USE==TEST_MOSQUITTO
#define SERVER_ADDR		"test.mosquitto.org"
#elif SERVER_TO_USE==MQTT_ECLIPSE
#define SERVER_ADDR		"mqtt.eclipse.org"
#else //local
#define SERVER_ADDR		"192.168.0.150"
#endif

#define SERVER_PORT		1883

/* Maximum buffer size for a message received from mqtt broker */
#define BUFF_LEN		200

/* Wifi network parameters */
#define WIFI_SSID		"analog_hotspot"
#define WIFI_PASS		"12345678"


/* Default UART parameters for esp8266 */
#define UART_CONFIG_BAUDRATE	BD_115200
#define UART_CONFIG_PARITY	UART_NO_PARITY
#define UART_CONFIG_STOP_BITS	UART_ONE_STOPBIT
#define UART_CONFIG_WORD_LEN	UART_WORDLEN_8BITS
#define UART_CONFIG_IRQ_ID	ADUCM_UART_INT_ID

/* Aducm3029 only have one UART periferal so only 0 can be used */
#define UART_DEVICE_ID		0
/* Timer ID to be used for mqtt timeouts */
#define TIMER_ID		1

/* Mqtt configuration */
#define MQTT_PUBLISH_TOPIC	"adxl"
#define MQTT_SUBSCRIBE_TOPIC	"aducm3029_messages"
#define MQTT_CONFIG_CMD_TIMEOUT	20000
#define MQTT_CONFIG_VERSION	MQTT_VERSION_3_1
#define MQTT_CONFIG_KEEP_ALIVE	7200
#define MQTT_CONFIG_CLIENT_NAME	"aducm3029-client"
#define MQTT_CONFIG_CLI_USER	NULL
//#define MQTT_CONFIG_CLI_USER	"username"
#define MQTT_CONFIG_CLI_PASS	NULL
//#define MQTT_CONFIG_CLI_PASS	"password"

/* Adxl362 configuration */
/* SPI arduino */
#define SPI_CONFIG_CHANNEL	SPI0
#define SPI_CONFIG_CS		1

#define SPI_CONFIG_MAX_SPEED	1000000 //Hz

/* Time between each scan mq*/
#define SCAN_SENSOR_TIME	500

/* Activity threshold value */
#define ACCEL_CFG_ACT_TRESH	50
/* Inactivity threshold value */
#define ACCEL_CFG_INACT_TRESH	50
/* Activity timer value in ms */
#define ACCEL_CFG_ACT_TIMER	100
/* Inactivity timer value in seconds */
#define ACCEL_CFG_INACT_TIMER	10

// Do I have to use interrupt pin and set gpio as output?
/* Select ADXL362 INT pin based on jumper P7 setting.
 * Available values:
 *    INTACC_PIN_1
 *    INTACC_PIN_2 */
#define ACCEL_CFG_INT_SEL        INTACC_PIN_1

/* ACC INT pin configuration */
#define ACCEL_INT_GPIO_NB	15

#endif // __PARAMETERS_H__
