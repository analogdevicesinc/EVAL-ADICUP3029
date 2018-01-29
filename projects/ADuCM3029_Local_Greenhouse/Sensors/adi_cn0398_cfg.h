/*
 * adi_cn0398_cfg.h
 *
 *  Created on: Jun 23, 2017
 *      Author: mcaprior
 */

#ifndef _ADI_CN0398_CFG_H_
#define _ADI_CN0398_CFG_H_


/********************************* Configuration****************************************/
#define DISPLAY_REFRESH 1000       //ms
#define USE_PH_CALIBRATION        YES// select to do calibration in two point at init
#define USE_LOAD_FACTOR           NO

#define USE_MANUFACTURER_MOISTURE_EQ  //comment if don't want to use

#define TEMPERATURE_SENSOR_PRESENT
#define MOISTURE_SENSOR_PRESENT
#define PH_SENSOR_PRESENT

//#define USE_LINEAR_TEMP_EQ  // comment if you don't want to use it


#endif /* _ADI_CN0398_CFG_H_ */
