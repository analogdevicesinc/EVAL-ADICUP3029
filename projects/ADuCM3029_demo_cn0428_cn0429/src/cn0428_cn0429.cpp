/*******************************************************************************
*   @file     cn0428_cn0429.cpp
*   @brief    Contains driver and support functions for CN0429/CN0428
*   @version  V0.1
*   @author   ADI
********************************************************************************
 * Copyright 2018(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "cn0428_cn0429.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
 *	Variables
 */
SENSOR_RESULT			eSensorResult;

extern const ADI_GPIO_PORT	CS_Port[];
extern const ADI_GPIO_DATA	CS_Pin[];
extern const uint8_t		sensor_addresses[];
extern uint8_t			TXbuff[256];
extern char			&tempChar; /* Guaranteed to be followed by 0 */

M355_GAS m355_gas_sensor;
Gas_Reading *pGasSensor = &m355_gas_sensor;

uint8_t uiDefaultSite = 0;		/* selected sensor location */
uint8_t uiDefaultAddress = 0;		/* address of the selectet sensor location */
uint8_t uiPulseAmpl = 0;		/* pulse test amplitude */
uint8_t uiPulseDur = 0;			/* pulse test duration */

uint8_t rcal = 0;
uint8_t gBuff[64] = { 0 };			/* general use buffer */
uint8_t pulseResultBuffer[6144] = { 0 };	/* buffer holding pulse test result */
uint8_t EISResponseBuff[1024] = { 0 };		/* buffer holding EIS results */
uint32_t EISresults[192] = { 0 };		/* array of EIS results */
int32_t EISpartialDFTresult[8] = { 0 };		/* array of partial results (DFT impedance) of EIS */
double EISpartialresult[8] = { 0 };		/* array of partial (one line) EIS results */

/* borrow this memory space for water quality results */
uint8_t *Slave_Rx_Buffer = pulseResultBuffer;
uint16_t Slave_Rx_Index = 0;

const char* rtia[] = { "0R", "200R", "1k", "2k", "3k", "4k", "6k", "8k", "10k",
		       "12k", "16k", "20k", "24k", "30k", "32k", "40k", "48k", "64k",
		       "85k", "96k", "100k", "120k", "128k", "160k", "196k", "256k",
		       "512k"
		     };

const char* rload[] = { "0R", "10R", "30R", "50R", "100R", "1k6", "3k1", "3k6" };

const bool no_sensors[NUM_SENSORS] = { };
extern bool detected_sensors[];

extern uint16_t streamTickCnt;
extern uint16_t streamTickCfg;

extern eFSM_State FSM_State;

/*
 *	CN0429 Functions
 */

/* Available CLI commands */
char const *CmdCommands[] = {
	"",
	"help",
	"defaultsensor",
	"sensorsconnected",
	"readconfigs",
	"readtemp",
	"readhum",
	"setmeastime",
	"startmeas",
	"stopmeas",
	"setrtia",
	"setrload",
	"setvbias",
	"setsensitivity",
	"runeis",
	"readeis",
	"readeisfull",
	"readrcal",
	"runpulse",
	"readpulse",
	"pulseamplitude",
	"pulseduration",
	"readsensors",
	"stopread",
	"setupdaterate",
	""
};

/* Functions for available CLI commands */
cmdFunc CmdFun[] = {
	DoNothing,
	CN0429_CmdHelp,
	CN0429_SetDefaultSns,
	CN0429_SnsConnected,
	CN0429_RdCfgs,
	CN0429_RdTemp,
	CN0429_RdHum,
	CN0429_CfgMeasTime,
	CN0429_StartMeas,
	CN0429_StopMeas,
	CN0429_CfgRtia,
	CN0429_CfgRload,
	CN0429_CfgVbias,
	CN0429_CfgSens,
	CN0429_RunEIS,
	CN0429_RdEIS,
	CN0429_RdEISfull,
	CN0429_RdRcal,
	CN0429_RunPulse,
	CN0429_ReadPulse,
	CN0429_SetPulseAmpl,
	CN0429_SetPulseDuration,
	CN0429_RdSensors,
	CN0429_StopRd,
	CN0429_CfgUpdateRate
};

/**
 @brief Display info for <help> command

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_CmdHelp(uint8_t *args)
{
	UART_TX("help                           -Print commands" _EOS);
	UART_TX("defaultsensor <site>           -Set default sensor site" _EOS);
	UART_TX("                                  <site> = 1, 2, 3, 4" _EOS);
	UART_TX("sensorsconnected               -Print which sensors are connected"
		_EOS);
	UART_TX("readconfigs                    -Read sensor configurations" _EOS);
	UART_TX("readtemp                       -Read temperature from on-board sensor"
		_EOS);
	UART_TX("readhum                        -Read humidity from on-board sensor"
		_EOS);
	UART_TX("setmeastime <time>             -Set ADC sampling time in msec, default - 500"
		_EOS);
	UART_TX("                                  <time> = <50-32000>" _EOS);
	UART_TX("                                  ADC performs avg of 10 samples at this interval"
		_EOS);
	UART_TX("startmeas                      -Start ADC sampling at configured interval"
		_EOS);
	UART_TX("stopmeas                       -Stop ADC sampling the sensor" _EOS);
	UART_TX("setrtia <rtia>                 -Configure TIA resistance" _EOS);
	UART_TX("                                  <rtia> = <0-26> equals 0R - 512k"
		_EOS);
	UART_TX("                                  see user guide for detailed table"
		_EOS);
	UART_TX("setrload <rload>               -Configure sensor load resistor" _EOS);
	UART_TX("                                  <rload> = <0-7> equals 0R - 3k6"
		_EOS);
	UART_TX("                                  see user guide for detailed table"
		_EOS);
	UART_TX("setvbias <vbias>               -Configure sensor bias voltage" _EOS);
	UART_TX("                                  <vbias> = bias voltage in mV" _EOS);
	UART_TX("setsensitivity <sens>          -Configure sensitivity" _EOS);
	UART_TX("                                  <sens> = sensitivity in nA/ppm"
		_EOS);
	UART_TX("runeis                         -Run impedance spectroscopy" _EOS);
	UART_TX("readeis                        -Read impedance spectroscopy results"
		_EOS);
	UART_TX("readeisfull                    -Read impedance spectroscopy FULL results"
		_EOS);
	UART_TX("readrcal                       -Read 200R calibration resistor" _EOS);
	UART_TX("runpulse                       -Run pulse test. Set amplitude and duration first!"
		_EOS);
	UART_TX("readpulse                      -Read pulse test results" _EOS);
	UART_TX("pulseamplitude <ampl>          -Set amplitude for pulse test" _EOS);
	UART_TX("                                  <ampl> = amplitude in mV, 1 - 3 mV"
		_EOS);
	UART_TX("pulseduration <dur>            -Set duration for pulse test" _EOS);
	UART_TX("                                  <dur> = duration in ms, 10 to 200"
		_EOS);
	UART_TX("readsensors                    -Read the ppb value of all sensors"
		_EOS);
	UART_TX("setupdaterate <time>           -Configure update rate for stream mode"
		_EOS);
	UART_TX("stopread                       -Stop sensor data reading" _EOS);
	UART_TX("\r\n" _EOS);
}

/**
 @brief Finds the next command line argument

 @param args - pointer to the arguments on the command line.

 @return pointer to the next argument.

 **/
uint8_t *FindArgv(uint8_t *args)
{
	uint8_t *p = args;
	int fl = 0;

	while (*p != 0) {
		if ((*p == _SPC))
			fl = 1;
		else if (fl)
			break;
		p++;
	}

	return p;
}

/**
 @brief Separates a command line argument

 @param dst - pointer to a buffer where the argument will be copied
 @param args - pointer to the current position of the command line .

 @return none

 **/
void GetArgv(char *dst, uint8_t *args)
{
	uint8_t *s = args;
	char *d = dst;

	while (*s) {
		if (*s == _SPC)
			break;
		*d++ = *s++;
	}

	*d = '\0';
}

/**
 @brief Command line process function

 @return none
 **/
void CmdProcess(void)
{
	cmdFunc func;

	/* Find needed function based on typed command */
	func = FindCommand((char *) cmdInString);

	/* Check if there is a valid command */
	if (func)
		/* Call the desired function */
		(*func)(&cmdInString[2]);
	else
		UART_TX("Unknown command!" _EOS);
	/* Prepare for next <ENTER> */
	CN0429_CmdPrompt();
}

/**
 @brief Command line prompt. Caller must verify that only the enabled board can run this.

 @return int - UART status: UART_SUCCESS or error status
 **/
void CN0429_CmdPrompt(void)
{
	static uint8_t count = 0;

	/* Print welcome message after reset */
	if (count == 0) {
		UART_TX("Welcome to CN0429!" _EOS);
		UART_TX("Type <help> to see available commands..." _EOS);
		count++;
	}
}

/**
 @brief Find available commands

 @param cmd - command to search

 @return cmdFunc - return the specific function for available command or NULL
 for invalid command
 **/
cmdFunc FindCommand(char *cmd)
{
	cmdFunc func = NULL;
	char *p = cmd;
	int i = 0;
	int f1 = 0;
	int cmdlength = 0;

	while (*p != 0) {
		if (*p == _SPC)
			f1 = 1;
		else if (!f1)
			cmdlength++;
		p++;
	}

	while (CmdFun[i] != NULL) {
		if (strncmp(cmd, CmdCommands[i], cmdlength) == 0) {
			func = CmdFun[i];
			break;
		}
		i++;
	}

	return func;
}

void DoNothing(uint8_t *args)
{
	/* nothing to do here
	 * this function runs for zero-length command
	 */
}

/**
 @brief Set sensor to use for single sensor commands

 @param args - pointer to the arguments on the command line.
 site: 1, 2, 3, or 4

 @return none
 **/
void CN0429_SetDefaultSns(uint8_t *args)
{
	uint8_t *p = args;
	char arg[17];
	char buff[20] = { 0 };

	/* Check if this function gets an argument */
	while (*(p = FindArgv(p)) != '\0') {
		/* Save channel parameter */
		GetArgv(arg, p);
	}

	/*DEFAULT CONFIGURATION*/
	if (strncmp(arg, "1", 2) == 0) {
		uiDefaultSite = 1;
		uiDefaultAddress = sensor_addresses[0];
	} else if (strncmp(arg, "2", 2) == 0) {
		uiDefaultSite = 2;
		uiDefaultAddress = sensor_addresses[1];
	} else if (strncmp(arg, "3", 2) == 0) {
		uiDefaultSite = 3;
		uiDefaultAddress = sensor_addresses[2];
	} else if (strncmp(arg, "4", 2) == 0) {
		uiDefaultSite = 4;
		uiDefaultAddress = sensor_addresses[3];
	} else {
		UART_TX("Invalid site" _EOS);
	}
	sprintf(buff, "Selected site: %d%s", uiDefaultSite, _EOS);
	UART_TX((const char*) buff);
}

/*!
 * @brief      Reads sensor config
 *
 * @details    Reads sensor config of the sensor with address specified in input parameter
 */
SENSOR_RESULT CN0429_GetSensorCfg(uint8_t sensor_address)
{
	uint8_t pBuff[8] = { 0 };
	int16_t sigVal = 0;
	uint16_t unsigVal = 0;
	uint32_t unsiglVal = 0;

	eSensorResult = pGasSensor->openWithAddr(sensor_address);
	if (eSensorResult != SENSOR_ERROR_NONE)
		return eSensorResult;
	delay_ms(5);

	for (uint8_t i = 0; i < NUM_SENSORS; i++) {
		if (sensor_address == sensor_addresses[i])
			sprintf((char*) &TXbuff,
				"Config of sensor on site %d:%s", i + 1,
				_EOS);
	}

	UART_TX((const char*) TXbuff);
	delay_ms(5);

	flushBuff(pBuff, sizeof(pBuff));
	eSensorResult = pGasSensor->ReadTIAGain(pBuff);
	if (eSensorResult != SENSOR_ERROR_NONE)
		return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	sprintf((char*) TXbuff, "RTIA = %s ohm%s", rtia[pBuff[0]], _EOS);
	UART_TX((const char*) TXbuff);
	delay_ms(5);

	flushBuff(pBuff, sizeof(pBuff));
	eSensorResult = pGasSensor->ReadRload(pBuff);
	if (eSensorResult != SENSOR_ERROR_NONE)
		return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	sprintf((char*) TXbuff, "Rload = %s ohm%s", rload[pBuff[0]], _EOS);
	UART_TX((const char*) TXbuff);
	delay_ms(5);

	eSensorResult = pGasSensor->ReadSensorBias(&sigVal);
	if (eSensorResult != SENSOR_ERROR_NONE)
		return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	sprintf((char*) TXbuff, "Vbias = %d mV%s", sigVal, _EOS);
	UART_TX((const char*) TXbuff);
	delay_ms(5);

	eSensorResult = pGasSensor->ReadSensorSensitivity(&unsiglVal);
	if (eSensorResult != SENSOR_ERROR_NONE)
		return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	sprintf((char*) TXbuff, "Sensitivity = %.2f nA/ppm%s",
		(unsiglVal / 100.0), _EOS);
	UART_TX((const char*) TXbuff);
	delay_ms(5);

	eSensorResult = pGasSensor->ReadMeasurementTime(&unsigVal);
	if (eSensorResult != SENSOR_ERROR_NONE)
		return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	sprintf((char*) TXbuff, "Measurement Time = %d msec%s", unsigVal, _EOS);
	UART_TX((const char*) TXbuff);
	delay_ms(5);

	eSensorResult = pGasSensor->close();
	delay_ms(5);

	return eSensorResult;
}

/*!
 * @brief      Reads averaged ppb value from gas sensor
 *
 * @details    Reads sensor data utilizing custom gas sensor I2C library and sends this data over UART.
 */
int32_t CN0429_SensorReadoutPPB(uint8_t sensor_address)
{
	int32_t PPB_Gas_Reading = 0;
	eSensorResult = pGasSensor->openWithAddr(sensor_address);
	eSensorResult = pGasSensor->ReadDataPPB(&PPB_Gas_Reading);
	eSensorResult = pGasSensor->close();
	delay_ms(10);
	return PPB_Gas_Reading;
}

/**
 @brief Print which sensors are connected

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_SnsConnected(uint8_t *args)
{
	for (uint8_t i = 0; i < NUM_SENSORS; i++) {
		if (detected_sensors[i]) {
			flushBuff(TXbuff, sizeof(TXbuff));
			sprintf((char*) TXbuff, "Sensor detected on site %d%s",
				(i + 1), _EOS);
			UART_TX((const char*) TXbuff);
		}
	}
}

/**
 @brief Get configuration of sensors

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_RdCfgs(uint8_t *args)
{
	for (uint8_t i = 0; i < NUM_SENSORS; i++) {
		if (detected_sensors[i]) {
			CN0429_GetSensorCfg(sensor_addresses[i]);
			delay_ms(10);
		}
	}
}

/**
 @brief Read temperature from the daughter board's on-board T&RH sensor

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_RdTemp(uint8_t *args)
{
	int16_t temperature = 0;
	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->ReadTemperature(&temperature);
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "Temperature = %2.1f degC%s",
			(temperature / 100.0), _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}
/**
 @brief Read humidity from the daughter board's on-board T&RH sensor

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_RdHum(uint8_t *args)
{
	int16_t humidity = 0;
	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->ReadHumidity(&humidity);
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "Humidity = %2.1f %%RH%s",
			(humidity / 100.0), _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Set measurement time (ms)

 @param args - pointer to the arguments on the command line.
 time: [msec] 50 - 32000

 @return none
 **/
void CN0429_CfgMeasTime(uint8_t *args)
{
	uint8_t *p = args;
	char arg[17];
	uint8_t i;
	uint8_t digitcount = 0;
	uint32_t value = 0;

	/* Check if this function gets an argument */
	while (*(p = FindArgv(p)) != '\0') {
		/* Save channel parameter */
		GetArgv(arg, p);
	}

	/* Check if arg is numeric */
	for (i = 0; arg[i] != '\0'; i++) {
		/* check for decimal digits */
		if (isdigit(arg[i]) != 0)
			digitcount++;
	}
	if (digitcount == i) /* All characters are numeric */
		value = strtol(arg, (char **) NULL, 10); /* convert string to number */
	else
		UART_TX("Invalid entry!" _EOS);

	if (value > 32000 || value < 50) {
		value = 500;
		UART_TX("Out of range! Set to default = 500 ms" _EOS);
	}

	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->SetMeasurementTime(value);
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "Measurement time set to %ld msec%s",
			value, _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Start ADC sampling the sensor

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_StartMeas(uint8_t *args)
{
	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->StartMeasurements();
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "Measurement started%s", _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Stop ADC sampling the sensor

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_StopMeas(uint8_t *args)
{
	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->StopMeasurements();
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "Measurement stopped%s", _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Program the TIA with the requested gain resistor

 @param args - pointer to the arguments on the command line.
 Rtia: from 0 to 26 equals 0R - 512k , see user guide for details

 @return none
 **/
void CN0429_CfgRtia(uint8_t *args)
{
	uint8_t *p = args;
	char arg[17];
	uint8_t i;
	uint8_t digitcount = 0;
	uint32_t value = 0;

	/* Check if this function gets an argument */
	while (*(p = FindArgv(p)) != '\0') {
		/* Save channel parameter */
		GetArgv(arg, p);
	}

	/* Check if arg is numeric */
	for (i = 0; arg[i] != '\0'; i++) {
		/* check for decimal digits */
		if (isdigit(arg[i]) != 0)
			digitcount++;
	}
	if (digitcount == i) /* All characters are numeric */
		/* convert string to number */
		value = (uint8_t) strtol(arg, (char **) NULL, 10);
	else
		UART_TX("Invalid entry!" _EOS);

	if (value > 26 || value < 0) {
		value = 1;
		UART_TX("Out of range! Set to default = 200 ohm" _EOS);
	}

	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->SetTIAGain(value);
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "TIA gain resistor set to %s ohm%s",
			rtia[value], _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Set sensor bias voltage

 @param args - pointer to the arguments on the command line.
 sensor bias: [mV] from -2200 to 2200

 @return none
 **/
void CN0429_CfgVbias(uint8_t *args)
{
	uint8_t *p = args;
	char arg[17];
	uint8_t i;
	uint8_t digitcount = 0;
	int32_t value = 0;

	/* Check if this function gets an argument */
	while (*(p = FindArgv(p)) != '\0') {
		/* Save channel parameter */
		GetArgv(arg, p);
	}

	if (arg[0] == '-')
		digitcount++;
	/* Check if arg is numeric */
	for (i = 0; arg[i] != '\0'; i++) {
		/* check for decimal digits */
		if (isdigit(arg[i]) != 0)
			digitcount++;
	}
	if (digitcount == i) /* All characters are numeric */
		/* convert string to number */
		value = (int32_t) strtol(arg, (char **) NULL, 10);
	else
		UART_TX("Invalid entry!" _EOS);

	if (value > 2200 || value < -2200) {
		value = 0;
		UART_TX("Out of range! Set to default = 0 mV" _EOS);
	}

	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->SetSensorBias(value);
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "Sensor bias set to %ld mV%s", value,
			_EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Configure sensor sensitivity

 @param args - pointer to the arguments on the command line.
 sensitivity: [nA/ppm]

 @return none
 **/
void CN0429_CfgSens(uint8_t *args)
{
	uint8_t *p = args;
	char arg[17];
	uint8_t i;
	uint8_t digitcount = 0;
	uint8_t dotcount = 0;
	float value = 0;

	/* Check if this function gets an argument */
	while (*(p = FindArgv(p)) != '\0') {
		/* Save channel parameter */
		GetArgv(arg, p);
	}

	/* Check if arg is numeric */
	for (i = 0; arg[i] != '\0'; i++) {
		/* check for decimal digits */
		if (isdigit(arg[i]) != 0)
			digitcount++;

		if (arg[i] == '.') {
			digitcount++;
			dotcount++;
		}
	}
	if (digitcount == i) { /* All characters are numeric */
		value = strtof(arg, (char **) NULL); /* convert string to number */
	} else {
		UART_TX("Invalid entry!" _EOS);
		return;
	}

	if (dotcount > 1) {
		UART_TX("Invalid entry!" _EOS);
		return;
	}

	if (value >= 32768 || value < 0) {
		UART_TX("Out of range!" _EOS);
		return;
	}

	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->SetSensorSensitivity(value * 100);
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff,
			"Sensor sensitivity set to %2.2f nA/ppm%s",
			value, _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Start electrochemical impedance spectroscopy test

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_RunEIS(uint8_t *args)
{
	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->RunEISMeasurement();
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "EIS test started%s", _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Read electrochemical impedance spectroscopy results

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_RdEIS(uint8_t *args)
{
	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	memset(EISResponseBuff, 0, sizeof(EISResponseBuff));
	memset(EISresults, 0, sizeof(EISresults));
	eSensorResult = pGasSensor->ReadEISResults(EISResponseBuff);

	for (int i = 0; i < 36; i++) {
		gBuff[0] = EISResponseBuff[0 + (i * 4)];
		gBuff[1] = EISResponseBuff[1 + (i * 4)];
		gBuff[2] = EISResponseBuff[2 + (i * 4)];
		gBuff[3] = EISResponseBuff[3 + (i * 4)];
		memcpy(&EISresults[i], gBuff, 4);
	}

	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
		pGasSensor->close();
		return;
	}

	sprintf((char*) TXbuff, "Frequency, Magnitude, Phase%s", _EOS);
	UART_TX((const char*) TXbuff);
	for (int j = 0; j < 12; j++) {
		int2binString(EISresults[0 + (j * 3)], gBuff, 32);
		EISpartialresult[0] = Ieee754ConvertToDouble(gBuff);
		int2binString(EISresults[1 + (j * 3)], gBuff, 32);
		EISpartialresult[1] = Ieee754ConvertToDouble(gBuff);
		int2binString(EISresults[2 + (j * 3)], gBuff, 32);
		EISpartialresult[2] = Ieee754ConvertToDouble(gBuff);

		flushBuff(TXbuff, sizeof(TXbuff));
		sprintf((char*) TXbuff, "%f, %f, %f%s", EISpartialresult[0],
			EISpartialresult[1], EISpartialresult[2], _EOS);
		UART_TX((const char*) TXbuff);
	}

	pGasSensor->close();
}

/**
 @brief Read electrochemical impedance spectroscopy FULL results

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_RdEISfull(uint8_t *args)
{
	uint8_t i;
	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	memset(EISResponseBuff, 0, sizeof(EISResponseBuff));
	memset(EISresults, 0, sizeof(EISresults));
	eSensorResult = pGasSensor->ReadEISResultsFull(EISResponseBuff);

	for (i = 0; i < 180; i++) {
		gBuff[0] = EISResponseBuff[0 + (i * 4)];
		gBuff[1] = EISResponseBuff[1 + (i * 4)];
		gBuff[2] = EISResponseBuff[2 + (i * 4)];
		gBuff[3] = EISResponseBuff[3 + (i * 4)];
		memcpy(&EISresults[i], gBuff, 4);
	}

	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
		pGasSensor->close();
		return;
	}

	sprintf((char*) TXbuff,
		"Frequency, Rload+Rsens_real, Rload+Rsens_img, Rload_real, Rload_imag, Rcal_real, Rcal_imag, Mag_Rsens+Rload, Mag_Rload, Mag_Rcal, Mag_Rsens, MAG, PHASE%s",
		_EOS);
	UART_TX((const char*) TXbuff);
	for (int j = 0; j < 12; j++) {
		int2binString(EISresults[0 + (j * 15)], gBuff, 32);
		EISpartialresult[0] = Ieee754ConvertToDouble(gBuff);

		for (i = 0; i < 6; i++) {
			memcpy(&EISpartialDFTresult[i],
			       (int32_t*) &EISresults[i + 1 + (j * 15)],
			       sizeof(EISresults[i + 1 + (j * 15)]));
		}

		for (i = 1; i < 7; i++) {
			int2binString(EISresults[i + 6 + (j * 15)], gBuff, 32);
			EISpartialresult[i] = Ieee754ConvertToDouble(gBuff);
		}

		flushBuff(TXbuff, sizeof(TXbuff));
		sprintf((char*) TXbuff,
			"%f, %ld, %ld, %ld, %ld, %ld, %ld, %f, %f, %f, %f, %f, %f%s",
			EISpartialresult[0], EISpartialDFTresult[0],
			EISpartialDFTresult[1], EISpartialDFTresult[2],
			EISpartialDFTresult[3], EISpartialDFTresult[4],
			EISpartialDFTresult[5], EISpartialresult[1],
			EISpartialresult[2], EISpartialresult[3],
			EISpartialresult[4], EISpartialresult[5],
			EISpartialresult[6], _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Read the value of the 200R calibration resistor

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_RdRcal(uint8_t *args)
{
	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	flushBuff(gBuff, sizeof(gBuff));
	eSensorResult = pGasSensor->Read200RCal(gBuff);
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "Rcal = %d ohm%s", gBuff[0], _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Start chronoamperometry pulse test

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_RunPulse(uint8_t *args)
{
	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);

	flushBuff(gBuff, sizeof(gBuff));
	eSensorResult = pGasSensor->Read200RCal(gBuff);
	rcal = gBuff[0];

	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->RunPulseTest(uiPulseAmpl, uiPulseDur);
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "Pulse test started%s", _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Read results of chronoamperometry pulse test

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_ReadPulse(uint8_t *args)
{
	uint16_t value = 0;
	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->ReadPulseTestResults(&pulseResultBuffer[0],
			uiPulseAmpl, uiPulseDur);
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
		pGasSensor->close();
		return;
	}

	UART_TX("time [msec], current [uA]" _EOS);
	uint16_t valToSend = (((uiPulseDur + 10) * 1000) / 110);
	uint16_t cnt = 0;
	uint16_t cnt2 = 1;
	while (cnt < valToSend) {
		flushBuff(TXbuff, sizeof(TXbuff));
		value = ((pulseResultBuffer[cnt2] << 8)
			 | pulseResultBuffer[cnt2 + 1]);
		sprintf((char*) TXbuff, "%d, %.2f%s", ((cnt) * 110),
			((32768.0 - value) / 65535)
			* ((1835 * 2 * 1000) / rcal),
			_EOS);
		UART_TX((const char*) TXbuff);
		cnt++;
		cnt2 += 2;
	}

	pGasSensor->close();
}

/**
 @brief Set step amplitude for pulse test

 @param args - pointer to the arguments on the command line.
 amplitude: [mV] - Amplitude of the pulse in mV (typically 1mV)

 @return none
 **/
void CN0429_SetPulseAmpl(uint8_t *args)
{
	uint8_t *p = args;
	char arg[17];
	uint8_t i;
	uint8_t digitcount = 0;

	/* Check if this function gets an argument */
	while (*(p = FindArgv(p)) != '\0') {
		/* Save channel parameter */
		GetArgv(arg, p);
	}

	/* Check if arg is numeric */
	for (i = 0; arg[i] != '\0'; i++) {
		/* check for decimal digits */
		if (isdigit(arg[i]) != 0)
			digitcount++;
	}
	if (digitcount == i) { /* All characters are numeric */
		/* convert string to number */
		uiPulseAmpl = (uint8_t) strtol(arg, (char **) NULL, 10);
	} else {
		UART_TX("Invalid entry!" _EOS);
		return;
	}

	if (uiPulseAmpl >= 3 || uiPulseAmpl < 1) {
		uiPulseAmpl = 1;
		UART_TX("Out of range! Set to default = 1 mV" _EOS);
		return;
	}

	flushBuff(gBuff, sizeof(gBuff));
	sprintf((char *) gBuff, "Pulse amplitude set to %d mV%s", uiPulseAmpl,
		_EOS);
	UART_TX((const char*) gBuff);
}

/**
 @brief Set duration for CAPA test

 @param args - pointer to the arguments on the command line.
 duration: [milliseconds] - Duration of the pulse (keep <200 ms)

 @return none
 **/
void CN0429_SetPulseDuration(uint8_t *args)
{
	uint8_t *p = args;
	char arg[17];
	uint8_t i;
	uint8_t digitcount = 0;

	/* Check if this function gets an argument */
	while (*(p = FindArgv(p)) != '\0') {
		/* Save channel parameter */
		GetArgv(arg, p);
	}

	/* Check if arg is numeric */
	for (i = 0; arg[i] != '\0'; i++) {
		/* check for decimal digits */
		if (isdigit(arg[i]) != 0)
			digitcount++;
	}
	if (digitcount == i) /* All characters are numeric */
		/* convert string to number */
		uiPulseDur = (uint8_t) strtol(arg, (char **) NULL, 10);
	else
		UART_TX("Invalid entry!" _EOS);

	if (uiPulseDur > 200 || uiPulseDur < 1) {
		uiPulseDur = 50;
		UART_TX("Out of range! Set to default = 50 msec" _EOS);
	}

	flushBuff(gBuff, sizeof(gBuff));
	sprintf((char *) gBuff, "Pulse duration set to %d msec%s", uiPulseDur,
		_EOS);
	UART_TX((const char*) gBuff);
}

/**
 @brief Select sensor load resistor

 @param args - pointer to the arguments on the command line.
 Rtia: from 0 to 7 equals 0R - 3k6 , see user guide for details

 @return none
 **/
void CN0429_CfgRload(uint8_t *args)
{
	uint8_t *p = args;
	char arg[17];
	uint8_t i;
	uint8_t digitcount = 0;
	uint32_t value = 0;

	/* Check if this function gets an argument */
	while (*(p = FindArgv(p)) != '\0') {
		/* Save channel parameter */
		GetArgv(arg, p);
	}

	/* Check if arg is numeric */
	for (i = 0; arg[i] != '\0'; i++) {
		/* check for decimal digits */
		if (isdigit(arg[i]) != 0)
			digitcount++;
	}
	if (digitcount == i) /* All characters are numeric */
		/* convert string to number */
		value = (uint8_t) strtol(arg, (char **) NULL, 10);
	else
		UART_TX("Invalid entry!" _EOS);

	if (value > 7 || value < 0) {
		value = 1;
		UART_TX("Out of range! Set to default = 10 ohm" _EOS);
	}

	eSensorResult = pGasSensor->openWithAddr(uiDefaultAddress);
	flushBuff(TXbuff, sizeof(TXbuff));
	eSensorResult = pGasSensor->SetRload((uint8_t) value);
	if (eSensorResult != SENSOR_ERROR_NONE) {
		UART_TX("ERROR!" _EOS);
	} else {
		sprintf((char*) TXbuff, "Load resistor set to %s ohm%s",
			rload[(uint8_t) value], _EOS);
		UART_TX((const char*) TXbuff);
	}
	pGasSensor->close();
}

/**
 @brief Read ppb value of all sensors

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_RdSensors(uint8_t *args)
{
	flushBuff(TXbuff, sizeof(TXbuff));
	if (detected_sensors[0])
		strcat((char*) TXbuff, "Sensor 1[ppb], ");
	if (detected_sensors[1])
		strcat((char*) TXbuff, "Sensor 2[ppb], ");
	if (detected_sensors[2])
		strcat((char*) TXbuff, "Sensor 3[ppb], ");
	if (detected_sensors[3])
		strcat((char*) TXbuff, "Sensor 4[ppb], ");
	UART_TX((const char*) TXbuff);
	FSM_State = STREAM_GAS;
}

/**
 @brief Stop reading sensor data

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_StopRd(uint8_t *args)
{
	FSM_State = COMMAND;
	streamTickCnt = 0;
	UART_TX("Data reading interrupted!" _EOS);
}

/**
 @brief Configure update rate in STREAM mode

 @param args - pointer to the arguments on the command line.

 @return none
 **/
void CN0429_CfgUpdateRate(uint8_t *args)
{
	uint8_t *p = args;
	char arg[17];
	uint8_t i;
	uint8_t digitcount = 0;
	uint32_t value = 0;

	/* Check if this function gets an argument */
	while (*(p = FindArgv(p)) != '\0') {
		/* Save channel parameter */
		GetArgv(arg, p);
	}

	/* Check if arg is numeric */
	for (i = 0; arg[i] != '\0'; i++) {
		/* check for decimal digits */
		if (isdigit(arg[i]) != 0)
			digitcount++;
	}
	if (digitcount == i) /* All characters are numeric */
		/* convert string to number */
		value = (uint8_t) strtol(arg, (char **) NULL, 10);
	else
		UART_TX("Invalid entry!" _EOS);

	if (value > 3600 || value < 1) {
		value = 1;
		UART_TX("Out of range! Set to default = 1 sec" _EOS);
	}

	streamTickCfg = value;
	flushBuff(gBuff, sizeof(gBuff));
	sprintf((char *) gBuff, "Stream data update rate set to %d sec %s",
		streamTickCfg, _EOS);
	UART_TX((const char *) gBuff);
}

/**
 @brief Stream data from gas sensor at specified intervals.

 @return none
 **/
void CN0429_StreamData(void)
{
	flushBuff(TXbuff, sizeof(TXbuff));
	for (uint8_t i = 0; i < NUM_SENSORS; i++) {
		if (detected_sensors[i]) {
			flushBuff(gBuff, sizeof(gBuff));
			sprintf((char*) gBuff, "%ld, ",
				CN0429_SensorReadoutPPB(sensor_addresses[i]));
			strcat((char*) TXbuff, (char*) gBuff);
		}
	}
	UART_TX((const char*) TXbuff);
}

/*
 *	CN0428 Functions
 */
/**
 @brief Empty the slave I2C buffer and optionally loop until slave complete signal

 @param continuous - run until slave sends the ~ command complete signal

 @return none
 **/
void CN0428_RdSensorBuff(uint8_t continuous)
{
	bool exitloop;
	uint16_t timeout;
	uint8_t tempError;
	uint8_t numbytes = 0;

	exitloop = !continuous;
	timeout = 1000; /* EIS result can take minutes depending on chosen frequencies */
	do { /* 0.1s loop waiting for slave */
		delay_ms(95);
		numbytes = 0;
		pGasSensor->I2CReadWrite(READ,
					 BYTES_TO_READ,
					 (uint8_t *) &numbytes,
					 1);
		delay_ms(5);

		/* read <=255B chunks until slave buffer is empty */
		while (numbytes > 0) { /* data available */
			tempError = pGasSensor->I2CReadWrite(READ,
							     0x63,
							     &Slave_Rx_Buffer[0],
							     numbytes);
			if (tempError != SENSOR_ERROR_NONE) {
				UART_TX("Error reading from sensor" _EOS);
				break;
			}
			delay_ms(2);
			for (Slave_Rx_Index = 0; Slave_Rx_Index < numbytes;
			     Slave_Rx_Index++) {
				tempChar = Slave_Rx_Buffer[Slave_Rx_Index];
				Slave_Rx_Buffer[Slave_Rx_Index] = '\0';
				if (tempChar == '~') /* Slave finished */
					exitloop = true;
				else
					UART_TX_DIR((const char *) &tempChar);
			}
			pGasSensor->I2CReadWrite(READ,
						 BYTES_TO_READ,
						 (uint8_t *) &numbytes,
						 1);
			delay_ms(2);
		} /* end data available while loop. Slave may be processing. */

		delay_us(100);
		timeout--;
	} while (timeout > 0 && !exitloop);

	if (timeout == 0)
		UART_TX("Slave timeout." _EOS);
}

/**
 @brief Set up water quality sensor and do initial printout.

 @return none
 **/
void CN0428_Setup(void)
{
	/* Do initial read of water quality board transmit buffer */
	if (!any_sensor_inited()) {
		UART_TX("Error: No board found!" _EOS);
		return;
	}

	uint8_t i = 0;
	uint8_t sensor_enabled = 0;
	do {
		if (detected_sensors[i]) {
			eSensorResult = pGasSensor->openWithAddr(
						sensor_addresses[i]);
			flushBuff(TXbuff, sizeof(TXbuff));
			if (eSensorResult != SENSOR_ERROR_NONE) {
				sprintf((char*) TXbuff,
					"Error opening sensor %d!%s",
					(i + 1), _EOS);
			} else {
				sprintf((char*) TXbuff, "Sensor Site: %d!%s",
					(i + 1), _EOS);
				sensor_enabled = 1;
			}
			UART_TX((const char*) TXbuff);
		}
		i++;
	} while (i < NUM_SENSORS && !sensor_enabled);

	CN0428_RdSensorBuff(0); /* Print initial contents of slave buffer */
}

/**
 @brief Switch Water Quality Sensors.

 @param siteNum - Number of site to switch to (1 to 4)

 @return none
 **/
void CN0428_SwitchSensor(char siteNum)
{
	uint8_t success = 0;
	tempChar = siteNum;
	UART_TX_DIR("Switching to site " _EOS);
	UART_TX((const char *) &tempChar);
	for (uint8_t i = 0; i < NUM_SENSORS; i++) {
		if (siteNum == i + 49 && detected_sensors[i]) {
			pGasSensor->close();
			delay_us(100);
			eSensorResult = pGasSensor->openWithAddr(
						sensor_addresses[i]);
			delay_us(100);
			if(eSensorResult == SENSOR_ERROR_NONE)
				success = 1;
		}
	}
	if(!success)
		UART_TX("Invalid Site. Not switched." _EOS);
	else
		CN0428_RdSensorBuff(0); /* Print contents of slave buffer */
}

/**
 @brief Main Water Quality Command Loop.

 @return none
 **/
void CN0428_CommandLoop(void)
{
	uint8_t tempError;
	uint8_t cmdLen;

	/* Command Received. Check if we need to switch sensor, otherwise send command to sensor board. */
	cmdInString[sizeof(cmdInString) - 1] = 0; /* ensure strlen is bounded */
	cmdLen = strlen((const char*) cmdInString);
	if (cmdLen == 0) {
		memset(cmdInString, 0, cmdLen + 1);
	} else if (strncmp((const char *) cmdInString, "switchsensor", 12)
		   == 0) {
		tempChar = cmdInString[cmdLen - 1];
		CN0428_SwitchSensor(tempChar);
		memset(cmdInString, 0, cmdLen + 1);
	} else { /* Send UART command to I2C, including return key. */
		cmdInString[cmdLen] = _CR;
		tempError = pGasSensor->I2CReadWrite(WRITE, 0x63, cmdInString,
						     cmdLen + 1);
		if (tempError != SENSOR_ERROR_NONE) {
			UART_TX("Error writing to sensor" _EOS);
			return;
		}
		memset(cmdInString, 0, cmdLen + 1);
		/* poll sensor until slave signals command complete */
		CN0428_RdSensorBuff(1);
	}
}

/*
*	Other Support Functions
*/

/*!
 * @brief      Initializes gas sensor and slave I2C address.
 *
 * @details    Called for each site, so the I2C address is set at run-time based on the site
 * 				the board is plugged into. Slave has a function that sets its I2C address to
 *				new_sensor_address only if the /SS GPIO is set low. This function sets the
 *				gpios and then calls that function.
 */
SENSOR_RESULT SensorInitwithI2CAddr(uint8_t new_sensor_address, uint8_t site)
{
	uint8_t dataPayload[64];
	uint8_t i = 0;

	/* Set target site /CS low and all other sites high */
	for (i = 0; i < 4; i++) {
		if (site == i + 1)
			adi_gpio_SetLow(CS_Port[i], CS_Pin[i]);
		else
			adi_gpio_SetHigh(CS_Port[i], CS_Pin[i]);
		delay_ms(50);
	}

	eSensorResult = pGasSensor->SetI2CAddr(new_sensor_address);
	if (eSensorResult == SENSOR_ERROR_NONE)
		sprintf((char*) &dataPayload,
			" Gas Sensor in site %d initialized successfully with address 0x%02X!%s",
			site, new_sensor_address, _EOS);
	else if (eSensorResult == SENSOR_ERROR_PH)
		/* SENSOR_ERROR_PH signal used to identify that a water quality probe was found */
		sprintf((char*) &dataPayload,
			" Water Quality Sensor in site %d initialized successfully. Address 0x%02X!%s",
			site, new_sensor_address, _EOS);
	else
		sprintf((char*) &dataPayload,
			" Sensor in site %d: 0x%02X initialization error. Address 0x%02X!%s",
			site, (unsigned int) eSensorResult,
			new_sensor_address, _EOS);
	UART_TX((const char*) dataPayload);
	pGasSensor->close();

	return eSensorResult;
}

/*!
 * @brief      Returns array of initialization states for all sensor locations
 *
 * @details
 */
bool any_sensor_inited()
{
	return !!memcmp(detected_sensors, no_sensors, NUM_SENSORS);
}

/* General Delay milliseconds */
void delay_ms(uint32_t mSec)
{
	uint32_t loop = CYCLES_PER_MS * mSec;
	while (loop > 0)
		loop--;
}

/* General Delay microseconds */
void delay_us(uint32_t uSec)
{
	uint32_t loop = CYCLES_PER_US * uSec;
	while (loop > 0)
		loop--;
}

/* Flush any buffer */
void flushBuff(uint8_t *buff, uint16_t len)
{
	memset(buff, 0, len);
}

uint8_t *int2binString(int32_t a, uint8_t *buff, uint8_t bufSize)
{
	buff += (bufSize - 1);

	for (int i = 31; i >= 0; i--) {
		*buff-- = (a & 1) + '0';

		a >>= 1;
	}

	return buff;
}

double Ieee754ConvertToDouble(uint8_t s[32])
{
	double f;
	int16_t sign, exponent;
	uint32_t mantissa;
	int16_t i;

	sign = s[0] - '0';

	exponent = 0;
	for (i = 1; i <= 8; i++)
		exponent = exponent * 2 + (s[i] - '0');

	exponent -= 127;

	if (exponent > -127) {
		mantissa = 1;
		exponent -= 23;
	} else {
		mantissa = 0;
		exponent = -126;
		exponent -= 23;
	}

	for (i = 9; i <= 31; i++)
		mantissa = mantissa * 2 + (s[i] - '0');

	f = mantissa;

	while (exponent > 0)
		f *= 2, exponent--;

	while (exponent < 0)
		f /= 2, exponent++;

	if (sign)
		f = -f;

	return f;
}
