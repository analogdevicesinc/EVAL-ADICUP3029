/**
******************************************************************************
*   @file     ESP8266.h
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

#ifndef ESP8266_H_
#define ESP8266_H_

#define SSID 			"****"
#define PASS			"****"
#define MQTT_SERVER 	"****"
#define PORT			"1883"
#define CMD_NUMBER		7

struct ESP8266_CmdParams {
	char *Cmd;
    uint32_t Timeout;
    char *ExpectedReply;
};

typedef enum {
	AT,
	RST,
	CWMODE,
	GMR,
	CWJAP,
	CIPSTART,
	CIPSEND
}ESP8266_CMD;

static const struct ESP8266_CmdParams CmdParams[] = {
	/*CMD			Timeout	 	Reply*/
	{"AT",	 		1000, 		"OK"	},
	{"AT+RST",		2000, 		"OK"	},
	{"AT+CWMODE",	1000, 		"OK"	},
	{"AT+GMR",		0, 			"OK"	},
	{"AT+CWJAP", 	10000, 		"GOT IP"},
	{"AT+CIPSTART", 3000, 		"OK"	},
	{"AT+CIPSEND",	3000, 		"OK"	}
};

typedef enum {
	ESP8266_OK,
	ESP8266_ERROR,
	ESP8266_TIMEOUT,
	ESP8266_FAILED		// Generic Failure
}ESP8266_RESULT;

ESP8266_RESULT ESP8266_CmdWriteRead(char *CmdBuf);
ESP8266_RESULT ESP8266_GetResponse(ESP8266_CMD CmdPos);
void ESP8266_SendCmd(char *CmdBuf);
void ESP8266_Init(void);
uint8_t ESP8266_ConnectAccessPoint(char *ssid, char *pass);
char ESP8266_GetIpAddress();
void ESP8266_ConnectTCP(char *ip, char *port);
void ESP8266_SendTCPData(char *message, int len);
int ESP8266_ReadTCPData(unsigned char *buf, int count);
void ESP8266_WaitForBrokerResponse(uint8_t ui8Response);
uint8_t ESP8266_WaitForBrokerResponseNonBlocking(uint8_t ui8Response, uint32_t ui32Timeout);


#endif /* ESP8266_H_ */
