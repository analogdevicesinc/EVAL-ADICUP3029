/***************************************************************************//**
 *   @file   communication.c
 *   @brief  Interface for communication layer.
 *   @author mihail.chindris@analog.com
********************************************************************************
 * Copyright 2021(c) Analog Devices, Inc.
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

#ifndef COMUNICATION_H
#define COMUNICATION_H

#include "irq.h"
#include "uart.h"
#include "mqtt_client.h"

#define MQTT 1
#define UART 0

/* Configuration parameters */

#define COMUNICATION_METHOD	MQTT
//#define COMUNICATION_METHOD	UART

/* UART configuration parameters */
#define CONFIG_UART_PARITY	UART_NO_PARITY
#define CONFIG_UART_STOPBITS	UART_ONE_STOPBIT
#define CONFIG_UART_WORD_LEN	UART_WORDLEN_8BITS
#define CONFIG_UART_BAUDRATE	BD_115200

/* WIFI */
#define WIFI_SSID		"analog_hotspot"
#define WIFI_PASS		"12345678"

/* MQTT Server */
#define MQTT_SERVER_ADDRESS	"test.mosquitto.org"
#ifndef DISABLE_SECURE_SOCKET
#define SERVER_PORT		8883
#else
#define SERVER_PORT		1883
#endif

/* MQTT Client */
#define MQTT_BUFFER_SIZE	500
#define MQTT_TIMEOUT		20000 //ms
#define MQTT_KEEPALIVE		7200 //ms
#define MQTT_CLIENT_NAME	"analog_client"
#define MQTT_PUBLISH_TOPIC	"analog_test_topic"

int32_t init_communication(struct uart_desc **desc, struct irq_ctrl_desc *idesc);
int32_t send_data(struct uart_desc *desc, char *data, int len);

#endif
