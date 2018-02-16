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
#include "common.h"
/************************** Variable Definitions ******************************/

/* Master SPI device handle */
ADI_SPI_HANDLE hMSpiDevice;
/* Device memory required for operate Master SPI device */
uint8_t MasterSpidevicemem[ADI_SPI_MEMORY_SIZE];
ADI_SPI_TRANSCEIVER MSpitransceive;

/************************* Functions Definitions ******************************/
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
   @brief Writes a data, a command or a register to the LCD or to ACC via SPI.

   @param ui8address - ACC register address
   @param ui8Data - value to be written
   @enMode ui8Data - write mode

   @return none

**/
void SPI_Write(uint8_t ui8address, uint8_t ui8Data, enWriteData enMode)
{
	ADI_SPI_RESULT eSpiResult;
	uint8_t aui8Tx_Buf[3];

	/*enMode = SPI_WRITE_REG - ToDo for other modes (LCD)*/
	aui8Tx_Buf[0] = COMM_WRITE;
	aui8Tx_Buf[1] = ui8address;
	aui8Tx_Buf[2] = ui8Data;

	/* Prepare the buffer for submitting to Master */
	MSpitransceive.pTransmitter = &aui8Tx_Buf[0]; // initialize data attributes
	MSpitransceive.TransmitterBytes = 3; 	// link transceive data size to the remaining count
	MSpitransceive.nTxIncrement = 1;		// auto increment buffer
	MSpitransceive.pReceiver = NULL;
	MSpitransceive.ReceiverBytes = 0;
	MSpitransceive.nRxIncrement = 0;

	/* Submit the buffer to Master */
	if ((eSpiResult = adi_spi_MasterSubmitBuffer(hMSpiDevice,&MSpitransceive)) != ADI_SPI_SUCCESS)
	{
		DEBUG_MESSAGE("Master - Data failure");
	}

	/* Wait till the data transmission is over */
	adi_spi_MasterReadWrite(hMSpiDevice,&MSpitransceive);

}

/**
   @brief Reads a specified register or two registers address in the accelerometer via SPI.

   @param ui8address - register address
   @param enRegs - register number

   @return reading result

**/
uint16_t SPI_Read(uint8_t ui8address, enRegsNum enRegs)
{
	uint8_t aui8Rx_Buf[4];
	uint8_t aui8Tx_Buf[4] = {0x00, 0x00, 0x00, 0x00};

	ADI_SPI_RESULT eSpiResult;

	aui8Tx_Buf[0] = COMM_READ;
	aui8Tx_Buf[1] = ui8address;

	/* Prepare the buffer for submitting to Master */
	MSpitransceive.pTransmitter = &aui8Tx_Buf[0]; // initialize data attributes

	MSpitransceive.nTxIncrement = 1;		// auto increment buffer
	MSpitransceive.pReceiver = &aui8Rx_Buf[0];
	MSpitransceive.ReceiverBytes = 0;
	MSpitransceive.nRxIncrement = 1;
	MSpitransceive.TransmitterBytes = 4; 	// link transceive data size to the remaining count
	MSpitransceive.ReceiverBytes = 4;


	/* Submit the buffer to Master */
	if ((eSpiResult = adi_spi_MasterSubmitBuffer(hMSpiDevice,&MSpitransceive)) != ADI_SPI_SUCCESS)
	{
		DEBUG_MESSAGE("Master - Data failure");
	}

	/* Wait till the data transmission is over */
	adi_spi_MasterReadWrite(hMSpiDevice,&MSpitransceive);

	if (enRegs == SPI_READ_ONE_REG) {

		return (uint16_t)aui8Rx_Buf[2];

	} else {

		return (uint16_t)((uint16_t)(aui8Rx_Buf[2] << 8) + aui8Rx_Buf[3]);
	}
}
