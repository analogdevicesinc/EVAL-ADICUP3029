/*!
 *****************************************************************************
  @file    adi_wifi.h

  @brief   This header file contains the radio specific interface command opcodes,
           status codes and radio interface specific functions for Wi-Fi radio.
  -----------------------------------------------------------------------------

Copyright (c) 2017 Analog Devices, Inc.

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

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF CLAIMS OF INTELLECTUAL
PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/


/** 
 *  @addtogroup radio Radio Module
 *  @ingroup WiFi
 *  @{
 *
 */

#ifndef ADI_WIFI_H
#define ADI_WIFI_H

/*! \cond PRIVATE */
#include <stdint.h>
#include <string.h>
#include <adi_callback.h>
#include <rtos_map/adi_rtos_map.h>

#include <adi_wifi_transport.h>
#include <adi_wifi_logevent.h>

#include <MQTTPacket.h>
#include <MQTTConnect.h>

/*! Lock the radio as it is not thread safe. */
#define ADI_WIFI_LOCK(timeout)  

/*! Unlock the radio. */
#define ADI_WIFI_UNLOCK_RADIO()      

/*! Wait for the current command to complete. */
#define ADI_WIFI_WAIT_FOR_COMPLETION(timeout) adi_wifi_WaitForResponseWithTimeout(timeout, gWiFiDevice.eCurCmdOpCode);

/*! Start the radio command. */
#define ADI_WIFI_RADIO_CMD_START(x) {																	                                  \
										memset(pWiFiDevice->pDataPkt, 0u, sizeof(pWiFiDevice->pDataPkt)); \
										pWiFiDevice->eCurCmdOpCode = x;								      \
										pWiFiDevice->nResponseSize = 0;								      \
									 }

/*! Clean up the radio command. */
#define ADI_WIFI_RADIO_CMD_END()	{								 		\
										pWiFiDevice->eCurCmdOpCode = 0; 	\
									}

/*! Max time (ms) a command will wait for a response or event before a timeout occurs. */
#define ADI_WIFI_RADIO_CMD_TIMEOUT              (10000u)

/*! Delay for the AT RST command to complete. */
#define ADI_WIFI_RADIO_RST_DELAY                (0xFFFFFFu)

/*! Delay for the Wi-Fi module to start up on a power up. */
#define ADI_WIFI_RADIO_BOOT_DELAY               (0xFFFFFu)

/*! Number of PUBREL retries. */
#define ADI_WIFI_RADIO_PUBREL_RETRIES           (0x5u)

/*! Maximum mode type for the AT command CWMODE. */
#define ADI_WIFI_RADIO_MAX_MODE                 (3u)

/*! Maximum number of socket connections the Wi-Fi module can have open at one time.  */
#define ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS      (5u)

/*! Maximum TCP keepalive time in seconds. */
#define ADI_WIFI_RADIO_MAX_KEEPALIVE            (7200u)

/*! Expected response for the CIPSTATUS AT command. */
#define ADI_WIFI_RADIO_CIPSTATUS_RESP           (0x3a)

/*! Maximum number of bytes for an SSID. */
#define ADI_WIFI_MAX_SSID                       (32u)

/*! Maximum number of bytes for an AP's password. */
#define ADI_WIFI_MAX_PASSWORD                   (64u)

/*! Maximum number of bytes for an AP's MAC address . */
#define ADI_WIFI_MAX_BSSID                      (48u)

/*! Maximum number of bytes in an ip address. For example "192.10.1.1" is 10 bytes. */
#define ADI_WIFI_MAX_IP_ADDR_SIZE               (39u)

/*! Maximum number of bytes in a port number. For example "7882" is 4 bytes. */
#define ADI_WIFI_MAX_PORT_SIZE                  (5u)

/*! AT command string. */
#define ADI_WIFI_AT_CMD			                 "AT\r\n"

/*! Reset command string. */ 
#define ADI_WIFI_RST_CMD		                 "AT+RST\r\n"

/*! Getversion info command string. */
#define ADI_WIFI_GMR_CMD		                 "AT+GMR\r\n"

/*! Continuous wave mode command string. */
#define ADI_WIFI_CWMODE_CMD		                 "AT+CWMODE"

/*! Join access point command string. */
#define ADI_WIFI_CWJAP_CMD                       "AT+CWJAP"

/*! Quit access point command string. */
#define ADI_WIFI_CWQAP_CMD                       "AT+CWQAP\r\n"

/*! Set up TCP or UDP connection command string. */
#define ADI_WIFI_CIPSTART_CMD                    "AT+CIPSTART="

/*! Send TCP/IP data command string. */
#define ADI_WIFI_CIPSEND_CMD                     "AT+CIPSEND="

/*! Multiple connection command string. */
#define ADI_WIFI_CIPMUX_CMD		                 "AT+CIPMUX"

/*! TCP/IP connection status command string. */
#define ADI_WIFI_CIPSTATUS_CMD	                 "AT+CIPSTATUS"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @enum ADI_WIFI_AT_CMDCODE
 * @brief Specifies command codes for the AT interace.
 */
typedef enum
{
	/* Overriding command interface with application specific callback events.           */
    CMD_NONE,                               /*!< No event.							     */
    CMD_HW_ERROR,                           /*!< HW error occured.				         */

    /* Basic AT commands */
    CMD_AT,                                 /*!< Startup/Test AT interface.            */
    CMD_AT_RST,                             /*!< Restart Module.                       */
    CMD_AT_GMR,                             /*!< Get Version Information.              */
    CMD_AT_GSLP,                            /*!< Disable Echo.                         */

    /* WiFi layer commands */
    CMD_AT_CWMODE,                          /*!< List valid modes.                     */
    CMD_AT_CWJAP,                           /*!< Connect to AP.                        */
    CMD_AT_CWLAP,                           /*!< List available APs.                   */
    CMD_AT_CWQAP,                           /*!< Disconnect from AP.                   */
    CMD_AT_CWSAP,                           /*!< Configuration of softAP mode.         */
    CMD_AT_CWLIF,                           /*!< List of clients connected to softAP.  */
    CMD_AT_CWDHCP,                          /*!< Enable Disable DHCP.                  */
    CMD_AT_CIPSTAMAC,                       /*!< Set static MAC station.               */
    CMD_AT_CIPAPMAC,                        /*!< Set static MAC softAP.                */
    CMD_AT_CIPSTA,                          /*!< Set IP address for station.           */
    CMD_AT_CIPAP,                           /*!< Set IP address fori softAP.           */

    /* TCP/IP layer  */
    CMD_AT_CIPSTATUS,                       /*!< Information about the connection.     */
    CMD_AT_CIPSTART,                        /*!< Start connection.                     */
  	CMD_AT_CIPSEND,                         /*!< Send Data. (data size)                */
  	CMD_AT_CIPSEND_CONN,                    /*!< Send Data. (connect)                  */
	CMD_AT_CIPSEND_DISCONN,                 /*!< Send Data. (disconnect)               */
	CMD_AT_CIPSEND_PUB,                     /*!< Send Data. (publisher)                */
	CMD_AT_CIPSEND_PUBREL,                  /*!< Send Data. (PUBREL)                   */
	CMD_AT_CIPSEND_SUB,                     /*!< Send Data. (subscriber)               */
	CMD_AT_CIPSEND_PINGREQ,                 /*!< Send Data. (PINGREQ)                  */
    CMD_AT_CIPCLOSE,                        /*!< Close TCP/UDP connection.             */
    CMD_AT_CIFSR,                           /*!< Get local ip address.                 */
    CMD_AT_CIPMUX,                          /*!< Enable/Disbale Multiple connections.  */
    CMD_AT_CIPSERVER,                       /*!< Configure as server.                  */
    CMD_AT_CIPMODE,                         /*!< Set transfer mode.                    */
    CMD_AT_CIPSTO,                          /*!< Set server timeout.                   */
    CMD_AT_CIUPDATE,                        /*!< Update through network.               */
    CMD_AT_IPD,                             /*!< Receive Network data.                 */

	CMD_AT_INVALID                          /*!< INVALID COMMAND NOT SUPPORTED.        */
} ADI_WIFI_AT_CMDCODE;


/**
 * @enum ADI_WIFI_RESULT
 * @brief All wifi companion module interface function return this code.
 */
typedef enum
{
    ADI_WIFI_SUCCESS,     /*!< WiFi module API succeeded. */
    ADI_WIFI_FAILURE,     /*!< WiFi module API failed.    */

} ADI_WIFI_RESULT;

/*!
 *  @struct ADI_WIFI_TCP_CONNECT_CONFIG
 *
 *  @brief  Data structure used in making a TCP connection.
 *
 */
typedef struct
{
	uint8_t     nLinkID;		    /*!< Used for multiple connections to identify the link.  				  */
	uint8_t    *pType;      	    /*!< Connection type. 									                  */
	uint8_t    *pIP;        	    /*!< Remote IP address.									                  */
	uint8_t    *pPort;			    /*!< Remote port number.								                  */
	uint16_t    nTCPKeepAlive;      /*!< Keep alive.										                  */
}ADI_WIFI_TCP_CONNECT_CONFIG;

/*!
 *  @struct ADI_WIFI_MQTT_CONNECT_CONFIG
 *
 *  @brief  Data structure used in making a MQTT client to broker connection.
 *
 */
typedef struct
{
	uint8_t 	nLinkID;	    /*!< Used for multiple connections to identify the link. 		    */
	uint8_t     nVersion;      	/*!< MQTT publisher version. 	 								    */
	uint8_t    *pName;          /*!< MQTT publisher name.									  	    */
	uint16_t    nMQTTKeepAlive; /*!< MQTT publisher <-> broker connection keep alive timeout.	    */
	uint8_t    *pUsername;      /*!< MQTT username.									  	    */
	uint8_t    *pPassword;      /*!< MQTT password.									  	    */
}ADI_WIFI_MQTT_CONNECT_CONFIG;

/*!
 *  @struct ADI_WIFI_PUBLISH_CONFIG
 *
 *  @brief  Data structure used to configure a packet for the client to publish.
 *
 */
typedef struct
{
	uint8_t 			   *pMQTTData;		  /*!< MQTT payload to publish.   						  		   */
	uint32_t 	 			nMQTTDataSize;    /*!< Size of MQTT payload to publish.   				  		   */
	uint8_t 			   *pTopic;       	  /*!< MQTT topic to publish to.							  	   */
	uint8_t 	 			nLinkID;	      /*!< Used for multiple connections to identify the link.  	   */
	uint8_t      			nQos;  			  /*!< Quality of service for the published packet.         	   */
	uint8_t      			nPacketId;        /*!< Id of the packet. This is used for multiple connections.    */
}ADI_WIFI_PUBLISH_CONFIG;

/*!
 *  @struct ADI_WIFI_SEND_DATA_CONFIG
 *
 *  @brief  Data structure used to send TCP data.
 *
 */
typedef struct
{
	uint8_t   	nLinkID;	   /*!< Used for multiple connections to identify the link.  */
	uint8_t    *pData;         /*!< Data to send.  								 	     */
	uint16_t    nDataLength;   /*!< Length of data.  									 */
}ADI_WIFI_SEND_DATA_CONFIG;

/*!
 *  @struct ADI_WIFI_SUBSCRIBE_CONFIG
 *
 *  @brief  Data structure used to configure a client to subscribe to a topic.
 *
 */
typedef struct
{
	uint8_t 		       *pTopic;     /*!< MQTT topic to subscribe for.					                */
	uint8_t 	 			nLinkID;    /*!< Used for multiple connections to identify the link.            */
	uint8_t      		    nQos;  	    /*!< Requested quality of service for subscription process.         */
	uint8_t      			nPacketId;  /*!< Id of the packet. This is used for multiple connections.       */
}ADI_WIFI_SUBSCRIBE_CONFIG;

/* command APIs */
ADI_WIFI_RESULT adi_wifi_radio_Init(ADI_CALLBACK pCallbackFunc);
ADI_WIFI_RESULT adi_wifi_radio_TestAT(void);
ADI_WIFI_RESULT adi_wifi_radio_GetAP(uint8_t * const pAPInfo, uint32_t nBufSize);
ADI_WIFI_RESULT adi_wifi_radio_GetConnectionStatus(uint8_t * const pStatus);
ADI_WIFI_RESULT adi_wifi_radio_Restart(void);
ADI_WIFI_RESULT adi_wifi_radio_GetVersionInfo(uint8_t * const pVersionInfoBuf, uint32_t nBufSize);
ADI_WIFI_RESULT adi_wifi_radio_SetWiFiMode(uint32_t nMode);
ADI_WIFI_RESULT adi_wifi_radio_ConnectToAP(const uint8_t * const pSSID, const uint8_t * const pPassword, const uint8_t * const pBSSID);
ADI_WIFI_RESULT adi_wifi_radio_DisconnectFromAP(void);
ADI_WIFI_RESULT adi_wifi_radio_EstablishTCPConnection(ADI_WIFI_TCP_CONNECT_CONFIG *eTCPConnect);
ADI_WIFI_RESULT adi_wifi_radio_SendData(ADI_WIFI_SEND_DATA_CONFIG  * const pSendDataConfig, ADI_WIFI_AT_CMDCODE eCipSendCmd);
ADI_WIFI_RESULT adi_wifi_radio_EnableMultipleConnections(uint8_t nEnable);
ADI_WIFI_RESULT adi_wifi_radio_MQTTConnect(ADI_WIFI_MQTT_CONNECT_CONFIG  * const pMQTTConnect);
ADI_WIFI_RESULT adi_wifi_radio_MQTTPublish(ADI_WIFI_PUBLISH_CONFIG * const ePublishConfig);
ADI_WIFI_RESULT adi_wifi_radio_MQTTPing(uint32_t nLinkID);
ADI_WIFI_RESULT adi_wifi_radio_MQTTDisconnect(uint32_t nLinkID);
ADI_WIFI_RESULT adi_wifi_radio_MQTTSubscribe(ADI_WIFI_SUBSCRIBE_CONFIG * const pSubscribeConfig);
ADI_WIFI_RESULT adi_wifi_ParseSubscriberData(void);
ADI_WIFI_RESULT adi_wifi_GetEvent(ADI_WIFI_AT_CMDCODE eCommandType);
void adi_wifi_GetData(uint8_t * const pPacket, uint32_t * nValidBytes);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ADI_WIFI_H */


/* @} */
