/**
******************************************************************************
*   @file     ADuCM3029_demo_cn0428_cn0429.cpp
*   @brief    Main project source file
*   @version  V0.1
*   @author   ADI
*
*******************************************************************************
*
*Copyright 2015-2018(c) Analog Devices, Inc.
*
*All rights reserved.
*
*Redistribution and use in source and binary forms, with or without modification,
*are permitted provided that the following conditions are met:
*    - Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the following disclaimer.
*    - Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the following disclaimer in
*      the documentation and/or other materials provided with the
*      distribution.
*    - Neither the name of Analog Devices, Inc. nor the names of its
*      contributors may be used to endorse or promote products derived
*      from this software without specific prior written permission.
*    - The use of this software may or may not infringe the patent rights
*      of one or more patent holders.  This license does not release you
*      from the requirement that you obtain separate licenses from these
*      patent holders to use this software.
*    - Use of the software either in source or binary form, must be run
*      on or directly connected to an Analog Devices Inc. component.
*
*THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
*PARTICULAR PURPOSE ARE DISCLAIMED.
*
*IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
*RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
*BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
**/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "ADuCM3029_demo_cn0428_cn0429.h"
#include "adi_m355_gas_sensor.h"
#include <stdio.h>
#include <string.h>
extern "C"{
#include "RingBuffer.h"
#include "cli.h"
}

using namespace adi_sensor_swpack;

/* ================================================== */
/* 	SYSTEM VARIABLES - handles, memory, error flags	  */
/* ================================================== */
ADI_UART_RESULT		  eUartResult;
ADI_GPIO_RESULT 	  eGpioResult;
ADI_PWR_RESULT        ePwrResult;
SENSOR_RESULT   	  eSensorResult;

/* Memory for the GPIO driver */
uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE] = {0};

/* Handle for the UART device. */
ADI_UART_HANDLE hDevice;

/* Memory for the UART driver. */
static uint8_t UartDeviceMem[ADI_UART_MEMORY_SIZE];

/* ================================================== */
/* 						VARIABLES					  */
/* ================================================== */
const ADI_GPIO_PORT CS_Port [4] = {ADI_GPIO_PORT0, ADI_GPIO_PORT1, ADI_GPIO_PORT2, ADI_GPIO_PORT0};		// Store Chip Select GPIO Port address
const ADI_GPIO_DATA CS_Pin [4] = {ADI_GPIO_PIN_8, ADI_GPIO_PIN_11, ADI_GPIO_PIN_1, ADI_GPIO_PIN_9};		// Store Chip Select GPIO Pin address
const ADI_GPIO_PORT INT_Port [4] = {ADI_GPIO_PORT0, ADI_GPIO_PORT0, ADI_GPIO_PORT1, ADI_GPIO_PORT1};	// Store Interrupt GPIO Port address
const ADI_GPIO_DATA INT_Pin [4] = {ADI_GPIO_PIN_15, ADI_GPIO_PIN_13, ADI_GPIO_PIN_12, ADI_GPIO_PIN_14};	// Store Interrupt GPIO Pin address

/* Ringbuffer structure */
struct RingBuf RX, TX;

const uint8_t M355_Demo_Msg[64]		= "-------- M355 Gas Sensing Demo --------";
const uint8_t M355_Demo_Msg_Wq[64]	= "-------- M355 Water Quality Demo --------";

uint8_t 			TXbuff[256]	= {0};		// UART transmit buffer
uint8_t 			gBuff[64]	= {0};		// general use buffer
volatile uint8_t	TXcompleteFlag = 1;		// indicates UART TX complete
uint8_t				RXchar = 0;				// character received over UART (user input)
uint8_t				cmdInString[64] = "";	// holds incoming user command
uint8_t				cmdInCnt = 0;			// counts incoming bytes over UART

M355_GAS			m355_gas_sensor;
Gas_Reading			*pGasSensor = &m355_gas_sensor;

uint8_t rcal = 0;
uint8_t pulseResultBuffer [6144] = {0};		// buffer holding pulse test result
uint8_t EISResponseBuff[1024] = {0};		// buffer holding EIS results

uint8_t *Slave_Rx_Buffer = pulseResultBuffer; //borrow this memory space for water quality results
uint16_t Slave_Rx_Index = 0;
uint8_t UART_Temp_Buffer [64] = {0};
uint8_t UART_Temp_Buffer_Index = 0;
uint8_t initialcycle = 1;

const char* rtia[] 	= { "0R", "200R", "1k", "2k", "3k", "4k", "6k", "8k", "10k", "12k", "16k", "20k",
						"24k", "30k", "32k", "40k", "48k", "64k", "85k", "96k", "100k", "120k",
						"128k", "160k", "196k", "256k", "512k" };

const char* rload[]	= { "0R", "10R", "30R", "50R", "100R", "1k6", "3k1", "3k6"};

bool S1_detected = false;
bool S2_detected = false;
bool S3_detected = false;
bool S4_detected = false;

uint16_t ledTick = 0;

uint16_t streamTickCnt = 0;		// used for timing the data update rate when in STREAM state

uint16_t streamTickCfg = 1;		// used as configuration parameter for data update rate in STREAM mode
								// default value is 1, i.e. 1 second update rate

eFSM_State FSM_State = INIT;

/* ================================================== */
/* 						FUNCTIONS					  */
/* ================================================== */

/* General Delay */
void delay_ms (uint32_t mSec)
{
	uint32_t loop = 0xA28 * mSec;
	while(loop > 0) loop--;
}

void delay_us (uint32_t uSec)
{
	uint32_t loop = 0x03 * uSec;
	while(loop > 0) loop--;
}

void flushBuff(uint8_t *buff, uint16_t len) { memset(buff, 0, len); }

/* UART Callback handler */
static void uartCallback(void *pAppHandle, uint32_t nEvent, void *pArg)
{
    switch (nEvent)
    {
        case ADI_UART_EVENT_TX_BUFFER_PROCESSED:
        	TXcompleteFlag = 1;
        	 break;
        case ADI_UART_EVENT_RX_BUFFER_PROCESSED:
        	cmdInString[cmdInCnt++] = RXchar;
        	if (cmdInCnt >= 64) //Prevent buffer overflow
			{
				cmdInCnt = 0;
        		cmdReceived = 1;
			}
			else if (cmdInString[cmdInCnt - 1] == _CR)   //check for carriage return
			{
				cmdInString[cmdInCnt - 1] = '\0'; // end of string indicator
				cmdInCnt = 0;
				cmdReceived = 1;
			}
        	adi_uart_SubmitRxBuffer(hDevice, &RXchar, 1, 0u);
            break;

        default:
            break;
    }
}

/* UART initialization */
ADI_UART_RESULT UART_Init()
{
    eUartResult = adi_uart_Open(UART_DEVICE_NUM, ADI_UART_DIR_BIDIRECTION, UartDeviceMem, ADI_UART_MEMORY_SIZE, &hDevice);
    if (eUartResult != ADI_UART_SUCCESS)
	{
    	DEBUG_MESSAGE("Failed to open the UART device");
		return eUartResult;
	}

    eUartResult = adi_uart_SetConfiguration(hDevice, ADI_UART_NO_PARITY, ADI_UART_ONE_AND_HALF_TWO_STOPBITS, ADI_UART_WORDLEN_8BITS);
    if (eUartResult != ADI_UART_SUCCESS)
	{
    	DEBUG_MESSAGE("Failed to configure UART");
		return eUartResult;
	}


	eUartResult = adi_uart_ConfigBaudRate(hDevice,
												  UART_DIV_C_115200,
												  UART_DIV_M_115200,
												  UART_DIV_N_115200,
												  UART_OSR_115200);
    if (eUartResult != ADI_UART_SUCCESS)
	{
    	DEBUG_MESSAGE("Baudrate configuration failed");
		return eUartResult;
	}

    eUartResult = adi_uart_RegisterCallback(hDevice, uartCallback, NULL);
    if (eUartResult != ADI_UART_SUCCESS)
	{
    	DEBUG_MESSAGE("Call back registration failed");
		return eUartResult;
	}

    eUartResult = adi_uart_SubmitRxBuffer(hDevice, &RXchar, 1, 0u);
    if (eUartResult != ADI_UART_SUCCESS) return eUartResult;

    return eUartResult;
}

/* GPIO initialization */
ADI_GPIO_RESULT GPIO_Init()
{
	uint8_t i = 0;

	eGpioResult = adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE);

	//ENABLE CS as GPIO for address setting and set high by default
	for(i = 0; i<4; i++){
		eGpioResult = adi_gpio_OutputEnable(CS_Port[i], CS_Pin[i], true);	// SS signal output enable
		eGpioResult = adi_gpio_SetHigh(CS_Port[i], CS_Pin[i]);				// Set SS signal high
	}

	// configure on-board LEDs
	/*eGpioResult = adi_gpio_OutputEnable(ADI_GPIO_PORT1, ADI_GPIO_PIN_15, true);		// Blue LED output enable
	eGpioResult = adi_gpio_OutputEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_0, true);		// Green LED output enable*/

    return eGpioResult;
}

/*!
 * @brief      UART Transmit function
 *
 * @details    Including CR and LF characters at the end of each transmission and timeout in case
 * 			   the transmission is not successful.
 */
GResult UART_TX(const char *cmd)
{
	GResult result = Failure;

	uint8_t crlf[2] = {0x0D, 0x0A};
	uint16_t tx_bytes = 0;
	uint32_t timeout = 0x00989680;	// 10*10^6

	uint16_t len = strlen((char*)cmd);

	while(tx_bytes != len)
	{
		while(TXcompleteFlag == 0)
		{
			timeout--;
			if (timeout == 0) {
				break;	// Timeout!!!
			}
		}
		TXcompleteFlag = 0;
		if ((adi_uart_SubmitTxBuffer(hDevice, (void*)cmd++, 1u, 0u)) != ADI_UART_SUCCESS) return Failure;
			else result = Success;
		tx_bytes++;
	}


	timeout = 0x00989680;	// 10*10^6
	while(TXcompleteFlag == 0)
	{
		timeout--;
		if (timeout == 0) {
			break;	// Timeout!!!
		}
	}
	TXcompleteFlag = 0;
	if ((adi_uart_SubmitTxBuffer(hDevice, (char*)&crlf[0], 1u, 0u)) != ADI_UART_SUCCESS) return Failure;
		else result = Success;


	timeout = 0x00989680;	// 10*10^6
	while(TXcompleteFlag == 0)
	{
		timeout--;
		if (timeout == 0) {
			break;	// Timeout!!!
		}
	}
	TXcompleteFlag = 0;
	if ((adi_uart_SubmitTxBuffer(hDevice, (char*)&crlf[1], 1u, 0u)) != ADI_UART_SUCCESS) return Failure;
		else result = Success;

	delay_ms(2); // temporary

	return result;
}

/*!
 * @brief      Direct UART Transmit function
 *
 * @details    Transmit via UART without CR and LF characters at the end of each transmission
 */
GResult UART_TX_DIR(const char *cmd)
{
	GResult result = Failure;

	uint16_t tx_bytes = 0;
	uint32_t timeout = 0x00989680;	// 10*10^6

	uint16_t len = strlen((char*)cmd);

	while(tx_bytes != len)
	{
		while(TXcompleteFlag == 0)
		{
			timeout--;
			if (timeout == 0) {
				break;	// Timeout!!!
			}
		}
		TXcompleteFlag = 0;
		if ((adi_uart_SubmitTxBuffer(hDevice, (void*)cmd++, 1u, 0u)) != ADI_UART_SUCCESS) return Failure;
			else result = Success;
		tx_bytes++;
	}


	timeout = 0x00989680;	// 10*10^6
	while(TXcompleteFlag == 0)
	{
		timeout--;
		if (timeout == 0) {
			break;	// Timeout!!!
		}
	}
//	TXcompleteFlag = 0;
//	if ((adi_uart_SubmitTxBuffer(hDevice, (char*)&crlf[0], 1u, 0u)) != ADI_UART_SUCCESS) return Failure;
//		else result = Success;
//
//
//	timeout = 0x00989680;	// 10*10^6
//	while(TXcompleteFlag == 0)
//	{
//		timeout--;
//		if (timeout == 0) {
//			break;	// Timeout!!!
//		}
//	}
//	TXcompleteFlag = 0;
//	if ((adi_uart_SubmitTxBuffer(hDevice, (char*)&crlf[1], 1u, 0u)) != ADI_UART_SUCCESS) return Failure;
//		else result = Success;

	//delay_ms(2); // temporary

	return result;
}

/*********************************************************************

    Function:       check_string

    Description:    Function looks for string (string_to_search) with
    				length string_len within the ring buffer. If this
    				string is found, function returns Success and
    				deletes this string from ring buffer. Otherwise
    				returns Failure.

*********************************************************************/
GResult check_string(uint8_t *string_to_search, uint16_t string_len)
{
	uint8_t read_char = 0;
	uint16_t  match_success = 0, loop = 0;

	// Find first character of search string in read buffer
	do
	{
		read_char = GetChar();
	    loop++;
	} while((read_char != string_to_search[0])&&(loop <= MAX_BUFLEN));                     // 35 is the size of the RX buffer, if it's all read out and no match then reset

	if(loop >= MAX_BUFLEN)
	{
		return Failure;                                                              // no match
	}
	else                                                                          // Got a match, check if its as expected
	{
		match_success+=1;                                                           // if we got here then there was a match in the first character

		for(loop = 1;loop < string_len; loop++)                                    // Loop through the match string and start at 1 as already found first character
	    {
			read_char = GetChar();
			if(read_char == string_to_search[loop])
			{
				match_success += 1;
			}
	    }
	    if(match_success == string_len)                                            // have all chars matched ?
	    {
	    	DeleteChar(string_len);												// delete command from the ring buffer
	    	return Success;
	    }
	    else
	    {
	    	Empty_Ring();
	    	return Failure;
	    }
	}
}

/*!
 * @brief      Initializes gas sensor and slave I2C address
 *
 * @details    Called for each site, so the I2C address is set at run-time based on the site
 * 				the board is plugged into. Slave has a function that sets its I2C address to
 *				new_sensor_address only if the /SS GPIO is set low. This function sets the
 *				gpios and then calls that function.
 */
SENSOR_RESULT SensorInitwithI2CAddr(uint8_t new_sensor_address, uint8_t site)
{
	uint8_t dataPayload	[64];
	uint8_t i = 0;

	//	Set target site /CS low and all other sites high
	for(i = 0; i<4; i++){
		if(site == i + 1){
			eGpioResult = adi_gpio_SetLow(CS_Port[i], CS_Pin[i]);				// Set Sensor CS Low if correct site
		}
		else{
			eGpioResult = adi_gpio_SetHigh(CS_Port[i], CS_Pin[i]);				// Otherwise set Sensor CS High
		}
		delay_ms(50);
	}

	eSensorResult = pGasSensor->SetI2CAddr(new_sensor_address);
	if (eSensorResult == SENSOR_ERROR_NONE)
	{
		//DEBUG_MESSAGE("Sensor: 0x%x initialized successfully!\r\n", sensor_address);
		sprintf((char*)&dataPayload, " Gas Sensor in site %d initialized successfully with address 0x%02X!%s", site, new_sensor_address, _EOS);
	}
	else if (eSensorResult == SENSOR_ERROR_PH) //Used to identify that a water quality probe was found
	{
		//DEBUG_MESSAGE("Sensor: 0x%x initialization error!\r\n", sensor_address);
		sprintf((char*)&dataPayload, " Water Quality Sensor in site %d initialized successfully. Address 0x%02X!%s", site, new_sensor_address, _EOS);
	}
	else
	{
		//DEBUG_MESSAGE("Sensor: 0x%x initialization error!\r\n", sensor_address);
		sprintf((char*)&dataPayload, " Sensor in site %d: 0x%02X initialization error. Address 0x%02X!%s", site, (unsigned int) eSensorResult, new_sensor_address, _EOS);
	}
	UART_TX((const char*)dataPayload);
	pGasSensor->close();

	return eSensorResult;
}

/*!
 * @brief      Read sensor config
 *
 * @details    Reads sensor config of the sensor with address specified in input parameter
 */
SENSOR_RESULT GetSensorCfg(uint8_t sensor_address)
{
	uint8_t pBuff[8] = {0};
	int16_t sigVal = 0;
	uint16_t unsigVal = 0;
	uint32_t unsiglVal = 0;

	eSensorResult = pGasSensor->openWithAddr(sensor_address);
	if(eSensorResult != SENSOR_ERROR_NONE) return eSensorResult;
	delay_ms(5);

	if (sensor_address == 0x0A) sprintf ((char*)&TXbuff, "Config of sensor on site 1:%s", _EOS);
	if (sensor_address == 0x0B) sprintf ((char*)&TXbuff, "Config of sensor on site 2:%s", _EOS);
	if (sensor_address == 0x0C) sprintf ((char*)&TXbuff, "Config of sensor on site 3:%s", _EOS);
	if (sensor_address == 0x0D) sprintf ((char*)&TXbuff, "Config of sensor on site 4:%s", _EOS);
	UART_TX((const char*)TXbuff);
	delay_ms(5);

	flushBuff(pBuff, sizeof(pBuff));
	eSensorResult = pGasSensor->ReadTIAGain(pBuff);
	if(eSensorResult != SENSOR_ERROR_NONE) return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	sprintf((char*)TXbuff,"RTIA = %s ohm%s",rtia[pBuff[0]], _EOS);
	UART_TX((const char*)TXbuff);
	delay_ms(5);

	flushBuff(pBuff, sizeof(pBuff));
	eSensorResult = pGasSensor->ReadRload(pBuff);
	if(eSensorResult != SENSOR_ERROR_NONE) return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	sprintf((char*)TXbuff,"Rload = %s ohm%s",rload[pBuff[0]], _EOS);
	UART_TX((const char*)TXbuff);
	delay_ms(5);

	eSensorResult = pGasSensor->ReadSensorBias(&sigVal);
	if(eSensorResult != SENSOR_ERROR_NONE) return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	sprintf((char*)TXbuff,"Vbias = %d mV%s",sigVal, _EOS);
	UART_TX((const char*)TXbuff);
	delay_ms(5);

	eSensorResult = pGasSensor->ReadSensorSensitivity(&unsiglVal);
	if(eSensorResult != SENSOR_ERROR_NONE) return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	sprintf((char*)TXbuff,"Sensitivity = %.2f nA/ppm%s", (unsiglVal / 100.0), _EOS);
	UART_TX((const char*)TXbuff);
	delay_ms(5);

	eSensorResult = pGasSensor->ReadMeasurementTime(&unsigVal);
	if(eSensorResult != SENSOR_ERROR_NONE) return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	sprintf((char*)TXbuff,"Measurement Time = %d msec%s", unsigVal, _EOS);
	UART_TX((const char*)TXbuff);
	delay_ms(5);

    // === NOT USED IN THIS VERSION! ===
	/*flushBuff(pBuff, sizeof(pBuff));
	eSensorResult = pGasSensor->ReadTempCompCfg(pBuff);
	if(eSensorResult != SENSOR_ERROR_NONE) return eSensorResult;
	flushBuff(TXbuff, sizeof(TXbuff));
	if (pBuff[0] == 0) sprintf((char*)TXbuff,"Temp Comp = disabled%s", _EOS);
	if (pBuff[0] == 1) sprintf((char*)TXbuff,"Temp Comp = enabled%s", _EOS);
	UART_TX((const char*)TXbuff);
	delay_ms(5);*/

	eSensorResult = pGasSensor->close();
    delay_ms(5);

	return eSensorResult;
}

/*!
 * @brief      Reads averaged ppb value from gas sensor
 *
 * @details    Reads sensor data utilizing custom gas sensor I2C library and sends this data over UART.
 */
int32_t SensorReadoutPPB(uint8_t sensor_address)
{
	int32_t PPB_Gas_Reading = 0;
	eSensorResult = pGasSensor->openWithAddr(sensor_address);
	eSensorResult = pGasSensor->ReadDataPPB(&PPB_Gas_Reading);
	eSensorResult = pGasSensor->close();
    delay_ms(10);         
	return PPB_Gas_Reading;
}

int32_t ExtractCfgVal()
{
	uint8_t buffer[16] = {0};
	uint8_t read_char;
	int i = 0;
	for (i = 0; i < 16; i++)
	{
		read_char = GetChar();
		if (read_char == '\0') break;
		else buffer[i] = read_char;
	}
	DeleteChar(i + 1);
	return strtol((const char*)buffer, NULL, 10);
}

uint8_t *int2binString(int32_t a, uint8_t *buff, uint8_t bufSize) {
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

	if (exponent > -127)
	{
		mantissa = 1;
		exponent -= 23;
	}
	else
	{
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

/* ================================================== */
/* 					 State Machine					  */
/* ================================================== */
void DataDisplayFSM(void)
{
	uint8_t 	readChar = 0;	// variable holding address of selected sensor
	uint8_t 	cmdResponseBuff[256] = {0};	// buffer holding sensor response to commands
	uint8_t		tempBuff[40] = {0};			// buffer for EIS results parsing
	double		EISpartialresult[8] = {0};	// array of partial results (one line) of EIS
	int32_t		EISpartialDFTresult[8] = {0};	// array of partial results (DFT impedance) of EIS
	uint32_t	EISresults[192] = {0};		// array of EIS results
	int16_t 	sigVal = 0;		// temporary variable for signed value
	uint16_t 	unsigVal = 0;	// temporary variable for unsigned value
	uint32_t	unsiglVal = 0;	// temporary variable for long unsigned value
    char		tempChar = 0;   // temporary variable for char
	bool 		exitloop = false;

	uint8_t pulseDuration, pulseAmplitude;	// Pulse test parameters

	uint8_t tempError;
	uint8_t numbytes;
	uint8_t cmdLen;
	uint16_t timeout;

	switch(FSM_State)
	{
		case INIT:
			FSM_State = COMMAND;

			tempError = SensorInitwithI2CAddr(ADI_CFG_I2C_SENSOR1_ADDR, 1);
			if (tempError == SENSOR_ERROR_NONE) {
				S1_detected = true;
			}
			else if (tempError == SENSOR_ERROR_PH){	//Water Quality Board installed
				S1_detected = true;
				FSM_State = WATER;
			}

			tempError = SensorInitwithI2CAddr(ADI_CFG_I2C_SENSOR2_ADDR, 2);
			if (tempError == SENSOR_ERROR_NONE) {
				S2_detected = true;
			}
			else if (tempError == SENSOR_ERROR_PH){	//Water Quality Board installed
				S2_detected = true;
				FSM_State = WATER;
			}

			tempError = SensorInitwithI2CAddr(ADI_CFG_I2C_SENSOR3_ADDR, 3);
			if (tempError == SENSOR_ERROR_NONE) {
				S3_detected = true;
			}
			else if (tempError == SENSOR_ERROR_PH){	//Water Quality Board installed
				S3_detected = true;
				FSM_State = WATER;
			}

			tempError = SensorInitwithI2CAddr(ADI_CFG_I2C_SENSOR4_ADDR, 4);
			if (tempError == SENSOR_ERROR_NONE) {
				S4_detected = true;
			}
			else if (tempError == SENSOR_ERROR_PH){	//Water Quality Board installed
				S4_detected = true;
				FSM_State = WATER;
			}

			if(S1_detected||S2_detected||S3_detected||S4_detected) {
				UART_TX("----------------------------------------------" _EOS);
				cmdReceived = 1; //Ensure welcome message printout
			}
			else UART_TX("No Sensor Board Found! Please check connections and reset ADICUP3029." _EOS);
			break;

		case COMMAND:
			if (check_string((uint8_t *)"PRESENCE_CHECK", 14))
			{
				if (S1_detected) UART_TX("Sensor detected on site 1" _EOS);
				if (S2_detected) UART_TX("Sensor detected on site 2" _EOS);
				if (S3_detected) UART_TX("Sensor detected on site 3" _EOS);
				if (S4_detected) UART_TX("Sensor detected on site 4" _EOS);
			}

			if (check_string((uint8_t *)"READ_CFGS", 9))
			{
				if (S1_detected) GetSensorCfg(ADI_CFG_I2C_SENSOR1_ADDR);
				delay_ms(10);
				if (S2_detected) GetSensorCfg(ADI_CFG_I2C_SENSOR2_ADDR);
				delay_ms(10);
				if (S3_detected) GetSensorCfg(ADI_CFG_I2C_SENSOR3_ADDR);
				delay_ms(10);
				if (S4_detected) GetSensorCfg(ADI_CFG_I2C_SENSOR4_ADDR);
				delay_ms(10);
			}

			if (check_string((uint8_t *)"READ_TEMP", 9))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->ReadTemperature(&sigVal);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Temperature = %2.1f degC%s", (sigVal/100.0), _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"READ_HUM", 8))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->ReadHumidity(&sigVal);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Humidity = %2.1f %%RH%s", (sigVal/100.0), _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"CFG_MEASTIME", 12))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				unsigVal = (uint16_t)ExtractCfgVal();
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->SetMeasurementTime(unsigVal);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Measurement time set to %d msec%s", unsigVal, _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"READ_MEASTIME", 13))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->ReadMeasurementTime(&unsigVal);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Measurement time is currently %d msec%s", unsigVal, _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"STARTMEAS", 9))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->StartMeasurements();
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Measurement started%s", _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"STOPMEAS", 8))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->StopMeasurements();
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Measurement stopped%s", _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"CFG_RTIA", 8))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				unsigVal = (uint16_t)ExtractCfgVal();
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->SetTIAGain(unsigVal);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"TIA gain resistor set to %s ohm%s", rtia[unsigVal], _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"CFG_RLOAD", 9))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				unsigVal = (uint16_t)ExtractCfgVal();
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->SetRload(unsigVal);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Load resistor set to %s ohm%s", rload[unsigVal], _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"CFG_VBIAS", 9))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				sigVal = ExtractCfgVal();
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->SetSensorBias(sigVal);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Sensor bias set to %d mV%s", sigVal, _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"CFG_SENS", 8))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				unsiglVal = (uint32_t)ExtractCfgVal();
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->SetSensorSensitivity(unsiglVal);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Sensor sensitivity set to %2.2f nA/ppm%s", (unsiglVal/100.0), _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			// === NOT USED IN THIS VERSION! ===
			/*if (check_string((uint8_t *)"CFG_TEMPCOMP", 12))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				unsigVal = (uint16_t)ExtractCfgVal();
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->ConfigureTempComp((uint8_t)unsigVal);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					if (unsigVal == 0)
						sprintf((char*)TXbuff,"Temperature compensation disabled%s", _EOS);
					if (unsigVal == 1)
						sprintf((char*)TXbuff,"Temperature compensation enabled%s", _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}*/

			if (check_string((uint8_t *)"RUN_EIS", 7))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->RunEISMeasurement();
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"EIS test started%s", _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"READ_EIS", 8))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				flushBuff(TXbuff, sizeof(TXbuff));
				memset(EISResponseBuff, 0, sizeof(EISResponseBuff));
				memset(EISresults, 0, sizeof(EISresults));
				eSensorResult = pGasSensor->ReadEISResults(EISResponseBuff);

				for (int i = 0; i < 36; i++) {
					tempBuff[0] = EISResponseBuff[0+(i*4)];
					tempBuff[1] = EISResponseBuff[1+(i*4)];
					tempBuff[2] = EISResponseBuff[2+(i*4)];
					tempBuff[3] = EISResponseBuff[3+(i*4)];
					memcpy(&EISresults[i], tempBuff, 4);
				}
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Frequency, Magnitude, Phase%s", _EOS);
					UART_TX((const char*)TXbuff);
					for (int j = 0; j < 12; j++)
					{
						int2binString(EISresults[0+(j*3)], tempBuff, 32);
						EISpartialresult[0] = Ieee754ConvertToDouble(tempBuff);
						int2binString(EISresults[1+(j*3)], tempBuff, 32);
						EISpartialresult[1] = Ieee754ConvertToDouble(tempBuff);
						int2binString(EISresults[2+(j*3)], tempBuff, 32);
						EISpartialresult[2] = Ieee754ConvertToDouble(tempBuff);

						flushBuff(TXbuff, sizeof(TXbuff));
						sprintf((char*)TXbuff,"%f, %f, %f%s", EISpartialresult[0], EISpartialresult[1], EISpartialresult[2],  _EOS);
						UART_TX((const char*)TXbuff);
					}
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"READ_FULL_EIS", 13))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				flushBuff(TXbuff, sizeof(TXbuff));
				memset(EISResponseBuff, 0, sizeof(EISResponseBuff));
				memset(EISresults, 0, sizeof(EISresults));
				eSensorResult = pGasSensor->ReadEISResultsFull(EISResponseBuff);

				for (int i = 0; i < 180; i++) {
					tempBuff[0] = EISResponseBuff[0+(i*4)];
					tempBuff[1] = EISResponseBuff[1+(i*4)];
					tempBuff[2] = EISResponseBuff[2+(i*4)];
					tempBuff[3] = EISResponseBuff[3+(i*4)];
					memcpy(&EISresults[i], tempBuff, 4);
				}
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Frequency, Rload+Rsens_real, Rload+Rsens_img, Rload_real, Rload_imag, Rcal_real, Rcal_imag, Mag_Rsens+Rload, Mag_Rload, Mag_Rcal, Mag_Rsens, MAG, PHASE%s", _EOS);
					UART_TX((const char*)TXbuff);
					for (int j = 0; j < 12; j++)
					{
						int2binString(EISresults[0+(j*15)], tempBuff, 32);
						EISpartialresult[0] = Ieee754ConvertToDouble(tempBuff);

						memcpy(&EISpartialDFTresult[0], (int32_t*)&EISresults[1+(j*15)], sizeof(EISresults[1+(j*15)]));
						memcpy(&EISpartialDFTresult[1], (int32_t*)&EISresults[2+(j*15)], sizeof(EISresults[2+(j*15)]));
						memcpy(&EISpartialDFTresult[2], (int32_t*)&EISresults[3+(j*15)], sizeof(EISresults[3+(j*15)]));
						memcpy(&EISpartialDFTresult[3], (int32_t*)&EISresults[4+(j*15)], sizeof(EISresults[4+(j*15)]));
						memcpy(&EISpartialDFTresult[4], (int32_t*)&EISresults[5+(j*15)], sizeof(EISresults[5+(j*15)]));
						memcpy(&EISpartialDFTresult[5], (int32_t*)&EISresults[6+(j*15)], sizeof(EISresults[6+(j*15)]));

						int2binString(EISresults[7+(j*15)], tempBuff, 32);
						EISpartialresult[1] = Ieee754ConvertToDouble(tempBuff);
						int2binString(EISresults[8+(j*15)], tempBuff, 32);
						EISpartialresult[2] = Ieee754ConvertToDouble(tempBuff);
						int2binString(EISresults[9+(j*15)], tempBuff, 32);
						EISpartialresult[3] = Ieee754ConvertToDouble(tempBuff);
						int2binString(EISresults[10+(j*15)], tempBuff, 32);
						EISpartialresult[4] = Ieee754ConvertToDouble(tempBuff);
						int2binString(EISresults[11+(j*15)], tempBuff, 32);
						EISpartialresult[5] = Ieee754ConvertToDouble(tempBuff);
						int2binString(EISresults[12+(j*15)], tempBuff, 32);
						EISpartialresult[6] = Ieee754ConvertToDouble(tempBuff);

						flushBuff(TXbuff, sizeof(TXbuff));
						sprintf((char*)TXbuff,"%f, %ld, %ld, %ld, %ld, %ld, %ld, %f, %f, %f, %f, %f, %f%s", EISpartialresult[0], EISpartialDFTresult[0], EISpartialDFTresult[1], EISpartialDFTresult[2], EISpartialDFTresult[3], EISpartialDFTresult[4], EISpartialDFTresult[5], EISpartialresult[1], EISpartialresult[2], EISpartialresult[3], EISpartialresult[4], EISpartialresult[5], EISpartialresult[6],  _EOS);
						UART_TX((const char*)TXbuff);
					}
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"READ_RCAL", 9))
			{
				readChar = GetChar(); DeleteChar(1);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				flushBuff(TXbuff, sizeof(TXbuff));
				memset(cmdResponseBuff, 0, sizeof(cmdResponseBuff));
				eSensorResult = pGasSensor->Read200RCal(cmdResponseBuff);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Rcal = %d ohm%s",cmdResponseBuff[0], _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"RUN_PULSE", 9))
			{
				readChar = GetChar();
				pulseAmplitude = GetChar();	// read pulseAmplitude as hex
				pulseDuration = GetChar();	// read pulseDuration as hex
				DeleteChar(3);
				eSensorResult = pGasSensor->openWithAddr(readChar);

				memset(cmdResponseBuff, 0, sizeof(cmdResponseBuff));
				eSensorResult = pGasSensor->Read200RCal(cmdResponseBuff);
				rcal = cmdResponseBuff[0];

				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->RunPulseTest(pulseAmplitude, pulseDuration);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else {
					sprintf((char*)TXbuff,"Pulse test started%s", _EOS);
					UART_TX((const char*)TXbuff);
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"PULSE_RESULT", 12))
			{
				readChar = GetChar();
				pulseAmplitude = GetChar();	// read pulseAmplitude as hex
				pulseDuration = GetChar();	// read pulseDuration as hex
				DeleteChar(3);
				eSensorResult = pGasSensor->openWithAddr(readChar);
				flushBuff(TXbuff, sizeof(TXbuff));
				eSensorResult = pGasSensor->ReadPulseTestResults(&pulseResultBuffer[0], pulseAmplitude, pulseDuration);
				if (eSensorResult != SENSOR_ERROR_NONE) UART_TX("ERROR!" _EOS);
				else
				{
					UART_TX("time [msec], current [uA]" _EOS);
					uint16_t valToSend = (((pulseDuration + 10) * 1000) / 110);
					uint16_t cnt = 0;
					uint16_t cnt2 = 1;
					while (cnt < valToSend) {
						flushBuff(TXbuff, sizeof(TXbuff));
						unsigVal = ((pulseResultBuffer[cnt2] << 8) | pulseResultBuffer[cnt2+1]);
						sprintf((char*)TXbuff,"%d, %.2f%s", ((cnt)*110), ((32768.0 - unsigVal)/65535) * ((1835 * 2 * 1000)/rcal), _EOS);
						UART_TX((const char*)TXbuff);
						cnt++;
						cnt2 += 2;
					}
				}
				pGasSensor->close();
			}

			if (check_string((uint8_t *)"READALLSENSORS", 14))
			{
				flushBuff(TXbuff, sizeof(TXbuff));
				if (S1_detected) strcat((char*)TXbuff, "Sensor 1[ppb], ");
				if (S2_detected) strcat((char*)TXbuff, "Sensor 2[ppb], ");
				if (S3_detected) strcat((char*)TXbuff, "Sensor 3[ppb], ");
				if (S4_detected) strcat((char*)TXbuff, "Sensor 4[ppb], ");
				UART_TX((const char*)TXbuff);
				FSM_State = STREAM_GAS;
			}
			break;

		case STREAM_GAS:
			flushBuff(TXbuff, sizeof(TXbuff));
			if (S1_detected) {
				flushBuff(gBuff, sizeof(gBuff));
				sprintf((char*)gBuff, "%ld, ", SensorReadoutPPB(ADI_CFG_I2C_SENSOR1_ADDR));
				strcat((char*)TXbuff, (char*)gBuff);
			}
			if (S2_detected) {
				flushBuff(gBuff, sizeof(gBuff));
				sprintf((char*)gBuff, "%ld, ", SensorReadoutPPB(ADI_CFG_I2C_SENSOR2_ADDR));
				strcat((char*)TXbuff, (char*)gBuff);
			}
			if (S3_detected) {
				flushBuff(gBuff, sizeof(gBuff));
				sprintf((char*)gBuff, "%ld, ", SensorReadoutPPB(ADI_CFG_I2C_SENSOR3_ADDR));
				strcat((char*)TXbuff, (char*)gBuff);
			}
			if (S4_detected) {
				flushBuff(gBuff, sizeof(gBuff));
				sprintf((char*)gBuff, "%ld, ", SensorReadoutPPB(ADI_CFG_I2C_SENSOR4_ADDR));
				strcat((char*)TXbuff, (char*)gBuff);
			}
			UART_TX((const char*)TXbuff);
			break;

		case WATER:
		{
			if(initialcycle){
			//Do initial read of water quality board transmit buffer
				if (S1_detected) {
					eSensorResult = pGasSensor->openWithAddr(ADI_CFG_I2C_SENSOR1_ADDR);
					if(eSensorResult != SENSOR_ERROR_NONE) UART_TX("Error opening sensor 1!" _EOS);
					else UART_TX("Sensor Site: 1");
				}
				else if (S2_detected) {
					eSensorResult = pGasSensor->openWithAddr(ADI_CFG_I2C_SENSOR2_ADDR);
					if(eSensorResult != SENSOR_ERROR_NONE) UART_TX("Error opening sensor 2!" _EOS);
					else UART_TX("Sensor Site: 2");
				}
				else if (S3_detected) {
					eSensorResult = pGasSensor->openWithAddr(ADI_CFG_I2C_SENSOR3_ADDR);
					if(eSensorResult != SENSOR_ERROR_NONE) UART_TX("Error opening sensor 3!" _EOS);
					else UART_TX("Sensor Site: 3");
				}
				else if (S4_detected) {
					eSensorResult = pGasSensor->openWithAddr(ADI_CFG_I2C_SENSOR4_ADDR);
					if(eSensorResult != SENSOR_ERROR_NONE) UART_TX("Error opening sensor 4!" _EOS);
					else UART_TX("Sensor Site: 1");
				}
				else {
					UART_TX("Error: No board found!" _EOS);
					break;
				}
				numbytes = 0;
				pGasSensor->I2CReadWrite(READ, BYTES_TO_READ, (uint8_t *)&numbytes, 1);//numbytes can be 0 to 255, but there can be more than 255 bytes in slave buffer
				delay_ms(5);
				while(numbytes > 0){
					pGasSensor->I2CReadWrite(READ, 0x63, &Slave_Rx_Buffer[0], numbytes);
					delay_ms(10);
					for(Slave_Rx_Index = 0;Slave_Rx_Index<numbytes;Slave_Rx_Index++){
						tempChar = Slave_Rx_Buffer[Slave_Rx_Index];
						Slave_Rx_Buffer[Slave_Rx_Index] = '\0';
						if(tempChar != '~' && tempChar != '\0'){
							UART_TX_DIR((const char *)&tempChar);
						}
					}
					pGasSensor->I2CReadWrite(READ, BYTES_TO_READ, (uint8_t *)&numbytes, 1);
					delay_ms(5);
				} //end while data available loop
				initialcycle = 0;
			}
			/*******************Streaming data from UART to I2C and back********************/
			while(true){ //main water quality loop
				if (cmdReceived){
					//Command Received. Check if we need to switch sensor, otherwise send command to sensor board.
					cmdReceived = 0;
					cmdLen = strlen((const char*)cmdInString);
					cmdInString[cmdLen] = _CR; //add carriage return back in to send to slave
					if(strncmp((const char *)cmdInString, "switchsensor", 11) == 0){
						tempChar = cmdInString[cmdLen - 1];//character before the enter key
						UART_TX_DIR("Switching to site ");
						UART_TX((const char *)&tempChar);
						if (tempChar == '1' && S1_detected) {
							pGasSensor->close();
							delay_us(100);
							eSensorResult = pGasSensor->openWithAddr(ADI_CFG_I2C_SENSOR1_ADDR);
						}
						else if (tempChar == '2' && S2_detected) {
							pGasSensor->close();
							delay_us(100);
							eSensorResult = pGasSensor->openWithAddr(ADI_CFG_I2C_SENSOR2_ADDR);
						}
						else if (tempChar == '3' && S3_detected) {
							pGasSensor->close();
							delay_us(100);
							eSensorResult = pGasSensor->openWithAddr(ADI_CFG_I2C_SENSOR3_ADDR);
						}
						else if (tempChar == '4' && S4_detected) {
							pGasSensor->close();
							delay_us(100);
							eSensorResult = pGasSensor->openWithAddr(ADI_CFG_I2C_SENSOR4_ADDR);
						}
						else{
							UART_TX("Invalid Site. Not switched." _EOS);
						}
						memset(cmdInString,0,cmdLen+1); //reset all array elements to char(0)
					}
					else{
						//Send UART command to I2C, including return key.
						tempError = pGasSensor->I2CReadWrite(WRITE, 0x63, cmdInString, cmdLen+1);
						if(tempError != SENSOR_ERROR_NONE){
							UART_TX("Error writing to sensor" _EOS);
							break;
						}
						memset(cmdInString,0,cmdLen+1); //reset all array elements to char(0)
						//poll sensor until end of message character is read.
						timeout = 600; //EIS result can take 45s depending on chosen frequencies
						exitloop = false;
						do{					//0.2s loop
							delay_ms(185);
							numbytes = 0;
							pGasSensor->I2CReadWrite(READ, BYTES_TO_READ, (uint8_t *)&numbytes, 1); //numbytes can be 0 to 255, but there can be more than 255 bytes from slave
							delay_ms(5);
							while(numbytes > 0){ //data available
								tempError = pGasSensor->I2CReadWrite(READ, 0x63, &Slave_Rx_Buffer[0], numbytes);
								if(tempError != SENSOR_ERROR_NONE){
									UART_TX("Error reading from sensor" _EOS);
									break;
								}
								delay_ms(10);
								for(Slave_Rx_Index = 0;Slave_Rx_Index<numbytes;Slave_Rx_Index++){
									tempChar = Slave_Rx_Buffer[Slave_Rx_Index];
									Slave_Rx_Buffer[Slave_Rx_Index] = '\0'; //clean buffer
									if(tempChar == '~'){
										exitloop = true;
									}
									else { //end message character received
										UART_TX_DIR((const char *)&tempChar);
									}
								}
								pGasSensor->I2CReadWrite(READ, BYTES_TO_READ, (uint8_t *)&numbytes, 1);
								delay_ms(5);
							}// end data available while loop
							delay_us(100);
							timeout--;
						}while(timeout > 0 && !exitloop);
						if(timeout == 0){
							UART_TX("Slave timeout." _EOS);
						}
					} //end slave command loop
				} //end command received loop
				delay_ms(100); //wait to check the ring buffer for new characters
			} //end while(1) loop
		} //end WATER state
			break;

		default:
			break;
	}




}


/* ================================================== */
/* 						MAIN APP					  */
/* ================================================== */

int main(int argc, char *argv[])
{
	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	
	/* Explicitly disable the watchdog timer */
	*pREG_WDT0_CTL = 0x0u;

	/* Initialize ring buffer */
	Rb_Init();

	if(ADI_PWR_SUCCESS !=(ePwrResult =adi_pwr_Init()))
	{
		DEBUG_RESULT("\n Failed to init pwr", ePwrResult, ADI_PWR_SUCCESS);
		return(ePwrResult);
	}

	if( ADI_PWR_SUCCESS != (ePwrResult = adi_pwr_SetLFClockMux(ADI_CLOCK_MUX_LFCLK_LFXTAL)))	// select LFXTL clock
	{
		DEBUG_RESULT("\n Failed to set the LF Clock Mux", ePwrResult, ADI_PWR_SUCCESS);
		return(ADI_RTC_FAILURE);
	}

	if(ADI_PWR_SUCCESS !=(ePwrResult =adi_pwr_EnableClockSource(ADI_CLOCK_SOURCE_LFXTAL,true)))
	{
		DEBUG_RESULT("\n Failed to enable clock source", ePwrResult, ADI_PWR_SUCCESS);
		return(ePwrResult);
	}

	if(ADI_PWR_SUCCESS !=(ePwrResult =adi_pwr_SetClockDivider(ADI_CLOCK_HCLK,1)))
	{
		DEBUG_RESULT("\n Failed to set clock divider", ePwrResult, ADI_PWR_SUCCESS);
		return(ePwrResult);
	}
	if(ADI_PWR_SUCCESS !=(ePwrResult =adi_pwr_SetClockDivider(ADI_CLOCK_PCLK,1)))
	{
		DEBUG_RESULT("\n Failed to set clock divider", ePwrResult, ADI_PWR_SUCCESS);
		return(ePwrResult);
	}


    /* Initialize GPIO */
    eGpioResult = GPIO_Init();

    /* Initialize UART */
	eUartResult = UART_Init();

	UART_TX("Please wait for sensor startup and initialization:" _EOS);
	delay_ms(10000);	// delay to allow sensors start properly

    while(1)
    {
    	delay_ms(10);

    	// blinking green LED indicates operation
    	/*ledTick++;
    	if(ledTick >= 100)
		{
    		GREEN_LED_TOGGLE;
    		ledTick = 0;
		}*/

    	if(cmdReceived && FSM_State != WATER)
    	{
    		CmdProcess();
    		cmdReceived = 0;
    	}

		if (FSM_State == STREAM_GAS)
		{
			streamTickCnt++;
			if (streamTickCnt >= ((100 * streamTickCfg) - 25)) 	// -25 to account for delays in readout
			{
				DataDisplayFSM();
				streamTickCnt = 0;
			}
		}
		else DataDisplayFSM();
    }

}
/* ================================================== */
/* ================================================== */


