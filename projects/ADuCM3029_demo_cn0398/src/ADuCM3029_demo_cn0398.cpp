/**
******************************************************************************
*   @file     ADuCM3029_demo_cn0398.cpp
*   @brief    Main source file for CN0398 demo
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

#include <sys/platform.h>
#include "ADuCM3029_demo_cn0398.h"
#include <common.h>
#include <common/adi_timestamp.h>
#include "Timer.h"

#define MQTT_HEADER_SIZE 4

using namespace adi_sensor_swpack;

/* Static data */
ADI_WIFI_TCP_CONNECT_CONFIG  gTCPConnect = {
	.nLinkID = ADI_WIFI_CONNECTION_ID,
	.pType = (uint8_t *) "TCP",
	.pIP   = aMQTTBrokerIp,
	.pPort =  aMQTTBrokerPort,
	.nTCPKeepAlive = ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE
};

ADI_WIFI_MQTT_CONNECT_CONFIG gMQTTConnect = {
	.nLinkID = ADI_WIFI_CONNECTION_ID,
	.nVersion = ADI_WIFI_MQTT_PUBLISHER_VERSION,
	.pName = aMQTTPublisherName,
	.nMQTTKeepAlive = ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE
};

ADI_WIFI_PUBLISH_CONFIG 		gPublishConfig = {
	.pMQTTData = NULL,
	.nMQTTDataSize = 0,
	.pTopic = aMQTTTopicName,
	.nLinkID = ADI_WIFI_CONNECTION_ID,
	.nQos = ADI_WIFI_MQTT_PUBLISER_QOS
};
ADI_WIFI_SUBSCRIBE_CONFIG 	gSubscribeConfig = {
	.pTopic = aMQTTTopic,
	.nLinkID = ADI_WIFI_CONNECTION_ID,
	.nQos = ADI_WIFI_MQTT_PUBLISER_QOS,
	.nPacketId = 0
};

extern volatile uint8_t ui8S[200];
uint8_t *payload_ptr = (uint8_t*)(ui8S + MQTT_HEADER_SIZE +
								  sizeof(aMQTTTopic) - 1);
uint32_t packet_size = 0;
extern volatile bool rec_flag;

/* Local Wi-Fi functions */
static void InitWiFiConnection(void);
static void adi_wifi_AplicationCallback(void * pCBParam,
                                        uint32_t nEvent,
                                        void * pArg);
static void MQTTPublishCN0398(CN0398 pCn);

/* Local system functions */
static void InitSystem(void);
static void Trap(void);
static void RepairConnection(void);

int main(int argc, char *argv[])
{
	CN0398         	cn0398;
	SENSOR_RESULT   eSensorResult;

	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */

	/* Initialize the system */
	InitSystem();

	/* Open ADC */
	eSensorResult = cn0398.open();

	if(eSensorResult != SENSOR_ERROR_NONE) {
		PRINT_SENSOR_ERROR(DEBUG_MESSAGE, eSensorResult);
		Trap();
	}

	/* Start measurement */
	eSensorResult = cn0398.start();

	if(eSensorResult != SENSOR_ERROR_NONE) {
		PRINT_SENSOR_ERROR(DEBUG_MESSAGE, eSensorResult);
		Trap();
	}

	/* Publish AD7124 ADC data using MQTT */
	MQTTPublishCN0398(cn0398);

	return 0;
}

/*!
 * @brief      Application Callback
 *
 * @details    Called by the framework layer (adi_wifi_noos.c) when an event
 * 			   occurs.
 *
 * @param [in] pCBParam : Callback parameter (unused)
 *
 * @param [in] Event : Event of type #ADI_WIFI_AT_CMDCODE.
 *
 * @param [in] pArg : Callback argument (unused)
 *
 */
void adi_wifi_AplicationCallback(void * pCBParam,
                                 uint32_t nEvent,
                                 void * pArg)
{
	switch (nEvent) {
	case CMD_NONE:
		/* Read the publish Data. */
		adi_wifi_ParseSubscriberData();
		adi_wifi_GetData((uint8_t*)ui8S, &packet_size);
		rec_flag = true;
		break;
	default:
		DEBUG_MESSAGE("WiFi event error!\n");
		break;
	}

	return;
}

static void MQTTPublishCN0398(CN0398 pCn)
{
	ADI_WIFI_RESULT eResult;
	uint32_t        nPacketId = 0u;
	uint8_t         nConnectionFailFlag;

	/* Initialize the ESP8266 Wi-Fi module and establish a TCP connection */
	InitWiFiConnection();

	/* Send MQTT "CONNECT" message to broker */
	eResult = adi_wifi_radio_MQTTConnect(&gMQTTConnect);
	DEBUG_RESULT("Error connecting to the MQTT broker.\r\n",
	             eResult,
	             ADI_WIFI_SUCCESS);

	/* Subscribe to receive topic */
	eResult = adi_wifi_radio_MQTTSubscribe(&gSubscribeConfig);
	DEBUG_RESULT("Error subscribing to hidden_channel topic.\r\n",
	             eResult,
	             ADI_WIFI_SUCCESS);

	/* Calibration sequence */
	sprintf((char *)ui8S, "Initialization complete!\r\n\n");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);

	sprintf((char *)ui8S, "Do you want to perform pH calibration [y/N]?\n\n");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	while(!rec_flag) {
		adi_wifi_DispatchEvents(1000);
	}
	rec_flag = false;

	if(payload_ptr[0] == 'y' || payload_ptr[0] == 'Y') {
		pCn.calibrate_ph();
	} else {
		pCn.use_nernst = true;
		sprintf((char *)ui8S,
				"Do you want to load default calibration?[y/N].");
		gPublishConfig.pMQTTData = (uint8_t*)ui8S;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
		adi_wifi_radio_MQTTPublish(&gPublishConfig);
		sprintf((char *)ui8S,
				"If not[N], the Nernst equation will be used.\n");
		gPublishConfig.pMQTTData = (uint8_t*)ui8S;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
		adi_wifi_radio_MQTTPublish(&gPublishConfig);
		while(!rec_flag)
			adi_wifi_DispatchEvents(1000);
		rec_flag = false;
		if(payload_ptr[0] == 'y' || payload_ptr[0] == 'Y')
			pCn.use_nernst = false;
	}

	/* Start the ping timer to ping the MQTT broker before
	 * the keep alive expires
	 * */
	if(adi_wifi_StartTimer(ADI_WIFI_MQTT_PING_TIMEOUT, 1u) ==
	    ADI_WIFI_SUCCESS) {
		DEBUG_MESSAGE("Starting to Publish\n");

		gPublishConfig.nPacketId = nPacketId;

		while(1) {
			nConnectionFailFlag = 0;

			/* If ping timer has expired */
			if (adi_wifi_IsTimerDone(1u) == 1u) {
				/* Send MQTT "PINGREQ" message to broker to
				 * keep the connection alive.
				 * */
				eResult = adi_wifi_radio_MQTTPing(ADI_WIFI_CONNECTION_ID);

				/* Restart the ping timer */
				if((adi_wifi_StopTimer(1u) == ADI_WIFI_SUCCESS)
				    && (eResult == ADI_WIFI_SUCCESS)) {
					if(adi_wifi_StartTimer(ADI_WIFI_MQTT_PING_TIMEOUT, 1u) !=
					    ADI_WIFI_SUCCESS)
						DEBUG_MESSAGE("Failed to start ping timer\n");
				} else {
					DEBUG_MESSAGE("Failed to ping broker\n");
					nConnectionFailFlag = 1;
				}
			}

			/* If we did not fail to ping the broker, publish data */
			if(nConnectionFailFlag == 0u) {
				DEBUG_MESSAGE("Publishing data..\n");

				timer_sleep(500);
				pCn.set_data();
				pCn.display_data();
			}

			/* If we failed to ping the broker or failed to publish data,
			 * check the connection
			 * */
			if(nConnectionFailFlag == 1u) {
				DEBUG_MESSAGE("Troubleshooting failed connection..\n");
				RepairConnection();
			}

			gPublishConfig.nPacketId++;
		}
	}
}

/*!
 * @brief      Initializes the ESP8266 Wi-Fi module.
 *
 * @details    Initialize the ESP8266 Wi-Fi module and test the
 *             hardware connectivity. Try to connect to an access
 *             point and establish a TCP connection.
 */
void InitWiFiConnection()
{
	ADI_WIFI_RESULT eResult;

	/* Initialize the ESP8266 Wi-Fi module */
	eResult = adi_wifi_Init(adi_wifi_AplicationCallback, NULL);
	DEBUG_RESULT("Error initializing the ESP8266 module.\r\n",
	             eResult,
	             ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_TestAT();
	DEBUG_RESULT("Error testing hardware connectivity.\r\n",
	             eResult,
	             ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_SetWiFiMode(1);
	DEBUG_RESULT("Error establishing a tcp connection.\r\n",
	             eResult,
	             ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_DisconnectFromAP();
	DEBUG_RESULT("Error disconnecting from access point.\r\n",
	             eResult,
	             ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_ConnectToAP(aWifiSSID, aWifiPassword,  NULL);
	DEBUG_RESULT("Error connecting to access point.\r\n",
	             eResult,
	             ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_EstablishTCPConnection(&gTCPConnect);
	DEBUG_RESULT("Error establishing a tcp connection.\r\n",
	             eResult,
	             ADI_WIFI_SUCCESS);
}

/*!
 * @brief      Initializes the system
 *
 * @details    This function is responsible for initializing the pinmuxing,
 * 			   power service and wifi subsystem. It also initializes the
 * 			   realtime clock for to timestamp the outgoing sensor data
 * 			   packets.
 */
static void InitSystem(void)
{
	ADI_PWR_RESULT  ePwr;

	/* Explicitly disable the watchdog timer */
	*pREG_WDT0_CTL = 0x0u;

	/* Pinmux */
	adi_initpinmux();

	/* Initialize clocks */
	ePwr = adi_pwr_Init();
	DEBUG_RESULT("Error initializing the power service.\r\n",
	             ePwr,
	             ADI_PWR_SUCCESS);

	ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u);
	DEBUG_RESULT("Error configuring the core clock.\r\n",
	             ePwr,
	             ADI_PWR_SUCCESS);

	ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u);
	DEBUG_RESULT("Error configuring the peripheral clock.\r\n",
	             ePwr,
	             ADI_PWR_SUCCESS);

	/* Init timestamping */
	INIT_TIME();

	timer_start();

	DEBUG_MESSAGE("Starting MQTT publisher Wi-Fi CN0398 demo application\r\n");
}

/*!
 * @brief      Troubleshoot and attempt to fix connection
 *
 * @details    This function is called if the ESP8266 has failed to communicate
 * 			   with the AP or broker. It will attempt to diagnose and repair
 * 			   the connectivity problem.
 */
static void RepairConnection()
{
	uint8_t nConnectionStatus;

	if(adi_wifi_radio_GetConnectionStatus(&nConnectionStatus) ==
	    ADI_WIFI_SUCCESS) {
		switch(nConnectionStatus) {
		/*The  Station is connected to an AP and its IP is obtained,
		 * so try to connect to broker.
		 * */
		case 2:
			/* Send MQTT "CONNECT" message to broker */
			if(adi_wifi_radio_MQTTConnect(&gMQTTConnect) != ADI_WIFI_SUCCESS)
				DEBUG_MESSAGE("Failed to do MQTT Connect\n");
			break;
		/* The  Station has created a TCP or UDP transmission
		 * so you should try to publish again.
		 * */
		case 3:
			break;
		/* The TCP or UDP transmission of Station is disconnected.
		 * So try to establish a TCP
		 * connection and then connect to the broker.
		 */
		case 4:
			if(adi_wifi_radio_EstablishTCPConnection(&gTCPConnect) ==
			    ADI_WIFI_SUCCESS) {
				if(adi_wifi_radio_MQTTConnect(&gMQTTConnect) !=
				    ADI_WIFI_SUCCESS)
					DEBUG_MESSAGE("Failed to do MQTT Connect\n");
			} else {
				DEBUG_MESSAGE("Failed to establish connection to broker\n");
			}
			break;
		/*The  Station does NOT connect to an AP.
		 * So try to connect to the AP.
		 * */
		case 5:
			if(adi_wifi_radio_ConnectToAP(aWifiSSID, aWifiPassword,  NULL) ==
			    ADI_WIFI_SUCCESS) {
				if(adi_wifi_radio_EstablishTCPConnection(&gTCPConnect) ==
				    ADI_WIFI_SUCCESS) {
					if(adi_wifi_radio_MQTTConnect(&gMQTTConnect) !=
					    ADI_WIFI_SUCCESS)
						DEBUG_MESSAGE("Failed to do MQTT Connect\n");
				} else {
					DEBUG_MESSAGE("Failed to establish connection to broker\n");
				}
			} else {
				DEBUG_MESSAGE("Failed to Connect to AP\n");
			}
			break;
		default:
			break;
		}
	}
}

/*!
 * @brief      Trap function
 *
 * @details    In case of catastrophic errors this function is called to block
 *             infinitely.
 */
static void Trap()
{
	while(1);
}

