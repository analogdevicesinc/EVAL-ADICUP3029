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

/************************* Functions Definitions ******************************/
uint8_t Buffer[200];

uint8_t u8Index;
bool boCommandReceived = false;

void UARTCallback(
		void      *pCBParam,         /*!< Client supplied callback param */
		uint32_t   Event,            /*!< Event ID specific to the Driver/Service */
		void      *pArg)

{
	uint32_t             *pHwError;

	if (Event == ADI_UART_EVENT_RX_DATA)
	{
		adi_uart_ReadChar(hUartDevice, &Buffer[u8Index], pHwError);
		u8Index++;
	}

	if ((Buffer[u8Index - 1] == '\r'))
	{
		boCommandReceived = true;
		u8Index = 0;
	}
}

uint8_t ProcessCommand(uint8_t * u8Command)
{

	if (strstr((char *)Buffer, (char *)"read"))
	{
		*u8Command = 'r';
		return 0;
	}
	else
	{

		if (strstr((char *)Buffer, (char *)"write"))
		{
			*u8Command = 'w';
			return 0;
		}
		else return -1;
	}
}

/**
  @brief UART Initialization

  @return none

**/
void UART_Init()
{
  	/* Variable for storing the return code from UART device */
	ADI_UART_RESULT  eUartResult;

    /* Open the UART device.Data transfer is bidirectional with NORMAL mode by default.  */
    if((eUartResult = adi_uart_Open(UART_DEVICE_NUM, ADI_UART_DIR_BIDIRECTION, UartDeviceMem, ADI_UART_MEMORY_SIZE, &hUartDevice)) != ADI_UART_SUCCESS)
    {
    	DEBUG_MESSAGE("\n\t UART device open  failed");
    }

    /* Configure  UART device with NO-PARITY, ONE STOP BIT and 8bit word length. */
    if((eUartResult = adi_uart_SetConfiguration(hUartDevice, ADI_UART_NO_PARITY, ADI_UART_ONE_STOPBIT, ADI_UART_WORDLEN_8BITS)) != ADI_UART_SUCCESS)
    {
    	DEBUG_MESSAGE("UART device configuration failed ");
    }

    /* Baud rate div values are calculated for PCLK 26Mhz */
    if((eUartResult = adi_uart_ConfigBaudRate(hUartDevice, UART_DIV_C, UART_DIV_M, UART_DIV_N, UART_OSR)) != ADI_UART_SUCCESS) // 9600 baud rate
    {
    	DEBUG_MESSAGE("UART device baud rate configuration failed");
    }

    if((eUartResult = adi_uart_RegisterCallback(hUartDevice, UARTCallback)) != ADI_UART_SUCCESS) // set callback function
    {
    	DEBUG_MESSAGE("UART registering callback failed");
    }
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

	eUartResult = adi_uart_Write(hUartDevice, &data, 1, &u32HwError); //write a char to the UART

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
	ADI_UART_RESULT  eUartResult;

   while (*string != '\0') {
	   eUartResult = UART_WriteChar(*string++);

      if (eUartResult != ADI_UART_SUCCESS) {
         break;
      }
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
   char buff[300];

   va_list args;
   va_start (args, fmt);

   vsprintf (buff, fmt, args);
   va_end (args);

   UART_WriteString(buff);
}

