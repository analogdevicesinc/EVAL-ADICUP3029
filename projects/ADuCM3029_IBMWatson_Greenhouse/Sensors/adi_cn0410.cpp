

#include "adi_cn0410.h"
#include "adi_cn0410_cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

namespace adi_sensor_swpack {

uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE];

	CN0410::CN0410(VisibleLight   *pLight)
	{
		//Constructor for CN0410
		this->pLight = pLight;
	}

	int8_t CN0410::Init()
	{
		//Initialization for SPI and GPIO
		ADI_SPI_RESULT eSpiResult;
		int8_t i8Result;

		/* Initialize SPI driver */
		if ((eSpiResult = adi_spi_Open(CN0410_SPI_DEV_CFG , m_spi_memory, ADI_SPI_MEMORY_SIZE, &m_spi_handle)) != ADI_SPI_SUCCESS)
		{
			i8Result = -1;
		}

		if((eSpiResult = adi_spi_SetMasterMode(m_spi_handle, CN0410_SPI_MASTER_CFG )) != ADI_SPI_SUCCESS)
		{
			i8Result = -1;
		}
		if((eSpiResult = adi_spi_SetBitrate(m_spi_handle, CN0410_SPI_BITRATE_CFG )) != ADI_SPI_SUCCESS)
		{
			i8Result = -1;
		}

		if((eSpiResult = adi_spi_SetChipSelect(m_spi_handle, CN0410_SPI_CS_CFG )) != ADI_SPI_SUCCESS)
		{
			i8Result = -1;
		}

		/* Set the SPI clock polarity */
		if(adi_spi_SetClockPolarity(m_spi_handle, true) != ADI_SPI_SUCCESS)
		{
			i8Result = -1;
		}

		/* Set the SPI clock phase */
		if(adi_spi_SetClockPhase(m_spi_handle, true) != ADI_SPI_SUCCESS)
		{
			i8Result = -1;
		}

		if(adi_spi_SetContinuousMode(m_spi_handle, true) != ADI_SPI_SUCCESS)
		{
			i8Result = -1;
		}
		if (adi_spi_SetIrqmode(m_spi_handle, true) != ADI_SPI_SUCCESS)
		{
			i8Result = -1;
		}

		adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE); //initialize gpio

		//Setup LDAC & Reset pins
		adi_gpio_OutputEnable(LDAC_PORT, LDAC_PIN, true);
		adi_gpio_InputEnable(LDAC_PORT, LDAC_PIN, false);
		adi_gpio_PullUpEnable(LDAC_PORT, LDAC_PIN, false);
		adi_gpio_SetLow(LDAC_PORT, LDAC_PIN);

		adi_gpio_OutputEnable(RESET_PORT, RESET_PIN, true);
		adi_gpio_InputEnable(RESET_PORT, RESET_PIN, false);
		adi_gpio_PullUpEnable(RESET_PORT, RESET_PIN, false);
		adi_gpio_SetHigh(RESET_PORT, RESET_PIN);

		//set chip select pin
		adi_gpio_OutputEnable(SYNC_PORT, SYNC_PIN, true);
		adi_gpio_InputEnable(SYNC_PORT, SYNC_PIN, false);
		adi_gpio_PullUpEnable(SYNC_PORT, SYNC_PIN, false);
		adi_gpio_SetHigh(SYNC_PORT, SYNC_PIN);

		return(i8Result);
	}

	void CN0410::Reset()
	{
		//Reset all led values
		this->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_A, 0x00);
		this->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_B, 0x00);
		this->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_C, 0x00);
		this->SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_D, 0x00);
	}

	void CN0410::SetChannelLuxValue(float fpValue)
	{
		//Set lux value for DAC
	}

	void CN0410::SetChannelValue(uint16_t u16Value)
	{
		//Set value for DAC
	}

	void CN0410::UpdateDAC()
	{
		//Update the DAC register by pulsing the LDAC pin
		adi_gpio_SetHigh(LDAC_PORT, LDAC_PIN);
		adi_gpio_SetLow(LDAC_PORT, LDAC_PIN);
	}

	int8_t CN0410::SendCommand(uint8_t u8Command, uint8_t u8Channel, uint16_t u16Value)
	{
		ADI_SPI_RESULT eSpiResult;
		uint8_t u32Buffer[3];

		uint8_t test = (u8Command & 0x0F) << 4;

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
		{
			if(eSpiResult == ADI_SPI_HW_ERROR_OCCURRED)
			{
				//TODO: Set hardware error
			}

			return -1;
		}

		return(0);

	}

	uint32_t CN0410::ReadBack(uint8_t u8DacChannelAddr)
	{
		ADI_SPI_RESULT eSpiResult;
		uint32_t u32ChannelValue = 0;
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

	void CN0410::Setup(void){

		SendCommand(AD5686_ITERNAL_REFERENCE, 0x00, 0x00); //enable internal reference
		SendCommand(AD5686_POWER, 0x00, 0x00); //normal power mode for all channels
		SendCommand(AD5686_POWER, 0x00, 0x00); //normal power mode for all channels
		SendCommand(AD5686_POWER, 0x00, 0x00); //normal power mode for all channels
		SendCommand(AD5686_POWER, 0x00, 0x00); //normal power mode for all channels

		timer_sleep(10); //10 ms delay

		SendCommand(AD5686_RESET, 0x00, 0x00); //soft reset to zero scale
		Reset(); // reset all leds
	}

	void CN0410::Controll(float red, float blue, float green)
	{

		struct stLedCommands LedCommand = {0,0,0};
		ADI_VISUAL_LIGHT_DATA eIntensity;
		bool valid = true;
		float fpRedError, fpBlueError, fpGreenError;
		float fpRedI, fpBlueI, fpGreenI;
		float kp =0.6, ki = 0.04;
		float commandRed, commandBlue, commandGreen;
		uint32_t u32Timeout = 0; //3 sec timeout


		/*PI controller*/
		while(valid)
		{
			u32Timeout++;

			pLight->getLightIntensity((float *)&eIntensity.fData_Red);

			fpRedError = red - eIntensity.fData_Red;
			fpBlueError = blue - eIntensity.fData_Blue;
			fpGreenError = green - eIntensity.fData_Green;

			if ( (fabs(fpRedError) < 50) && (fabs(fpBlueError) < 50) && (fabs(fpGreenError) < 50))
			{
				break;
			}

			fpRedI += fpRedError;
			fpBlueI += fpBlueError;
			fpGreenI += fpGreenError;

			commandRed = (kp *  fpRedError) + (ki * fpRedI);
			commandBlue = (kp *  fpBlueError) + (ki * fpBlueI);
			commandGreen = (kp *  fpGreenError) + (ki * fpGreenI);

			if (commandRed > 65535)
			{
				LedCommand.u16RedCommand = 65535;
			}
			else
			{
				if (commandRed < 0)
				{
					LedCommand.u16RedCommand = 0;
				}
				else
				{
					LedCommand.u16RedCommand = (uint16_t)commandRed;
				}
			}

			if (commandBlue > 65535)
			{
				LedCommand.u16BlueCommand = 65535;
			}
			else
			{
				if (commandBlue < 0)
				{
					LedCommand.u16BlueCommand = 0;
				}
				else
				{
					LedCommand.u16BlueCommand = (uint16_t)commandBlue;
				}
			}

			if (commandGreen > 65535)
			{
				LedCommand.u16GreenCommand = 65535;
			}
			else
			{
				if (commandGreen < 0)
				{
					LedCommand.u16GreenCommand = 0;
				}
				else
				{
					LedCommand.u16GreenCommand = (uint16_t)commandGreen;
				}
			}

			SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_B, LedCommand.u16BlueCommand);
			SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_C, LedCommand.u16RedCommand);
			SendCommand(AD5686_WRITE_UPDATE, AD5686_DAC_A, LedCommand.u16GreenCommand);

			if (u32Timeout == 200)
			{
				break;
			}
		}
	}
}


