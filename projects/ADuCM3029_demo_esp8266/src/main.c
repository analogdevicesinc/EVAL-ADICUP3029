/*****************************************************************************
 * main.c
 *****************************************************************************/

/***************************** Include Files **********************************/
#include <stdio.h>

#include <sys/platform.h>
#include "adi_initialize.h"
#include "Communication.h"
#include "Timer.h"
#include "ESP8266.h"
#include "MQTTPacket.h"
#include "ADXL362.h"
#include <drivers/uart/adi_uart.h>
#include "string.h"

/************************** Variable Definitions ******************************/
uint32_t   nRxCallbackCounter=0;
uint32_t   nTxCallbackCounter=0;

/************************* Functions Definitions ******************************/
/* Pin muxing */
extern int32_t adi_initpinmux(void);

/*********************************************************************

    Function:       UARTCallback

    Description:    In the example, we've configured the inbound buffer
                    to generate a callback when it is full

*********************************************************************/
void UARTCallback(
    void        *pAppHandle,
    uint32_t     nEvent,
    void        *pArg
)
{
    /* CASEOF (event type) */
    switch (nEvent)
    {
        /* CASE (buffer processed) */
        case ADI_UART_EVENT_TX_BUFFER_PROCESSED:
        	nTxCallbackCounter++;
        	 break;
        case ADI_UART_EVENT_RX_BUFFER_PROCESSED:
        	nRxCallbackCounter++;
            break;
    }
    /* return */
}

/**
  @brief Main function

**/
int main(int argc, char *argv[])
{
	uint8_t ui8x[22];
	uint8_t ui8y[22];
	uint8_t ui8z[22];
	uint8_t ui8awake = 0;
	uint8_t ui8awake_old = 0;
	uint16_t ui16PinValue;

	/* Initialize managed drivers and/or services */
	adi_initComponents();

	timer_start(); // Start timer

	adi_initpinmux(); // Init port configuration for UART, SPI and I2C

	ESP8266_Init();

	/* Initialize SPI */
	SPI_Init();

	/* Initialize accelerometer */
	Sensor_Init();

	/* Start accelerometer measurement mode */
	Sensor_Start();

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	char buf[200] = "";
	int buflen = sizeof(buf);
	MQTTString topicString = MQTTString_initializer;
	int len = 0;
	int msgid = 1;
	int req_qos = 0;

	data.MQTTVersion = 3;
	data.clientID.cstring = "me";
	data.keepAliveInterval = 3200;
	data.cleansession = 1;

	ESP8266_ConnectAccessPoint(SSID, PASS);
	ESP8266_ConnectTCP(MQTT_SERVER, PORT);
	timer_sleep(1000);

	len = MQTTSerialize_connect((unsigned char *)buf, buflen, &data);
	ESP8266_SendTCPData(buf, len); // send connect to the server
	timer_sleep(1000);

	/* subscribe */
	topicString.cstring = "subtopic";
	len = MQTTSerialize_subscribe((unsigned char *)buf, buflen, 0, msgid, 1, &topicString, &req_qos);
	ESP8266_SendTCPData(buf, len); // send subscribe data to the broker
	// The reply format is +IPD,n:message, so we look for ":" in the reply
	ESP8266_WaitForBrokerResponse(0x3A);

	if (MQTTPacket_read((unsigned char *)buf, buflen, ESP8266_ReadTCPData) == SUBACK) // check if suback is received
	{
		unsigned short submsgid;
		int subcount;
		int granted_qos;

		MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, (unsigned char *)buf, buflen);
		if (granted_qos != 0)
		{
			printf("granted qos != 0, %d\n", granted_qos);
		}
		printf("SUBACK received\n");
	}
	else
		printf("Unable to read SUBACK\n");

	timer_sleep(1000);

	/* loop getting msgs on subscribed topic */
	topicString.cstring = "adxl";
	printf("Waiting for PUBLISH\n");

	while(1) {

		adi_gpio_GetData(INTACC_PORT, INTACC_PIN, &ui16PinValue);

		if (ui16PinValue & INTACC_PIN)
			ui8awake = 1;
		 else
			ui8awake = 0;

		/* Make only one sensor reading */
		if ((ui8awake == 1) &&
			(ui8awake_old == 0)) {

			Sensor_Scan();

			printf("publishing sensor reading\n");

			uint8_t ui8s[72] = "";

			sprintf((char *)ui8x, "x = % 5d", i16SensorX);
			sprintf((char *)ui8y, "y = % 5d", i16SensorY);
			sprintf((char *)ui8z, "z = % 5d", i16SensorZ);

			strncat((char *)ui8s, (char *)ui8x, strlen((char *)ui8x));
			strncat((char *)ui8s, "\r\n", 2);
			strncat((char *)ui8s, (char *)ui8y, strlen((char *)ui8y));
			strncat((char *)ui8s, "\r\n", 2);
			strncat((char *)ui8s, (char *)ui8z, strlen((char *)ui8z));
			strncat((char *)ui8s, "\r\n", 2);

			len = MQTTSerialize_publish((unsigned char *)buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)ui8s, strlen((char *)ui8s));
			ESP8266_SendTCPData(buf, len);
		}
		ui8awake_old = ui8awake;
	}
	return 0;
}
