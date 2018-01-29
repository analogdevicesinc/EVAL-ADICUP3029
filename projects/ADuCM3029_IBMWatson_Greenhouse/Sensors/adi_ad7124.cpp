
#include "adi_ad7124.h"
#include "adi_ad7124_cfg.h"
#include "Timer.h"

uint8_t convFlag = 0;

namespace adi_sensor_swpack
{
	/**
	 * @brief AD7790 constructor, sets CS pin and SPI format
	 * @param CS - (optional)chip select of the AD7790
	 * @param MOSI - (optional)pin of the SPI interface
	 * @param MISO - (optional)pin of the SPI interface
	 * @param SCK  - (optional)pin of the SPI interface
	 */
	AD7124::AD7124()
	{
		this->regs = ad7124_regs;
		this->check_ready = 0;
		this->useCRC = AD7124_DISABLE_CRC;
		this->spi_rdy_poll_cnt = 25000;
		this->ad7124_reg_access = AD7124_RW;
	}

	/***************************************************************************//**
	 * @brief Reads the value of the specified register without checking if the
	 *        device is ready to accept user requests.
	 *
	 * @param device - The handler of the instance of the driver.
	 * @param pReg - Pointer to the register structure holding info about the
	 *               register to be read. The read value is stored inside the
	 *               register structure.
	 *
	 * @return Returns 0 for success or negative error code.
	 *******************************************************************************/
	int32_t AD7124::NoCheckReadRegister(ad7124_st_reg* pReg)
	{
		int32_t ret       = 0;
		uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		uint8_t i         = 0;
		uint8_t check8    = 0;
		uint8_t msgBuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};


		check8 = useCRC;

		/* Build the Command word */
		buffer[0] = AD7124_COMM_REG_WEN | AD7124_COMM_REG_RD |
				AD7124_COMM_REG_RA(pReg->addr);

		/* Read data from the device */
		ret = SPI_Read(buffer,
				((useCRC != AD7124_DISABLE_CRC) ? pReg->size + 1
						: pReg->size) + 1);
		if(ret < 0)
			return ret;

		/* Check the CRC */
		if(check8 == AD7124_USE_CRC) {
			msgBuf[0] = AD7124_COMM_REG_WEN | AD7124_COMM_REG_RD |
					AD7124_COMM_REG_RA(pReg->addr);
			for(i = 1; i < pReg->size + 2; ++i) {
				msgBuf[i] = buffer[i];
			}
			check8 = ComputeCRC8(msgBuf, pReg->size + 2);
		}

		if(check8 != 0) {
			/* ReadRegister checksum failed. */
			return COMM_ERR;
		}

		/* Build the result */
		pReg->value = 0;
		for(i = 1; i < pReg->size + 1; i++) {
			pReg->value <<= 8;
			pReg->value += buffer[i];
		}

		return ret;
	}

	/***************************************************************************//**
	 * @brief Writes the value of the specified register without checking if the
	 *        device is ready to accept user requests.
	 *
	 * @param device - The handler of the instance of the driver.
	 * @param reg - Register structure holding info about the register to be written
	 *
	 * @return Returns 0 for success or negative error code.
	 *******************************************************************************/
	int32_t AD7124::NoCheckWriteRegister(ad7124_st_reg reg)
	{
		int32_t ret      = 0;
		int32_t regValue = 0;
		uint8_t wrBuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		uint8_t i        = 0;
		uint8_t crc8     = 0;


		/* Build the Command word */
		wrBuf[0] = AD7124_COMM_REG_WEN | AD7124_COMM_REG_WR |
				AD7124_COMM_REG_RA(reg.addr);

		/* Fill the write buffer */
		regValue = reg.value;
		for(i = 0; i < reg.size; i++) {
			wrBuf[reg.size - i] = regValue & 0xFF;
			regValue >>= 8;
		}

		/* Compute the CRC */
		if(useCRC != AD7124_DISABLE_CRC) {
			crc8 = ComputeCRC8(wrBuf, reg.size + 1);
			wrBuf[reg.size + 1] = crc8;
		}

		/* Write data to the device */
		ret = SPI_Write(wrBuf,
				(useCRC != AD7124_DISABLE_CRC) ? reg.size + 2
						: reg.size + 1);

		return ret;
	}

	/***************************************************************************//**
	 * @brief Reads the value of the specified register only when the device is ready
	 *        to accept user requests. If the device ready flag is deactivated the
	 *        read operation will be executed without checking the device state.
	 *
	 * @param device - The handler of the instance of the driver.
	 * @param pReg - Pointer to the register structure holding info about the
	 *               register to be read. The read value is stored inside the
	 *               register structure.
	 *
	 * @return Returns 0 for success or negative error code.
	 *******************************************************************************/
	int32_t AD7124::ReadRegister(ad7124_st_reg* pReg)
	{
		int32_t ret;

		if (pReg->addr != ERR_REG && check_ready) {
			ret = WaitForSpiReady(spi_rdy_poll_cnt);
			if (ret < 0)
				return ret;
		}
		ret = NoCheckReadRegister(pReg);

		return ret;
	}

	/***************************************************************************//**
	 * @brief Writes the value of the specified register only when the device is
	 *        ready to accept user requests. If the device ready flag is deactivated
	 *        the write operation will be executed without checking the device state.
	 *
	 * @param device - The handler of the instance of the driver.
	 * @param reg - Register structure holding info about the register to be written
	 *
	 * @return Returns 0 for success or negative error code.
	 *******************************************************************************/
	int32_t AD7124::WriteRegister(ad7124_st_reg pReg)
	{
		int32_t ret;

		if (check_ready) {
			ret = WaitForSpiReady(spi_rdy_poll_cnt);
			if (ret < 0)
				return ret;
		}
		ret = NoCheckWriteRegister(pReg);

		return ret;
	}

	/***************************************************************************//**
	 * @brief Reads and returns the value of a device register. The read value is
	 *        also stored in software register list of the device.
	 *
	 * @param device - The handler of the instance of the driver.
	 * @param reg - Which register to read from.
	 * @param pError - Pointer to the location where to store the error code if an
	 *                 error occurs. Stores 0 for success or negative error code.
	 *                 Does not store anything if pErorr = NULL;
	 *
	 * @return Returns the value read from the specified register.
	 *******************************************************************************/
	uint32_t AD7124::ReadDeviceRegister(enum ad7124_registers reg)
	{
		ReadRegister(&regs[reg]);
		return (regs[reg].value);
	}

	/***************************************************************************//**
	 * @brief Writes the specified value to a device register. The value to be
	 *        written is also stored in the software register list of the device.
	 *
	 * @param device - The handler of the instance of the driver.
	 * @param reg - Which register to write to.
	 * @param value - The value to be written to the reigster of the device.
	 *
	 * @return Returns 0 for success or negative error code.
	 *******************************************************************************/
	int32_t AD7124::WriteDeviceRegister(enum ad7124_registers reg, uint32_t value)
	{
		regs[reg].value = value;
		return(WriteRegister(regs[reg]));
	}

	/***************************************************************************//**
	 * @brief Resets the device.
	 *
	 * @param device - The handler of the instance of the driver.
	 *
	 * @return Returns 0 for success or negative error code.
	 *******************************************************************************/
	int32_t AD7124::Reset()
	{
		int32_t ret = 0;
		uint8_t wrBuf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

		ret = SPI_Write( wrBuf, 8);

		timer_sleep(200);

		return ret;
	}

	/***************************************************************************//**
	 * @brief Waits until the device can accept read and write user actions.
	 *
	 * @param device - The handler of the instance of the driver.
	 * @param timeout - Count representing the number of polls to be done until the
	 *                  function returns.
	 *
	 * @return Returns 0 for success or negative error code.
	 *******************************************************************************/
	int32_t AD7124::WaitForSpiReady(uint32_t timeout)
	{
		ad7124_st_reg *regs;
		int32_t ret;
		int8_t ready = 0;

		regs = this->regs;

		while(!ready && --timeout) {
			/* Read the value of the Error Register */
			ret = ReadRegister(&regs[AD7124_Error]);
			if(ret < 0)
				return ret;

			/* Check the SPI IGNORE Error bit in the Error Register */
			ready = (regs[AD7124_Error].value &
					AD7124_ERR_REG_SPI_IGNORE_ERR) == 0;
		}

		return timeout ? 0 : TIMEOUT;
	}

	/***************************************************************************//**
	 * @brief Waits until a new conversion result is available.
	 *
	 * @param device - The handler of the instance of the driver.
	 * @param timeout - Count representing the number of polls to be done until the
	 *                  function returns if no new data is available.
	 *
	 * @return Returns 0 for success or negative error code.
	 *******************************************************************************/
	int32_t AD7124::WaitForConvReady(uint32_t timeout)
	{
		ad7124_st_reg *regs;
		int32_t ret;
		int8_t ready = 0;

		regs = this->regs;

		while(!ready && --timeout) {
			/* Read the value of the Status Register */
			ret = ReadRegister(&regs[AD7124_Status]);
			if(ret < 0)
				return ret;

			/* Check the RDY bit in the Status Register */
			ready = (regs[AD7124_Status].value &
					AD7124_STATUS_REG_RDY) == 0;
		}

		return timeout ? 0 : TIMEOUT;
	}

	/***************************************************************************//**
	 * @brief Reads the conversion result from the device.
	 *
	 * @param device - The handler of the instance of the driver.
	 * @param pData - Pointer to store the read data.
	 *
	 * @return Returns 0 for success or negative error code.
	 *******************************************************************************/
	int32_t AD7124::ReadData( int32_t* pData)
	{
		int32_t ret       = 0;
		uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		uint8_t i         = 0;
		ad7124_st_reg *pReg;

		if( !pData)
			return INVALID_VAL;

		pReg = &regs[AD7124_Data];

		/* Build the Command word */
		buffer[0] = AD7124_COMM_REG_WEN | AD7124_COMM_REG_RD |
				AD7124_COMM_REG_RA(pReg->addr);

		SPI_Read(buffer, pReg->size + 1);


		if(ret < 0)
			return ret;

		/* Build the result */
		*pData = 0;
		for(i = 1; i < pReg->size + 1; i++) {
			*pData <<= 8;
			*pData += buffer[i];
		}
		return ret;
	}

	/***************************************************************************//**
	 * @brief Computes the CRC checksum for a data buffer.
	 *
	 * @param pBuf - Data buffer
	 * @param bufSize - Data buffer size in bytes
	 *
	 * @return Returns the computed CRC checksum.
	 *******************************************************************************/
	uint8_t AD7124::ComputeCRC8(uint8_t * pBuf, uint8_t bufSize)
	{
		uint8_t i   = 0;
		uint8_t crc = 0;

		while(bufSize) {
			for(i = 0x80; i != 0; i >>= 1) {
				if(((crc & 0x80) != 0) != ((*pBuf & i) != 0)) { /* MSB of CRC register XOR input Bit from Data */
					crc <<= 1;
					crc ^= AD7124_CRC8_POLYNOMIAL_REPRESENTATION;
				} else {
					crc <<= 1;
				}
			}
			pBuf++;
			bufSize--;
		}
		return crc;
	}


	/***************************************************************************//**
	 * @brief Updates the device SPI interface settings.
	 *
	 * @param device - The handler of the instance of the driver.
	 *
	 * @return None.
	 *******************************************************************************/
	void AD7124::UpdateDevSpiSettings()
	{
		ad7124_st_reg *regs;

		regs = this->regs;

		if (regs[AD7124_Error_En].value & AD7124_ERREN_REG_SPI_IGNORE_ERR_EN) {
			// check_ready = 1;
		} else {
			check_ready = 0;
		}
	}

	/***************************************************************************//**
	 * @brief Initializes the AD7124.
	 *
	 * @param device - The handler of the instance of the driver.
	 * @param slave_select - The Slave Chip Select Id to be passed to the SPI calls.
	 * @param regs - The list of registers of the device (initialized or not) to be
	 *               added to the instance of the driver.
	 *
	 * @return Returns 0 for success or negative error code.
	 *******************************************************************************/
	int32_t AD7124::Setup()
	{
		int32_t ret;
		enum ad7124_registers regNr;

		spi_rdy_poll_cnt = 25000;

		initSPI();
		/*  Reset the device interface.*/
		ret = Reset();
		if (ret < 0)
			return ret;

		check_ready = 0;

		/* Initialize registers AD7124_ADC_Control through AD7124_Filter_7. */
		for(regNr = AD7124_Status; (regNr < AD7124_Offset_0) && !(ret < 0);regNr = static_cast<ad7124_registers>(regNr + 1)) {
			if (regs[regNr].rw == AD7124_RW) {
				ret = WriteRegister(regs[regNr]);
				if (ret < 0)
					break;
			}

			/* Get CRC State and device SPI interface settings */
			if (regNr == AD7124_Error_En) {
				UpdateDevSpiSettings();
			}
		}

		return ret;
	}

	ADI_SPI_RESULT AD7124::initSPI()
	{
		ADI_SPI_RESULT eSpiResult;

		/* Initialize SPI driver */
		if ((eSpiResult = adi_spi_Open(AD7124_SPI_DEV_CFG , m_spi_memory, ADI_SPI_MEMORY_SIZE, &m_spi_handle)) == ADI_SPI_SUCCESS)
		{
			if((eSpiResult = adi_spi_SetMasterMode(m_spi_handle, AD7124_SPI_MASTER_CFG )) == ADI_SPI_SUCCESS)
			{
				if((eSpiResult = adi_spi_SetBitrate(m_spi_handle, AD7124_SPI_BITRATE_CFG )) == ADI_SPI_SUCCESS)
				{
					eSpiResult = adi_spi_SetChipSelect(m_spi_handle, ADI_SPI_CS_NONE );

				}
			}
		}

		/* Set the SPI clock polarity */
		if(adi_spi_SetClockPolarity(m_spi_handle, true) != ADI_SPI_SUCCESS)
		{
			eSpiResult =  ADI_SPI_FAILURE;
		}

		/* Set the SPI clock phase */
		if(adi_spi_SetClockPhase(m_spi_handle, true) != ADI_SPI_SUCCESS)
		{
			eSpiResult =  ADI_SPI_FAILURE;
		}

		adi_spi_SetContinuousMode(m_spi_handle, true);
//		adi_spi_SetIrqmode(m_spi_handle, true);

		return(eSpiResult);
	}

	int8_t AD7124::SPI_Read(uint8_t *data, uint8_t bytes_number)
	{

		uint8_t regValue[8];
		uint8_t u8Count = 0;
		ADI_SPI_RESULT eSpiResult;

		if(convFlag == 0)
		{
			m_transceive.bRD_CTL          = false;
			m_transceive.TransmitterBytes = bytes_number;
			m_transceive.ReceiverBytes    = bytes_number;
			m_transceive.pTransmitter     = data;
			m_transceive.pReceiver        = &regValue[0];
			m_transceive.nRxIncrement     = true;
			m_transceive.nTxIncrement     = true;

			adi_gpio_SetLow(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN);
			eSpiResult = adi_spi_MasterReadWrite(m_spi_handle, &m_transceive);
			adi_gpio_SetHigh(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN);

			if(eSpiResult != ADI_SPI_SUCCESS)
			{
				if(eSpiResult == ADI_SPI_HW_ERROR_OCCURRED)
				{
					//TODO: Set hardware error
				}

				return -1;
			}

			for (u8Count  = 0; u8Count < bytes_number; u8Count++)
			{
				data[u8Count]  = regValue[u8Count];
			}

			return (SENSOR_ERROR_NONE);
		}
		else
		{
			m_transceive.bRD_CTL          = false;
			m_transceive.TransmitterBytes = bytes_number;
			m_transceive.ReceiverBytes    = bytes_number;
			m_transceive.pTransmitter     = data;
			m_transceive.pReceiver        = &regValue[0];
			m_transceive.nRxIncrement     = true;
			m_transceive.nTxIncrement     = true;

			//adi_gpio_SetLow(SPI_CS_PORT, SPI_CS_PIN);
			eSpiResult = adi_spi_MasterReadWrite(m_spi_handle, &m_transceive);
			//adi_gpio_SetHigh(SPI_CS_PORT, SPI_CS_PIN);

			if(eSpiResult != ADI_SPI_SUCCESS)
			{
				if(eSpiResult == ADI_SPI_HW_ERROR_OCCURRED)
				{
					//TODO: Set hardware error
				}

				return -1;
			}

			for (u8Count  = 0; u8Count < bytes_number; u8Count++)
			{
				data[u8Count]  = regValue[u8Count];
			}

			return (SENSOR_ERROR_NONE);
		}

	}

	int8_t AD7124::SPI_Write(uint8_t *data, uint8_t bytes_number)
	{
		ADI_SPI_RESULT eSpiResult;
		uint8_t u8Count = 0;


		if(convFlag == 0)
		{
			m_transceive.TransmitterBytes = bytes_number;
			m_transceive.ReceiverBytes    = 0x00u;
			m_transceive.pTransmitter     = data;
			m_transceive.pReceiver        = NULL;
			m_transceive.bRD_CTL          = false;
			/* auto increment both buffers */
			m_transceive.nTxIncrement = true;
			m_transceive.nRxIncrement = false;
			m_transceive.bDMA = false;


			adi_gpio_SetLow(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN);
			eSpiResult = adi_spi_MasterReadWrite(m_spi_handle, &m_transceive);
			adi_gpio_SetHigh(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN);

			if((eSpiResult) != ADI_SPI_SUCCESS)
			{
				if(eSpiResult == ADI_SPI_HW_ERROR_OCCURRED)
				{
					//TODO: Set hardware error
				}

				return -1;
			}

			return(SENSOR_ERROR_NONE);
		}
		else
		{
			m_transceive.TransmitterBytes = bytes_number;
			m_transceive.ReceiverBytes    = 0x00u;
			m_transceive.pTransmitter     = data;
			m_transceive.pReceiver        = NULL;
			m_transceive.bRD_CTL          = false;
			/* auto increment both buffers */
			m_transceive.nTxIncrement = true;
			m_transceive.nRxIncrement = false;
			m_transceive.bDMA = false;


			//adi_gpio_SetLow(SPI_CS_PORT, SPI_CS_PIN);
			eSpiResult = adi_spi_MasterReadWrite(m_spi_handle, &m_transceive);
			//adi_gpio_SetHigh(SPI_CS_PORT, SPI_CS_PIN);

			if((eSpiResult) != ADI_SPI_SUCCESS)
			{
				if(eSpiResult == ADI_SPI_HW_ERROR_OCCURRED)
				{
					//TODO: Set hardware error
				}

				return -1;
			}

			return(SENSOR_ERROR_NONE);
		}

	}


	SENSOR_RESULT AD7124::init()
	{
//		ADI_SPI_RESULT eSpiResult;
//		SENSOR_RESULT eSensorResult;
//		uint8_t       nID;
//
//		/* Configure the SPI bus */
//		if((eSpiResult = this->initSPI()) == ADI_SPI_SUCCESS)
//		{
//			/* Reset the AD7124 */
//			if((eSensorResult = this->Reset()) == SENSOR_ERROR_NONE)
//			{
//				/* Read the ADC ID register to make sure it is responding properly */
//				if((eSensorResult = ReadRegister(&regs[AD7124_ID])) == SENSOR_ERROR_NONE)
//				{
//					if((regs[AD7124::ID_REG].value & AD7124_ID_MASK) != AD7124_ID_VALUE)
//					{
//						return(SET_SENSOR_ERROR(SENSOR_ERROR_ADC, AD7124::ERROR_CODE_ID_MISMATCH));
//					}
//				}
//			}
//		}
//		return(SET_SENSOR_ERROR(SENSOR_ERROR_SPI, eSpiResult));
	}

	SENSOR_RESULT AD7124::reset()
	{
		//ToDo
		return 0;
	}

	SENSOR_RESULT   AD7124::readRegister(uint32_t regAddress, uint32_t size, uint8_t * regValue)
	{

	}

	SENSOR_RESULT   AD7124::writeRegister(uint32_t regAddress, uint32_t size, uint32_t regValue)
	{
		//ToDo
		//Implement SPI write
		return 0;
	}

}
