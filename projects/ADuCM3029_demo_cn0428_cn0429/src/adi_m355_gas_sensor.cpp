/*******************************************************************************
 *   @file     adi_m355_gas_sensor.cpp
 *   @brief    Gas sensor class object source file
 *   @version  V0.2
 *   @author   ADI
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
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

#include "adi_m355_gas_sensor.h"
#include "adi_m355_gas_sensor_config.h"
#include "base_sensor/adi_sensor_errors.h"
#include "ADuCM3029_demo_cn0428_cn0429.h"

namespace adi_sensor_swpack
{

/*
 * Open M355 gas sensor
 */
SENSOR_RESULT M355_GAS::open()
{
	pADI_GPIO0->DS |= (1 << 4) | (1 << 5);
	this->m_i2c_address = ADI_CFG_I2C_DEFAULT_ADDR;
	return this->InitI2C();
}

/*
 * Open M355 gas sensor with selected address
 */
SENSOR_RESULT M355_GAS::open(uint8_t sensor_address)
{
	pADI_GPIO0->DS |= (1 << 4) | (1 << 5);
	this->m_i2c_address = sensor_address;
	return this->InitI2C();
}
/*
 * Starts measurement
 */
SENSOR_RESULT M355_GAS::start()
{
	return SENSOR_ERROR_NONE;
}

/*
 * Stops measurement
 */
SENSOR_RESULT M355_GAS::stop()
{
	return SENSOR_ERROR_NONE;
}

/*
 * Close device
 */
SENSOR_RESULT M355_GAS::close()
{
	ADI_I2C_RESULT eI2cResult;

	if ((eI2cResult = adi_i2c_Close(m_i2c_handle)) == ADI_I2C_SUCCESS)
		return SENSOR_ERROR_NONE;
	else
		return SET_SENSOR_ERROR(SENSOR_ERROR_I2C, eI2cResult);
}

/**
 * @brief  Returns the temperature value
 *
 * @return SENSOR_RESULT
 *
 * @details
 */
SENSOR_RESULT M355_GAS::InitI2C()
{
	ADI_I2C_RESULT eI2cResult;

	eI2cResult = adi_i2c_Open(ADI_CFG_I2C_DEV_NUM, &m_I2C_memory,
				  ADI_I2C_MEMORY_SIZE, &m_i2c_handle);
	if (eI2cResult != ADI_I2C_SUCCESS)
		return (SET_SENSOR_ERROR(SENSOR_ERROR_I2C, eI2cResult));

	eI2cResult = adi_i2c_Reset(m_i2c_handle);
	if (eI2cResult != ADI_I2C_SUCCESS)
		return (SET_SENSOR_ERROR(SENSOR_ERROR_I2C, eI2cResult));

	eI2cResult = adi_i2c_SetBitRate(m_i2c_handle, ADI_CFG_I2C_BIT_RATE);
	if (eI2cResult != ADI_I2C_SUCCESS)
		return (SET_SENSOR_ERROR(SENSOR_ERROR_I2C, eI2cResult));

	return SENSOR_ERROR_NONE;
}

/**
 * @brief  reads or writes data to/from the gas sensor
 *
 * @param  uint8_t RW, uint8_t RegAddr, uint8_t *pData, uint16_t size
 *
 * @return returns result of I2C communication
 *
 * @details
 */
SENSOR_RESULT M355_GAS::I2CReadWrite(uint8_t RW, uint8_t RegAddr,
				     uint8_t *pData, uint16_t size)
{
	ADI_I2C_TRANSACTION sTransfer;
	ADI_I2C_RESULT eI2cResult;
	uint32_t nHwErrors;
	uint8_t aPrologueData[5u];

	/* Set slave address for ADI M355 gas sensor */
	eI2cResult = adi_i2c_SetSlaveAddress(m_i2c_handle, this->m_i2c_address);

	if (eI2cResult != ADI_I2C_SUCCESS)
		return SET_SENSOR_ERROR(SENSOR_ERROR_I2C, eI2cResult);

	aPrologueData[0] = RegAddr;
	sTransfer.pPrologue = &aPrologueData[0u];
	sTransfer.nPrologueSize = 1u;
	sTransfer.pData = pData;
	sTransfer.nDataSize = size;
	sTransfer.bReadNotWrite = RW == READ ? true : false;
	sTransfer.bRepeatStart = true;

	eI2cResult = adi_i2c_ReadWrite(m_i2c_handle, &sTransfer,
					   &nHwErrors);
	if (eI2cResult == ADI_I2C_SUCCESS) {
		return SENSOR_ERROR_NONE;
	}

	return (SET_SENSOR_ERROR(SENSOR_ERROR_I2C, eI2cResult));
}

/**
 * @brief  initializes the sensor - requesting sensor status
 * 			until 0x01 response received (IDLE)
 *
 * @param  uint8_t sensor_address
 *
 * @return returns result of I2C communication
 *
 * @details
 */
SENSOR_RESULT M355_GAS::SensorInit(uint8_t sensor_address)
{
	uint8_t pBuff[2] = { 0 };
	uint16_t timeout = 5;
	SENSOR_RESULT Result = this->open(sensor_address);
	if (Result != SENSOR_ERROR_NONE)
		return Result;

	do {
		Result = this->I2CReadWrite(READ, GET_STATUS, &pBuff[0], 1u);
		delay_ms(100);
		timeout--;
	} while ((pBuff[0] != 0x01) && (timeout > 0));

	if (timeout == 0)
		return SENSOR_ERROR_I2C;

	return Result;
}

/**
 * @brief Arbitrate address collisions using GPIOs to send a new address to
 *			slave if GPIO (SS) is low. All slaves can start with
 *			ADI_CFG_I2C_DEFAULT_ADDR, then call this function to set
 *			new address for given site.
 *
 * @param uint8_t new_I2C_address
 *
 * @return returns result of I2C communication
 */
SENSOR_RESULT M355_GAS::SetI2CAddr(uint8_t new_I2C_address)
{
	uint8_t *pBuff;
	uint16_t timeout = 5;
	SENSOR_RESULT Result;
	pBuff = &new_I2C_address;

	/* IMPORTANT NOTE
	 *	IF USING THIS DRIVER, ENSURE ONLY ONE SLAVE CS IS SET LOW BEFORE
	 *	CALLING OTHERWISE ADDRESS CONFLICTS COULD HAPPEN
	 */
	/* Send command to change I2C address */
	Result = this->open(); /* uses default address */

	delay_ms(50);
	Result = this->I2CReadWrite(WRITE, SET_I2C_ADDRESS, pBuff, 1);
	/* Do not raise error in case of soft reset (address set previously) */

	delay_ms(1000); /* wait for selected slave to change its address */

	this->m_i2c_address = new_I2C_address;
	adi_i2c_SetSlaveAddress(m_i2c_handle, this->m_i2c_address);

	*pBuff = 0;

	do {
		Result = this->I2CReadWrite(READ, GET_STATUS, pBuff, 1);
		delay_ms(100);
		timeout--;
	} while ((*pBuff == 0) && (timeout > 0));

	this->close();

	if (timeout == 0)
		return SENSOR_ERROR_I2C;
	if (*pBuff == 3)
		return SENSOR_ERROR_PH; /* To indicate CN0428 Board */
	/* Otherwise, Result = SENSOR_ERROR_NONE, which indicates CN0429 Board */

	return Result;
}

/**
 * @brief  	Read temperature
 *
 * @param  	int16_t *pSensData
 *
 * @return 	returns result of I2C communication
 *
 * @details This command provides temperature information in signed 16bit format
 * 		which is used for sensor temperature compensation. The data is
 * 		transmitted in integer format * 100:
 *  		For example,	0x09E4 =>	2532 / 100 =>	+25.32�C
 *  				0xFB50 =>	-1200 / 100 =>	-12�C
 */
SENSOR_RESULT M355_GAS::ReadTemperature(int16_t *pSensData)
{
	uint8_t pBuff[2] = { 0 };
	int16_t val = 0;
	SENSOR_RESULT Result;

	Result = this->I2CReadWrite(READ, READ_TEMPERATURE, pBuff, 2);
	delay_ms(50);
	Result = this->I2CReadWrite(READ, READ_TEMPERATURE, pBuff, 2);

	val = (pBuff[0] << 8) | pBuff[1];
	*pSensData = val;

	return Result;
}

/**
 * @brief  	Read humidity
 *
 * @param  	int16_t *pSensData
 *
 * @return 	returns result of I2C communication
 *
 * @details This command provides humidity information in signed 16bit format.
 * 			The data is transmitted in integer format * 100:
 *  		For example, 	0x0E10 =>   3600 /100 => +36%
 */
SENSOR_RESULT M355_GAS::ReadHumidity(int16_t *pSensData)
{
	uint8_t pBuff[2] = { 0 };
	int16_t val = 0;
	SENSOR_RESULT Result;

	Result = this->I2CReadWrite(READ, READ_HUMIDITY, pBuff, 2);
	delay_ms(50);
	Result = this->I2CReadWrite(READ, READ_HUMIDITY, pBuff, 2);

	val = (pBuff[0] << 8) | pBuff[1];
	*pSensData = val;

	return Result;
}

/**
 * @brief  	Set measurement time(ms)
 *
 * @param  	uint16_t pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details This write command programs the ADC Sensor sampling time with an interval
 * 			resolution of 1ms / LSB. Default is 500ms (or 0x01F4). The ADC then performs
 * 			an average of 10 2.2ms samples at this interval.
 */
SENSOR_RESULT M355_GAS::SetMeasurementTime(uint16_t pCfgData)
{
	SENSOR_RESULT Result;
	uint8_t pBuff[2] = { 0 };
	pBuff[0] = (uint8_t) (pCfgData >> 8);
	pBuff[1] = (uint8_t) (pCfgData & 0xFF);
	Result = this->I2CReadWrite(WRITE, SET_MEAS_TIME_MS, pBuff, 2);
	delay_us(500);

	return Result;
}

/**
 * @brief  	Read measurement time(ms)
 *
 * @param  	uint16_t *pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details This command reads the ADC Sensor sampling time with an interval
 * 			resolution of 1ms / LSB.
 */
SENSOR_RESULT M355_GAS::ReadMeasurementTime(uint16_t *pCfgData)
{
	SENSOR_RESULT Result;
	uint8_t pBuff[2] = { 0 };
	int16_t val = 0;
	Result = this->I2CReadWrite(READ, READ_MEAS_TIME_MS, pBuff, 2u);
	delay_ms(50);
	Result = this->I2CReadWrite(READ, READ_MEAS_TIME_MS, pBuff, 2u);
	if (pBuff[0] & 0x80)
		val = 0xff;
	val = (val << 8) | pBuff[0];
	val = (val << 8) | pBuff[1];

	*pCfgData = val;

	return Result;
}

/**
 * @brief  	Start measurements
 *
 * @param  	none
 *
 * @return 	returns result of I2C communication
 *
 * @details This command tells the ADuCM355 to start ADC sampling the sensor at the sampling
 *  		interval set in the "Set Measurement Time" command. Note that this is already set
 *  		on programming and is set to 500ms.
 */
SENSOR_RESULT M355_GAS::StartMeasurements()
{
	SENSOR_RESULT Result;
	uint8_t dummy = 0xFF;
	Result = this->I2CReadWrite(WRITE, START_MEASUREMENTS, &dummy, 1);
	delay_us(500);

	return Result;
}

/**
 * @brief  	Stop measurements
 *
 * @param  	none
 *
 * @return 	returns result of I2C communication
 *
 * @details This command tells the ADuCM355 to stop ADC sampling the sensor. The sensor will
 * 			still be biased and powered.
 */
SENSOR_RESULT M355_GAS::StopMeasurements()
{
	SENSOR_RESULT Result;
	uint8_t dummy = 0xFF;
	Result = this->I2CReadWrite(WRITE, STOP_MEASUREMENTS, &dummy, 1);
	delay_us(500);

	return Result;
}

/**
 * @brief  	Set TIA gain
 *
 * @param  	uint8_t pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details Programs the TIA with the requested gain resistor as instructed by the first data byte.
 For example, data byte 1 from the Master contains 0x0B. This equates to a 20k resistor.
 */
SENSOR_RESULT M355_GAS::SetTIAGain(uint8_t pCfgData)
{
	SENSOR_RESULT Result;
	Result = this->I2CReadWrite(WRITE, SET_TIA_GAIN, &pCfgData, 1);
	delay_us(500);

	return Result;
}

/**
 * @brief  	Read TIA gain
 *
 * @param  	uint8_t *pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details Reads the current TIA gain resistor selected and populates the first data byte with its
 * 			reference value. This needs to be indexed to the eGainResistor enum.
 * 			For example, data byte 1 from the M355 contains 0x11. This equates to a 64k resistor.
 */
SENSOR_RESULT M355_GAS::ReadTIAGain(uint8_t *pCfgData)
{
	this->I2CReadWrite(READ, READ_TIA_GAIN, pCfgData, 1u);
	delay_ms(50);

	return this->I2CReadWrite(READ, READ_TIA_GAIN, pCfgData, 1u);
}

/**
 * @brief  	Set sensor bias voltage
 *
 * @param  	int16_t pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details This write command takes 2 bytes representing a mV Bias voltage. It is a signed 16bit value
 * 			so negative values can be used as some sensors require a negative bias. The first byte
 * 			transmitted is the upper byte then followed by the lower. So 100mV Bias setting would be
 * 			transmitted as the command 0x10 followed by two data bytes, 0x00 and then 0x64.
 * 			For example, 100mV is represented by 0x64 and -100mV is 0xFF9C.
 * 			This is a volatile setting and is reverted to the default sensor setting on a reset.
 */
SENSOR_RESULT M355_GAS::SetSensorBias(int16_t pCfgData)
{
	uint8_t pBuff[2] = { 0 };
	pBuff[0] = (uint8_t) (pCfgData >> 8);
	pBuff[1] = (uint8_t) (pCfgData & 0xFF);
	SENSOR_RESULT Result = this->I2CReadWrite(WRITE, SET_VBIAS, pBuff, 2);
	delay_us(500);

	return Result;
}

/**
 * @brief  	Read sensor bias voltage
 *
 * @param  	int16_t *pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details Reads the current mV Bias value as two 8bit values which represent a 16bit signed number.
 * 			For example if the bytes returned are 0xFF, 0xEC, this represents 0xFFEC which is -20mV bias.
 */
SENSOR_RESULT M355_GAS::ReadSensorBias(int16_t *pCfgData)
{
	uint8_t pBuff[2] = { 0 };
	int16_t val = 0;
	SENSOR_RESULT Result = this->I2CReadWrite(READ, READ_VBIAS, pBuff, 2u);
	delay_ms(50);
	Result = this->I2CReadWrite(READ, READ_VBIAS, pBuff, 2u);
	if (pBuff[0] & 0x80)
		val = 0xff;
	val = (val << 8) | pBuff[0];
	val = (val << 8) | pBuff[1];

	*pCfgData = val;

	return Result;
}

/**
 * @brief  	Set sensor sensitivity (nA/ppm)
 *
 * @param  	uint32_t pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details Configures the sensitivity nA/ppm settings of each sensor board is important for best
 * 			performance as each EC sensor is unique. Many sensors have a label on them indicating
 * 			its response, and this value needs to be inputed into the gas sensor daughter board
 * 			so the ADuCM355 can use it to determine ppb calculations. When programming the sensor
 * 			sensitivity, read the label nA/ppm and multiply by 100 to shift up two decimal points:
 * 			For example a 7.53nA/ppm sensor simply becomes 753. Load this value across a 3byte command
 * 			to the M355. The firmware on the M355 captures this command and divides back
 * 			by 100 for nA/ppm then.
 */
SENSOR_RESULT M355_GAS::SetSensorSensitivity(uint32_t pCfgData)
{
	SENSOR_RESULT Result;
	uint8_t pBuff[3] = { 0 };
	pBuff[0] = (uint8_t) (pCfgData >> 16);
	pBuff[1] = (uint8_t) (pCfgData >> 8);
	pBuff[2] = (uint8_t) (pCfgData & 0xFF);
	Result = this->I2CReadWrite(WRITE, SET_SENSOR_SENS, pBuff, 3u);
	delay_us(500);

	return Result;
}

/**
 * @brief  	Read sensor sensitivity (nA/ppm)
 *
 * @param  	uint32_t *pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details Reads a number in the first data byte indicating the current sensor that the M355 daughter board has
 * 			initialized and configured. The format is as per the "Set Sensor Type" command
 */
SENSOR_RESULT M355_GAS::ReadSensorSensitivity(uint32_t *pCfgData)
{
	SENSOR_RESULT Result;
	uint8_t pBuff[3] = { 0 };
	uint32_t val = 0;
	Result = this->I2CReadWrite(READ, READ_SENSOR_SENS, pBuff, 3u);
	delay_ms(50);
	Result = this->I2CReadWrite(READ, READ_SENSOR_SENS, pBuff, 3u);

	val = ((pBuff[0] << 16) | (pBuff[1] << 8) | pBuff[2]);
	*pCfgData = val;

	return Result;
}

/**
 * @brief  	Configure Temperature Compensation
 *
 * @param  	uint8_t pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details This command enables temperature compensation on the active sensor. Write the first data
 * 			byte as 0x00 to turn off or 0x01 to turn on. Note that regular temperature updates should
 * 			be provided to the M355 sensor board for this feature to work optimally and it operates for
 * 			ppb calculated data only (not LSB data yet).
 */
SENSOR_RESULT M355_GAS::ConfigureTempComp(uint8_t pCfgData)
{
	SENSOR_RESULT Result;
	Result = this->I2CReadWrite(WRITE, SET_TEMP_COMP, &pCfgData, 1);
	delay_us(500);

	return Result;
}

/**
 * @brief  	Read Temperature Compensation settings
 *
 * @param  	uint8_t *pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details This command reads the Temperature Compensation status in the first data byte.
 * 			0x00 == Off
 * 			0x01 == On.
 */
SENSOR_RESULT M355_GAS::ReadTempCompCfg(uint8_t *pCfgData)
{
	this->I2CReadWrite(READ, READ_TEMP_COMP, pCfgData, 1u);
	delay_ms(50);

	return this->I2CReadWrite(READ, READ_TEMP_COMP, pCfgData, 1u);
}

/**
 * @brief  	Run EIS measurement
 *
 * @param  	none
 *
 * @return 	returns result of I2C communication
 *
 * @details This command instructs the M355 to perform an impedance spectroscopy measurement using
 * 			the following frequencies:
 * 			1kHz, 5kHz, 10kHz, 20kHz, 30kHz, 40kHz, 50kHz, 60kHz, 70kHz, 90kHz, 160kHz and 200kHz.
 * 			Results are read back with the Read EIS Results command in 4 hex byte format.
 */
SENSOR_RESULT M355_GAS::RunEISMeasurement()
{
	uint8_t dummy = 0x01;
	SENSOR_RESULT Result = this->I2CReadWrite(WRITE, RUN_EIS, &dummy, 1);
	delay_us(500);

	return Result;
}

/**
 * @brief  	Read EIS results
 *
 * @param  	uint8_t *pSensData
 *
 * @return 	returns result of I2C communication
 *
 * @details This write command instructs the M355 board to begin returning EIS
 *		data representing Frequency, Magnitude, Phase and Complex Magnitude
 *		(real and imaginary) when the next read command is issued. Continue
 *		reading until 0xFF is read continuously. Normally 240 bytes are expected.
 *		This consists of 20 bytes for the 12 frequencies (as detailed in the
 *		Run EIS	command). Frequency, Magnitude, Phase and Complex Magnitude
 *		are 4 bytes each. So 20 bytes for 12 frequency points = 240 bytes.
 *		The data returned is in byte array format representing IEEE754 single
 *		precision 32bit	floating point data.
 */
SENSOR_RESULT M355_GAS::ReadEISResults(uint8_t *pSensData)
{
	this->I2CReadWrite(READ, READ_EIS_RESULTS, pSensData, NO_OF_EIS_RESULTS);
	delay_ms(100);

	return this->I2CReadWrite(READ, READ_EIS_RESULTS, pSensData, NO_OF_EIS_RESULTS);
}

/**
 * @brief  	Read EIS results
 *
 * @param  	uint8_t *pSensData
 *
 * @return 	returns result of I2C communication
 *
 * @details This write command instructs the M355 board to begin returning
 *		EIS data representing DFT Impedance and Magnitude, Bode
 *		Magnitude, Bode Phase, RLoadMagnitude and Capacity when the next
 *		read command is issued. Continue reading until 0xFF is read
 *		continuously. Normally 864 bytes are expected. This consists of
 *		72 bytes for the 12 frequencies (as detailed in the Run EIS
 *		command). Every value is 4 bytes each. So 72 bytes for 12
 *		frequency points = 864 bytes. The data returned is in byte array
 *		format representing IEEE754 single precision 32bit floating
 *		point data.
 */
SENSOR_RESULT M355_GAS::ReadEISResultsFull(uint8_t *pSensData)
{
	this->I2CReadWrite(READ, READ_EIS_RESULTS_FULL, pSensData, NO_OF_EIS_FULL_RESULTS);
	delay_ms(150);

	return this->I2CReadWrite(READ, READ_EIS_RESULTS_FULL, pSensData, NO_OF_EIS_FULL_RESULTS);
}

/**
 * @brief  	Read 200R RTIA resistor
 *
 * @param  	uint8_t *pSensData
 *
 * @return 	returns result of I2C communication
 *
 * @details As part of the initial calibration of the ADuCM355 AFE, the 200R
 *		RTIA is calibrated using the internal DAC and external reference
 *		resistor. This resistance needs to be used to obtain correct uA
 *		levels when converting the 16bit ADC results from the pulse
 *		test to current using the formula
 */
SENSOR_RESULT M355_GAS::Read200RCal(uint8_t *pSensData)
{
	this->I2CReadWrite(READ, READ_200R_RTIA_CAL_RESULT, pSensData, 1);
	delay_ms(50);

	return this->I2CReadWrite(READ, READ_200R_RTIA_CAL_RESULT, pSensData, 1);
}

/**
 * @brief  	Performs Pulse Test
 *
 * @param  	uint8_t pulseAmplitude - Amplitude of the pulse in mV (typically 1mV)
 * 			uint8_t pulseDuration  - Duration of the pulse (keep <200 ms)
 *
 * @return 	returns result of I2C communication
 *
 * @details	Pulse tests can be used for sensor diagnostics to determine sensor
 * 			state of health and lifetime. Results are stored in SRAM and will
 * 			need to be read by the i2C Master device.
 */
SENSOR_RESULT M355_GAS::RunPulseTest(uint8_t pulseAmplitude,
				     uint8_t pulseDuration)
{
	SENSOR_RESULT Result;
	uint8_t data[2] = { 0 };
	if (pulseAmplitude < PULSE_AMPLITUDE_MIN || pulseAmplitude > PULSE_AMPLITUDE_MAX)
		pulseAmplitude = PULSE_AMPLITUDE_MIN;
	if (pulseDuration < PULSE_DURATION_MIN || pulseDuration > PULSE_DURATION_MAX)
		pulseDuration = PULSE_DURATION_DEFAULT;
	data[0] = pulseAmplitude;
	data[1] = pulseDuration;
	Result = this->I2CReadWrite(WRITE, RUN_PULSE_TEST, data, 2u);
	delay_us(500);
	return Result;
}

/**
 * @brief  	Reads Pulse Test Results
 *
 * @param  	uint8_t *pSensData
 *
 * 		----- variables below MUST have the same value as above !! -----
 * 		uint8_t pulseAmplitude - Amplitude of the pulse in mV (typically 1mV)
 * 		uint8_t pulseDuration  - Duration of the pulse (keep <200 ms)
 *
 * @return 	returns result of I2C communication
 *
 * @details	Pulse tests can be used for sensor diagnostics to determine sensor
 * 			state of health and lifetime. Results are stored in SRAM and will
 * 			need to be read by the i2C Master device.
 */
SENSOR_RESULT M355_GAS::ReadPulseTestResults(uint8_t *pSensData,
		uint8_t pulseAmplitude, uint8_t pulseDuration)
{
	/* added 5 ms sampling before and after the test, 110 us ADC sample time */
	uint16_t intToRead = (((pulseDuration + 10) * 1000) / 110) * 2;

	this->I2CReadWrite(READ, READ_PULSE_TEST_RESULTS, &pSensData[0],
			   intToRead);
	delay_ms(150);

	return this->I2CReadWrite(READ, READ_PULSE_TEST_RESULTS, &pSensData[0],
				  intToRead);
}

/**
 * @brief  	Set Rload resistor
 *
 * @param  	uint8_t pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details This command programs the selected sensor load resistor. This resistor is in series
 * 			with the working electrode and is usually specified by the sensor manufacturer.
 It is a 1 byte command with the value of 0 to 7 being allowed.
 */
SENSOR_RESULT M355_GAS::SetRload(uint8_t pCfgData)
{
	SENSOR_RESULT Result;
	Result = this->I2CReadWrite(WRITE, SET_RLOAD, &pCfgData, 1);
	delay_us(500);

	return Result;
}

/**
 * @brief  	Read Rload resistor
 *
 * @param  	uint8_t *pCfgData
 *
 * @return 	returns result of I2C communication
 *
 * @details This command reads the currently selected sensor load resistor. This resistor is in
 * 			series with the working electrode and is usually specified by the sensor manufacturer.
 It is a 1 byte command with the value of 0 to 7 being allowed.
 */
SENSOR_RESULT M355_GAS::ReadRload(uint8_t *pCfgData)
{
	this->I2CReadWrite(READ, READ_RLOAD, pCfgData, 1u);
	delay_ms(50);

	return this->I2CReadWrite(READ, READ_RLOAD, pCfgData, 1u);
}

/**
 * @brief  	Read sensor data as PPB value
 *
 * @param  	int32_t *pSensData
 *
 * @return 	returns result of I2C communication
 *
 * @details	Instructs the M355 sensor board to begin delivering averaged 24bit ppb data when the next
 * 			read command is issued. Default is a 30sec average of data.
 */
SENSOR_RESULT M355_GAS::ReadDataPPB(int32_t *pSensData)
{
	uint8_t pBuff[4] = { 0 };
	int32_t val = 0;
	SENSOR_RESULT Result = this->I2CReadWrite(READ, READ_AVG_PPB, pBuff, 4);
	delay_ms(50);
	Result = this->I2CReadWrite(READ, READ_AVG_PPB, pBuff, 4);
	val = (pBuff[0] << 24) | (pBuff[1] << 16) | (pBuff[2] << 8) | pBuff[3];

	*pSensData = val;

	return Result;
}

/**
 * @brief  	Read sensor data as bits
 *
 * @param  	uint16_t *pSensData
 *
 * @return 	returns result of I2C communication
 *
 * @details Instructs the M355 sensor board to begin delivering averaged 16bit LSB data when the next
 * 			read command is issued. Default is a 30sec average of data.
 */
SENSOR_RESULT M355_GAS::ReadDataBits(uint16_t *pSensData)
{
	uint8_t pBuff[2] = { 0 };
	SENSOR_RESULT Result = this->I2CReadWrite(READ, READ_AVG_LSB, pBuff, 2);
	delay_ms(50);
	Result = this->I2CReadWrite(READ, READ_AVG_LSB, pBuff, 2);

	*pSensData = (pBuff[0] << 8) | pBuff[1];

	return Result;
}

}
