/**
******************************************************************************
*   @file     Test.c
*   @brief    Source file for Test project.
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
#include <ctype.h>
#include <stdio.h>
#include "Communication.h"
#include "common.h"

#include "Test.h"
#include "Test_Config.h"

/************************** Variable Definitions ******************************/
/* Rx buffer */
uint8_t RxBuffer[2];
/* Tx buffer */
uint8_t TxBuffer[8]  = {0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03};

/************************* Functions Definitions ******************************/

/**
  @brief UART, SPI, I2C, GPIO initialization

  @return none

**/
void Test_Init(void)
{
	if(FUNCTION_TO_TEST == SPI) { //Check if SPI is tested
		SPI_Init();         // SPI initialization

	} else if(FUNCTION_TO_TEST == UART) { // Check if UART is tested
		UART_Init();    	// UART initialization: 57600 baud rate

	} else if(FUNCTION_TO_TEST == I2C) { // Check if I2C is tested
		I2C_Init();         // I2C initialization

	} else if(FUNCTION_TO_TEST == GPIO) { // Check if GPIO is tested
		Port_Init(); 		// GPIO initialization
	}
}

/**
  @brief Port initialization

  @return none

**/
void Port_Init(void)
{
	adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE);

	switch(GPIO_PINS){
	case ADI_GPIO_PORT0:
		adi_gpio_OutputEnable(ADI_GPIO_PORT0, 0xFFFFu, true); 	// Set Port 0 as output
		adi_gpio_PullUpEnable(ADI_GPIO_PORT0, 0xFFFFu, false); 	// Disable all pull-up resistors
		adi_gpio_SetLow(ADI_GPIO_PORT0, 0xFFFFu); 				// Set all Port 0 outputs to low
		break;

	case ADI_GPIO_PORT1:
		adi_gpio_OutputEnable(ADI_GPIO_PORT1, 0xFFFFu, true); 	// Set Port 1 as output
		adi_gpio_PullUpEnable(ADI_GPIO_PORT1, 0xFFFFu, false); 	// Disable all pull-up resistors
		adi_gpio_SetLow(ADI_GPIO_PORT0, 0xFFFFu); 				// Set all Port 1 outputs to low
		break;

	case ADI_GPIO_PORT2:
		adi_gpio_OutputEnable(ADI_GPIO_PORT2, 0x0FFFu, true); 	// Set Port 2 as output
		adi_gpio_PullUpEnable(ADI_GPIO_PORT2, 0x0FFFu, false); 	// Disable all pull-up resistors
		adi_gpio_SetLow(ADI_GPIO_PORT0, 0x0FFFu); 				// Set all Port 2 outputs to low
		break;

	default:
		break;
	}
}

/**
  @brief Port test function

  @return none

**/
void Test_Port(void)
{
	switch(GPIO_PINS){
	case ADI_GPIO_PORT0:
		adi_gpio_Toggle(ADI_GPIO_PORT0, 0xFFFFu); // Toggle all Port 0 outputs
		break;

	case ADI_GPIO_PORT1:
		adi_gpio_Toggle(ADI_GPIO_PORT1, 0xFFFFu); // Toggle all Port 1 outputs
		break;

	case ADI_GPIO_PORT2:
		adi_gpio_Toggle(ADI_GPIO_PORT2, 0x0FFFu); // Toggle all Port 2 outputs
		break;
	default:
		break;
	}
}

/**
  @brief UART test function

  @return none

**/
void Test_UART(void)
{
    adi_uart_Read(hUartDevice,&RxBuffer[0],1); 	// Read a character from UART
    adi_uart_Write(hUartDevice,&RxBuffer[0],1); // Write it back to UART
}

/**
  @brief SPI test function

  @return none

**/
void Test_SPI(void)
{
	ADI_SPI_RESULT eSpiResult;
	bool_t bMasterComplete= false;

    /* Prepare the buffer for submitting to Master */
	MSpitransceive.pTransmitter = TxBuffer; // initialize data attributes
	MSpitransceive.TransmitterBytes = 8; 	// link transceive data size to the remaining count
	MSpitransceive.nTxIncrement = 1;		// auto increment buffer
	MSpitransceive.pReceiver = NULL;
	MSpitransceive.ReceiverBytes = 0;
	MSpitransceive.nRxIncrement = 0;

    /* Submit the buffer to Master */
	if ((eSpiResult = adi_spi_MasterTransfer(hMSpiDevice,&MSpitransceive)) != ADI_SPI_SUCCESS)
	{
		DEBUG_MESSAGE("Master - Data failure");
	}

    /* Wait till the data transmission is over */
	while( (bMasterComplete == false) ){
		   adi_spi_MasterComplete(hMSpiDevice,&bMasterComplete);
	}
}

/**
  @brief I2C test function

  @return none

**/
void Test_I2C(void)
{
	void* pBuffer;
	ADI_I2C_RESULT	eI2cResult = ADI_I2C_SUCCESS;

	/* Submit Buffers */
	if ((eI2cResult = adi_i2c_SubmitRxBuffer(slaveI2cDev, &RxBuffer[0], 1, false)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to Submit Rx buffer to slave");
	}
	if ((eI2cResult = adi_i2c_SubmitTxBuffer(masterI2cDev, &TxBuffer[0], 1, false)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to Submit Tx buffer to master");
	}

	/* Enable slave and master device */
	if ((eI2cResult = adi_i2c_Enable(slaveI2cDev, true)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to enable slave");
	}
	if ((eI2cResult = adi_i2c_Enable(masterI2cDev, true)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to enable master");
	}

	 /* Get Tx and Rx Buffers from master and slave */
	if ((eI2cResult = adi_i2c_GetTxBuffer(masterI2cDev, &pBuffer)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to get buffer from master");
	}
	if ((eI2cResult = adi_i2c_GetRxBuffer(slaveI2cDev, &pBuffer)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to get buffer from slave");
	}

	/* Disable slave and master device */
	if ((eI2cResult = adi_i2c_Enable(slaveI2cDev, false)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to disable slave");
	}
	if ((eI2cResult = adi_i2c_Enable(masterI2cDev, false)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to disable master");
	}
}

/**
  @brief Test peripherals function

  @return none

**/
void Test_Periph(void)
{
	if(FUNCTION_TO_TEST == SPI) { // Check if SPI is tested
		Test_SPI();            // Test SPI

	} else if(FUNCTION_TO_TEST == UART) { // Check if UART is tested
		Test_UART();           // Test UART

	} else if(FUNCTION_TO_TEST == I2C) { // Check if I2C is tested
		Test_I2C();           // Test I2C
	}
}
