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

extern ADI_UART_HANDLE  phDevice;


/************************* Functions Definitions ******************************/

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

	eUartResult = adi_uart_Write(phDevice, &data,
				     1, &u32HwError); //write a char to the UART

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
	char buff[256];

	va_list args;
	va_start (args, fmt);

	vsprintf (buff, fmt, args);
	va_end (args);

	UART_WriteString(buff);
}


