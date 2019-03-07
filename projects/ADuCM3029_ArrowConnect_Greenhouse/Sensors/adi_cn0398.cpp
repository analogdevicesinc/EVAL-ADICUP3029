/*!
 *****************************************************************************
 * @file:    adi_cn0398.cpp
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

#include "adi_cn0398_cfg.h"
#include "adi_cn0398.h"
#include "adi_ad7124.h"
#include "Timer.h"
#include "math.h"
#include <drivers/gpio/adi_gpio.h>
#include "string.h"

namespace adi_sensor_swpack
{
#define RREF (5000.0)
#define TEMP_GAIN (16.0)
#define PT100_RESISTANCE_TO_TEMP(x) ((x-100.0)/(0.385))
#define _2_23 (1<<23)

#define ms_delay (1)

	float temperature, pH, voltage[2], moisture;

	int32_t adcValue[3];

	CN0398::CN0398()
	{
		offset_voltage = default_offset_voltage;
		calibration_ph[0][0] = default_calibration_ph[0][0];
		calibration_ph[0][1] = default_calibration_ph[0][1];
		calibration_ph[1][0] = default_calibration_ph[1][0];
		calibration_ph[1][1] = default_calibration_ph[1][1];
		solution0 = 0;
		solution1 = 0;
	}

	void CN0398::calibrate_ph_pt0(float temperature)
	{
		uint8_t ui8S[200];

		set_digital_output(P2, true);
		int32_t data = read_channel(PH_CHANNEL);

		float volt = data_to_voltage_bipolar(data, 1, 3.3);

		if(temperature < 0) {
			calibration_ph[0][0] = ph_temp_lut[solution0][0];
		} else {
			for(uint8_t i = 1; i < NUMBER_OF_TEMPERATURE_ENTRIES; i++) {
				if(temperature > ph_temperatures[i - 1] && temperature <= ph_temperatures[i]) {
					calibration_ph[0][0] = ph_temp_lut[solution0][i];
					break;
				}
			}
		}

		calibration_ph[0][1] =  volt;
		set_digital_output(P2, false);

	}
	void CN0398::calibrate_ph_pt1(float temperature)
	{
		uint8_t ui8S[200];
		set_digital_output(P2, true);
		int32_t data = read_channel(PH_CHANNEL);

		float volt = data_to_voltage_bipolar(data, 1, 3.3);

		if(temperature < 0) {
			calibration_ph[1][0] = ph_temp_lut[solution1][0];
		} else {
			for(uint8_t i = 1; i < NUMBER_OF_TEMPERATURE_ENTRIES; i++) {
				if(temperature > ph_temperatures[i - 1] && temperature <= ph_temperatures[i]) {
					calibration_ph[1][0] = ph_temp_lut[solution1][i];
					break;
				}
			}
		}

		calibration_ph[1][1] =  volt;
		set_digital_output(P2, false);
	}

	void CN0398::calibrate_ph_offset()
	{
		set_digital_output(P2, true);

		int32_t data = read_channel(PH_CHANNEL);

		float volt = data_to_voltage_bipolar(data, 1, 3.3);
		offset_voltage = volt;

		set_digital_output(P2, false);
	}

	float CN0398::read_rtd()
	{
		float temperature = 0;

		int32_t data;

		adcValue[RTD_CHANNEL] = data = read_channel(RTD_CHANNEL);

		float resistance = ((static_cast<float>(data) - _2_23) * RREF) / (TEMP_GAIN * _2_23);

	#ifdef USE_LINEAR_TEMP_EQ
		temperature = PT100_RESISTANCE_TO_TEMP(resistance);
	#else

	#define A (3.9083*pow(10,-3))
	#define B (-5.775*pow(10,-7))
		if(resistance < R0)
			temperature = -242.02 + 2.228 * resistance + (2.5859 * pow(10, -3)) * pow(resistance, 2) - (48260.0 * pow(10, -6)) * pow(resistance, 3) - (2.8183 * pow(10, -3)) * pow(resistance, 4) + (1.5243 * pow(10, -10)) * pow(resistance, 5);
		else
			temperature = ((-A + sqrt(double(pow(A, 2) - 4 * B * (1 - resistance / R0))) ) / (2 * B));
	#endif
		return temperature;

	}

	int32_t CN0398::read_channel(uint8_t ch)
	{
		enable_channel(ch);

		adi_gpio_SetLow(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN);
		start_single_conversion();


		if (ad7124.WaitForConvReady(10000) == -3) {
			return -1;
		}

		int32_t data;

		ad7124.ReadData(&data);

		convFlag = 0;
		adi_gpio_SetHigh(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN);

		disable_channel(ch);

		return data;

	}

	float CN0398::read_ph(float temperature)
	{
		float ph = 0;

	#ifdef PH_SENSOR_PRESENT
		int32_t data;

		set_digital_output(P2, true);

		adcValue[PH_CHANNEL] = data = read_channel(PH_CHANNEL);

		float volt = voltage[PH_CHANNEL - 1] = data_to_voltage_bipolar(data, 1, 3.3);

		if(use_nernst)
		{
			ph  = PH_ISO -((volt - ZERO_POINT_TOLERANCE) / ((2.303 * AVOGADRO * (temperature + KELVIN_OFFSET)) / FARADAY_CONSTANT) );
		}
		else
		{
			float m =  (calibration_ph[1][0] - calibration_ph[0][0]) / (calibration_ph[1][1] - calibration_ph[0][1]);
			ph = m * (volt - calibration_ph[1][1] - offset_voltage) + calibration_ph[1][0];
		}

		set_digital_output(P2, false);

	#endif
		return ph;

	}

	float CN0398::read_moisture()
	{
		float moisture = 0;
	#ifdef MOISTURE_SENSOR_PRESENT

		adi_gpio_OutputEnable(ADP7118_PORT, ADP7118_PIN, true);
		adi_gpio_SetHigh(ADP7118_PORT, ADP7118_PIN);

		set_digital_output(P3, true);

		timer_sleep(SENSOR_SETTLING_TIME);
		int32_t data = adcValue[MOISTURE_CHANNEL]= read_channel(MOISTURE_CHANNEL);

		adi_gpio_SetLow(ADP7118_PORT, ADP7118_PIN);

		float volt = voltage[MOISTURE_CHANNEL - 1] = data_to_voltage_bipolar(data, 1, 3.3);

	#ifdef USE_MANUFACTURER_MOISTURE_EQ
		if(volt <= 1.1) {
			moisture = 10 * volt - 1;
		} else if(volt > 1.1 && volt <= 1.3) {
			moisture = 25 * volt - 17.5;
		} else if(volt > 1.3 && volt <= 1.82) {
			moisture = 48.08 * volt - 47.5;
		} else if(volt > 1.82) {
			moisture = 26.32 * volt - 7.89;
		}
	#else
		moisture = -1.18467 + 21.5371 * volt - 110.996 * (pow(volt, 2)) + 397.025 * (pow(volt, 3)) - 666.986 * (pow(volt, 4)) + 569.236 * (pow(volt, 5)) - 246.005 * (pow(volt, 6)) + 49.4867 * (pow(volt, 7)) - 3.37077 * (pow(volt, 8));
	#endif
		if(moisture > 100) moisture = 100;
		if(moisture < 0 ) moisture = 0;
	#endif

		set_digital_output(P3, false);

		return moisture;
	}

	float CN0398::data_to_voltage_bipolar(uint32_t data, uint8_t gain, float VREF)
	{
		data = data & 0xFFFFFF;
		return ((data / static_cast<float>(0xFFFFFF / 2)) - 1) * (VREF / gain);
	}

	float CN0398::data_to_voltage(uint32_t data, uint8_t gain, float VREF)
	{
		data = data & 0xFFFFFF;
		return (data / static_cast<float>(0xFFFFFF)) * (VREF / gain);
	}


	void CN0398::enable_channel(int channel)
	{
		AD7124::ad7124_registers regNr = static_cast<AD7124::ad7124_registers> (AD7124::AD7124_Channel_0 + channel); //Select ADC_Control register
		uint32_t setValue = ad7124.ReadDeviceRegister(regNr);
		setValue |= (uint32_t) AD7124_CH_MAP_REG_CH_ENABLE;  //Enable channel0
		setValue &= 0xFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);    // Write data to ADC
		timer_sleep(ms_delay);
	}

	void CN0398::disable_channel(int channel)
	{
		AD7124::ad7124_registers regNr = static_cast<AD7124::ad7124_registers> (AD7124::AD7124_Channel_0 + channel); //Select ADC_Control register
		uint32_t setValue = ad7124.ReadDeviceRegister(regNr);
		setValue &= (~(uint32_t) AD7124_CH_MAP_REG_CH_ENABLE);  //Disable channel
		setValue &= 0xFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);    // Write data to ADC
		timer_sleep(ms_delay);
	}

	void CN0398::enable_current_source0(int current_source_channel)
	{
		AD7124::ad7124_registers regNr = AD7124::AD7124_IOCon1; //Select ADC_Control register
		uint32_t setValue = ad7124.ReadDeviceRegister(regNr);
		setValue &= ~(AD7124_IO_CTRL1_REG_IOUT_CH0(0xF));
		setValue |= AD7124_IO_CTRL1_REG_IOUT_CH0(current_source_channel);// set IOUT0 current to 500uA
		setValue &= 0xFFFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);    // Write data to ADC
		timer_sleep(ms_delay);
	}

	void CN0398::enable_current_source1(int current_source_channel)
	{
		AD7124::ad7124_registers regNr = AD7124::AD7124_IOCon1; //Select ADC_Control register
		uint32_t setValue = ad7124.ReadDeviceRegister(regNr);
		setValue &= ~(AD7124_IO_CTRL1_REG_IOUT_CH1(0xF));
		setValue |= AD7124_IO_CTRL1_REG_IOUT_CH1(current_source_channel);// set IOUT0 current to 500uA
		setValue &= 0xFFFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);    // Write data to ADC
		timer_sleep(ms_delay);
	}

	void CN0398::set_digital_output(uint8_t p, bool state)
	{
		uint32_t test;

		AD7124::ad7124_registers regNr = AD7124::AD7124_IOCon1; //Select ADC_Control register
		uint32_t setValue = ad7124.ReadDeviceRegister(regNr);
		if(state)
			setValue |= ((AD7124_8_IO_CTRL1_REG_GPIO_DAT1) << p);
		else
			setValue &= (~(AD7124_8_IO_CTRL1_REG_GPIO_DAT1 << p));
		ad7124.WriteDeviceRegister(regNr, setValue);    // Write data to ADC
		timer_sleep(ms_delay);

		test = ad7124.ReadDeviceRegister(regNr);
	}



	void CN0398::start_single_conversion()
	{
		uint32_t test;
		AD7124::ad7124_registers regNr = AD7124::AD7124_ADC_Control; //Select ADC_Control register

		ad7124.WriteDeviceRegister(regNr, 0x0584);    // Write data to ADC

		timer_sleep(100);
		test = ad7124.ReadDeviceRegister(regNr);
	}


	void CN0398::reset()
	{
		ad7124.Reset();
	}

	void CN0398::setup()
	{
		ad7124.Setup();
	}


	void CN0398::init()
	{
		//Configure ADP7118 pin
		adi_gpio_OutputEnable(ADP7118_PORT, ADP7118_PIN, true);
		adi_gpio_InputEnable(ADP7118_PORT, ADP7118_PIN, false);    // Set INTACC_PORT as input
		adi_gpio_PullUpEnable(ADP7118_PORT, ADP7118_PIN, false);
		adi_gpio_SetLow(ADP7118_PORT, ADP7118_PIN);

		//Configure CS pin
		adi_gpio_OutputEnable(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN, true);
		adi_gpio_InputEnable(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN, false);    // Set INTACC_PORT as input
		adi_gpio_PullUpEnable(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN, false);
		adi_gpio_SetHigh(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN);

		ad7124.init();

		uint32_t setValue, test;
		enum AD7124::ad7124_registers regNr;

		// Set Config_0 0x19
		regNr = AD7124::AD7124_Config_0;               //Select Config_0 register - RTD
		test = ad7124.ReadDeviceRegister(regNr);
		setValue = 0;//ad7124.ReadDeviceRegister(regNr);
		setValue |= AD7124_CFG_REG_BIPOLAR;     //Select bipolar operation
		setValue |= AD7124_CFG_REG_BURNOUT(0);  //Burnout current source off
		setValue |= AD7124_CFG_REG_REF_BUFP;
		setValue |= AD7124_CFG_REG_REF_BUFM;
		setValue |= AD7124_CFG_REG_AIN_BUFP;
		setValue |= AD7124_CFG_REG_AINN_BUFM;
		setValue |= AD7124_CFG_REG_REF_SEL(1); //REFIN2(+)/REFIN2(−).
		setValue |= AD7124_CFG_REG_PGA(4);
		setValue &= 0xFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);   // Write data to ADC

		test = ad7124.ReadDeviceRegister(regNr);

		regNr = AD7124::AD7124_Config_1;               //Select Config_1 register - pH
		test = ad7124.ReadDeviceRegister(regNr);
		setValue = 0;//ad7124.ReadDeviceRegister(regNr);
		setValue |= AD7124_CFG_REG_BIPOLAR;     //Select bipolar operation
		setValue |= AD7124_CFG_REG_BURNOUT(0);  //Burnout current source off
		setValue |= AD7124_CFG_REG_REF_BUFP;
		setValue |= AD7124_CFG_REG_REF_BUFM;
		setValue |= AD7124_CFG_REG_AIN_BUFP;
		setValue |= AD7124_CFG_REG_AINN_BUFM;
		setValue |= AD7124_CFG_REG_REF_SEL(0); //REFIN1(+)/REFIN1(-).
		setValue |= AD7124_CFG_REG_PGA(0); // gain1
		setValue &= 0xFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);   // Write data to ADC
		test = ad7124.ReadDeviceRegister(regNr);

		// Set Channel_0 register 0x09
		regNr = AD7124::AD7124_Channel_0;  // RTD
		test = ad7124.ReadDeviceRegister(regNr);
		setValue = 0;
		setValue |= AD7124_CH_MAP_REG_SETUP(0);             // Select setup0
		setValue |= AD7124_CH_MAP_REG_AINP(9);         // Set AIN9 as positive input
		setValue |= AD7124_CH_MAP_REG_AINM(10);         // Set AIN10 as negative input
		setValue &= (~(uint32_t) AD7124_CH_MAP_REG_CH_ENABLE);  //Disable channel
		setValue &= 0xFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);   // Write data to ADC
		test = ad7124.ReadDeviceRegister(regNr);

		regNr = AD7124::AD7124_Channel_1; // pH
		test = ad7124.ReadDeviceRegister(regNr);
		setValue = 0;
		setValue |= AD7124_CH_MAP_REG_SETUP(1);             // Select setup2
		setValue |= AD7124_CH_MAP_REG_AINP(6);         // Set AIN8 as positive input
		setValue |= AD7124_CH_MAP_REG_AINM(7);         // Set gnd as negative input
		setValue &= (~(uint32_t) AD7124_CH_MAP_REG_CH_ENABLE);  //Disable channel
		setValue &= 0xFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);   // Write data to ADC
		test = ad7124.ReadDeviceRegister(regNr);

		regNr = AD7124::AD7124_Channel_2; // moisture
		test = ad7124.ReadDeviceRegister(regNr);
		setValue = 0;
		setValue |= AD7124_CH_MAP_REG_SETUP(1);             // Select setup2
		setValue |= AD7124_CH_MAP_REG_AINP(8);         // Set AIN8 as positive input
		setValue |= AD7124_CH_MAP_REG_AINM(19);         // Set gnd as negative input
		setValue &= (~(uint32_t) AD7124_CH_MAP_REG_CH_ENABLE);  //Disable channel
		setValue &= 0xFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);   // Write data to ADC
		test = ad7124.ReadDeviceRegister(regNr);

		// Set IO_Control_1 0x03
		regNr = AD7124::AD7124_IOCon1;               //Select IO_Control_1 register
		test = ad7124.ReadDeviceRegister(regNr);
		setValue = 0;
		setValue |= AD7124_8_IO_CTRL1_REG_GPIO_CTRL2; // enable AIN3 as digital output
		setValue |= AD7124_8_IO_CTRL1_REG_GPIO_CTRL3; // enable AIN4 as digital output
		setValue |= AD7124_IO_CTRL1_REG_IOUT_CH0(11); // source ain11
		setValue |= AD7124_IO_CTRL1_REG_IOUT_CH1(12); // source ain12
		setValue |= AD7124_IO_CTRL1_REG_IOUT0(0x4);// set IOUT0 current to 500uA
		setValue |= AD7124_IO_CTRL1_REG_IOUT1(0x4);// set IOUT0 current to 500uA
		setValue &= 0xFFFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);// Write data to ADC
		test = ad7124.ReadDeviceRegister(regNr);

		// Set IO_Control_2
		regNr = AD7124::AD7124_IOCon2;               //Select IO_Control_2 register
		test = ad7124.ReadDeviceRegister(regNr);
		setValue = 0;
		setValue |= AD7124_8_IO_CTRL2_REG_GPIO_VBIAS7; // enable AIN3 as digital output
		setValue &= 0xFFFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);// Write data to ADC
		test = ad7124.ReadDeviceRegister(regNr);

		// Set ADC_Control 0x01
		regNr = AD7124::AD7124_ADC_Control;            //Select ADC_Control register
		test = ad7124.ReadDeviceRegister(regNr);
		setValue = ad7124.ReadDeviceRegister(regNr);
		setValue |= AD7124_ADC_CTRL_REG_DATA_STATUS; // set data status bit in order to check on which channel the conversion is
		setValue &= 0xFFC3; // remove prev mode bits
		setValue |= AD7124_ADC_CTRL_REG_MODE(2); //standby mode
		setValue &= 0xFFFF;
		ad7124.WriteDeviceRegister(regNr, setValue);    // Write data to ADC
		test = ad7124.ReadDeviceRegister(regNr);
		timer_sleep(ms_delay);

	}

	void CN0398::set_data(void)
	{
		temperature = read_rtd();

		pH = read_ph(temperature);

		moisture = read_moisture();

	}

	void CN0398::display_data(void)
	{

	}
	void CN0398::calibrate_ph(void)
	{
	}
	/**
	 * @brief: Prints calibration solutions
	 */
	void CN0398::print_calibration_solutions()
	{
	}

	SENSOR_RESULT CN0398::open()
	{

	}
	SENSOR_RESULT CN0398::start()
	{

	}
	SENSOR_RESULT CN0398::stop()
	{

	}
	SENSOR_RESULT CN0398::close()
	{

	}

}
