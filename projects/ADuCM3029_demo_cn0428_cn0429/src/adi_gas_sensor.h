/**
******************************************************************************
*   @file     adi_gas_sensor.h
*   @brief    Gas sensor class object header file
*   @version  V0.1
*   @author   ADI
*
*******************************************************************************
*
*Copyright 2015-2018(c) Analog Devices, Inc.
*
*All rights reserved.
*
*Redistribution and use in source and binary forms, with or without modification,
*are permitted provided that the following conditions are met:
*    - Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the following disclaimer.
*    - Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the following disclaimer in
*      the documentation and/or other materials provided with the
*      distribution.
*    - Neither the name of Analog Devices, Inc. nor the names of its
*      contributors may be used to endorse or promote products derived
*      from this software without specific prior written permission.
*    - The use of this software may or may not infringe the patent rights
*      of one or more patent holders.  This license does not release you
*      from the requirement that you obtain separate licenses from these
*      patent holders to use this software.
*    - Use of the software either in source or binary form, must be run
*      on or directly connected to an Analog Devices Inc. component.
*
*THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
*PARTICULAR PURPOSE ARE DISCLAIMED.
*
*IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
*RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
*BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
**/

#ifndef ADI_GAS_SENSOR_H_
#define ADI_GAS_SENSOR_H_

#include "base_sensor/adi_sensor.h"
#include "adi_m355_gas_sensor_config.h"

namespace adi_sensor_swpack
{
/**
 * @class Temperature.
 *
 * @brief Temperature class abstracts the temperature sesor functionality.
 *
 **/
class Gas_Reading : public Sensor
{
public:

	/*!
	 * @brief   Returns temperature.
	*
	              * @param [in]  pTemperature Pointer where temperature data is returned.
		*
	              * @param [in]  sizeInBytes  Size of the buffer in bytes. The value depends on the
		*              underlying temperature sensor. ADT7420 requires 2-bytes to return
		*              the temperature data.
		*
	              * @return   SENSOR_RESULT  In case of success SENSOR_ERROR_NONE is returned. Upon
	              *           failure applications must use GET_SENSOR_ERROR_TYPE() and
	              *           GET_DRIVER_ERROR_CODE() to determine the error.
		*
	              * @details  This method is used to return the temperature.
	              */

	virtual SENSOR_RESULT SensorInit(uint8_t sensor_address) = 0;

	virtual SENSOR_RESULT SetI2CAddr(uint8_t new_I2C_address) = 0;

	virtual SENSOR_RESULT ReadTemperature(int16_t *pSensData) = 0;

	virtual SENSOR_RESULT ReadHumidity(int16_t *pSensData) = 0;

	virtual SENSOR_RESULT SetMeasurementTime(uint16_t pCfgData) = 0;

	virtual SENSOR_RESULT ReadMeasurementTime(uint16_t *pCfgData) = 0;

	virtual SENSOR_RESULT StartMeasurements() = 0;

	virtual SENSOR_RESULT StopMeasurements() = 0;

	virtual SENSOR_RESULT SetTIAGain(uint8_t pCfgData) = 0;

	virtual SENSOR_RESULT ReadTIAGain(uint8_t *pCfgData) = 0;

	virtual SENSOR_RESULT SetSensorBias(int16_t pCfgData) = 0;

	virtual SENSOR_RESULT ReadSensorBias(int16_t *pCfgData) = 0;

	virtual SENSOR_RESULT SetSensorSensitivity(uint32_t pCfgData) = 0;

	virtual SENSOR_RESULT ReadSensorSensitivity(uint32_t *pCfgData) = 0;

	virtual SENSOR_RESULT ConfigureTempComp(uint8_t pCfgData) = 0;

	virtual SENSOR_RESULT ReadTempCompCfg(uint8_t *pCfgData) = 0;

	virtual SENSOR_RESULT RunEISMeasurement() = 0;

	virtual SENSOR_RESULT ReadEISResults(uint8_t *pSensData) = 0;

	virtual SENSOR_RESULT ReadEISResultsFull(uint8_t *pSensData) = 0;

	virtual SENSOR_RESULT Read200RCal(uint8_t *pSensData) = 0;

	virtual SENSOR_RESULT RunPulseTest(uint8_t pulseAmplitude,
					   uint8_t pulseDuration) = 0;

	virtual SENSOR_RESULT ReadPulseTestResults(uint8_t *pSensData,
			uint8_t pulseAmplitude, uint8_t pulseDuration) = 0;

	virtual SENSOR_RESULT SetRload(uint8_t pCfgData) = 0;

	virtual SENSOR_RESULT ReadRload(uint8_t *pCfgData) = 0;

	virtual SENSOR_RESULT ReadDataPPB(int32_t *pSensData) = 0;

	virtual SENSOR_RESULT ReadDataBits(uint16_t *pSensData) = 0;

	virtual SENSOR_RESULT openWithAddr(uint8_t sensor_address) = 0;

	virtual SENSOR_RESULT I2CReadWrite(uint8_t RW, uint8_t RegAddr, uint8_t *pData,
					   uint16_t size) = 0;



	/*!
	 * @brief    Opens the temperature sensor and configures it.
	 *
	 * @return   SENSOR_RESULT  In case of success SENSOR_ERROR_NONE is returned. Upon
	 *           failure applications must use GET_SENSOR_ERROR_TYPE() and
	 *           GET_DRIVER_ERROR_CODE() to determine the error.
	 *
	 * @details  Initializes the temperature sensor and configures it. Temperature
	 *           sensors use serial interfaces like SPI or I2C. This function opens the
	 *           underlying peripheral and configures it with the default configuration
	 *           parameters. Default configuration parameters are statically defined in
	 *           the associated sensor class configuration header.For example the
	 *           configuration for ADT7420 is defined in adi_adt7420_cfg.h and the sensor
	 *           software is implemented in adi_adt7420.cpp
	 */
	virtual SENSOR_RESULT   open() = 0;

	/*!
	 * @brief   Start function puts the temperature sensor in measurement mode.
	 *
	 * @return   SENSOR_RESULT  In case of success SENSOR_ERROR_NONE is returned. Upon
	 *           failure applications must use GET_SENSOR_ERROR_TYPE() and
	 *           GET_DRIVER_ERROR_CODE() to determine the error.
	 *
	 * @details  Start function should be called only after the temperature sensor is
	 *           successfully opened. Start puts the temperature sensor in measurement mode.
	 *           Once in measurement mode temperature sensor's measure acceleration on x,y,z
	 *           axis.
	 */
	virtual SENSOR_RESULT   start() = 0;

	/*!
	 * @brief    Stops the temperature sensor.
	 *
	 * @return   SENSOR_RESULT  In case of success SENSOR_ERROR_NONE is returned. Upon
	 *           failure applications must use GET_SENSOR_ERROR_TYPE() and
	 *           GET_DRIVER_ERROR_CODE() to determine the error.
	 *
	 * @details  Stop function of an temperature sensor puts the temperature sensor
	*           in standby
	              *           mode. With this temperature sensor no longer measures the acceleration.
	              *           Applications can issue start function to enable measurement mode.
	              */
	virtual SENSOR_RESULT   stop() = 0;

	/*!
	 * @brief    Stops the temperature sensor and closes the underlying peripheral
	 *
	 * @return   SENSOR_RESULT  In case of success SENSOR_ERROR_NONE is returned. Upon
	 *           failure applications must use GET_SENSOR_ERROR_TYPE() and
	 *           GET_DRIVER_ERROR_CODE() to determine the error.
	 *
	 * @details  Close function stops the measurements and temperature sensor is kept in
	 *           stanby mode. Underlying peripheral is closed. Applications must use
	 *           open again inorder to enable the measurement mode.
	 */
	virtual SENSOR_RESULT   close() = 0;

	/* Constructor */
	Gas_Reading() { }

	/* Destructor */
	virtual ~Gas_Reading()  { }

	/**
	 * @brief Returns I2C slave address
	*
	              * @return i2c slave address
		*
	              * @details This method returns i2c slave address
	              */
	uint8_t getSlaveAddress()
	{
		return m_slave_addr;
	}

	/**
	 * @brief  sets the i2c slave address
	*
	              * @param  slaveAddr i2c slave address
		*
	              * @details This method is used to set the i2c slave address
	              */
	void    setSlaveAddress(const uint8_t slaveAddr)
	{
		m_slave_addr = slaveAddr;
	}

protected:

	uint8_t m_slave_addr;
};
}

#endif /* ADI_GAS_SENSOR_H_ */
