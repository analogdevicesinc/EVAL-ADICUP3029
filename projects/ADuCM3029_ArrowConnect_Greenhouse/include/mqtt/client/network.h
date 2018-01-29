/*
 * MQTTAris.h
 *
 *  Created on: 18 окт. 2016 г.
 *      Author: ddemidov
 */

#ifndef MQTTCLIENT_C_SRC_ARIS_MQTTARIS_H_
#define MQTTCLIENT_C_SRC_ARIS_MQTTARIS_H_

#include <config.h>

#if !defined(USHRT_MAX)
# define USHRT_MAX 0xFFFFU
#endif

#if defined(WIN32_DLL) || defined(WIN64_DLL)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#elif defined(LINUX_SO)
  #define DLLImport extern
  #define DLLExport  __attribute__ ((visibility ("default")))
#else
  #define DLLImport
  #define DLLExport
#endif

#include <arrow/mem.h>
#include <time/time.h>
#include <bsd/socket.h>

#if defined(_ARIS_)
# if defined(ETH_MODE)
#  include <nx_api.h>
# else
#  include "socket/include/socket.h"
# endif
#endif

typedef struct Network {
    int my_socket;
    int (*mqttread) (struct Network*, unsigned char*, int, int);
    int (*mqttwrite) (struct Network*, unsigned char*, int, int);
} Network;

typedef struct TimerInterval {
    struct timeval end_time;
} TimerInterval;

void TimerInit(TimerInterval*);
char TimerIsExpired(TimerInterval*);
void TimerCountdownMS(TimerInterval*, unsigned int);
void TimerCountdown(TimerInterval*, unsigned int);
int TimerLeftMS(TimerInterval*);

DLLExport void NetworkInit(Network*);
DLLExport int NetworkConnect(Network*, char*, int);
DLLExport void NetworkDisconnect(Network*);

#endif /* MQTTCLIENT_C_SRC_ARIS_MQTTARIS_H_ */
