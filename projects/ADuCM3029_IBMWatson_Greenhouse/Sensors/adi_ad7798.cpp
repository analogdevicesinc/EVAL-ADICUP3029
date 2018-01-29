/*! 
 *****************************************************************************
  @file  adi_ad7798.cpp
 
  @brief Defines the AD7798 interface file
 
 -----------------------------------------------------------------------------
Copyright (c) 2017 Analog Devices, Inc.

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

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-
INFRINGEMENT, TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF
CLAIMS OF INTELLECTUAL PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/

#include <adi_ad7798.h>
#include "adi_ad7798_cfg.h"
#include <drivers/gpio/adi_gpio.h>

#define AD7798_SPI_CS_PORT ADI_GPIO_PORT1
#define AD7798_SPI_CS_PIN  ADI_GPIO_PIN_14

namespace adi_sensor_swpack
{
    AD7798::AD7798()
    {
         /* Initialize SPI transceive structure static members  */
         m_transceive.nTxIncrement = 1u;
         m_transceive.nRxIncrement = 1u;
         m_transceive.bDMA         = AD7798_SPI_DMA_CFG ;
    }

    SENSOR_RESULT AD7798::init()
    {
        ADI_SPI_RESULT eSpiResult;
        SENSOR_RESULT eSensorResult;
        uint8_t       nID;

        uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE];

        adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE); //initialize gpio

        adi_gpio_OutputEnable(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN, true);
        adi_gpio_InputEnable(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN, false);    // Set INTACC_PORT as input
        adi_gpio_PullUpEnable(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN, false);
        adi_gpio_SetHigh(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);
        adi_gpio_SetLow(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);

        /* Configure the SPI bus */   
        if((eSpiResult = this->initSPI()) == ADI_SPI_SUCCESS)
        {
            /* Reset the AD7798 */
            if((eSensorResult = this->reset()) == SENSOR_ERROR_NONE)
            {
				/* Read the ADC ID register to make sure it is responding properly */
				if((eSensorResult = readRegister(AD7798::REGISTER_ID, 1u, &nID)) == SENSOR_ERROR_NONE)
				{
					if((nID & AD7798_ID_MASK) != AD7798_ID)
					{
						return(SET_SENSOR_ERROR(SENSOR_ERROR_ADC, AD7798::ERROR_CODE_ID_MISMATCH));
					}
					else
					{
                        return(applyStaticConfig());
					}
				}
			}
        }
        return(SET_SENSOR_ERROR(SENSOR_ERROR_SPI, eSpiResult));
    }


     SENSOR_RESULT AD7798::reset()
    {
        ADI_SPI_RESULT eSpiResult;

        /* Write 32 consecutive 1's on the SPI bus to issue a reset to the ADC */
        m_txBuffer[0] = AD7798_RESET;
        m_txBuffer[1] = AD7798_RESET;
        m_txBuffer[2] = AD7798_RESET;
        m_txBuffer[3] = AD7798_RESET;

        m_transceive.TransmitterBytes = 4u;
        m_transceive.ReceiverBytes    = 0u;
        m_transceive.bRD_CTL          = false;
        m_transceive.pTransmitter     = m_txBuffer;
        m_transceive.pReceiver        = NULL;
        
        adi_gpio_SetLow(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);
        if((eSpiResult = adi_spi_MasterReadWrite(m_spi_handle, &m_transceive)) != ADI_SPI_SUCCESS)
        {
        	adi_gpio_SetHigh(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);
            if(eSpiResult == ADI_SPI_HW_ERROR_OCCURRED)
            {
                //TODO: Set hardware error
            }

            return(SET_SENSOR_ERROR(SENSOR_ERROR_SPI, eSpiResult));
        }
        adi_gpio_SetHigh(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);

        return(SENSOR_ERROR_NONE);
    }


     SENSOR_RESULT   AD7798::readRegister(uint32_t regAddress, uint32_t size, uint8_t * regValue)
    {
        ADI_SPI_RESULT eSpiResult;
        uint8_t u8Count;

        ASSERT(size <= AD7798_MAX_REG_SIZE);
        ASSERT(regValue != NULL);



        m_txBuffer[0] = AD7798_COMM_READ_REG((uint8_t)regAddress);
        m_txBuffer[1] = 0x00;
        m_txBuffer[2] = 0x00;

        m_transceive.bRD_CTL          = false;
        m_transceive.TransmitterBytes = size + 1;
        m_transceive.ReceiverBytes    = size + 1;
        m_transceive.pTransmitter     = &m_txBuffer[0];
        m_transceive.pReceiver        = &au8Value[0];
        
        adi_gpio_SetLow(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);
        if((eSpiResult = adi_spi_MasterReadWrite(m_spi_handle, &m_transceive)) != ADI_SPI_SUCCESS)
        {
        	adi_gpio_SetHigh(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);
            if(eSpiResult == ADI_SPI_HW_ERROR_OCCURRED)
            {
                //TODO: Set hardware error
            }

            return(SET_SENSOR_ERROR(SENSOR_ERROR_SPI, eSpiResult));
        }
        adi_gpio_SetHigh(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);

        for (u8Count = 0; u8Count < size; u8Count++)
        {
        	regValue[u8Count] = au8Value[u8Count + 1];
        }

        return(SENSOR_ERROR_NONE);
    }


     SENSOR_RESULT   AD7798::writeRegister(uint32_t regAddress, uint32_t size, uint32_t regValue)
    {
        ADI_SPI_RESULT eSpiResult;

        ASSERT(size <= AD7798_MAX_REG_SIZE);

        m_txBuffer[0] = AD7798_COMM_WRITE_REG(regAddress);
        /* Bitwise operations are required to break regValue into single bytes for transmission */
        m_txBuffer[1] = ((regValue & 0xFF00u) >> 0x08u);
        m_txBuffer[2] = (regValue & 0xFFu);

        m_transceive.TransmitterBytes = size + 0x01u;
        m_transceive.ReceiverBytes    = 0x00u;
        m_transceive.pTransmitter     = &m_txBuffer[0];
        m_transceive.pReceiver        = NULL;
        m_transceive.bRD_CTL          = false;

        adi_gpio_SetLow(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);
        if((eSpiResult = adi_spi_MasterReadWrite(m_spi_handle, &m_transceive)) != ADI_SPI_SUCCESS)
        {
        	adi_gpio_SetHigh(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);
            if(eSpiResult == ADI_SPI_HW_ERROR_OCCURRED)
            {
                //TODO: Set hardware error
            }

            return(SET_SENSOR_ERROR(SENSOR_ERROR_SPI, eSpiResult));
        }
        adi_gpio_SetHigh(AD7798_SPI_CS_PORT, AD7798_SPI_CS_PIN);

        return(SENSOR_ERROR_NONE);
    }


    ADI_SPI_RESULT AD7798::initSPI()
    {
        ADI_SPI_RESULT eSpiResult;
   
        /* Initialize SPI driver */
        if ((eSpiResult = adi_spi_Open(AD7798_SPI_DEV_CFG , m_spi_memory, ADI_SPI_MEMORY_SIZE, &m_spi_handle)) == ADI_SPI_SUCCESS)
        {
        	if((eSpiResult = adi_spi_SetMasterMode(m_spi_handle, AD7798_SPI_MASTER_CFG )) == ADI_SPI_SUCCESS)
        	{
        		if((eSpiResult = adi_spi_SetBitrate(m_spi_handle, AD7798_SPI_BITRATE_CFG )) == ADI_SPI_SUCCESS)
        		{
        			if((eSpiResult = adi_spi_SetChipSelect(m_spi_handle, ADI_SPI_CS_NONE )) == ADI_SPI_SUCCESS)
        			{
        				adi_spi_SetClockPolarity (m_spi_handle, true);
        				adi_spi_SetClockPhase (m_spi_handle, true);
        				adi_spi_SetContinuousMode(m_spi_handle, true);
        			}

        		}
        	}
        }

        return(eSpiResult);
    }

    SENSOR_RESULT AD7798::applyStaticConfig(void)
    {
        SENSOR_RESULT eSensorResult;
        uint32_t   nRegVal;

        /* Set the AD7798 coding mode to that defined in the configuration file */
        if((eSensorResult = this->setCodingMode((AD7798::CODING_MODE)AD7798_CODING_MODE_CFG)) == SENSOR_ERROR_NONE)
        {
            /* Set the AD7798 gain to that defined in the configuration file */
            if((eSensorResult = this->setGain((AD7798::GAIN)AD7798_GAIN_CFG)) == SENSOR_ERROR_NONE)
            {
                /* Set the AD7798 filter to that defined in the configuration file */
                if((eSensorResult = this->setFilter((AD7798::FILTER_RATE)AD7798_FILTER_RATE_CFG)) == SENSOR_ERROR_NONE)
                {
					/* Set the AD7798 reference detect function to that defined in the configuration file */
					if((eSensorResult = this->setReference((AD7798::REF_DET)AD7798_REFERENCE_CFG)) == SENSOR_ERROR_NONE)
					{
						/* Set the AD7798 power switch control bit to that defined in the configuration file */
						/* Read the register to save off current state */
						if((eSensorResult = this->readRegister(AD7798::REGISTER_MODE, 2u, &m_rxBuffer[0])) == SENSOR_ERROR_NONE)
						{
							/* Update the current register value with the mode being set */
							nRegVal = AD7798_FORMAT_DATA(m_rxBuffer[0], m_rxBuffer[1]) & ~AD7798_MODE_PSW(0x1u);
							nRegVal |= AD7798_MODE_PSW(AD7798_PSW_CFG);

							if((eSensorResult = this->writeRegister(AD7798::REGISTER_MODE, 2u, nRegVal)) == SENSOR_ERROR_NONE)
							{

								eSensorResult = this->readRegister(AD7798::REGISTER_MODE, 2u, &m_rxBuffer[0]);

								/* Set the AD7798 burnout current but bit to that defined in the configuration file */
								/* Read the register to save off current state */
								if((eSensorResult = this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0])) == SENSOR_ERROR_NONE)
								{
									/* Update the current register value with the mode being set */
									nRegVal = AD7798_FORMAT_DATA(m_rxBuffer[0], m_rxBuffer[1]) & ~AD7798_CONF_BO(0x1u);
									nRegVal |= AD7798_CONF_BO(AD7798_BO_CFG);

									if((eSensorResult = this->writeRegister(AD7798::REGISTER_CONF, 2u, nRegVal)) == SENSOR_ERROR_NONE)
									{

										this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0]);

										/* Set the AD7798 buffered/unbuffered bit to that defined in the configuration file */
										/* Read the register to save off current state */
										if((eSensorResult = this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0])) == SENSOR_ERROR_NONE)
										{
											/* Update the current register value with the mode being set */
											nRegVal = AD7798_FORMAT_DATA(m_rxBuffer[0], m_rxBuffer[1]) & ~AD7798_CONF_BUF(0x1u);
											nRegVal |= AD7798_CONF_BUF(AD7798_BUF_CFG);

											return(this->writeRegister(AD7798::REGISTER_CONF, 2u, nRegVal));


										}
									}
								}
							}
						}
					}
                }
            }
        }
        return(eSensorResult);
    }


    SENSOR_RESULT AD7798::setChannel(AD7798::CHANNEL channel)
    {
        SENSOR_RESULT eSensorResult;
        uint16_t   nRegVal;
               
        /* Read the register to save off current state */
        eSensorResult = this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0]);

        if(eSensorResult == SENSOR_ERROR_NONE)
        {
            /* Update the current register value with the channel being set */
            nRegVal = AD7798_FORMAT_DATA(m_rxBuffer[0], m_rxBuffer[1]) & ~(0x7);
            nRegVal |= channel;

            eSensorResult = this->writeRegister(AD7798::REGISTER_CONF, 2u, nRegVal);
        }

        return(eSensorResult);
    }

    SENSOR_RESULT AD7798::setOperatingMode(AD7798::OPERATING_MODE mode)
    {
        SENSOR_RESULT eSensorResult;
        uint32_t   nRegVal;
        uint8_t nID;
        
        if((eSensorResult = readRegister(AD7798::REGISTER_ID, 1u, &nID)) == SENSOR_ERROR_NONE)
        {
        	if((nID & AD7798_ID_MASK) != AD7798_ID)
        	{
        		return(SET_SENSOR_ERROR(SENSOR_ERROR_ADC, AD7798::ERROR_CODE_ID_MISMATCH));
        	}
        }

        /* Read the register to save off current state */
        eSensorResult = this->readRegister(AD7798::REGISTER_MODE, 2u, &m_rxBuffer[0]);

        if(eSensorResult == SENSOR_ERROR_NONE)
        {
            /* Update the current register value with the mode being set */
            nRegVal = AD7798_FORMAT_DATA(m_rxBuffer[0], m_rxBuffer[1]) & ~AD7798_MODE_SEL(0x7u);
            nRegVal |= AD7798_MODE_SEL(mode);

            eSensorResult = this->writeRegister(AD7798::REGISTER_MODE, 2u, nRegVal);
        }

        return (eSensorResult);
    }


    SENSOR_RESULT AD7798::setGain(AD7798::GAIN gain)
    {
        SENSOR_RESULT eSensorResult;
        uint16_t   nRegVal;
        
        m_gain = gain;

        /* Read the register to save off current state */
        eSensorResult = this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0]);

        if(eSensorResult == SENSOR_ERROR_NONE)
        {
            /* Update the current register value with the gain being set */
            nRegVal = AD7798_FORMAT_DATA(m_rxBuffer[0], m_rxBuffer[1]) & ~AD7798_CONF_GAIN(0x07u);
            nRegVal |= AD7798_CONF_GAIN(gain);

            eSensorResult = this->writeRegister(AD7798::REGISTER_CONF, 2u, nRegVal);

            this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0]);
        }

        return (eSensorResult);
    }

    AD7798::GAIN   AD7798::getGain()
    {
    	return(m_gain);
    }


    SENSOR_RESULT AD7798::setCodingMode(AD7798::CODING_MODE mode)
    {
        SENSOR_RESULT eSensorResult;
        uint16_t   nRegVal;

        /* Read the register to save off current state */
        eSensorResult = this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0]);

        if(eSensorResult == SENSOR_ERROR_NONE)
        {
            /* Update the current register value with the mode being set */
            nRegVal = AD7798_FORMAT_DATA(m_rxBuffer[0], m_rxBuffer[1]) & ~AD7798_CONF_CODING(0x01u);
            nRegVal |= AD7798_CONF_CODING(mode);

            eSensorResult = this->writeRegister(AD7798::REGISTER_CONF, 2u, nRegVal);

            this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0]);
        }

        return(eSensorResult);
    }

    SENSOR_RESULT AD7798::setFilter(AD7798::FILTER_RATE rate)
    {
        SENSOR_RESULT eSensorResult;
        uint16_t   nRegVal;
        
        /* Read the register to save off current state */
        eSensorResult = this->readRegister(AD7798::REGISTER_MODE, 2u, &m_rxBuffer[0]);

        if(eSensorResult == SENSOR_ERROR_NONE)
        {
            /* Update the current register value with the filter rate being set */
            nRegVal = AD7798_FORMAT_DATA(m_rxBuffer[0], m_rxBuffer[1]) & ~AD7798_MODE_RATE(0xFu);
            nRegVal |= AD7798_MODE_RATE(rate); 

            eSensorResult = this->writeRegister(AD7798::REGISTER_MODE, 2u, nRegVal);

            this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0]);
        }

        return (eSensorResult);
    }

   
    SENSOR_RESULT AD7798::setReference(AD7798::REF_DET reference)
    {
        SENSOR_RESULT eSensorResult;
        uint16_t   nRegVal;
        
        /* Read the register to save off current state */
        eSensorResult = this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0]);

        if(eSensorResult == SENSOR_ERROR_NONE)
        {
            /* Update the current register value with the reference flag being set */
            nRegVal = AD7798_FORMAT_DATA(m_rxBuffer[0], m_rxBuffer[1]) & ~AD7798_CONF_REFDET(0x1u) ;
            nRegVal |= AD7798_CONF_REFDET(reference);

            eSensorResult = this->writeRegister(AD7798::REGISTER_CONF, 2u, nRegVal);

            this->readRegister(AD7798::REGISTER_CONF, 2u, &m_rxBuffer[0]);
        }

        return (eSensorResult);
    }


    SENSOR_RESULT AD7798::fullScaleSystemCalibration()
    {
        SENSOR_RESULT eSensorResult;
        uint32_t   nTicks = 0;
        
        /* Set the operating mode to full system calibration to begin the calibration */
        if((eSensorResult = setOperatingMode(AD7798::OPERATING_MODE_CAL_SYS_FULL)) == SENSOR_ERROR_NONE)
        {
            /* Poll the status register for ready signal to go low. The ready bit will be cleared once the calibration is complete. */
            do
            {
                if((eSensorResult = this->readRegister(AD7798::REGISTER_COMM_STAT, 1u, &m_rxBuffer[0])) != SENSOR_ERROR_NONE)
                {
                    return(eSensorResult);
                }
                
                /* Check for timeout */
                if(nTicks == AD7798_TRANSACTION_TIMEOUT)
                {
                    return(SET_SENSOR_ERROR(SENSOR_ERROR_ADC, AD7798::ERROR_CODE_RDY_TIMEOUT));
                }

                nTicks++;

            }while (((m_rxBuffer[0] & AD7798_STAT_RDY) == AD7798_STAT_RDY));
        }

        return (eSensorResult);
    }


    SENSOR_RESULT AD7798::zeroScaleSystemCalibration()
    {
        SENSOR_RESULT eSensorResult;
        uint32_t   nTicks = 0;
        
        /* Set the operating mode to zero system calibration to begin the calibration */
        if((eSensorResult = setOperatingMode(AD7798::OPERATING_MODE_CAL_SYS_ZERO)) == SENSOR_ERROR_NONE)
        {
            /* Poll the status register for ready signal to go low. The ready bit will be cleared once the calibration is complete. */
            do
            {
                if((eSensorResult = this->readRegister(AD7798::REGISTER_COMM_STAT, 1u, &m_rxBuffer[0])) != SENSOR_ERROR_NONE)
                {
                    return(eSensorResult);
                }
                
                /* Check for timeout */
                if(nTicks == AD7798_TRANSACTION_TIMEOUT)
                {
                    return(SET_SENSOR_ERROR(SENSOR_ERROR_ADC, AD7798::ERROR_CODE_RDY_TIMEOUT));
                }

                nTicks++;

            }while (((m_rxBuffer[0] & AD7798_STAT_RDY) == AD7798_STAT_RDY));
        }

        return (eSensorResult);
    }

    SENSOR_RESULT AD7798::getData(uint16_t * data)
    {
        SENSOR_RESULT eSensorResult;
        uint32_t   nTicks = 0;

		/* Poll the status register for ready signal to go low. The ready bit will be cleared once the conversion is complete. */
		do
		{
			if((eSensorResult = this->readRegister(AD7798::REGISTER_COMM_STAT, 1u, &m_rxBuffer[0])) != SENSOR_ERROR_NONE)
			{
				return(eSensorResult);
			}

			/* Check for timeout */
			if(nTicks == AD7798_TRANSACTION_TIMEOUT)
			{
				return(SET_SENSOR_ERROR(SENSOR_ERROR_ADC, AD7798::ERROR_CODE_RDY_TIMEOUT));
			}

			nTicks++;

		}while (((m_rxBuffer[0] & AD7798_STAT_RDY) == AD7798_STAT_RDY));

		/* Read the Data register */
		if((eSensorResult = this->readRegister(AD7798::REGISTER_DATA, 2u, &m_rxBuffer[0])) == SENSOR_ERROR_NONE)
		{
			*data =  (uint16_t) AD7798_FORMAT_DATA(m_rxBuffer[0], m_rxBuffer[1]);
		}
        return (eSensorResult);
    }
}

