/**
******************************************************************************
*   @file     Communication.c
*   @brief    Source file for communication part.
*   @version  V0.1
*   @author   ADI
*   @date     January 2017
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
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "Communication.h"
#include "common.h"

/************************** Variable Definitions ******************************/

/* Handle for UART device */
ADI_UART_HANDLE          hUartDevice __attribute__ ((aligned (4)));
/* Memory for  UART driver */
uint8_t UartDeviceMem[ADI_UART_MEMORY_SIZE] __attribute__ ((aligned (4)));

ADI_BLE_GAP_MODE   gGapMode;

/************************* Functions Definitions ******************************/
void adi_DataExchange_Callback(void *pParam, uint32_t Event, void *pData);

/**
  @brief UART Initialization

  @return none

**/
void UART_Init()
{
	/* Variable for storing the return code from UART device */
	ADI_UART_RESULT  eUartResult;

	/* Open the UART device.Data transfer is bidirectional with NORMAL mode by default.  */
	if((eUartResult = adi_uart_Open(UART_DEVICE_NUM, ADI_UART_DIR_BIDIRECTION,
					UartDeviceMem, ADI_UART_MEMORY_SIZE, &hUartDevice)) != ADI_UART_SUCCESS)
		DEBUG_MESSAGE("\n\t UART device open  failed");

	/* Configure  UART device with NO-PARITY, ONE STOP BIT and 8bit word length. */
	if((eUartResult = adi_uart_SetConfiguration(hUartDevice, ADI_UART_NO_PARITY,
			  ADI_UART_ONE_STOPBIT, ADI_UART_WORDLEN_8BITS)) != ADI_UART_SUCCESS)
		DEBUG_MESSAGE("UART device configuration failed ");

	/* Baud rate div values are calculated for PCLK 26Mhz */
	if((eUartResult = adi_uart_ConfigBaudRate(hUartDevice, UART_DIV_C, UART_DIV_M,
			  UART_DIV_N, UART_OSR)) != ADI_UART_SUCCESS) // 9600 baud rate
		DEBUG_MESSAGE("UART device baud rate configuration failed");
}

/**
  @brief Writes one character to UART.

  @param data - Character to write.

  @return ADI_UART_SUCCESS or error code.

**/
int UART_WriteChar(char data)
{
	/* Variable for storing the return code from UART device */
	ADI_UART_RESULT  eUartResult;
	uint32_t         u32HwError;

	eUartResult = adi_uart_Write(hUartDevice, &data, 1, false,
				     &u32HwError); //write a char to the UART

	return eUartResult;
}

/**
  @brief Writes string to UART.

  @param string - string to write.

  @return ADI_UART_SUCCESS or error code.

**/
int UART_WriteString(char *string)
{
	/* Variable for storing the return code from UART device */
	ADI_UART_RESULT  eUartResult = ADI_UART_FAILED;

	while (*string != '\0') {
		eUartResult = UART_WriteChar(*string++);

		if (eUartResult != ADI_UART_SUCCESS)
			break;
	}

	return eUartResult;
}

/**
  @brief Writes content of pointer to UART.

  @param const char - point to memory location data.

  @return none

**/
void AppPrintf(const char *fmt, ...)
{
	char buff[256];

	va_list args;
	va_start (args, fmt);

	vsprintf (buff, fmt, args);
	va_end (args);

	UART_WriteString(buff);
}

/**
 * @brief Configure the BLE radio.
 * @return None.
 */
void configure_ble_radio(void)
{
	ADI_BLER_RESULT      eResult;
	uint8_t *       pDeviceName = (unsigned char *) "Adi_Impact_Detection";

	/* Configure radio */
	eResult = adi_ble_Init(adi_DataExchange_Callback, NULL);
	DEBUG_RESULT("Error initializing the radio.\r\n", eResult, ADI_BLER_SUCCESS);

	eResult = adi_radio_RegisterDevice(ADI_BLE_ROLE_PERIPHERAL);
	DEBUG_RESULT("Error registering the radio.\r\n", eResult, ADI_BLER_SUCCESS);

	eResult = adi_radio_SetLocalBluetoothDevName(pDeviceName,
			strlen((const char*)pDeviceName), 0u, 0u);
	DEBUG_RESULT("Error setting local device name.\r\n", eResult, ADI_BLER_SUCCESS);

	SetAdvertisingMode();

	eResult = adi_radio_Register_DataExchangeServer();
	DEBUG_RESULT("adi_radio_Register_DataExchange\r\n",eResult,ADI_BLER_SUCCESS);

}

/*!
 * @brief      Set Advertising Mode
 *
 * @details    Helper function to avoid repeated code in main.
 *
 */
void SetAdvertisingMode(void)
{
	ADI_BLER_RESULT eResult;

	eResult = adi_radio_SetMode(PERIPHERAL_MODE, 0u, 0u);
	DEBUG_RESULT("Error setting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

	eResult = adi_ble_WaitForEventWithTimeout(GAP_EVENT_MODE_CHANGE, 5000u);
	DEBUG_RESULT("Error waiting for GAP_EVENT_MODE_CHANGE.\r\n", eResult,
		     ADI_BLER_SUCCESS);

	eResult = adi_radio_GetMode(&gGapMode);
	DEBUG_RESULT("Error getting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

	if(gGapMode != PERIPHERAL_MODE) {
		DEBUG_MESSAGE("Error in SetAdvertisingMode.\r\n");
	}
}
