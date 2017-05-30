/*
 * ESP8266.h
 *
 *  Created on: Feb 23, 2017
 *      Author: SPopa
 */

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
