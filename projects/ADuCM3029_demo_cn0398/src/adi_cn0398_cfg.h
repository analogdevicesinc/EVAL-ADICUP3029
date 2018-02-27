/*
 * adi_cn0398_cfg.h
 *
 *  Created on: Jun 23, 2017
 *      Author: mcaprior
 */

#ifndef _ADI_CN0398_CFG_H_
#define _ADI_CN0398_CFG_H_


/*************************** Configuration ***********************************/
/* ms */
#define DISPLAY_REFRESH 1000
/* select to do calibration in two point at init */
#define USE_PH_CALIBRATION YES
#define USE_LOAD_FACTOR NO

/* comment if don't want to use */
#define USE_MANUFACTURER_MOISTURE_EQ

#define TEMPERATURE_SENSOR_PRESENT
#define MOISTURE_SENSOR_PRESENT
#define PH_SENSOR_PRESENT

/* comment if you don't want to use it */
//#define USE_LINEAR_TEMP_EQ


#endif /* _ADI_CN0398_CFG_H_ */
