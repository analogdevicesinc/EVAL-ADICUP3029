/***************************************************************************//**
 *   @file   communication.c
 *   @brief  Implementation of communication layer.
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

#ifdef _DEBUG
#include <stdio.h>
#endif

#include "communication.h"
#include "uart.h"
#include "irq.h"
#include "irq_extra.h"
#include "uart_extra.h"
#include "wifi.h"
#include "tcp_socket.h"
#include "mqtt_client.h"
#include "test_mosquitto_cert.h"
#include "debug.h"

/* Initialization structures */

struct aducm_uart_init_param extra_uart = {.parity = CONFIG_UART_PARITY,
	       .stop_bits = CONFIG_UART_STOPBITS,
	       .word_length = CONFIG_UART_WORD_LEN
};
struct uart_init_param uart_conf = {.baud_rate = CONFIG_UART_BAUDRATE,
	       .device_id = 0,
	       .extra = &extra_uart
};

#ifndef DISABLE_SECURE_SOCKET
struct secure_init_param tls_param = {
		.ca_cert = NULL,
		.ca_cert_len = 0,
		.cli_cert = NULL,
		.cli_cert_len = 0,
		.cli_pk = NULL,
		.cli_pk_len = 0,
		.trng_init_param = NULL
};
#endif /* DISABLE_SECURE_SOCKET */

struct socket_address addr = {
		.addr = MQTT_SERVER_ADDRESS,
		.port = SERVER_PORT};

uint8_t send_buff[MQTT_BUFFER_SIZE];
uint8_t read_buff[MQTT_BUFFER_SIZE];
struct mqtt_init_param	mqtt_init_param = {
	.timer_id = 0,
	.extra_timer_init_param = NULL,
	.command_timeout_ms = MQTT_TIMEOUT,
	.send_buff = send_buff,
	.read_buff = read_buff,
	.send_buff_size = MQTT_BUFFER_SIZE,
	.read_buff_size = MQTT_BUFFER_SIZE,
	.message_handler = NULL
};

struct mqtt_connect_config conn_config = {
	.version = MQTT_VERSION_3_1,
	.keep_alive_ms = MQTT_KEEPALIVE,
	.client_name = (int8_t *)MQTT_CLIENT_NAME,
	.username = NULL,
	.password = NULL
};

static bool use_mqtt = COMUNICATION_METHOD;
static struct wifi_desc		*wifi;
static struct mqtt_desc		*mqtt;
static struct tcp_socket_desc	*sock;
static struct uart_desc		*udesc;

bool init_mqtt(struct mqtt_desc **desc, struct irq_ctrl_desc *idesc) {
	struct wifi_init_param		wifi_param;
	struct tcp_socket_init_param	tcp_param;
	int32_t				ret;

	ret = uart_init(&udesc, &uart_conf);
	if (IS_ERR_VALUE(ret))
		goto fail;

	wifi_param.irq_desc = idesc;
	wifi_param.uart_desc = udesc;
	wifi_param.uart_irq_conf = udesc;
	wifi_param.uart_irq_id = ADUCM_UART_INT_ID;

	ret = wifi_init(&wifi, &wifi_param);
	if (IS_ERR_VALUE(ret))
		goto fail;
	ret = wifi_connect(wifi, WIFI_SSID, WIFI_PASS);
	if (IS_ERR_VALUE(ret))
		goto fail;

	wifi_get_network_interface(wifi, &tcp_param.net);
	/* Default buffer size */
	tcp_param.max_buff_size = 0;
#ifndef DISABLE_SECURE_SOCKET
	tcp_param.secure_init_param = &tls_param;
#endif /* DISABLE_SECURE_SOCKET */
	ret = socket_init(&sock, &tcp_param);
	if (IS_ERR_VALUE(ret))
		goto fail;

	ret = socket_connect(sock, &addr);
	if (IS_ERR_VALUE(ret))
		goto fail;

	mqtt_init_param.sock = sock;
 	ret = mqtt_init(&mqtt, &mqtt_init_param);
	if (IS_ERR_VALUE(ret))
		goto fail;

	ret = mqtt_connect(mqtt, &conn_config, NULL);
	if (IS_ERR_VALUE(ret))
		goto fail;

	*desc = mqtt;

	return SUCCESS;

fail:
	uart_remove(udesc);
	return FAILURE;
}

/* Initialize communication.
 * If use_mqtt is set the program will try to connect to the mqtt server.
 * If it fails, UART will be initialized.
 */
int32_t init_communication(struct uart_desc **desc, struct irq_ctrl_desc *idesc)
{
	if (use_mqtt) {
		if (SUCCESS == init_mqtt(desc, idesc)) {
			pr_debug("MQTT comunication initialized\n");

			return SUCCESS;
		}
	}

	use_mqtt = 0;
	if (SUCCESS == uart_init(desc, &uart_conf)) {
		pr_debug("UART comunication initialized\n");

		return SUCCESS;
	}

	return FAILURE;
}

/* Send data over UART or to MQTT server */
int32_t send_data(struct uart_desc *desc, char *data, int len) {
	if (use_mqtt) {
	 	struct mqtt_message msg = {
	 		.qos = MQTT_QOS0,
	 		.retained = true,
	 		.payload = (uint8_t *)data,
	 		.len = len
	 	};
	 	return mqtt_publish(desc, (int8_t *)MQTT_PUBLISH_TOPIC, &msg);
	}
	else
		return uart_write(desc, (uint8_t *)data, len);
}
