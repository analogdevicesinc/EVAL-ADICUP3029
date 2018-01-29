/*****************************************************************************
 * ADuCM3029_IBMWatson.cpp
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "ADuCM3029_IBMWatson.h"



#include <common.h>
//#include <common/adi_timestamp.h>

#include <adi_timestamp.h>

/*Circuit note includes*/
#include "adi_cn0398.h"
#include "adi_cn0397.h"
#include "adi_cn0410.h"

using namespace adi_sensor_swpack;

uint8_t         m_spi_memory[ADI_SPI_MEMORY_SIZE];
uint8_t         gpioMemory[ADI_GPIO_MEMORY_SIZE];

/*Sensor objects*/
CN0398  * cn0398 = new CN0398();
SENSOR_RESULT   eSensorResult;

CN0397     *cn0397 = new CN0397();
VisibleLight   *pLight = cn0397;

CN0410 *cn0410 = new CN0410(pLight);

sensors_data_t  data;

uint8_t valid_bytes[300];
uint32_t number_bytes;

/* Static data */
static ADI_WIFI_TCP_CONNECT_CONFIG  gTCPConnect = {
													.nLinkID = ADI_WIFI_CONNECTION_ID,
													.pType = (uint8_t *) "TCP",
													.pIP   = aMQTTBrokerIp,
													.pPort =  aMQTTBrokerPort,
													.nTCPKeepAlive = ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE};

static ADI_WIFI_MQTT_CONNECT_CONFIG gMQTTConnect = {
													.nLinkID = ADI_WIFI_CONNECTION_ID,
													.nVersion = ADI_WIFI_MQTT_PUBLISHER_VERSION,
													.pName = aMQTTPublisherName,
													.nMQTTKeepAlive = ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE,
													.pUsername = aMQTTUsername,
													.pPassword = aMQTTPassword};

static ADI_WIFI_PUBLISH_CONFIG 		gPublishConfig = {
		 	 	 	 	 	 	 	 	 	 	 	 .pMQTTData = (uint8_t *) &data,
		 	 	 	 	 	 	 	 	 	 	 	 .nMQTTDataSize = sizeof(ADI_DATA_PACKET),
													 .pTopic = aMQTTTopicName,
													 .nLinkID = ADI_WIFI_CONNECTION_ID,
													 .nQos = ADI_WIFI_MQTT_PUBLISER_QOS};

static ADI_WIFI_SUBSCRIBE_CONFIG    gSubscribeConfig = {
													   .pTopic = aMQTTSubscribeTopic,
													   .nLinkID = ADI_WIFI_CONNECTION_ID,
													   .nQos = ADI_WIFI_MQTT_PUBLISER_QOS,
													   .nPacketId = 1};



/************************* Functions Definitions ******************************/

/* Pin muxing */
extern "C" int32_t adi_initpinmux(void);

/* Local Wi-Fi functions */
static void            InitWiFiConnection(void);
static void            adi_wifi_AplicationCallback(void * pCBParam, uint32_t nEvent, void * pArg);
static void            RepairConnection();

/*!
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
static void adi_wifi_AplicationCallback(void * pCBParam, uint32_t nEvent, void * pArg)
{
	switch (nEvent)
		{
			case CMD_NONE:
			{
				/* Read the publish Data. */
				adi_wifi_ParseSubscriberData();
				memset(valid_bytes, 0, number_bytes);
				adi_wifi_GetData(valid_bytes, &number_bytes);

				/*Check name to call command*/
				if(strstr((char *)&valid_bytes[4], (char *)"ADI_GreenHouse_cmd"))
				{
					Leds_Controll(strstr((char *)&valid_bytes[4], (char *)"{"));
				}
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

/*!
 * @brief      Initializes the system
 *
 * @details    This function is responsible for initializing the pinmuxing, power service
 *             and wifi subsystem. It also initializes the realtime clock for to timestamp
 *             the outgoing sensor data packets.
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
    DEBUG_RESULT("Error initializing the power service.\r\n", ePwr, ADI_PWR_SUCCESS);

    ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u);
    DEBUG_RESULT("Error configuring the core clock.\r\n", ePwr, ADI_PWR_SUCCESS);

    ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u);
    DEBUG_RESULT("Error configuring the peripheral clock.\r\n", ePwr, ADI_PWR_SUCCESS);

    /* Init timestamping */
    INIT_TIME();

    DEBUG_MESSAGE("Starting MQTT publisher Wi-Fi\r\n");

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
    DEBUG_RESULT("Error initializing the ESP8266 module.\r\n", eResult, ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_TestAT();
    DEBUG_RESULT("Error testing hardware connectivity.\r\n", eResult, ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_SetWiFiMode(1);
    DEBUG_RESULT("Error establishing a tcp connection.\r\n", eResult, ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_DisconnectFromAP();
    DEBUG_RESULT("Error disconnecting from access point.\r\n", eResult, ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_ConnectToAP(aWifiSSID, aWifiPassword,  NULL);
    DEBUG_RESULT("Error connecting to access point.\r\n", eResult, ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_EstablishTCPConnection(&gTCPConnect);
    DEBUG_RESULT("Error establishing a tcp connection.\r\n", eResult, ADI_WIFI_SUCCESS);

}

/**
   @brief Callback function called when a command is received from the cloud.

   @param str - pointer to a constant string to get the command value

   @return void

**/
int Leds_Controll(const char *str)
{
	JsonNode *_main = json_decode(str);

	JsonNode *red = json_find_member(_main, "red");
	JsonNode *blue = json_find_member(_main, "blue");
	JsonNode *green = json_find_member(_main, "green");

	red->number_ = atof(red->string_);
	blue->number_ = atof(blue->string_);
	green->number_ = atof(green->string_);

	/*Control algorith for CN0410 led bar*/
	cn0410->Controll((float)red->number_, (float)blue->number_, (float)green->number_);

	json_delete(_main);

	return 0;
}

/*!
 * @brief      ESP8266 MQTT publisher demo using ADXL362 accelerometer data.
 *
 * @details    Publishes data over Wi-Fi to the broker.
 */
static void MQTTPublish(void)
{
	ADI_WIFI_RESULT eResult;
    uint32_t        nTime = 0u;
    uint32_t        nPacketId = 0u;
    uint8_t         nConnectionFailFlag;

    /* Initialize the ESP8266 Wi-Fi module and establish a TCP connection */
    InitWiFiConnection();

    /* Send MQTT "CONNECT" message to broker */
    eResult = adi_wifi_radio_MQTTConnect(&gMQTTConnect);
    //DEBUG_RESULT("Error connecting to the MQTT broker.\r\n", eResult, ADI_WIFI_SUCCESS);

    eResult = adi_wifi_radio_MQTTSubscribe(&gSubscribeConfig);

    /* Start the ping timer to ping the MQTT broker before the keep alive expires */
//    if(adi_wifi_StartTimer(ADI_WIFI_MQTT_PING_TIMEOUT, 1u) == ADI_WIFI_SUCCESS)
//    {
    	DEBUG_MESSAGE("Starting to Publish\n");

		gPublishConfig.nPacketId = nPacketId;

		/*enable wdt*/
		adi_wdt_Enable	(	true, NULL	);

		while(1)
		{
			adi_wdt_Kick();
			nConnectionFailFlag = 0;

            /* If ping timer has expired */
            if (adi_wifi_IsTimerDone(1u) == 1u)
            {
        		/* Send MQTT "PINGREQ" message to broker to keep the connection alive. */
        		eResult = adi_wifi_radio_MQTTPing(ADI_WIFI_CONNECTION_ID);

        		/* Restart the ping timer */
        		if((adi_wifi_StopTimer(1u) == ADI_WIFI_SUCCESS) && (eResult == ADI_WIFI_SUCCESS))
            	{
        			if(adi_wifi_StartTimer(ADI_WIFI_MQTT_PING_TIMEOUT, 1u) != ADI_WIFI_SUCCESS)
                    {
        		    	DEBUG_MESSAGE("Failed to start ping timer\n");

                    }
            	}
            	else
            	{
	    	        DEBUG_MESSAGE("Failed to ping broker\n");
    		    	nConnectionFailFlag = 1;
    		    }
            }

           	/* If we did not fail to ping the broker, publish data */
            if(nConnectionFailFlag == 0u)
            {
            	DEBUG_MESSAGE("Publishing data..\n");

            	PrepareMqttPayload(&data);

            	char *payload = telemetry_serialize(&data);

            	gPublishConfig.pMQTTData = (uint8_t *)payload;
            	gPublishConfig.nMQTTDataSize = strlen(payload);


            	if(adi_wifi_radio_MQTTPublish(&gPublishConfig) == ADI_WIFI_FAILURE)
            	{
            		nConnectionFailFlag = 1u;
            	}
            	free(payload);
            	adi_wifi_DispatchEvents(6000);
            }

            /* If we failed to ping the broker or failed to publish data, check the connection */
			if(nConnectionFailFlag == 1u)
			{
				DEBUG_MESSAGE("Troubleshooting failed connection..\n");
				RepairConnection();
			}

			gPublishConfig.nPacketId++;
		}
//    }
}

/*!
 * @brief      Troubleshoot and attempt to fix connection
 *
 * @details    This function is called if the ESP8266 has failed to communicate with the AP or broker.
 *             It will attempt to diagnose and repair the connectivity problem.
 */
static void RepairConnection()
{
	uint8_t         nConnectionStatus;

	adi_wifi_radio_GetConnectionStatus(&nConnectionStatus);


	switch(nConnectionStatus)
	{
	/*The  Station is connected to an AP and its IP is obtained, so try to connect to broker. */
	case 2:
	{
		/* Send MQTT "CONNECT" message to broker */
		if(adi_wifi_radio_MQTTConnect(&gMQTTConnect) != ADI_WIFI_SUCCESS)
		{
			DEBUG_MESSAGE("Failed to do MQTT Connect\n");
		}
		break;
	}

	/* The  Station has created a TCP or UDP transmission so you should try to publish again. */
	case 3:
		break;

		/* The TCP or UDP transmission of Station is disconnected. So try to establish a TCP
		 * connection and then connect to the broker.
		 */
	case 4:
	{
		if(adi_wifi_radio_EstablishTCPConnection(&gTCPConnect) == ADI_WIFI_SUCCESS)
		{
			if(adi_wifi_radio_MQTTConnect(&gMQTTConnect) != ADI_WIFI_SUCCESS)
			{
				DEBUG_MESSAGE("Failed to do MQTT Connect\n");
			}
		}
		else
		{
			DEBUG_MESSAGE("Failed to establish connection to broker\n");
		}
		break;
	}
	/*The  Station does NOT connect to an AP. So try to connect to the AP. */
	case 5:
	{
		if(adi_wifi_radio_ConnectToAP(aWifiSSID, aWifiPassword,  NULL) == ADI_WIFI_SUCCESS)
		{
			if(adi_wifi_radio_EstablishTCPConnection(&gTCPConnect) == ADI_WIFI_SUCCESS)
			{
				if(adi_wifi_radio_MQTTConnect(&gMQTTConnect) != ADI_WIFI_SUCCESS)
				{
					DEBUG_MESSAGE("Failed to do MQTT Connect\n");
				}
			}
			else
			{
				DEBUG_MESSAGE("Failed to establish connection to broker\n");
			}

		}
		else
		{
			DEBUG_MESSAGE("Failed to Connect to AP\n");
		}
		break;
	}
	default:
	{
		break;
	}
	}

}

int main(int argc, char *argv[])
{
	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();

	timer_start();

	adi_initpinmux(); // Init port configuration for UART, SPI and I2C
	adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE); //initialize gpio

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
	cn0410->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_A, 100);
	cn0410->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_B, 0);
	cn0410->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_C, 100);


	MQTTPublish(); //publish data to MQTT broker

	return 0;
}

