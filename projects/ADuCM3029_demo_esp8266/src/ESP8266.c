/**
******************************************************************************
*   @file     ESP8266.c
*   @brief    ESP8266 Library for ADICUP3029
*   @version  V0.1
*   @author   SPopa (stefan.popa@analog.com)
*   @date     May 2017
*
*******************************************************************************
* Copyright 2017(c) Analog Devices, Inc.
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
*******************************************************************************
**/

/***************************** Include Files **********************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "Communication.h"
#include "common.h"
#include "Timer.h"
#include "string.h"
#include "ESP8266.h"
#include <drivers/gpio/adi_gpio.h>

/************************** Variable Definitions ******************************/
/* Handle for UART device */
extern ADI_UART_HANDLE          hUartDevice __attribute__ ((aligned (4)));

uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE];

/************************* Functions Definitions ******************************/

void ESP8266_Init(void)
{
	UART_Init();

	adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE); //initialize gpio
	adi_gpio_OutputEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_10, true);
	adi_gpio_PullUpEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_10,
			      true); 	// Enable pull-up resistors
	adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_10);

}

int8_t ESP8266_FindCommand(char *CmdBuf)
{
	uint8_t pos = 0;
	uint8_t CmdLen = strlen(CmdBuf);
	char *p;
	p = strchr(CmdBuf, '=');

	if(p) {
		CmdLen = p - CmdBuf;
	}

	for (pos = 0; pos < CMD_NUMBER; pos++) { // search command
		if(strncmp(CmdParams[pos].Cmd, CmdBuf, CmdLen) == 0) {
			return pos;
		}
	}

	return -1;
}


/**
   @brief Function that checks if the input is a valid AT command and calls ESP8266_SendCmd and ESP8266_GetResponse.
   	   	  It writes the command to UART and receives the response from ESP8266

   @param *CmdBuf - pointer to the desired command string; ex: "AT", "AT+RST"

   @return ESP8266_OK, ESP8266_ERROR, ESP8266_TIMEOUT, ESP8266_FAILED
**/
ESP8266_RESULT ESP8266_CmdWriteRead(char *CmdBuf)
{
	int8_t CmdSetPos;

	CmdSetPos = ESP8266_FindCommand(CmdBuf);

	if (CmdSetPos >= 0) {
		// Cmd found, concatenate "\r\n" and send it via UART
		ESP8266_SendCmd(CmdBuf);

		return ESP8266_GetResponse(CmdSetPos);
	} else {
		printf("Command %s not found\n", CmdBuf);
		return ESP8266_FAILED;
	}
}

/**
   @brief Helper function to read the reply from ESP8266 via UART. Once the Rx buffer is complete,
   	   	  we check that the reply is the expected one and return

    @param *RxBuf - pointer to the reply buffer received from the ESP8266

    @param ui32RxLen - Number of bytes to read via UART

    @param *ExpectedReply - Pointer to the expected reply character array. The reply can be: "OK", "GOT IP" etc

   @return ESP8266_OK, ESP8266_ERROR, ESP8266_TIMEOUT, ESP8266_FAILED
**/
ESP8266_RESULT ESP8266_GetResponse(ESP8266_CMD CmdPos)
{
	char *pNULL = NULL;
	char *ExpectedReply = CmdParams[CmdPos].ExpectedReply;
	char msg[250] = "";
	int i = 0;
	uint8_t ui8Char=0x00;
	uint32_t nHardwareError;

	adi_uart_Read(hUartDevice, &ui8Char, 1, 0, &nHardwareError);
	msg[i] = ui8Char;

	while(pNULL == strstr(msg, ExpectedReply)) {
		i++;
		adi_uart_Read(hUartDevice, &ui8Char, 1, 0, &nHardwareError);
		msg[i] = ui8Char;
	}

	printf("%s\n",  msg);
	fflush(stdout);

	return ESP8266_OK;
}


/**
   @brief Helper function to write the command buffer to the UART

   @param *CmdBuf - pointer to the desired command string; ex: "AT", "AT+RST"

   @return None.
**/
void ESP8266_SendCmd(char *CmdBuf)
{
	char 	command[250] = "";
	char 	rn[2] = "\r\n";
	uint8_t CmdLen = strlen(CmdBuf);
	uint32_t nHardwareError;

	strncat(command, CmdBuf, CmdLen);
	strncat(command, rn, 2);

	adi_uart_Write(hUartDevice, command, CmdLen+2, 0,
		       &nHardwareError); // Send the command via UART
}


/**
   @brief Function that connects ESP8266 to the WiFi

   @param *ssid - pointer to the desired ssid; ex: "AnalogDevices"
   @param *pass - pointer to the password string; ex: "123456"

   @return ip - the received IP after connecting to the WiFi network.
**/
uint8_t ESP8266_ConnectAccessPoint(char *ssid, char *pass)
{
	char connection[250];

	strcpy(connection, "AT+CWJAP=\"");
	strncat(connection, ssid, strlen(ssid));
	strncat(connection, "\",\"", 5);
	strncat(connection, pass, strlen(pass));
	strncat(connection, "\"", 2);

	// send test command
	ESP8266_CmdWriteRead("AT");
	timer_sleep(2000);

	// set mode
	ESP8266_CmdWriteRead("AT+CWMODE=3");
	timer_sleep(1000);

	ESP8266_CmdWriteRead(connection);
	timer_sleep(5000);

	return 0;
}

/**
   @brief Function that connects ESP8266 to a TCP Server

   @param *ip - pointer to the desired server ip; ex: "iot.eclipse.org"
   @param port - port number; ex: 1883

   @return None
**/
void ESP8266_ConnectTCP(char *ip, char *port)
{
	char connection[250];
	char *keep_alive = "7200";

	strcpy(connection, "AT+CIPSTART=\"TCP\",\"");
	strncat(connection, ip, strlen(ip));
	strncat(connection, "\",", 3);
	strncat(connection, port, strlen(port));
	strncat(connection, ",", 1);
	strncat(connection, keep_alive, strlen(keep_alive));

	ESP8266_CmdWriteRead(connection);

	timer_sleep(1000);
}

/**
   @brief Function that sends data

   @param *message - pointer to the desired message to send

   @return None
**/
void ESP8266_SendTCPData(char *message, int len)
{
	char connection[250];
	char msg[200] = "";
	char MsgLen[256];
	int i = 0;
	uint32_t nHardwareError;

	sprintf(MsgLen, "%d", len);
	strcpy(connection, "AT+CIPSEND=");
	strncat(connection, MsgLen, strlen(MsgLen));

	adi_uart_Write(hUartDevice, connection, strlen(connection), 0, &nHardwareError);
	adi_uart_Write(hUartDevice, "\r\n", 2, 0, &nHardwareError);

	timer_sleep(5000);

	for (i = 0; i < len; i++) {
		msg[i] = *message;
		message++;
	}

	adi_uart_Write(hUartDevice, msg, len, 0, &nHardwareError);
}

/**
   @brief Function that reads incoming TCP data

   @param None

   @return data
**/
int ESP8266_ReadTCPData(unsigned char* buf, int count)
{
	uint32_t nHardwareError;

	adi_uart_Read(hUartDevice, buf, count, 0, &nHardwareError);

	return count;
}

void ESP8266_WaitForBrokerResponse(uint8_t ui8Response)
{
	uint8_t ui8Char=0x00;
	uint32_t nHardwareError;

	while(ui8Char != ui8Response) {
		adi_uart_Read(hUartDevice, &ui8Char, 1, 0, &nHardwareError);
	}
}

uint8_t ESP8266_WaitForBrokerResponseNonBlocking(uint8_t ui8Response,
		uint32_t ui32Timeout)
{
	/* Bool variable for peek function */
	bool bRxBufferComplete = false;
	uint8_t ui8Char=0x00;

	start_esp_timer_ms();

	while (ui8Char != ui8Response) {
		adi_uart_SubmitRxBuffer(hUartDevice, &ui8Char, 1, 0u);

		while(true != bRxBufferComplete) {
			if (get_esp_timer_ms() >= ui32Timeout)
				return 0;
			adi_uart_IsRxBufferAvailable(hUartDevice, &bRxBufferComplete);
		}
	}
	return 1;
}
