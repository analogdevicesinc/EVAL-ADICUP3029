/*!
 *****************************************************************************
   @file    adi_wifi.c

   @brief   Core interface layer with the Wi-Fi radio. Handles the communication
             with radio using the underlying transport layer. Handles the state
             and events and passes up the critical events to the Wi-Fi framework
             layer.
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
 *  @defgroup WiFi Wi-Fi System Software
 */

/** 
 *  @addtogroup radio Radio Module
 *  @ingroup WiFi
 *  @{
 *
 *  @brief Radio Module
 *  @details The set of functions in this module provide an interface for applications
 *           to interact with the radio. This module provides encoding and decoding of 
 *           of radio packets and calls into the Transport Abstraction Layer for 
 *           physical layer communication. 
 */

/*! \cond PRIVATE */
#include "adi_wifi_internal.h"
//#include <framework/noos/adi_wifi_noos.h>
#include <adi_wifi_noos.h>

/* Local memory */
static ADI_WIFI_DEVICE gWiFiDevice;
static ADI_WIFI_DEVICE *pWiFiDevice = &gWiFiDevice;
static ADI_MQTT_DEVICE gMQTTDevice;
static ADI_MQTT_DEVICE *pMQTTDevice = &gMQTTDevice;

/* Local functions */
static ADI_WIFI_RESULT adi_wifi_ProcessCmd(uint32_t nBufferLen);
static ADI_WIFI_RESULT adi_wifi_ValidateHeader(ADI_WIFI_AT_CMDCODE eCipSendCmd);
/*! \endcond */


/*********************************************************************************
                                   Init
*********************************************************************************/


/*!
 * @brief      Initialize Wi-Fi module.
 *
 * @details    Initialize the Wi-Fi transport layer driver and register the callback function.
 *
 * @param [in] pCallbackFunc : Pointer to the callback function to be passed to the transport layer.
 *
 * @return     ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
ADI_WIFI_RESULT adi_wifi_radio_Init(ADI_CALLBACK pCallbackFunc)
{
    /* This small delay gives the Wi-Fi module time to boot up */
    for(volatile uint32_t i = 0; i < ADI_WIFI_RADIO_BOOT_DELAY; i++);


	if(adi_wifi_tal_Init(pCallbackFunc) != ADI_WIFI_TAL_SUCCESS)
	{
		return ADI_WIFI_FAILURE;
	}

	return ADI_WIFI_SUCCESS;
}

/*********************************************************************************
                                 Query Commands
*********************************************************************************/

/*!
 * @brief       Get information about the connection.
 *
 * @details     The type of this command is "execute". It's used to get the connection status.
 *
 * @param [out] pStatus : One byte status of the Wi-Fi module.
 * 					      - 2: The Station is connected to an AP and its IP is obtained.
 * 					      - 3: The Station has created a TCP or UDP transmission.
 * 					      - 4: The TCP or UDP transmission of Station is disconnected.
 * 					      - 5: The Station does NOT connect to an AP.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
ADI_WIFI_RESULT adi_wifi_radio_GetConnectionStatus(uint8_t * const pStatus)
{
	ADI_WIFI_RESULT eWifiResult;
	uint16_t        nIndex = 0;

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT_CIPSTATUS);
	ADI_WIFI_RADIO_CMD_START(CMD_AT_CIPSTATUS);

	ASSERT(pStatus != NULL);

	/* Copy the AT+CIPSTATUS command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_CIPSTATUS_CMD);

	/* All commands end with the "\r\n" */
	strcat((char *)pWiFiDevice->pDataPkt, "\r\n");

	/* Send the AT command */
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the response */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);

		if(eWifiResult == ADI_WIFI_SUCCESS)
		{
			while(pWiFiDevice->pDataPkt[nIndex] != ADI_WIFI_RADIO_CIPSTATUS_RESP)
			{
				nIndex++;
				if(nIndex == ADI_WIFI_MAX_DATA_SIZE)
				{
					return(ADI_WIFI_FAILURE);
				}
			}
			/* Copy the status byte into the user supplied buffer */
			pStatus[0] = pWiFiDevice->pDataPkt[nIndex + 1] - '0';
		}
	}

	ADI_WIFI_RADIO_CMD_END();
	return(eWifiResult);
}

/*!
 * @brief      Get information on the access point the Wi-Fi module is connected to.
 *
 * @details    The type of this command is "query". It's used to  to query the AP to which the
 *             Station is already connected.
 *
 * @param [out] pAPInfo : Information about the access point in which the Wi-Fi module is connected.
 * 						  The format of this packet should resemble: <ssid>,<bssid>,<channel>,<rssi>.
 *
 * @param [in] nBufSize : Size of the buffer pAPInfo.
 *
 * @return     ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
ADI_WIFI_RESULT adi_wifi_radio_GetAP(uint8_t * const pAPInfo, uint32_t nBufSize)
{
	ADI_WIFI_RESULT eWifiResult;

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT_CWJAP);
	ADI_WIFI_RADIO_CMD_START(CMD_AT_CWJAP);

	ASSERT(pAPInfo != NULL);

	/* Copy the AT+CWJAP command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_CWJAP_CMD);

	/* Add the Query Parameter ('?') to the command. */
	strcat((char *)pWiFiDevice->pDataPkt, "?");

	/* All commands end with the "\r\n" */
	strcat((char *)pWiFiDevice->pDataPkt, "\r\n");

	/* Send the AT command */
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the response */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);

		if(eWifiResult == ADI_WIFI_SUCCESS)
		{
			if(nBufSize >= pWiFiDevice->nResponseSize)
			{
				/* If the response was successfully received and there is enough room in the buffer,
				 * copy the result into the provided buffer
				 */
				memcpy(&pAPInfo[0], &pWiFiDevice->pDataPkt[0], pWiFiDevice->nResponseSize);
			}
			else
			{
				/* If the response was successfully received and there is not enough room in the buffer,
				 * copy only the amount of data that will fit into the buffer.
				 */
				memcpy(&pAPInfo[0], &pWiFiDevice->pDataPkt[0], nBufSize);
			}
		}
	}

	ADI_WIFI_RADIO_CMD_END();
	return(eWifiResult);
}


/*********************************************************************************
                                    Basic AT Commands
*********************************************************************************/

/*!
 * @brief      Test AT command.
 *
 * @details    The type of this command is "executed". It's used to test the hardware setup of the Wi-Fi module.
 *
 * @return     ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
ADI_WIFI_RESULT adi_wifi_radio_TestAT()
{
	ADI_WIFI_RESULT eWifiResult;

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT);
	ADI_WIFI_RADIO_CMD_START(CMD_AT);

	/* Copy the AT command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_AT_CMD);

	/* Send the AT command */
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the response */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);
	}

	ADI_WIFI_RADIO_CMD_END();
	return(eWifiResult);
}

/*!
 * @brief      Restart Wi-Fi module.
 *
 * @details    The type of this command is "executed". It’s used to restart the module.
 *
 * @return     ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
ADI_WIFI_RESULT adi_wifi_radio_Restart()
{
	ADI_WIFI_RESULT eWifiResult;

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT_RST);
	ADI_WIFI_RADIO_CMD_START(CMD_AT_RST);

	/* Copy the AT command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_RST_CMD);

	/* Send the AT command */
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the response */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);

		if(eWifiResult == ADI_WIFI_SUCCESS)
		{
			/* Delay while the Wi-Fi module reboots, then clear the buffer. */
			for(volatile uint32_t i = 0u; i < ADI_WIFI_RADIO_RST_DELAY; i++)
			{
				;
			}
			if(adi_wifi_tal_FlushRxChannel() != ADI_WIFI_TAL_SUCCESS)
			{
				eWifiResult = ADI_WIFI_FAILURE;
			}
		}
	}

	ADI_WIFI_RADIO_CMD_END();
	return (eWifiResult);
}

/*!
 * @brief      	Get the AT command version and SDK version info.
 *
 * @details    	This AT command is used to check the version of AT commands and SDK, the type of which is "executed".
 *
 * @param [in] nBufSize : Size of the buffer pVersionInfoBuf.
 *
 * @param [out] pVersionInfoBuf : Pointer to the buffer where the version info will be written.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
ADI_WIFI_RESULT adi_wifi_radio_GetVersionInfo(uint8_t * const pVersionInfoBuf, uint32_t nBufSize)
{
	ADI_WIFI_RESULT eWifiResult;

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT_GMR);
	ADI_WIFI_RADIO_CMD_START(CMD_AT_GMR);

	ASSERT(pVersionInfoBuf != NULL);

	/* Copy the AT command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_GMR_CMD);

	/* Send the AT command */
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the response */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);

		if(eWifiResult == ADI_WIFI_SUCCESS)
		{
			if(nBufSize >= pWiFiDevice->nResponseSize)
			{
				/* If the response was successfully received and there is enough room in the buffer,
				 * copy the result into the provided buffer
				 */
				memcpy(&pVersionInfoBuf[0], &pWiFiDevice->pDataPkt[0], pWiFiDevice->nResponseSize);
			}
			else
			{
				/* If the response was successfully received and there is not enough room in the buffer,
				 * copy only the amount of data that will fit into the buffer.
				 */
				memcpy(&pVersionInfoBuf[0], &pWiFiDevice->pDataPkt[0], nBufSize);
			}
		}
	}

	ADI_WIFI_RADIO_CMD_END();
	return (eWifiResult);
}

/*********************************************************************************
                                    Wi-Fi AT Commands
*********************************************************************************/

/*!
 * @brief      	Set the Wi-Fi mode.
 *
 * @details    	The type of this command is "executed". This setting will be stored in the flash system parameter area. It won’t be erased
 *  			even when the power is off.
 *
 * @param [in]  nMode : Mode to set.
 * 							1: Station mode.
 * 							2: SoftAP mode (access point).
 * 							3: SoftAP + Station mode.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
ADI_WIFI_RESULT adi_wifi_radio_SetWiFiMode(uint32_t nMode)
{
	ADI_WIFI_RESULT eWifiResult;

	ASSERT(nMode <= ADI_WIFI_RADIO_MAX_MODE);

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT_CWMODE);
	ADI_WIFI_RADIO_CMD_START(CMD_AT_CWMODE);

	/* Copy the AT command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_CWMODE_CMD);

	/* Format the parameters */
	strcpy((char *)&pWiFiDevice->pDataPkt[strlen(ADI_WIFI_CWMODE_CMD)], "=x\r\n");
	pWiFiDevice->pDataPkt[strlen(ADI_WIFI_CWMODE_CMD) + 1u] = nMode + '0';

	/* Send the AT command */
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the response */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);
	}

	ADI_WIFI_RADIO_CMD_END();
	return (eWifiResult);
}

/*!
 * @brief      	Connect to the access point.
 *
 * @param [in] pSSID 		: Access point's SSID.
 * @param [in] pPassword 	: Access point's password.
 * @param [in] pBSSID 		: (optional) Access point's MAC address. Used when there are access points with the same SSID. 
 *                             Set as NULL to ignore. 
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 * @note : Escape character syntax is needed if SSID or password contains any special characters, such as , or ” or \.
 */
ADI_WIFI_RESULT adi_wifi_radio_ConnectToAP(const uint8_t * const pSSID, const uint8_t * const pPassword, const uint8_t * const pBSSID)
{
	ADI_WIFI_RESULT eWifiResult;

	ASSERT(pSSID != NULL);
	ASSERT(pPassword != NULL);
	ASSERT(strlen((char *)pSSID) <= ADI_WIFI_MAX_SSID);
	ASSERT(strlen((char *)pPassword) <= ADI_WIFI_MAX_PASSWORD);

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT_CWJAP);
	ADI_WIFI_RADIO_CMD_START(CMD_AT_CWJAP);

	/* Copy the AT command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_CWJAP_CMD);

	/* Send the SSID and password as ascii strings */
	strcpy((char *)&pWiFiDevice->pDataPkt[strlen(ADI_WIFI_CWJAP_CMD)], "=\"");
	strcpy((char *)&pWiFiDevice->pDataPkt[strlen((char *)pWiFiDevice->pDataPkt)], (char *)pSSID);
	strcpy((char *)&pWiFiDevice->pDataPkt[strlen((char *)pWiFiDevice->pDataPkt)], "\",\"");
	strcpy((char *)&pWiFiDevice->pDataPkt[strlen((char *)pWiFiDevice->pDataPkt)], (char *)pPassword);
	strcpy((char *)&pWiFiDevice->pDataPkt[strlen((char *)pWiFiDevice->pDataPkt)], "\"");

	if(pBSSID != NULL)
	{
		ASSERT(strlen((char *)pBSSID) <= ADI_WIFI_MAX_BSSID);

		/* Send the BSSID as a ascii string */
		strcpy((char *)&pWiFiDevice->pDataPkt[strlen((char *)pWiFiDevice->pDataPkt)], ",\"");
		strcpy((char *)&pWiFiDevice->pDataPkt[strlen((char *)pWiFiDevice->pDataPkt)], (char *)pBSSID);
	}

	/* All commands end with the "\r\n" */
	strcpy((char *)&pWiFiDevice->pDataPkt[strlen((char *)pWiFiDevice->pDataPkt)], "\r\n");

	/* Send the AT command */
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the response */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);
	}

	ADI_WIFI_RADIO_CMD_END();
	return (eWifiResult);
}

/*!
 * @brief      	Disconnect from the access point.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 */
ADI_WIFI_RESULT adi_wifi_radio_DisconnectFromAP()
{
	ADI_WIFI_RESULT eWifiResult;

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT_CWQAP);
	ADI_WIFI_RADIO_CMD_START(CMD_AT_CWQAP);

	/* Copy the AT command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_CWQAP_CMD);

	/* Send the AT command */
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the response */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);
	}

	ADI_WIFI_RADIO_CMD_END();
	return (eWifiResult);
}

/*********************************************************************************
                                    TCP/IP AT Commands
*********************************************************************************/

/*!
 * @brief     Establish TCP connection.
 *
 * @param [in] pTCPConnect 	: Configuration data to establish the connection.
 *
 * 						 	- nLinkID is used in the case of multiple connections. It is the ID of the network connection.
 * 						 	  Can be 0-(#ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS - 1). For a single connection system, set to be ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS.
 *						 	- pType is the type of connection. Should be "TCP" or "SSL".
 *						 	- pIP is the remote server IP address as a string.
 *						 	- pPort is the remote server port.
 *						 	- nTCPKeepAlive is the detection time interval when TCP is kept alive, this function is disabled by default.
 *								0 : Disable TCP keep-alive.
 *							    1 ~ 7200 : Detection time interval, unit: second.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
ADI_WIFI_RESULT adi_wifi_radio_EstablishTCPConnection(ADI_WIFI_TCP_CONNECT_CONFIG  * const pTCPConnect)
{
	ADI_WIFI_RESULT eWifiResult;
	uint8_t 		nCmdLen = strlen(ADI_WIFI_CIPSTART_CMD);

	ASSERT(pTCPConnect != NULL);
	ASSERT(pTCPConnect->pType != NULL);
	ASSERT(pTCPConnect->pIP != NULL);
	ASSERT(pTCPConnect->pPort != NULL);
	ASSERT(strlen((char*)pTCPConnect->pType) == strlen("TCP"));
	ASSERT(strlen((char*)pTCPConnect->pIP) <= ADI_WIFI_MAX_IP_ADDR_SIZE);
	ASSERT(strlen((char*)pTCPConnect->pPort) <= ADI_WIFI_MAX_PORT_SIZE);
	ASSERT(pTCPConnect->nTCPKeepAlive <= ADI_WIFI_RADIO_MAX_KEEPALIVE);

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT_CIPSTART);
	ADI_WIFI_RADIO_CMD_START(CMD_AT_CIPSTART);

	/* Copy the AT command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_CIPSTART_CMD);

	/* If multiple connections are supported, add the link ID */
	if(pTCPConnect->nLinkID < ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS)
	{
		itoa(pTCPConnect->nLinkID, (char *)&pWiFiDevice->pDataPkt[nCmdLen], 10);
		nCmdLen += 1u;
		strcpy((char *)&pWiFiDevice->pDataPkt[nCmdLen], ",");
		nCmdLen += 1u;
	}

	/* Format the connection type, remote ip, remote port and keep alive */
	strcpy((char *)&pWiFiDevice->pDataPkt[nCmdLen], "\"");
	nCmdLen += 1u;
	strcpy((char *)&pWiFiDevice->pDataPkt[nCmdLen], (char*) pTCPConnect->pType);
	nCmdLen += 3u;
	strcpy((char *)&pWiFiDevice->pDataPkt[nCmdLen], "\",\"");
	nCmdLen += 3u;
	strcpy((char *)&pWiFiDevice->pDataPkt[nCmdLen], (char*) pTCPConnect->pIP);
	nCmdLen += strlen((char*)pTCPConnect->pIP);
	strcpy((char *)&pWiFiDevice->pDataPkt[nCmdLen], "\",");
	nCmdLen += 2u;
	strcpy((char *)&pWiFiDevice->pDataPkt[nCmdLen], (char*) pTCPConnect->pPort);
	nCmdLen +=  strlen((char*)pTCPConnect->pPort);
	strcpy((char *)&pWiFiDevice->pDataPkt[nCmdLen], ",");
	nCmdLen += 1u;
	itoa(pTCPConnect->nTCPKeepAlive, (char *)&pWiFiDevice->pDataPkt[nCmdLen],10);
	nCmdLen = strlen((char *)pWiFiDevice->pDataPkt);

	/* All commands end with the "\r\n" */
	strcpy((char *)&pWiFiDevice->pDataPkt[nCmdLen], "\r\n");

	/* Send the AT command */
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the response */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);
	}

	ADI_WIFI_RADIO_CMD_END();
	return (eWifiResult);
}

/*!
 * @brief      	Send data to server.
 *
 * @details     This command is actually three separate transactions. The first portion sends the length of data that will follow
 *              in the second data packet. Each data packet needs to be sent 20ms apart. The third portion of this only applies to
 *              packets who have a quality of service level greater than 0. This means that a response is expected to confirm that
 *              the data is been received by the server.
 *
 * @param [in] pSendDataConfig: Configuration structure to send data.
 * *
 * 						 	- nLinkID is used in the case of multiple connections. It is the ID of the network connection. Can be 0-4. Set to #ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS for
 * 						 	  single connection mode.
 *						 	- nDataLength is data length. Maximum is #ADI_WIFI_MAX_DATA_SIZE bytes.
 *						 	- pData is a pointer to the data buffer to be sent.
 *
 * @param [in] eCipSendCmd: Type of data to send to the remote server.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 * @note: When sending \0, please send it as \\0.
 */
ADI_WIFI_RESULT adi_wifi_radio_SendData(ADI_WIFI_SEND_DATA_CONFIG  * const pSendDataConfig, ADI_WIFI_AT_CMDCODE eCipSendCmd)
{
	ADI_WIFI_RESULT eWifiResult;
	uint8_t 		nCmdLen = strlen(ADI_WIFI_CIPSEND_CMD);

	ASSERT(pSendDataConfig != NULL);
	ASSERT(pSendDataConfig->nDataLength <= ADI_WIFI_MAX_DATA_SIZE);
	ASSERT((eCipSendCmd == CMD_AT_CIPSEND_CONN)   ||
		   (eCipSendCmd == CMD_AT_CIPSEND_PUB)    ||
		   (eCipSendCmd == CMD_AT_CIPSEND_PUBREL) ||
		   (eCipSendCmd == CMD_AT_CIPSEND_SUB)    ||
		   (eCipSendCmd == CMD_AT_CIPSEND_PINGREQ));

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT_CIPSEND);
	ADI_WIFI_RADIO_CMD_START(CMD_AT_CIPSEND);

	/* Copy the AT command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_CIPSEND_CMD);

	/* If multiple connections are supported, add the link ID */
	if(pSendDataConfig->nLinkID < ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS)
	{
		itoa(pSendDataConfig->nLinkID,(char *) &pWiFiDevice->pDataPkt[nCmdLen], 10);
		nCmdLen += 1u;
		strcpy((char *)&pWiFiDevice->pDataPkt[nCmdLen], ",");
		nCmdLen += 1u;
	}

	/* Format length of data being sent */
	itoa(pSendDataConfig->nDataLength,(char *) &pWiFiDevice->pDataPkt[nCmdLen], 10);

	/* All commands end with the "\r\n" */
	strncat((char *)pWiFiDevice->pDataPkt, "\r\n", strlen("\r\n"));

	/* Part 1: Send the length of the data payload*/
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the wrap to indicate the packet was received */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);

		/* Once the wrap command "<" has been received, send the data packet */
		if(eWifiResult == ADI_WIFI_SUCCESS)
		{
			/* Start the new command */
			ADI_WIFI_RADIO_CMD_START(eCipSendCmd);

			/* Now copy in the data */
			memcpy(&pWiFiDevice->pDataPkt[0], &pSendDataConfig->pData[0], pSendDataConfig->nDataLength);

			/* Part 2: Send the data payload */
			eWifiResult = adi_wifi_ProcessCmd(pSendDataConfig->nDataLength);

			if(eWifiResult == ADI_WIFI_SUCCESS)
			{
				/* Wait for the response */
				eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT)

				if(eWifiResult == ADI_WIFI_SUCCESS)
				{
					/* Receive the "ACK" if this packet is not a publisher packet with a quality of service 0 */
					if(!((eCipSendCmd == CMD_AT_CIPSEND_PUB) && (pMQTTDevice->nQosLevel == 0u)))
					{
						/* Parse the response for the length of the data to be coming next */
						pWiFiDevice->nResponseSize = pWiFiDevice->pDataPkt[strlen((char*)pWiFiDevice->pDataPkt) - 2u] - '0';

						/* Validate that response will not overflow buffer */
						if(pWiFiDevice->nResponseSize < ADI_WIFI_MAX_DATA_SIZE)
						{
							/* Part 3: Read and parse the response */
							return(adi_wifi_ValidateHeader(eCipSendCmd));

						}
					}


				}
			}
		}
	}

	ADI_WIFI_RADIO_CMD_END();
	return (eWifiResult);
}

/*!
 * @brief      	Enable/disable multiple connections.
 *
 *
 * @param [in]  nEnable : Set the state of multiple connections.
 * 						  1 : Enable.
 * 						  0 : Disable.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
ADI_WIFI_RESULT adi_wifi_radio_EnableMultipleConnections(uint8_t nEnable)
{
	ADI_WIFI_RESULT eWifiResult;

	ASSERT(nEnable <= 1u);

	ADI_WIFI_LOGEVENT(LOGID_CMD_AT_CIPMUX);
	ADI_WIFI_RADIO_CMD_START(CMD_AT_CIPMUX);

	/* Copy the AT command into the buffer */
	strcpy((char *)&pWiFiDevice->pDataPkt[0], ADI_WIFI_CIPMUX_CMD);

	/* Format the parameters */
	strcpy((char *)&pWiFiDevice->pDataPkt[strlen(ADI_WIFI_CIPMUX_CMD)], "=x\r\n");
	pWiFiDevice->pDataPkt[strlen(ADI_WIFI_CWMODE_CMD) + 1u] = nEnable + '0';

	/* Send the command */
	eWifiResult = adi_wifi_ProcessCmd(strlen((char *)pWiFiDevice->pDataPkt));

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		/* Wait for the response */
		eWifiResult = ADI_WIFI_WAIT_FOR_COMPLETION(ADI_WIFI_RADIO_CMD_TIMEOUT);
	}

	ADI_WIFI_RADIO_CMD_END();
	return (eWifiResult);
}

/*********************************************************************************
                                    MQTT Commands
*********************************************************************************/
/*!
 * @brief      	Connect to a MQTT broker.
 *
 * @param [in] pMQTTConnect: Structure used to configure the MQTT connection.
 * 						 	- nLinkID is used in the case of multiple connections. It is the ID of the network connection.
 * 						 	  Can be 0-4. Set to #ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS for single connection mode.
 *						 	- nVersion is the MQTT publisher version.
 *						 	- pName is a pointer to the data buffer containing the publisher name.
 *						 	- nMQTTKeepAlive is the MQTT publisher <-> broker connection keep alive timeout.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 */

ADI_WIFI_RESULT adi_wifi_radio_MQTTConnect(ADI_WIFI_MQTT_CONNECT_CONFIG  * const pMQTTConnect)
{
	ADI_WIFI_SEND_DATA_CONFIG   eSendDataConfig;
	MQTTPacket_connectData      data = MQTTPacket_connectData_initializer;

	ASSERT(pMQTTConnect != NULL);

	memset(&pMQTTDevice->pMQTTRequestPkt , 0u, ADI_WIFI_MQTT_PACKET_SIZE);

	data.MQTTVersion = pMQTTConnect->nVersion;
	data.clientID.cstring = (char *)  pMQTTConnect->pName;
	data.keepAliveInterval = pMQTTConnect->nMQTTKeepAlive;
	data.cleansession = 1u;
	data.username.cstring = (char *) pMQTTConnect->pUsername;
	data.password.cstring = (char *) pMQTTConnect->pPassword;

	/* Create a MQTT CONNECT message. */
	eSendDataConfig.nDataLength = MQTTSerialize_connect((unsigned char *)&pMQTTDevice->pMQTTRequestPkt, ADI_WIFI_MQTT_PACKET_SIZE, &data);

	eSendDataConfig.nLinkID = pMQTTConnect->nLinkID;
	eSendDataConfig.pData = pMQTTDevice->pMQTTRequestPkt;

	/* Use the Wi-Fi module to send the MQTT connect packet */
	return(adi_wifi_radio_SendData(&eSendDataConfig, CMD_AT_CIPSEND_CONN));
}

/*!
 * @brief      	Publish MQTT data.
 *
 * @param [in] ePublishConfig: Configuration structure to publish data.
 *
 * 						 	- nLinkID is used in the case of multiple connections. It is the ID of the network connection. Can be 0-4. Set to #ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS for
 * 						 	  single connection mode.
 *						 	- nQos is the type of quality of service for the packet. Valid options are 0, 1 and 2.
 *						 	- pMQTTData is a pointer to the MQTT data to publish.
 *						 	- nMQTTDataSize is size of the MQTT data being published.
 *						 	- pTopic is a pointer to the topic to publish to. Must be smaller than #ADI_WIFI_MQTT_PACKET_SIZE -sizeof(ADI_DATA_PACKET) - 2u
 *						 	- nPacketId is the packet id for this packet.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 * @note: When sending \0, please send it as \\0.
 */
ADI_WIFI_RESULT adi_wifi_radio_MQTTPublish(ADI_WIFI_PUBLISH_CONFIG * const pPublishConfig)
{
	ADI_WIFI_RESULT             eWifiResult;
	ADI_WIFI_SEND_DATA_CONFIG   eSendDataConfig;

	ASSERT(pPublishConfig != NULL);

	MQTTString                 topicString = MQTTString_initializer;
	topicString.cstring = (char *) pPublishConfig->pTopic;

	memset(&pMQTTDevice->pMQTTRequestPkt , 0u, ADI_WIFI_MQTT_PACKET_SIZE);

	/* Create MQTT "PUBLISH" message */
	eSendDataConfig.nDataLength = MQTTSerialize_publish((unsigned char *)&pMQTTDevice->pMQTTRequestPkt, ADI_WIFI_MQTT_PACKET_SIZE, 0u, pPublishConfig->nQos, 0u, pPublishConfig->nPacketId, topicString, (unsigned char *)pPublishConfig->pMQTTData, pPublishConfig->nMQTTDataSize);

	eSendDataConfig.nLinkID = pPublishConfig->nLinkID;
	eSendDataConfig.pData = pMQTTDevice->pMQTTRequestPkt;
	pMQTTDevice->nQosLevel = pPublishConfig->nQos;

	/* Use Wi-Fi module to send the MQTT "PUBLISH" packet */
	eWifiResult = adi_wifi_radio_SendData(&eSendDataConfig, CMD_AT_CIPSEND_PUB);

	if((pPublishConfig->nQos == 2u) && (eWifiResult == ADI_WIFI_SUCCESS))
	{
		/* Create MQTT "PUBREL" message */
		eSendDataConfig.nDataLength = MQTTSerialize_pubrel((unsigned char *)&pMQTTDevice->pMQTTRequestPkt, ADI_WIFI_MQTT_PACKET_SIZE, 0, pPublishConfig->nPacketId);
		uint8_t nCounter = 0;

		/* Use Wi-Fi module to send the MQTT "PUBREL" packet a maximum of 5 times */
		do
		{  
			eWifiResult = adi_wifi_radio_SendData(&eSendDataConfig, CMD_AT_CIPSEND_PUBREL);
		    
		    if(nCounter == ADI_WIFI_RADIO_PUBREL_RETRIES)
		    {
			  eWifiResult = ADI_WIFI_FAILURE;
			  break;
		    }

		    nCounter++;

		}while(eWifiResult != ADI_WIFI_SUCCESS);
	}

	return(eWifiResult);
}

/*!
 * @brief      	Disconnect from the MQTT broker.
 *
 * @param [in] nLinkID: It is the ID of the network connection in the case of multiple connections. Can be 0-4. 
 *                      Set to #ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS for single connection mode.
 * 						 	  
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 */
ADI_WIFI_RESULT adi_wifi_radio_MQTTDisconnect(uint32_t nLinkID)
{
	ADI_WIFI_RESULT             eWifiResult;
	ADI_WIFI_SEND_DATA_CONFIG   eSendDataConfig;

	memset(&pMQTTDevice->pMQTTRequestPkt , 0u, ADI_WIFI_MQTT_PACKET_SIZE);

	/* Create MQTT "DISCONNECT" message */
	eSendDataConfig.nDataLength = MQTTSerialize_disconnect((unsigned char *)&pMQTTDevice->pMQTTRequestPkt, ADI_WIFI_MQTT_PACKET_SIZE);

	eSendDataConfig.nLinkID = nLinkID;
	eSendDataConfig.pData = pMQTTDevice->pMQTTRequestPkt;

	/* Use Wi-Fi module to send the MQTT "DISCONNECT" packet */
	eWifiResult = adi_wifi_radio_SendData(&eSendDataConfig, CMD_AT_CIPSEND_DISCONN);
	RETURN_ERROR(eWifiResult, ADI_WIFI_SUCCESS, ADI_WIFI_FAILURE);

	return(ADI_WIFI_SUCCESS);
}

/*!
 * @brief      	Ping the MQTT broker.
 *
 * @details     This command can be used to ping the MQTT broker to keep the
 *              connection alive.
 *
 * @param [in] nLinkID: It is the ID of the network connection in the case of multiple connections. Can be 0-4. 
 *                      Set to #ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS for single connection mode.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 */
ADI_WIFI_RESULT adi_wifi_radio_MQTTPing(uint32_t nLinkID)
{
	ADI_WIFI_RESULT             eWifiResult;
	ADI_WIFI_SEND_DATA_CONFIG   eSendDataConfig;

	memset(&pMQTTDevice->pMQTTRequestPkt , 0u, ADI_WIFI_MQTT_PACKET_SIZE);

	/* Create MQTT "PINGREQ" message */
	eSendDataConfig.nDataLength = MQTTSerialize_pingreq((unsigned char *)&pMQTTDevice->pMQTTRequestPkt, ADI_WIFI_MQTT_PACKET_SIZE);

	eSendDataConfig.nLinkID = nLinkID; 
	eSendDataConfig.pData = pMQTTDevice->pMQTTRequestPkt;

	/* Use Wi-Fi module to send the MQTT "PINGREQ" packet */
	eWifiResult = adi_wifi_radio_SendData(&eSendDataConfig, CMD_AT_CIPSEND_PINGREQ);
	RETURN_ERROR(eWifiResult, ADI_WIFI_SUCCESS, ADI_WIFI_FAILURE);

	return(ADI_WIFI_SUCCESS);
}

/*!
 * @brief      	Subscribe to a topic.
 *
 * @param [in] pSubscribeConfig: Configuration structure to subscribe to a topic.
 *
 * 						 	- nLinkID is used in the case of multiple connections. It is the ID of the network connection. Can be 0-4. Set to #ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS for
 * 						 	  single connection mode.
 *						 	- nQos is the type of quality of service for the packet. Valid options are 0, 1 and 2.
 *						 	- pTopic is a pointer to the topic to subscribe to. Must be smaller than #ADI_WIFI_MQTT_PACKET_SIZE -sizeof(ADI_DATA_PACKET) - 2u
 *						 	- nPacketId is the packet id for this packet.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 * @note: When sending \0, please send it as \\0.
 */
ADI_WIFI_RESULT adi_wifi_radio_MQTTSubscribe(ADI_WIFI_SUBSCRIBE_CONFIG * const pSubscribeConfig)
{
	ADI_WIFI_RESULT             eWifiResult;
	ADI_WIFI_SEND_DATA_CONFIG   eSendDataConfig;

	ASSERT(pSubscribeConfig != NULL);

	MQTTString topicString = MQTTString_initializer;
	topicString.cstring = (char *) pSubscribeConfig->pTopic;

	memset(&pMQTTDevice->pMQTTRequestPkt , 0u, ADI_WIFI_MQTT_PACKET_SIZE);

	/* Create a MQTT "SUBSCRIBE" message. */
	eSendDataConfig.nDataLength = MQTTSerialize_subscribe((unsigned char *)&pMQTTDevice->pMQTTRequestPkt , ADI_WIFI_MQTT_PACKET_SIZE, 0u, pSubscribeConfig->nPacketId, 1u , &topicString, (int *)&pSubscribeConfig->nQos);

	eSendDataConfig.nLinkID = pSubscribeConfig->nLinkID;;
	eSendDataConfig.pData = pMQTTDevice->pMQTTRequestPkt;

	/* Use the Wi-Fi module to send the MQTT "SUBSCRIBE" packet */
	eWifiResult = adi_wifi_radio_SendData(&eSendDataConfig, CMD_AT_CIPSEND_SUB);
	RETURN_ERROR(eWifiResult, ADI_WIFI_SUCCESS, ADI_WIFI_FAILURE);

	return(ADI_WIFI_SUCCESS);
}

/*********************************************************************************
                            Command Logic
*********************************************************************************/

/*!
 *  @brief      Application event interface.
 *
 * @param [in]  eCommandType : Type of command that is being processed.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
ADI_WIFI_RESULT adi_wifi_GetEvent(ADI_WIFI_AT_CMDCODE eCommandType)
{
	ADI_WIFI_RESULT eWifiResult = ADI_WIFI_FAILURE;
    char 		   *pExpectedReply;
    char 		   *pErrorResponse;
    uint32_t	    nIndex = 0;

    /* Set the expected response for this particular command */
    switch(eCommandType){
		case CMD_NONE:
		{
			/* Clear the data buffer for the incoming subscriber data. */
			memset(&pWiFiDevice->pDataPkt[0], 0u, ADI_WIFI_MAX_DATA_SIZE);
			pExpectedReply = ",";
			pErrorResponse = "ERROR";
			break;
		}
		case CMD_AT_CIPSEND_SUB:
		case CMD_AT_CIPSEND_PINGREQ:
		case CMD_AT_CIPSEND_CONN:
		{
			pExpectedReply = ":";
			pErrorResponse = "ERROR";
			break;
		}
		case CMD_AT_CIPSEND_PUBREL:
		case CMD_AT_CIPSEND_PUB:
		{
			if(pMQTTDevice->nQosLevel == 1u)
			{
				pExpectedReply = ":";
			}
			else if(pMQTTDevice->nQosLevel == 2u)
			{
				pExpectedReply = ":";
			}
			else
			{
				pExpectedReply = "OK";
			}
			pErrorResponse = "ERROR";
			break;
		}
		case CMD_AT_CIPSEND:
    	{
    		pExpectedReply = ">";
    		pErrorResponse = "ERROR";
    		break;
    	}
    	case CMD_AT_CWJAP:
    	{
    		pExpectedReply = "OK";
    		pErrorResponse = "FAIL";
    		break;
    	}
    	default:
    	{
    		pExpectedReply = "OK";
    		pErrorResponse = "ERROR";
    		break;
    	}
    }

    /* Make sure the buffer does not overflow */
    while(nIndex < ADI_WIFI_MAX_DATA_SIZE)
    {
    	/* Read a single byte into the buffer */
    	if(adi_wifi_tal_Read(&pWiFiDevice->pDataPkt[nIndex], 1u) == ADI_WIFI_TAL_SUCCESS)
    	{
        	/* Check to see if the command has finished */
    		if(strstr((char *)pWiFiDevice->pDataPkt, pExpectedReply)){
    			/* Record the number of valid bytes in the pDataPkt */
    			pWiFiDevice->nResponseSize = ++nIndex;
    			eWifiResult = ADI_WIFI_SUCCESS;
    			break;
    		}

        	/* Check to see if the command has failed */
    		if(strstr((char *)pWiFiDevice->pDataPkt, pErrorResponse)){
    			break;
    		}

    		nIndex++;
    	}
    	else
    	{
    		break;
    	}
    }
	return (eWifiResult);
}

/*! @cond PRIVATE */

/*!
 *  @brief      Send data packet.
 *
 * @param [in]  nBufferLen : Number of valid bytes in pDataPkt.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned upon success.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
static ADI_WIFI_RESULT adi_wifi_ProcessCmd(uint32_t nBufferLen)
{
	if(adi_wifi_tal_Write((uint8_t *)&pWiFiDevice->pDataPkt[0], nBufferLen) != ADI_WIFI_TAL_SUCCESS)
	{
		return(ADI_WIFI_FAILURE);
	}

	return (ADI_WIFI_SUCCESS);
}

/*!
 *  @brief      Parse the response packet header and validate if the response was as expected.
 *
 * @param [in]  eCommandType : Type of command that was sent. This is used to identify the expected
 *                             response.
 *
 * @return      ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned if the expected response was received.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 */
static ADI_WIFI_RESULT adi_wifi_ValidateHeader(ADI_WIFI_AT_CMDCODE eCipSendCmd)
{
	ADI_WIFI_RESULT eWifiResult = ADI_WIFI_FAILURE;
	uint8_t         nExpectedResponse = 0;
	MQTTHeader header = {0};

	/* Read the remaining bytes in the +IPD response */
	if(adi_wifi_tal_Read((unsigned char*)pWiFiDevice->pDataPkt, pWiFiDevice->nResponseSize) == ADI_WIFI_TAL_SUCCESS)
	{
		switch(eCipSendCmd)
		{
			case CMD_AT_CIPSEND_CONN:
			{
				nExpectedResponse = CONNACK;
				break;
			}
			case CMD_AT_CIPSEND_PUB:
			{
				if(pMQTTDevice->nQosLevel == 1u)
				{
					nExpectedResponse = PUBACK;
				}
				else if(pMQTTDevice->nQosLevel == 2u)
				{
					nExpectedResponse = PUBREC;

				}
				break;
			}
			case CMD_AT_CIPSEND_PUBREL:
			{
				nExpectedResponse = PUBCOMP;
				break;
			}
			case CMD_AT_CIPSEND_SUB:
			{
				nExpectedResponse = SUBACK;
				break;
			}
			case CMD_AT_IPD:
			{
				nExpectedResponse = PUBLISH;
				break;
			}
			case CMD_AT_CIPSEND_PINGREQ:
			{
				nExpectedResponse = PINGRESP;
				break;
			}
			default:
			{
				break;
			}
		}

		/*Parse the response message and check if the header has the expected message type. */
		header.byte = pWiFiDevice->pDataPkt[0];

		if(header.bits.type == nExpectedResponse){
			eWifiResult = ADI_WIFI_SUCCESS;
		}
	}

	return(eWifiResult);
}

/*! @endcond */

/*********************************************************************************
                            EVENT DATA HANDLING
*********************************************************************************/

/*!
 * @brief      Parse the +IPD messages received when the Wi-Fi module is acting as an MQTT subscriber.
 *
 * @return     ADI_WIFI_RESULT
 *                       - #ADI_WIFI_SUCCESS is returned if the expected response was received.
 *                       - #ADI_WIFI_FAILURE is returned upon failure.
 *
 * @note	   The data obtained during this command is only valid until another AT command is called.
 */
ADI_WIFI_RESULT adi_wifi_ParseSubscriberData()
{
	ADI_WIFI_RESULT eWifiResult = ADI_WIFI_FAILURE;
	uint8_t        nNumber[ADI_WIFI_MAX_DATA_SIZE_ASCII];
	uint32_t       nIndex = 0;


	/* Read n in the +IPD,n:. n is an unknown number of digits.
	 * Ex: +IPD,23: (actual data) or Ex: +IPD,644: (actual data)
	 * */
	while(nIndex < ADI_WIFI_MAX_DATA_SIZE_ASCII)
	{
		/* Read a single byte into the buffer */
		if(adi_wifi_tal_Read(&nNumber[nIndex], 1u) == ADI_WIFI_TAL_SUCCESS)
		{
			/* Check to see if the command has finished */
			if(strstr((char *)nNumber, ":"))
			{
				/* Save the number of valid bytes in the pDataPkt */
				pWiFiDevice->nResponseSize = atoi((char*)nNumber);
				eWifiResult = ADI_WIFI_SUCCESS;
				break;
			}
			nIndex++;
		}
		else
		{
			break;
		}
	}

	/*  Read the rest of the data and parse the MQTT packet header to validate the response was as expected */
	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		eWifiResult = adi_wifi_ValidateHeader(CMD_AT_IPD);
	}

	return eWifiResult;
}

/*!
 *  @brief      Get data packet.
 *
 * @param [out]  pPacket : Data received by the Wi-Fi module. Must be #ADI_WIFI_MAX_DATA_SIZE bytes.
 *
 * @param [out]  nValidBytes : Number of valid bytes in pPacket.
 */
void adi_wifi_GetData(uint8_t * const pPacket, uint32_t * nValidBytes)
{
	ASSERT(pPacket != NULL);
	ASSERT(nValidBytes != NULL);

	/*Modification to the wifi driver @Mircea*/
	/* Parse the response for the length of the data to be coming next */
	//pWiFiDevice->nResponseSize = strlen((char*)pWiFiDevice->pDataPkt);

	if( pWiFiDevice->nResponseSize != 0u)
	{
		memcpy(&pPacket[0], &pWiFiDevice->pDataPkt[0], pWiFiDevice->nResponseSize);
	    *nValidBytes = pWiFiDevice->nResponseSize;
	}
}



/*@}*/
