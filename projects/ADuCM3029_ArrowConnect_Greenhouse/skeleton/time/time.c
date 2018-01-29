/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "time/time.h"
#include <drivers/rtc/adi_rtc.h>

ADI_RTC_HANDLE hDevRtc  = NULL;

/*! RTC device number */
#define ADI_RTC_DEVICE_NUM       (1u)


/*!  The RTC prescalar can be caluculated using the equation: 1/(32768/2^Prescalar). Set prescalar to 5u for .97 ms precision */
#define ADI_RTC_PRESCALAR       (15u)

/*! Device memory to operate the RTC device */
static uint8_t aRtcDevMem[ADI_RTC_MEMORY_SIZE];


int RTC_Init(void)
{
	ADI_RTC_RESULT eRTCResult;
	eRTCResult = adi_rtc_Open(ADI_RTC_DEVICE_NUM, aRtcDevMem, ADI_RTC_MEMORY_SIZE, &hDevRtc);

	if(eRTCResult == ADI_RTC_SUCCESS)
	{
		eRTCResult = adi_rtc_SetPreScale(hDevRtc, ADI_RTC_PRESCALAR);
	}

	if(eRTCResult == ADI_RTC_SUCCESS)
	{
		eRTCResult =  adi_rtc_Enable(hDevRtc, true);
	}

	return eRTCResult;
}

void get_time(char *ts) {

	char buffer[128];
	struct tm *tmp;
	int ms;

	time_t rawtime;

	/* get the RTC count through the "time" CRTL function */
	adi_rtc_GetCount(hDevRtc, &rawtime);
	tmp = gmtime(&rawtime); //fill tmp with coresponding time in UTC

	//need to get the ms from the RTC fraction
	ms = 0;


	sprintf(ts, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", 1900+tmp->tm_year, tmp->tm_mon+1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec, ms);
	printf("ts: %s\r\n", ts);
}

static void get_time_of_day(struct timeval *tval) {
    // FIXME implementation
}

int gettimeofday(struct timeval *__restrict __p,  void *__restrict __tz) {

	//get time of day in seconds and ms
	adi_rtc_GetCount(hDevRtc, &(__p->tv_sec));

	//ms read
	__p->tv_usec = 0;

    return 0;
}

time_t time(time_t *timer) {

	time_t rawtime;

	adi_rtc_GetCount(hDevRtc, &rawtime);

	if ( timer ) *timer = rawtime;
	return rawtime;

}

int stime(time_t *timer) {

	time_t rawtime = *timer;
	adi_rtc_SetCount (hDevRtc, *timer);
    return 0;
}

int msleep(int m_sec) {
	timer_sleep(m_sec);
	return 0;
}
