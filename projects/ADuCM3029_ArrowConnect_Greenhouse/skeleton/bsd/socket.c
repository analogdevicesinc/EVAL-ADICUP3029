/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "bsd/socket.h"
#include "config.h"
#include <adi_wifi_noos.h>
#include "../../wifi.h"
#include "string.h"

uint32_t u32Count = 0;
extern uint8_t         nHttpData[1000];
extern uint32_t        nValidBytes;
extern uint32_t test;

int busySocket = -1;

bool flag = true;

ADI_WIFI_AT_CMDCODE enCommand = CMD_AT_CIPSEND;

extern ADI_WIFI_TCP_CONNECT_CONFIG  gTCPConnect;

_stSockets _sockets[ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS] = {
		  {-1, 0, 0 },
		  {-1, 0, 0 },
		  {-1, 0, 0 },
		  {-1, 0, 0 },
		  {-1, 0, 0 }
		};

struct hostent* gethostbyname(const char *name) {
  static struct hostent s_hostent;
  // FIXME implementation

//  ESP8266_GetHostByName();
  s_hostent.h_addrtype = 2;
  s_hostent.h_addr = name;
  s_hostent.h_length = strlen(name);
  s_hostent.h_name = name;

  return &s_hostent;
}

int socket(int protocol_family, int socket_type, int protocol) {
	  SSP_PARAMETER_NOT_USED(protocol_family);

	  int socket = -1;
	  for(int i=0; i<ADI_WIFI_RADIO_MAX_NUM_CONNECTIONS; i++) {
	    if ( _sockets[i].socket < 0 ) {
	      // free sock!
	    	if( busySocket != i)
	    	{
	    		socket = i;
	    		break;
	    	}

	    }
	  }

	  if ( socket < 0 ) return socket;

	  switch(socket_type) {
	    case SOCK_STREAM:
//	      prot = WIFI_TCP_PROTOCOL;
	    break;
	    case SOCK_DGRAM:
	    {
//	      prot = WIFI_UDP_PROTOCOL;
	        socket = -1;
	      }
	    break;
	    default:
	    return -1;
	  }
	  _sockets[socket].socket = socket;
	  _sockets[socket].type = 0;
	  _sockets[socket].timeout = 3000;

	  return socket;
}

void soc_close(int socket) {
	_sockets[socket].socket = -1;
	busySocket = socket;

}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {

	ADI_WIFI_SEND_DATA_CONFIG   eSendDataConfig;

	ADI_WIFI_RESULT enResult;

	eSendDataConfig.nDataLength = len;
	eSendDataConfig.pData = buf;
	eSendDataConfig.nLinkID = sockfd;



	enResult = adi_wifi_radio_SendData(&eSendDataConfig, CMD_AT_CIPSEND);

	timer_sleep(200);

	if (enResult != ADI_WIFI_SUCCESS)
	{
		return -1;
	}
	return len;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen) {


	return len;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {

	if(nValidBytes == 0)
	{
		adi_wifi_radio_ReceiveData();
	}

	if (u32Count + len <= nValidBytes)
	{
		memcpy(buf, &nHttpData[u32Count], len);

	}
	else
		if(u32Count < nValidBytes)
		{
			len = nValidBytes - u32Count;
			memcpy(buf, &nHttpData[u32Count], len);

		}
		else
		{
			if (nValidBytes == 0)
			{
				return -1;
			}
			return 0;
		}


	if ((u32Count + len) == nValidBytes)
	{
		u32Count = 0;
		memset (nHttpData, 0, nValidBytes);
		nValidBytes = 0;
		test = 0;


	}
	else
	{
		u32Count += len;
	}

	return len;
}

ssize_t recvfrom(int sock, void *buf, size_t size, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen) {

	return size;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {

	ADI_WIFI_RESULT eWifiResult;
	uint16_t port;
	uint8_t buff[10];


	if ( !addr ) return -1;
	if ( addrlen != sizeof(struct sockaddr_in) ) return -1;
	struct sockaddr_in *rem_addr = (struct sockaddr_in *) addr;

	port = htons(rem_addr->sin_port);
	itoa(port, buff, 10);
	gTCPConnect.pPort = buff;

	gTCPConnect.nLinkID = sockfd;
	eWifiResult = adi_wifi_radio_EstablishTCPConnection(&gTCPConnect);

	if(eWifiResult == ADI_WIFI_SUCCESS)
	{
		return 0;
	}
	else
	{

		return -1;
	}

}

int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen) {
  // FIXME implementation
	return 0;
}

int bind(int sock, const struct sockaddr *addr, socklen_t addrlen) {
  // FIXME implementation
	return 0;
}

int listen(int sock, int backlog) {
  // FIXME implementation
	return 0;
}

int accept(int sock, struct sockaddr *addr, socklen_t *addrlen) {
  // FIXME implementation
	return 0;
}
