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
#include "Communication.h"
#include "Test_Config.h"
#include "common.h"

/************************** Variable Definitions ******************************/

/* Handle for UART device */
ADI_UART_HANDLE          hUartDevice __attribute__ ((aligned (4)));
/* Memory for  UART driver */
uint8_t UartDeviceMem[ADI_UART_MEMORY_SIZE] __attribute__ ((aligned (4)));

/* Master SPI device handle */
ADI_SPI_HANDLE hMSpiDevice;
/* Device memory required for operate Master SPI device */
uint8_t MasterSpidevicemem[ADI_SPI_MEMORY_SIZE];

/* Master I2C device handle */
ADI_I2C_HANDLE masterI2cDev;
/* Slave I2C device handle */
ADI_I2C_HANDLE slaveI2cDev;
/* Device memory required for operate Master I2C device */
uint8_t masterI2cDeviceMemory[ADI_I2C_MEMORY_SIZE];
/* Device memory required for operate Slave I2C device */
uint8_t slaveI2cDeviceMemory[ADI_I2C_MEMORY_SIZE];

/************************* Functions Definitions ******************************/

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
}


/**
  @brief SPI Initialization

  @return none

**/
void SPI_Init(void)
{
	/* Variable for storing the return code from SPI device */
	ADI_SPI_RESULT eSpiResult;

	/* Open the SPI device. It opens in Master mode by default */
	if ((eSpiResult = adi_spi_Open(SPI_CHANNEL,MasterSpidevicemem,ADI_SPI_MEMORY_SIZE,&hMSpiDevice)) != ADI_SPI_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to init SPI driver");
	}
	/* Set the bit rate  */
	if ((eSpiResult = adi_spi_SetBitrate(hMSpiDevice, 1000000)) != ADI_SPI_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to set Bitrate");
	}
	/* Set the chip select. */
	if(SPI_CHANNEL == SPI_ARDUINO) {
		if ((eSpiResult = adi_spi_SetChipSelect(hMSpiDevice, ADI_SPI_CS1)) != ADI_SPI_SUCCESS) // SPI_ARDUINO uses CS1
		{
			DEBUG_MESSAGE("Failed to set the chip select");
		}
	}
	else { // SPI_BLE or SPI_PMOD
		if ((eSpiResult = adi_spi_SetChipSelect(hMSpiDevice, ADI_SPI_CS0)) != ADI_SPI_SUCCESS) // SPI_BLE and SPI_PMOD use CS0
		{
			DEBUG_MESSAGE("Failed to set the chip select");
		}
	}
	adi_spi_SetContinuousMode(hMSpiDevice, true);
	adi_spi_SetIrqmode(hMSpiDevice, true);
	adi_spi_SetClockPolarity(hMSpiDevice, false); // set Clock polarity low
	adi_spi_SetClockPhase(hMSpiDevice, false);    // set Clock phase leading
}

/**
  @brief I2C Initialization

  @return none

**/
void I2C_Init(void)
{
	ADI_I2C_RESULT eI2cResult = ADI_I2C_SUCCESS;

	/* Temporary solution to enable the DS bit for P0.4 and P0.5 */
	pADI_GPIO0->DS |= (1<<4) | (1<<5);

	/* Open Master Device */
	if ((eI2cResult = adi_i2c_Open(I2C_CHANNEL, &masterI2cDeviceMemory[0], ADI_I2C_MEMORY_SIZE, &masterI2cDev)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("I2C Open Master failed");
	}
	/* Open Slave Device */
	if ((eI2cResult = adi_i2c_Open(I2C_CHANNEL, &slaveI2cDeviceMemory[0], ADI_I2C_MEMORY_SIZE, &slaveI2cDev)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("I2C Open Slave failed");
	}

	if ((eI2cResult = adi_i2c_SetBitRate(masterI2cDev, 100000u)) != ADI_I2C_SUCCESS) // 100 kHz
	{
		DEBUG_MESSAGE("adi_i2c_SetBitRate failed");
	}

	/* Set the hardware address for slave */
	if ((eI2cResult = adi_i2c_SetSlaveAddress(slaveI2cDev, 0x48)) != ADI_I2C_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to set slave hardware address");
	}
}
