/***************************************************************************//**
 *   @file   adi_cn0410.cpp
 *   @brief  Implementation of CN0410 dac interface.
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

#include "adi_cn0410.h"
#include "adi_cn0410_cfg.h"

namespace adi_sensor_swpack
{

	uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE];

	CN0410::CN0410()
	{
		/* Constructor for CN0410 */
	}

	int8_t CN0410::Init()
	{
		/* Initialization for SPI and GPIO */
		ADI_SPI_RESULT eSpiResult;
		int8_t i8Result = 0;

		/* Initialize SPI driver */
		if ((eSpiResult = adi_spi_Open(CN0410_SPI_DEV_CFG,
							m_spi_memory,
							ADI_SPI_MEMORY_SIZE,
							&m_spi_handle)) != ADI_SPI_SUCCESS)

			i8Result = -1;


		if((eSpiResult = adi_spi_SetMasterMode(m_spi_handle,
							CN0410_SPI_MASTER_CFG )) != ADI_SPI_SUCCESS)
			i8Result = -1;

		if((eSpiResult = adi_spi_SetBitrate(m_spi_handle,
							CN0410_SPI_BITRATE_CFG )) != ADI_SPI_SUCCESS)
			i8Result = -1;


		if((eSpiResult = adi_spi_SetChipSelect(m_spi_handle,
							CN0410_SPI_CS_CFG )) != ADI_SPI_SUCCESS)
			i8Result = -1;

		/* Set the SPI clock polarity */
		if(adi_spi_SetClockPolarity(m_spi_handle, true) != ADI_SPI_SUCCESS)
			i8Result = -1;


		/* Set the SPI clock phase */
		if(adi_spi_SetClockPhase(m_spi_handle, true) != ADI_SPI_SUCCESS)
			i8Result = -1;

		if(adi_spi_SetContinuousMode(m_spi_handle, true) != ADI_SPI_SUCCESS)
			i8Result = -1;

		if (adi_spi_SetIrqmode(m_spi_handle, true) != ADI_SPI_SUCCESS)
			i8Result = -1;


		adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE); /* initialize gpio */

		/* Setup LDAC & Reset pins */
		adi_gpio_OutputEnable(LDAC_PORT, LDAC_PIN, true);
		adi_gpio_InputEnable(LDAC_PORT, LDAC_PIN, false);
		adi_gpio_PullUpEnable(LDAC_PORT, LDAC_PIN, false);
		adi_gpio_SetLow(LDAC_PORT, LDAC_PIN);

		adi_gpio_OutputEnable(RESET_PORT, RESET_PIN, true);
		adi_gpio_InputEnable(RESET_PORT, RESET_PIN, false);
		adi_gpio_PullUpEnable(RESET_PORT, RESET_PIN, false);
		adi_gpio_SetHigh(RESET_PORT, RESET_PIN);

		/* set chip select pin */
		adi_gpio_OutputEnable(SYNC_PORT, SYNC_PIN, true);
		adi_gpio_InputEnable(SYNC_PORT, SYNC_PIN, false);
		adi_gpio_PullUpEnable(SYNC_PORT, SYNC_PIN, false);
		adi_gpio_SetHigh(SYNC_PORT, SYNC_PIN);

		return i8Result;
	}

	void CN0410::Reset()
	{
		/* Reset all led values */
		this->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_A, 0x00);
		this->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_B, 0x00);
		this->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_C, 0x00);
		this->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_D, 0x00);
	}

	void CN0410::UpdateDAC()
	{
		/* Update the DAC register by pulsing the LDAC pin */
		adi_gpio_SetHigh(LDAC_PORT, LDAC_PIN);
		adi_gpio_SetLow(LDAC_PORT, LDAC_PIN);
		adi_gpio_SetHigh(LDAC_PORT, LDAC_PIN);
	}

	int8_t CN0410::SendCommand(uint8_t u8Command,
				   uint8_t u8Channel, uint16_t u16Value)
	{
		ADI_SPI_RESULT eSpiResult;
		uint8_t u32Buffer[3];

		u32Buffer[0] = ((u8Command & 0x0F) << 4) + (u8Channel & 0x0F);
		u32Buffer[1] = u16Value >> 8;
		u32Buffer[2] = u16Value & 0xFF;

		m_transceive.TransmitterBytes = 3;
		m_transceive.ReceiverBytes    = 0x00u;
		m_transceive.pTransmitter     = &u32Buffer[0];
		m_transceive.pReceiver        = NULL;
		m_transceive.bRD_CTL          = false;
		/* auto increment both buffers */
		m_transceive.nTxIncrement = true;
		m_transceive.nRxIncrement = false;
		m_transceive.bDMA = false;

		adi_gpio_SetLow(SYNC_PORT, SYNC_PIN);

		eSpiResult = adi_spi_MasterReadWrite(m_spi_handle, &m_transceive);

		adi_gpio_SetHigh(SYNC_PORT, SYNC_PIN);

		if((eSpiResult) != ADI_SPI_SUCCESS)
			return -1;

		return 0;
	}

	uint32_t CN0410::ReadBack(uint8_t u8DacChannelAddr)
	{
		ADI_SPI_RESULT eSpiResult;
		uint32_t u32ChannelValue;
		uint8_t rxBuffer[3] = {0, 0, 0};
		uint8_t txBuffer[3] = {0, 0, 0};

		this->SendCommand(AD5686_SET_READBACK, u8DacChannelAddr, 0x00);

		m_transceive.TransmitterBytes = 3;
		m_transceive.ReceiverBytes    = 3;
		m_transceive.pTransmitter     = &txBuffer[0];
		m_transceive.pReceiver        = &rxBuffer[0];
		m_transceive.bRD_CTL          = false;
		/* auto increment both buffers */
		m_transceive.nTxIncrement = true;
		m_transceive.nRxIncrement = true;
		m_transceive.bDMA = false;

		adi_gpio_SetLow(SYNC_PORT, SYNC_PIN);

		eSpiResult = adi_spi_MasterReadWrite(m_spi_handle, &m_transceive);

		adi_gpio_SetHigh(SYNC_PORT, SYNC_PIN);

		u32ChannelValue = (rxBuffer[0] << 16) + (rxBuffer[1] << 8) + rxBuffer[2];

		return u32ChannelValue;
	}

	CN0410::~CN0410()
	{
		AppPrintf("CN0410 object has been destroyed");
	}
}


