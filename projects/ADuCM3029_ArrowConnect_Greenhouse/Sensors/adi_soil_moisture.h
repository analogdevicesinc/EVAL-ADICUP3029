/*
 * adi_soil_moisture.h
 *
 *  Created on: Jun 23, 2017
 *      Author: mcaprior
 */

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
