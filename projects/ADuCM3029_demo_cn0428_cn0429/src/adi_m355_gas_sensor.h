/**
******************************************************************************
*   @file     adi_m355_gas_sensor.h
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

#ifndef ADI_M355_GAS_SENSOR_H_
#define ADI_M355_GAS_SENSOR_H_

#include "adi_gas_sensor.h"
#include "adi_m355_gas_sensor_config.h"
#include <drivers/i2c/adi_i2c.h>
#include <drivers/spi/adi_spi.h>


#define	GET_STATUS              	0x02
#define SET_TEMPERATURE         	0x03
#define READ_TEMPERATURE        	0x04
#define SET_HUMIDITY            	0x05
#define	READ_HUMIDITY           	0x06
#define SET_MEAS_TIME_MS        	0x07
#define READ_MEAS_TIME_MS			0x53
#define START_MEASUREMENTS      	0x08
#define STOP_MEASUREMENTS       	0x0A
#define SET_TIA_GAIN            	0x0C
#define READ_TIA_GAIN           	0x0E
#define SET_VBIAS		        	0x10
#define READ_VBIAS			       	0x12
#define SET_VZERO_VOLTAGE       	0x14
#define READ_VZERO_VOLTAGE      	0x16
#define SET_SENSOR_TYPE         	0x18
#define SET_SENSOR_SENS         	0x19
#define READ_SENSOR_TYPE        	0x1A
#define READ_SENSOR_SENS        	0x1B
#define	SET_TEMP_COMP           	0x1C
#define READ_TEMP_COMP          	0x1D
#define RUN_EIS						0x1E
#define READ_EIS_RESULTS			0x1F
#define READ_AVG_LSB            	0x20
#define READ_EIS_RESULTS_FULL		0x25
#define READ_RAW_LSB            	0x26
#define READ_AVG_PPB            	0x30
#define READ_RAW_PPB            	0x40
#define RUN_PULSE_TEST          	0x45
#define READ_PULSE_TEST_RESULTS 	0x50
#define SET_RLOAD					0x51
#define READ_RLOAD					0x52
#define READ_200R_RTIA_CAL_RESULT 	0x55
#define SET_I2C_ADDRESS				0x80




typedef enum {
	READ = 0,
	WRITE
} eI2C_DIR;

typedef enum {
	R_0R = 0,
	R_200R,
	R_1K,
	R_2K,
	R_3K,
	R_4K,
	R_6K,
	R_8K,
	R_10K,
	R_12K,
	R_16K,
	R_20K,
	R_24K,
	R_30K,
	R_32K,
	R_40K,
	R_48K,
	R_64K,
	R_85K,
	R_96K,
	R_100K,
	R_120K,
	R_128K,
	R_160K,
	R_196K,
	R_256K,
	R_512K
} eGainResistor;

typedef enum {
	RLoad_0R = 0,
	RLoad_10R,
	RLoad_30R,
	RLoad_50R,
	RLoad_100R,
	RLoad_1k6,
	RLoad_3k1,
	RLoad_3k6,
} eLoadResistor;

namespace adi_sensor_swpack
{
    /**
     * @class ADT7420 Temperature Class
     *
     * @brief ADT7420 temperature class interface.
     *
     **/
    class M355_GAS : public Gas_Reading
    {
        public:
            /*!< Pure virtual functions must be implemented by the derived class */

    		virtual SENSOR_RESULT open();
            virtual SENSOR_RESULT start();
            virtual SENSOR_RESULT stop();
            virtual SENSOR_RESULT close();

            virtual SENSOR_RESULT openWithAddr(uint8_t sensor_address);
            virtual SENSOR_RESULT I2CReadWrite(uint8_t RW, uint8_t RegAddr, uint8_t *pData, uint16_t size);
            virtual SENSOR_RESULT SensorInit(uint8_t sensor_address);
            virtual SENSOR_RESULT SetI2CAddr(uint8_t new_I2C_address);
            virtual SENSOR_RESULT ReadTemperature(int16_t *pSensData);
			virtual SENSOR_RESULT ReadHumidity(int16_t *pSensData);
			virtual SENSOR_RESULT SetMeasurementTime(uint16_t pCfgData);
			virtual SENSOR_RESULT ReadMeasurementTime(uint16_t *pCfgData);
			virtual SENSOR_RESULT StartMeasurements();
			virtual SENSOR_RESULT StopMeasurements();
			virtual SENSOR_RESULT SetTIAGain(uint8_t pCfgData);
			virtual SENSOR_RESULT ReadTIAGain(uint8_t *pCfgData);
			virtual SENSOR_RESULT SetSensorBias(int16_t pCfgData);
			virtual SENSOR_RESULT ReadSensorBias(int16_t *pCfgData);
			virtual SENSOR_RESULT SetSensorSensitivity(uint32_t pCfgData);
			virtual SENSOR_RESULT ReadSensorSensitivity(uint32_t *pCfgData);
			virtual SENSOR_RESULT ConfigureTempComp(uint8_t pCfgData);
			virtual SENSOR_RESULT ReadTempCompCfg(uint8_t *pCfgData);
			virtual SENSOR_RESULT RunEISMeasurement();
			virtual SENSOR_RESULT ReadEISResults(uint8_t *pSensData);
			virtual SENSOR_RESULT ReadEISResultsFull(uint8_t *pSensData);

            virtual SENSOR_RESULT Read200RCal(uint8_t *pSensData);
            virtual SENSOR_RESULT RunPulseTest(uint8_t pulseAmplitude, uint8_t pulseDuration);
			virtual SENSOR_RESULT ReadPulseTestResults(uint8_t *pSensData, uint8_t pulseAmplitude, uint8_t pulseDuration);
            virtual SENSOR_RESULT SetRload(uint8_t pCfgData);
            virtual SENSOR_RESULT ReadRload(uint8_t *pCfgData);
			virtual SENSOR_RESULT ReadDataPPB(int32_t *pSensData);
            virtual SENSOR_RESULT ReadDataBits(uint16_t *pSensData);

            /*! I2C slave address defaults to static configuration but can be overriden at run-time.
             *  If there are multiple ADT7420 devices on a system (sharing an I2C bus), the addresses
             *  cannot map to the same static value.
             */

            uint8_t m_i2c_address;

        private:

            /*!
              * @brief   Initialize I2C.
              *
              * @details This method is used to initialize I2C.
              */
            SENSOR_RESULT InitI2C();


            ADI_I2C_HANDLE m_i2c_handle;
            uint8_t        m_I2C_memory[ADI_I2C_MEMORY_SIZE];


    };
}



#endif /* ADI_M355_GAS_SENSOR_H_ */
