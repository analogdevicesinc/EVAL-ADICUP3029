/*****************************************************************************
 * main.c
 *****************************************************************************/

/***************************** Include Files **********************************/
#include <stdio.h>

#include <sys/platform.h>
#include "adi_initialize.h"
#include "Communication.h"
#include "Timer.h"
#include <drivers/uart/adi_uart.h>
#include "string.h"
#include "wifi.h"

#include <arrow/routine.h>
#include <arrow/mqtt.h>
#include <arrow/storage.h>
#include <arrow/software_release.h>
#include "sensors_data.h"
#include <arrow/device_command.h>
#include <arrow/storage.h>

#include <adi_wifi.h>
#include "time/time.h"
#include "debug.h"

/*Circuit note includes*/
#include "adi_cn0398.h"
#include "adi_cn0397.h"
#include "adi_cn0410.h"

#include <adi_wifi_noos.h>

using namespace adi_sensor_swpack;

/************************** Variable Definitions ******************************/
uint32_t   nRxCallbackCounter=0;
uint32_t   nTxCallbackCounter=0;

/*Sensor objects*/
CN0398  * cn0398 = new CN0398();
SENSOR_RESULT   eSensorResult;

CN0397     *cn0397 = new CN0397();
VisibleLight   *pLight = cn0397;

CN0410 *cn0410 = new CN0410(pLight);;

uint8_t         gpioMemory[ADI_GPIO_MEMORY_SIZE];

uint8_t         nHttpData[1000];
uint32_t        nValidBytes;
uint8_t         au8Link[2];
uint32_t        test;

sensors_data_t  data;

uint8_t         m_spi_memory[ADI_SPI_MEMORY_SIZE];


ADI_WIFI_TCP_CONNECT_CONFIG  gTCPConnect = {
													.nLinkID = ADI_WIFI_CONNECTION_ID,
													.pType = (uint8_t *) "TCP",
													.pIP   = (uint8_t *)ARROW_ADDR,
													.pPort =  (uint8_t *)"12001",
													.nTCPKeepAlive = (uint16_t)ADI_WIFI_KEEPALIVE};


/************************* Functions Definitions ******************************/

/* Pin muxing */
extern "C" int32_t adi_initpinmux(void);

/*********************************************************************

    Function:       UARTCallback

    Description:    In the example, we've configured the inbound buffer
                    to generate a callback when it is full

*********************************************************************/
/**
 * @brief      Application Callback
 *
 * @details    Called by the framework layer (adi_wifi_noos.c) when an event occurs.
 *
 * @param [in] pCBParam : Callback parameter (unused)
 *
 * @param [in] Event : Event of type #ADI_WIFI_AT_CMDCODE.
 *
 * @param [in] pArg : Callback argument (unused)
 *
 */
void adi_wifi_AplicationCallback(void * pCBParam, uint32_t nEvent, void * pArg)
{
	uint8_t  u8Link;

	switch (nEvent)
	{
	case CMD_NONE:
	{
		/* Read the publish Data. */
		adi_wifi_ParseSubscriberData(nEvent);
		break;
	}
	case CMD_AT_HTTPGET:
	{
		if (gTCPConnect.nLinkID != 5)
		{
			if(adi_wifi_tal_Read(au8Link, 2u) == ADI_WIFI_TAL_SUCCESS)
			{
				//Get the link value to see if it corespons
				u8Link = atoi((char *)au8Link);
				if (u8Link != gTCPConnect.nLinkID)
				{
					printf("Bad link connection!\r\n");
				}
			}
		}

		/* Read the HTTP response Data. */
		adi_wifi_ParseSubscriberData(nEvent);

		if(adi_wifi_radio_IsTrunkedEncoding())
		{
			/* Update the User buffer with HTTP GET response data. */
			adi_wifi_GetData(&nHttpData[test], &test);
		}
		break;
	}
	case CMD_AT_HTTPHEAD:
	{
		/* Read the HTTP response Data. */
		adi_wifi_ParseSubscriberData(nEvent);
		break;
	}
	case CMD_HW_ERROR:
	{
		break;
	}

	default:
		break;
	}

	return;
}

/**
   @brief Initialize the WiFi connection

   @return void

**/
void InitWiFiConnection()
{
	ADI_WIFI_RESULT eResult;

    /* Initialize the ESP8266 Wi-Fi module */
	eResult = adi_wifi_Init(adi_wifi_AplicationCallback, NULL);

	eResult = adi_wifi_radio_TestAT();

	eResult = adi_wifi_radio_SetWiFiMode(1);

	eResult = adi_wifi_radio_DisconnectFromAP();

	eResult = adi_wifi_radio_ConnectToAP((uint8_t *)SSID, (uint8_t *)PASS,  NULL);

	eResult = adi_wifi_radio_EnableMultipleConnections(1);

}

/**
   @brief Callback function called when a command is received from the cloud.

   @param str - pointer to a constant string to get the command value

   @return void

**/
int Leds_Controll(const char *str)
{
	DBG("cmd: [%s]", str);
	JsonNode *_main = json_decode(str);

	JsonNode *red = json_find_member(_main, "red");
	JsonNode *blue = json_find_member(_main, "blue");
	JsonNode *green = json_find_member(_main, "green");

	/*Control algorith for CN0410 led bar*/
	cn0410->Controll((float)red->number_, (float)blue->number_, (float)green->number_);

	json_delete(_main);

	return 0;
}

/**
  @brief Main function

**/
int main(int argc, char *argv[])
{
	/* Initialize managed drivers and/or services */
	adi_initComponents();

	timer_start(); // Start timer

	adi_initpinmux(); // Init port configuration for UART, SPI and I2C
	adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE); //initialize gpio

	RTC_Init();

	/*Initialize the sensors*/
	cn0398->setup();
	cn0398->init();

	eSensorResult = pLight->open();

	cn0398->use_nernst = false; // use default calibration

	cn0398->set_data();

	/* Get the red, green and blue visual light data and concentration. */
	pLight->getLightIntensity((float *)&eIntensity.fData_Red);
	pLight->getLightConcentration((float *)&eConcentration.fData_Red);

	cn0410->Init();
	cn0410->Setup();

	/*reset led values*/
	cn0410->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_A, 0);
	cn0410->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_B, 0);
	cn0410->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_C, 0);

	/*prepare the data to send over MQTT*/
	PrepareMqttPayload(&data);

	InitWiFiConnection();

	// init a gateway and device by the cloud
	arrow_initialize_routine();

	PrepareMqttPayload(&data);

	// establish the MQTT connection
	arrow_mqtt_connect_routine();

	/*set command leds handler*/
	add_cmd_handler("leds", Leds_Controll);

	// use the MQTT connection to send a telemetry information
	arrow_mqtt_send_telemetry_routine(PrepareMqttPayload, &data);

}
