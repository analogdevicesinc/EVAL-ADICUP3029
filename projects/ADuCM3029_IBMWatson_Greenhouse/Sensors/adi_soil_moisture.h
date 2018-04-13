/*!
 *****************************************************************************
 * @file:    adi_soil_moisture.h
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

#ifndef _ADI_SOIL_MOISTURE_H_
#define _ADI_SOIL_MOISTURE_H_

#include <adi_sensor.h>

namespace adi_sensor_swpack
{
	/*!
	 * @class SoilMoisture
	 *
	 * @brief Class describes the interface for the Soil Moisture sensor.
	 *
	 **/
	class SoilMoisture : public Sensor
	{
	public:

		SoilMoisture(){};
	           /*!
	            * @brief  Initializes the visible light sensor and underlying peripherals.
	        	*
	        	* @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).
	        	*
	            * @details  Initializes the visible light sensor and configures it.This function opens the
	            *           underlying peripheral and configures it with the default configuration
	            *           parameters. Default configuration parameters are statically defined in
	            *           the associated sensor class configuration header.For example the
	            *           configuration for CN0397 is defined in adi_cn0397_cfg.h and the sensor
	            *           software is implemented in adi_cn0397.cpp
	            */
	    	  	virtual SENSOR_RESULT open()  = 0;

	           /*!
	            * @brief  Starts the visible light sensor data flow.
	            *
	            * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).
	            *
	            * @details  Start function should be called only after the visible light sensor is
	            *           successfully opened. If the sensor has an option for continuous read mode
	            *           the start function will put the sensor into this mode.
	            */
	           	virtual SENSOR_RESULT start() = 0;

	           /*!
	            * @brief  Stops the visible light sensor data flow.
	            *
	            * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).
	            *
	            * @details  Stop function of a visible light sensor puts the sensor in an idle
	            *           mode. In order to begin reading data, the application needs to call
	            *           the start function.
	            */
	           	virtual SENSOR_RESULT stop()  = 0;

	           /*!
	            * @brief  Close the visible light sensor.
	            *
	            * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).
	        	*
	            * @details  Close function closes the underlying peripherals. Applications must use
	            *           open again inorder to enable the measurement mode.
	            */
	           	virtual SENSOR_RESULT close() = 0;

	        private:
	};

}


#endif /* SENSORS_ADI_SOIL_MOISTURE_H_ */
