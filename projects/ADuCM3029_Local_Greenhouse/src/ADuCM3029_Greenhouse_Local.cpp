/*!
 *****************************************************************************
 * @file:    ADuCM3029_Greenhouse_Local.cpp
 * @brief:
 * @version: $Revision$
 * @date:    $Date$
 *-----------------------------------------------------------------------------
 *
Copyright (c) 2015-2018 Analog Devices, Inc.
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Modified versions of the software must be conspicuously marked as such.
  - This software is licensed solely and exclusively for use with processors
    manufactured by or for Analog Devices, Inc.
  - This software may not be combined or merged with other code in any manner
    that would cause the software to become subject to terms and conditions
    which differ from those listed here.
  - Neither the name of Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.
  - The use of this software may or may not infringe the patent rights of one
    or more patent holders.  This license does not release you from the
    requirement that you obtain separate licenses from these patent holders
    to use this software.
THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
NON-INFRINGEMENT, TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF
CLAIMS OF INTELLECTUAL PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "ADuCM3029_Greenhouse_Local.h"

/*Circuit note includes*/
#include "adi_cn0398.h"
#include "adi_cn0397.h"
#include "adi_cn0410.h"

using namespace adi_sensor_swpack;

uint8_t         m_spi_memory[ADI_SPI_MEMORY_SIZE];
uint8_t         gpioMemory[ADI_GPIO_MEMORY_SIZE];

/*Sensor objects*/
CN0398  * cn0398 = new CN0398();
SENSOR_RESULT   eSensorResult;

CN0397     *cn0397 = new CN0397();
VisibleLight   *pLight = cn0397;

CN0410 *cn0410 = new CN0410(pLight);

sensors_data_t  data;

/**
   @brief Callback function called when a command is received from the cloud.

   @param str - pointer to a constant string to get the command value

   @return void

**/
int Leds_Controll(const char *str)
{
	memset(strstr((char *)str, (char *)"}") + 1, 0, strlen(str));
	JsonNode *_main = json_decode(str);

	JsonNode *red = json_find_member(_main, "red");
	JsonNode *blue = json_find_member(_main, "blue");
	JsonNode *green = json_find_member(_main, "green");

	/*Control algorith for CN0410 led bar*/
	cn0410->Controll((float)red->number_, (float)blue->number_, (float)green->number_);

	json_delete(_main);

	return 0;
}

int main(int argc, char *argv[])
{
	uint8_t u8Command;
	uint8_t u8Error;

	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();

	timer_start();

	adi_initpinmux(); // Init port configuration for UART, SPI and I2C
	adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE); //initialize gpio

	/*Initialize the sensors*/
	cn0398->setup();
	cn0398->init();

	eSensorResult = pLight->open();

	cn0398->use_nernst = false; // use default calibration

	cn0398->set_data();

	/* Get the red, green and blue visual light data and concentration. */
	pLight->getLightIntensity((float *)&eIntensity.fData_Red);
	pLight->getLightConcentration((float *)&eConcentration.fData_Red);

	cn0410->Init();
	cn0410->Setup();

	/*reset led values*/
	cn0410->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_A, 100);
	cn0410->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_B, 0);
	cn0410->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_C, 100);

	UART_Init();
	AppPrintf("Local Greenhouse started..\n\r");

	while(1)
	{
		AppPrintf("\n\rSend a read or write command to the device!\n\r");
		AppPrintf("Read command: read followed by enter\n\r");
		AppPrintf("Write command: write {\"red\":1000, \"blue\":1000, \"green\":1000}\n\r");
		while (!boCommandReceived);
		adi_uart_FlushRxChannel(hUartDevice);
		boCommandReceived = false; //reset command received

		u8Error = ProcessCommand(&u8Command);

		if ( u8Error == 0 )
		{
			AppPrintf("Command received.\n\r");
			if(u8Command == 'r')
			{
				PreparePayload(&data);
				char *payload = telemetry_serialize(&data);
				AppPrintf("\n\r%s\n\r\n\r", payload);
				free(payload);
			}
			else {
				if(u8Command == 'w')
				{
					Leds_Controll(strstr((char *)Buffer, (char *)"{"));
				}
			}
		}
	}
}

