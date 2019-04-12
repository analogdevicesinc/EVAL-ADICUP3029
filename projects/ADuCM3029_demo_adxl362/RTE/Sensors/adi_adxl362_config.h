/*! 
 *****************************************************************************
  @file adi_adxl362_cfg.h
 
  @brief ADXL362 static configuration file.
 
  @details
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


/*! @addtogroup adxl362_static_config ADXL362 Static Configuration 
 *  @ingroup static_config
 *  @brief Configurable macros for the ADXL362 class.
 *  @{
 */
#ifndef ADI_ADXL362_CFG_H
#define ADI_ADXL362_CFG_H

#include <axl/adi_accelerometer.h>

/*!< ADXL362 shield is expected to be connected to SPI 1*/
#define ADI_CFG_SPI_DEV       (1u)
 
/*!< SPI chip select     */
#define ADI_CFG_SPI_CS        (ADI_SPI_CS0)

/*!< SPI Bit Rate        */
#define ADI_CFG_SPI_BITRATE   (500000u)

/*!< SPI Master          */
#define ADI_CFG_SPI_MASTER    (1u)

/*!< ADXL362 range is 2G */
#define ADI_CFG_ADXL362_RANGE (adi_sensor_swpack::Accelerometer::AXL_RANGE_2g)

/*!< ADXL362 output data rate 100Hz */
#define ADI_CFG_ADXL362_RATE  (adi_sensor_swpack::Accelerometer::AXL_ODR_100)

#endif  /* ADI_ADXL362_CFG_H */
/* @} */
