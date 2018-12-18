/*******************************************************************************
 *   @file     ADuCM3029_demo_cn0428_cn0429.cpp
 *   @brief    Main project source file
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
#include "ADuCM3029_demo_cn0428_cn0429.h"
#include "cn0428_cn0429.h"
#include <string.h>

/*
 *	SYSTEM VARIABLES - handles, memory, error flags
 */
ADI_UART_RESULT		eUartResult;
ADI_GPIO_RESULT		eGpioResult;
ADI_PWR_RESULT		ePwrResult;

/* Memory for the GPIO driver */
uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE] = { 0 };

/* Handle for the UART device. */
ADI_UART_HANDLE hDevice;

/* Memory for the UART driver. */
static uint8_t UartDeviceMem[ADI_UART_MEMORY_SIZE];

/*
 *	VARIABLES
 */
/* Store Chip Select GPIO Port address */
extern const ADI_GPIO_PORT CS_Port[4] = { ADI_GPIO_PORT0, ADI_GPIO_PORT1,
					  ADI_GPIO_PORT2, ADI_GPIO_PORT0
					};
/* Store Chip Select GPIO Pin address */
extern const ADI_GPIO_DATA CS_Pin[4] = { ADI_GPIO_PIN_8, ADI_GPIO_PIN_11,
					 ADI_GPIO_PIN_1, ADI_GPIO_PIN_9
				       };
/* Store Interrupt GPIO Port address */
const ADI_GPIO_PORT INT_Port[4] = { ADI_GPIO_PORT0, ADI_GPIO_PORT0,
				    ADI_GPIO_PORT1, ADI_GPIO_PORT1
				  };
/* Store Interrupt GPIO Pin address */
const ADI_GPIO_DATA INT_Pin[4] = { ADI_GPIO_PIN_15, ADI_GPIO_PIN_13,
				   ADI_GPIO_PIN_12, ADI_GPIO_PIN_14
				 };

extern const uint8_t sensor_addresses[4] = { ADI_CFG_I2C_SENSOR1_ADDR,
					     ADI_CFG_I2C_SENSOR2_ADDR,
					     ADI_CFG_I2C_SENSOR3_ADDR,
					     ADI_CFG_I2C_SENSOR4_ADDR
					   };

uint8_t TXbuff[256] = { 0 };		/* UART transmit buffer */
uint8_t safeTXbuff[256] = { 0 };	/* UART transmit buffer with ensured null-terminator */
volatile uint8_t TXcompleteFlag = 1;	/* indicates UART TX complete */
uint8_t RXchar = 0;			/* character received over UART (user input) */
uint8_t cmdInString[64] = "";		/* holds incoming user command */
uint8_t cmdInCnt = 0;			/* counts incoming bytes over UART */

/* flag - user command received (ENTER pressed) */
volatile uint8_t cmdReceived = 0;

uint8_t initialcycle = 1;
/* ensure EOS follows tempChar for printing */
char onecharstring[2] = { 0 };
char &tempChar = onecharstring[0];

bool detected_sensors[NUM_SENSORS] = { };

/* used for timing the data update rate when in STREAM state */
uint16_t streamTickCnt = 0;

/* used as configuration parameter for data update rate in STREAM mode
 *	default value is 1, i.e. 1 second update rate
 */
uint16_t streamTickCfg = 1;

eFSM_State FSM_State = INIT;

/*
 *	FUNCTIONS
 */

/* UART Callback handler */
static void uartCallback(void *pAppHandle, uint32_t nEvent, void *pArg)
{
	switch (nEvent) {
	case ADI_UART_EVENT_TX_BUFFER_PROCESSED:
		TXcompleteFlag = 1;
		break;
	case ADI_UART_EVENT_RX_BUFFER_PROCESSED:
		cmdInString[cmdInCnt++] = RXchar;
		if (cmdInCnt >= 64) { /* Prevent buffer overflow */
			cmdInCnt = 0;
			cmdReceived = 1;
		} else if (cmdInString[cmdInCnt - 1] == _CR) {
			cmdInString[cmdInCnt - 1] = '\0';
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
	eUartResult = adi_uart_Open(UART_DEVICE_NUM, ADI_UART_DIR_BIDIRECTION,
				    UartDeviceMem, ADI_UART_MEMORY_SIZE, &hDevice);
	if (eUartResult != ADI_UART_SUCCESS) {
		DEBUG_MESSAGE("Failed to open the UART device");
		return eUartResult;
	}

	eUartResult = adi_uart_SetConfiguration(hDevice, ADI_UART_NO_PARITY,
						ADI_UART_ONE_STOPBIT, ADI_UART_WORDLEN_8BITS);
	if (eUartResult != ADI_UART_SUCCESS) {
		DEBUG_MESSAGE("Failed to configure UART");
		return eUartResult;
	}

	eUartResult = adi_uart_ConfigBaudRate(hDevice,
					      UART_DIV_C_115200,
					      UART_DIV_M_115200,
					      UART_DIV_N_115200,
					      UART_OSR_115200);
	if (eUartResult != ADI_UART_SUCCESS) {
		DEBUG_MESSAGE("Baudrate configuration failed");
		return eUartResult;
	}

	eUartResult = adi_uart_RegisterCallback(hDevice, uartCallback, NULL);
	if (eUartResult != ADI_UART_SUCCESS) {
		DEBUG_MESSAGE("Call back registration failed");
		return eUartResult;
	}

	return adi_uart_SubmitRxBuffer(hDevice, &RXchar, 1, 0u);
}

/* GPIO initialization */
ADI_GPIO_RESULT GPIO_Init()
{
	uint8_t i = 0;

	eGpioResult = adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE);

	/* ENABLE GPIO for  I2C address setting and set high by default */
	for (i = 0; i < 4; i++) {
		eGpioResult = adi_gpio_OutputEnable(CS_Port[i], CS_Pin[i], true);
		eGpioResult = adi_gpio_SetHigh(CS_Port[i], CS_Pin[i]);
	}

	return eGpioResult;
}

/*!
 * @brief      UART Transmit function
 *
 * @details    Including CR and LF characters at the end of each transmission and timeout in case
 * 			   the transmission is not successful.
 */
GResult UART_TX(const char *initialcmd)
{
	GResult result = Failure;

	uint8_t crlf[2] = { _CR, _LF };
	uint32_t timeout = 0x00989680; /* 10*10^6 */
	uint16_t len;
	char *cmd;

	strncpy((char *) safeTXbuff, initialcmd, 255);
	safeTXbuff[255] = 0; /* ensure null termination */
	cmd = (char *) safeTXbuff;
	len = strlen(cmd);

	while (TXcompleteFlag == 0) {
		timeout--;
		if (timeout == 0)
			break;
	}
	TXcompleteFlag = 0;
	if ((adi_uart_SubmitTxBuffer(hDevice, (void*) cmd, len, 0u))
	    != ADI_UART_SUCCESS)
		return Failure;
	else
		result = Success;

	timeout = 0x00989680;
	while (TXcompleteFlag == 0) {
		timeout--;
		if (timeout == 0)
			break;
	}
	TXcompleteFlag = 0;
	if ((adi_uart_SubmitTxBuffer(hDevice, (char*) &crlf[0], 1u, 0u))
	    != ADI_UART_SUCCESS)
		return Failure;
	else
		result = Success;

	timeout = 0x00989680;
	while (TXcompleteFlag == 0) {
		timeout--;
		if (timeout == 0)
			break;
	}
	TXcompleteFlag = 0;
	if ((adi_uart_SubmitTxBuffer(hDevice, (char*) &crlf[1], 1u, 0u))
	    != ADI_UART_SUCCESS)
		return Failure;
	else
		result = Success;
	delay_ms(2);

	return result;
}

/*!
 * @brief      Direct UART Transmit function
 *
 * @details    Transmit via UART without CR and LF characters at the end of each transmission
 */
GResult UART_TX_DIR(const char *initialcmd)
{
	GResult result = Failure;

	uint32_t timeout = 0x00989680; /* 10*10^6 */
	uint16_t len;
	char *cmd;

	strncpy((char *) safeTXbuff, initialcmd, 255);
	safeTXbuff[255] = 0; /* ensure null termination */
	cmd = (char *) safeTXbuff;
	len = strlen(cmd);

	while (TXcompleteFlag == 0) {
		timeout--;
		if (timeout == 0)
			break;
	}
	TXcompleteFlag = 0;
	if ((adi_uart_SubmitTxBuffer(hDevice, (void*) cmd, len, 0u))
	    != ADI_UART_SUCCESS)
		return Failure;
	else
		result = Success;
	delay_ms(2);

	return result;
}

/**
 @brief Perform initialization of sensors.

 @return none
 **/
void InitState(void)
{
	uint8_t tempError;

	for (uint8_t i = 0; i < NUM_SENSORS; i++) {
		tempError = SensorInitwithI2CAddr(sensor_addresses[i], i + 1);
		if (tempError == SENSOR_ERROR_NONE) { /* CN0429 Board installed */
			if (FSM_State == WATER) {
				UART_TX("Error: CN0428 and CN0429 Boards cannot run at the same time." _EOS);
				UART_TX("Please restart the system with only 1 type plugged in." _EOS);
				while (1);
			}
			detected_sensors[i] = true;
			FSM_State = COMMAND;
		} else if (tempError == SENSOR_ERROR_PH) { /* CN0428 Board installed */
			if (FSM_State == COMMAND) {
				UART_TX("Error: CN0428 and CN0429 Boards cannot run at the same time." _EOS);
				UART_TX("Please restart the system with only 1 type plugged in." _EOS);
				while (1);
			}
			detected_sensors[i] = true;
			FSM_State = WATER;
		}
	}

	if (any_sensor_inited()) {
		UART_TX("----------------------------------------------" _EOS);
		cmdReceived = 1; /* Ensure welcome message printout */
	} else {
		UART_TX("No Sensor Board Found! Please check connections and reset ADICUP3029."
			_EOS);
		while (1);
	}
}

/*
 *	STATE MACHINE
 */
void DataDisplayFSM(void)
{
	switch (FSM_State) {
	case INIT:
		InitState();
		break;

	case COMMAND:
		/* waiting for user commands (CN0429) */
		break;

	case STREAM_GAS:
		CN0429_StreamData();
		break;

	case WATER:
		if (initialcycle) {
			/* Do initial read of water quality board transmit buffer */
			CN0428_Setup();
			/* Reset Command and reenable UART Interrupt */
			flushBuff(cmdInString, sizeof(cmdInString));
			cmdInCnt = 0;
			cmdReceived = 0;
			adi_uart_SubmitRxBuffer(hDevice, &RXchar, 1, 0u);
			initialcycle = 0;
		}
		if (cmdReceived) {
			cmdReceived = 0;
			CN0428_CommandLoop();
		}
		delay_ms(90); /* wait to check for new command */
		break;

	default:
		break;
	}
}

/*
 *	MAIN FUNCTION
 */
int main(int argc, char *argv[])
{
	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();

	*pREG_WDT0_CTL = DISABLE_WATCHDOG;

	if (ADI_PWR_SUCCESS != (ePwrResult = adi_pwr_Init())) {
		DEBUG_RESULT("\n Failed to init pwr", ePwrResult,
			     ADI_PWR_SUCCESS);
		return (ePwrResult);
	}

	if (ADI_PWR_SUCCESS
	    != (ePwrResult = adi_pwr_SetLFClockMux(
				     ADI_CLOCK_MUX_LFCLK_LFXTAL))) {
		/* select LFXTL clock */
		DEBUG_RESULT("\n Failed to set the LF Clock Mux", ePwrResult,
			     ADI_PWR_SUCCESS);
		return (ADI_RTC_FAILURE);
	}

	if (ADI_PWR_SUCCESS
	    != (ePwrResult = adi_pwr_EnableClockSource(
				     ADI_CLOCK_SOURCE_LFXTAL, true))) {
		DEBUG_RESULT("\n Failed to enable clock source", ePwrResult,
			     ADI_PWR_SUCCESS);
		return (ePwrResult);
	}

	if (ADI_PWR_SUCCESS
	    != (ePwrResult = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK,
			     1))) {
		DEBUG_RESULT("\n Failed to set clock divider", ePwrResult,
			     ADI_PWR_SUCCESS);
		return (ePwrResult);
	}
	if (ADI_PWR_SUCCESS
	    != (ePwrResult = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK,
			     1))) {
		DEBUG_RESULT("\n Failed to set clock divider", ePwrResult,
			     ADI_PWR_SUCCESS);
		return (ePwrResult);
	}

	/* Initialize GPIO */
	GPIO_Init();

	/* Initialize UART */
	UART_Init();

	UART_TX("Please wait for sensor startup and initialization:" _EOS);
	tempChar = '.';
	/* delay to allow sensors to start properly */
	for (uint8_t i = 0; i < 10; i++) {
		UART_TX_DIR((const char *) &tempChar);
		delay_ms(1000);
	}
	UART_TX((const char *) &tempChar);

	while (1) {
		delay_ms(10);

		if (cmdReceived && FSM_State != WATER) {
			CmdProcess();
			cmdReceived = 0;
		}

		if (FSM_State == STREAM_GAS) {
			streamTickCnt++;
			if (streamTickCnt >= ((100 * streamTickCfg) - 25)) {
				/* -25 to account for delays in readout */
				DataDisplayFSM();
				streamTickCnt = 0;
			}
		} else {
			DataDisplayFSM();
		}
	}

}
