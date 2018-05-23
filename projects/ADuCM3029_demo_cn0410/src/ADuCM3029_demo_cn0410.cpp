/***************************************************************************//**
 *   @file   ADuCM3029_demo_cn0410s.cpp
 *   @brief  Implementation of ADuCM3029_demo_cn0410 application.
 *   @author Mircea Caprioru (mircea.caprioru@analog.com)
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
#include "Timer.h"
#include "ADuCM3029_demo_cn0410.h"
#include "ADuCM3029_demo_cn0410_bindings.h"
#include "Communication.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <transport/adi_wifi_transport.h>
#include <transport/adi_uart.h>

using namespace adi_sensor_swpack;

ADI_UART_HANDLE  phDevice;
uint8_t         aDeviceMemory[ADI_UART_BIDIR_MEMORY_SIZE];

uint8_t Buffer[12];

uint8_t u8Index;
bool boCommandReceived = false;;

/**
  @brief UART interrupt callback.

  @param pCBParam - Client supplied callback param.
  @param Event - Event ID specific to the Driver/Service.
  @param pArg - parameters list.

  @return void.

**/
void UARTCallback(
	void      *pCBParam,	/*!< Client supplied callback param */
	uint32_t   Event,		/*!< Event ID specific to the Driver/Service */
	void      *pArg)

{
	uint32_t *pHwError;

	if (Event == ADI_UART_EVENT_RX_DATA) {
		adi_uart_ReadChar(phDevice, &Buffer[u8Index], pHwError);
		u8Index++;
	}

	if ((Buffer[u8Index - 1] == '\r') | (u8Index == 12)) {
		boCommandReceived = true;
		u8Index = 0;
	}
}

/**
  @brief Function to process commands received over UART.

  @param u8Channel - DAC channel to setup.
  @param u16DacValue - value to set on DAC output.

  @return 0 - Succes, -1 - Failure.

**/
uint8_t ProcessCommand(uint8_t * u8Channel, uint16_t * u16DacValue)
{
	uint16_t u16RawValue;

	if (((Buffer[4] != 'a') &  (Buffer[4] != 'b') &
	     (Buffer[4] != 'c') & (Buffer[4] != 'z')))
		return -1;

	if ((Buffer[4] != 'z')) 
		*u8Channel = Buffer[4]; /*channel letter*/
	else 
		*u8Channel = '0';
	

	u16RawValue = (uint16_t)atoi((char *)&Buffer[6]);
	*u16DacValue = u16RawValue;

	return 0;
}

int main(int argc, char *argv[])
{
	CN0410 cn0410;

	uint8_t u8Channel, u8Error;
	uint16_t u16DacValue, u16ChannelValue;

	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();

	timer_start(); /*init timer*/

	adi_uart_Open(ADI_UART_DEVICE_NUM_CFG, ADI_TAL_UART_DIRECTION,
					aDeviceMemory, ADI_UART_BIDIR_MEMORY_SIZE,
					&phDevice);
	adi_uart_RegisterCallback(phDevice, UARTCallback);
	adi_uart_SetConfiguration(phDevice, ADI_UART_NO_PARITY,
					ADI_UART_ONE_STOPBIT, ADI_UART_WORDLEN_8BITS);
	adi_uart_ConfigBaudRate(phDevice, UART_DIV_C, UART_DIV_M,
					UART_DIV_N, UART_OSR);

	cn0410.Init(); //init spi and pins

	cn0410.SendCommand(AD5686_ITERNAL_REFERENCE,
					AD5686_DAC_NONE, 0x00); /*enable internal reference*/
	cn0410.SendCommand(AD5686_POWER,
					AD5686_DAC_NONE, 0x00); /*normal power mode for all ch*/

	timer_sleep(10); //10 ms delay

	cn0410.SendCommand(AD5686_RESET,
					AD5686_DAC_NONE, 0x00); /*soft reset to zero scale*/

	AppPrintf("\n\n\rCN0410 Demo Software\n\r");
	AppPrintf("Enter command in the following format set_x value,\n\r \
					where x is the channel (a,b,c) and hit enter \n\r \
					To reset channels input command set_zero \n\r> ");

	while(1) {
		while (!boCommandReceived);
		adi_uart_FlushRxChannel(phDevice);
		boCommandReceived = false; /*reset command received*/

		u8Error = ProcessCommand(&u8Channel, &u16DacValue);

		if ( u8Error == 0 ) {
			AppPrintf("Command received.\n\r");

			switch(u8Channel) {
			case 'a':
				cn0410.SendCommand(AD5686_WRITE_UPDATE,
							AD5686_DAC_A, u16DacValue);
				u16ChannelValue = cn0410.ReadBack(AD5686_DAC_A);
				AppPrintf("Channel A set to: %d\n\r", u16ChannelValue);
				break;
			case 'b':
				cn0410.SendCommand(AD5686_WRITE_UPDATE,
							AD5686_DAC_B, u16DacValue);
				u16ChannelValue = cn0410.ReadBack(AD5686_DAC_B);
				AppPrintf("Channel B set to: %d\n\r", u16ChannelValue);
				break;
			case 'c':
				cn0410.SendCommand(AD5686_WRITE_UPDATE,
							AD5686_DAC_C, u16DacValue);
				u16ChannelValue = cn0410.ReadBack(AD5686_DAC_C);
				AppPrintf("Channel C set to: %d\n\r", u16ChannelValue);
				break;
			case '0':
				cn0410.Reset();
				AppPrintf("All channels reset to 0\n\r");
				break;
			default:
				AppPrintf("Invalid value\n\r");
				break;
			}
		} else {
			AppPrintf("Invalid command\n\r");
		}

		u16ChannelValue = cn0410.ReadBack(AD5686_DAC_A);
		AppPrintf("\n\rChannel A set to: %d\n\r", u16ChannelValue);
		u16ChannelValue = cn0410.ReadBack(AD5686_DAC_B);
		AppPrintf("Channel B set to: %d\n\r", u16ChannelValue);
		u16ChannelValue = cn0410.ReadBack(AD5686_DAC_C);
		AppPrintf("Channel C set to: %d\n\r", u16ChannelValue);

		timer_sleep(1000); //1 second sleep
		AppPrintf("\n\n\rEnter new command\n\r> ");
	}

	/* Begin adding your custom code here */

	return 0;
}

