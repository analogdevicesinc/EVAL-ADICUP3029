/***************************************************************************//**
 *   @file   ADuCM3029_demo_esp8266.c
 *   @brief  Example of using esp8266 module to send adxl362 data
 *   @author Mihail Chindirs (mihail.chindris@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
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

#include <stdio.h>

#include "no_os_delay.h"
#include "no_os_error.h"
#include "no_os_irq.h"
#include "irq_extra.h"
#include "mqtt_client.h"
#include "parameters.h"
#include "platform_init.h"
#include "spi_extra.h"
#include "tcp_socket.h"
#include "wifi.h"


#define PRINT_ERR_AND_RET(msg, ret) do {\
	printf("%s - Code: %d (-0x%x) \n", msg, ret, ret);\
	return ret;\
} while (0)

int32_t init_and_connect_wifi(struct wifi_desc **wifi)
{
	static struct no_os_irq_ctrl_desc	*irq_ctrl;
	static struct no_os_uart_desc		*udesc;

	struct no_os_irq_init_param		irq_par;
	struct wifi_init_param			wifi_param;
	struct no_os_uart_init_param		uart_param;
	struct aducm_uart_init_param		uart_platform_param;
	int32_t				ret;

	/* Initialize irq controller */
	irq_par = (struct no_os_irq_init_param) {
		.irq_ctrl_id = 0,
		.platform_ops = &aducm_irq_ops,
		.extra = 0
	};
	ret = no_os_irq_ctrl_init(&irq_ctrl, &irq_par);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error irq_ctrl_init", ret);

	/* Initialize uart device */
	uart_platform_param = (struct aducm_uart_init_param) {
		.parity = UART_CONFIG_PARITY,
		.stop_bits = UART_CONFIG_STOP_BITS,
		.word_length = UART_CONFIG_WORD_LEN
	};
	uart_param = (struct no_os_uart_init_param) {
		.device_id = UART_DEVICE_ID,
		.baud_rate = UART_CONFIG_BAUDRATE,
		.extra = &uart_platform_param
	};
	ret = no_os_uart_init(&udesc, &uart_param);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error uart_init", ret);

	/* Initialize wifi descriptor */
	wifi_param = (struct wifi_init_param) {
		.irq_desc = irq_ctrl,
		.uart_desc = udesc,
		.uart_irq_conf = udesc,
		.uart_irq_id = UART_CONFIG_IRQ_ID
	};
	ret = wifi_init(wifi, &wifi_param);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error wifi_init", ret);

	/* Connect to wifi network */
	ret = wifi_connect(*wifi, WIFI_SSID, WIFI_PASS);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error wifi_connect", ret);

	printf("Connected to: %s\n", WIFI_SSID);

	return SUCCESS;
}

void mqtt_message_handler(struct mqtt_message_data *msg)
{
	char	buff[101];
	int32_t	len;

	/* Message.payload don't have at the end '\0' so we have to add it. */
	len = msg->message.len > 100 ? 100 : msg->message.len;
	memcpy(buff, msg->message.payload, len);
	buff[len] = 0;

	printf("Topic:%s -- Payload: %s\n", msg->topic, buff);
}

int init_and_connect_to_mqtt_broker(struct mqtt_desc **mqtt,
				    struct wifi_desc *wifi)
{
	static uint8_t			send_buff[BUFF_LEN];
	static uint8_t			read_buff[BUFF_LEN];
	static struct tcp_socket_desc	*sock;

	struct mqtt_init_param		mqtt_init_param;
	struct tcp_socket_init_param	socket_init_param;
	struct socket_address		mqtt_broker_addr;
	struct mqtt_connect_config	conn_config;
	int32_t				ret;

	/* Initialize socket structure */
	socket_init_param.max_buff_size = 0; //Default buffer size
	ret = wifi_get_network_interface(wifi, &socket_init_param.net);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error wifi_get_network_interface", ret);

	ret = socket_init(&sock, &socket_init_param);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error socket_init", ret);

	/* Connect socket to mqtt borker server */
	mqtt_broker_addr = (struct socket_address) {
		.addr = SERVER_ADDR,
		.port = SERVER_PORT
	};
	ret = socket_connect(sock, &mqtt_broker_addr);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error socket_connect", ret);

	printf("Connection with \"%s\" established\n", SERVER_ADDR);

	/* Initialize mqtt descriptor */
	mqtt_init_param = (struct mqtt_init_param) {
		.timer_id = TIMER_ID,
		.extra_timer_init_param = NULL,
		.sock = sock,
		.command_timeout_ms = MQTT_CONFIG_CMD_TIMEOUT,
		.send_buff = send_buff,
		.read_buff = read_buff,
		.send_buff_size = BUFF_LEN,
		.read_buff_size = BUFF_LEN,
		.message_handler = mqtt_message_handler
	};
	ret = mqtt_init(mqtt, &mqtt_init_param);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error mqtt_init", ret);

	/* Connect to mqtt broker */
	conn_config = (struct mqtt_connect_config) {
		.version = MQTT_CONFIG_VERSION,
		.keep_alive_ms = MQTT_CONFIG_KEEP_ALIVE,
		.client_name = MQTT_CONFIG_CLIENT_NAME,
		.username = MQTT_CONFIG_CLI_USER,
		.password = MQTT_CONFIG_CLI_PASS
	};
	ret = mqtt_connect(*mqtt, &conn_config, NULL);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error mqtt_connect", ret);

	printf("Connected to mqtt broker\n");

	/* Subscribe for a topic */
	ret = mqtt_subscribe(*mqtt, MQTT_SUBSCRIBE_TOPIC, MQTT_QOS0, NULL);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error mqtt_subscribe", ret);
	printf("Subscribed to topic: %s\n", MQTT_SUBSCRIBE_TOPIC);

	return SUCCESS;
}

int32_t read_and_send(struct mqtt_desc *mqtt)
{
	struct mqtt_message	msg;
	uint8_t			buff[100];
	uint32_t		len;
	float			x;
	float			y;
	float			z;
	float			t;

	/* Serialize data */
	len = sprintf(buff, "X:adsd -- Y: dsadsad -- Z: dasdad -- Temp: dasda");
	/* Send data to mqtt broker */
	msg = (struct mqtt_message) {
		.qos = MQTT_QOS0,
		.payload = buff,
		.len = len,
		.retained = false
	};
	return mqtt_publish(mqtt, MQTT_PUBLISH_TOPIC, &msg);
}

int main(int argc, char *argv[])
{
	struct wifi_desc	*wifi;
	struct mqtt_desc	*mqtt;
	int32_t			ret;

	ret = platform_init();
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error platform_init", ret);

	ret = init_and_connect_wifi(&wifi);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error init_and_connect_wifi", ret);

	ret = init_and_connect_to_mqtt_broker(&mqtt, wifi);
	if (NO_OS_IS_ERR_VALUE(ret))
		PRINT_ERR_AND_RET("Error init_and_connect_to_mqtt_broker", ret);

	while (true) {

		ret = read_and_send(mqtt);
		if (NO_OS_IS_ERR_VALUE(ret))
			PRINT_ERR_AND_RET("Error read_and_send", ret);
		printf("Data sent to broker\n");

		/* Dispatch new mqtt mesages if any during SCAN_SENSOR_TIME */
		ret = mqtt_yield(mqtt, SCAN_SENSOR_TIME);
		if (NO_OS_IS_ERR_VALUE(ret))
			PRINT_ERR_AND_RET("Error mqtt_yield", ret);
	}

	return 0;
}
