/*!
 *****************************************************************************
  @file adi_ad7798_cfg.h

  @brief AD7798 static configuration file.

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


/*! @addtogroup ad7798_static_config AD7798 Static Configuration
 *  @ingroup static_config
 *  @brief Configurable macros for the AD7798 class.
 *  @{
 */

#ifndef ADI_AD7798_CFG_H
#define ADI_AD7798_CFG_H

#include <drivers/spi/adi_spi.h>
#include <adc/ad7798/adi_ad7798.h>

/****************** SPI Configuration ******************/

/*!
 * @brief SPI device number.
 * @details Choose the SPI bus that this device is connected to.
 */
#define AD7798_SPI_DEV_CFG       (0u)
/*!
 * @brief SPI CS configuration.
 *
 * @details Choose which of the hardware chip selects is
 *          used to communicate with the AD7798.
 *
 */
#define AD7798_SPI_CS_CFG        (ADI_SPI_CS1)
/*!
 * @brief SPI bitrate configuration.
 *
 * @details Set this macro to the SPI0 bit rate in hertz
 *
 */
#define AD7798_SPI_BITRATE_CFG   (1444444u)
/*!
 * @brief SPI master mode configuration.
 *
 * @details If using the SPI in master mode set this macro to true. For slave mode set this macro to false.
 *
 */
#define AD7798_SPI_MASTER_CFG    (true)
/*!
 * @breif Configure DMA use.
 *
 * @details  When initializing the SPI, this flag will either enable or disable
 *           DMA use for all the transactions.
 *            true  : use DMA
 *            false : use PIO
 */
#define AD7798_SPI_DMA_CFG        (false)

/****************** ADC Configuration ******************/
/*!
 * @brief AD7798 gain configuration.
 *
 * @details This should be set using a gain value of #AD7798:GAIN.
 *          0x00u : Gain 1 input range is 2.5V.
 *          0x01u : Gain 2 input range is 1.25V.
 *          0x02u : Gain 4 input range is 625mV.
 *          0x03u : Gain 8 input range is 312.5mV.
 *          0x04u : Gain 16 input range is 156.2mV.
 *          0x05u : Gain 32 input range is 78.125mV.
 *          0x06u : Gain 64 input range is 39.06mV.
 *          0x07u : Gain 128 input range is 19.53mV.
 */
#define AD7798_GAIN_CFG                 (0x00u)
/*!
 * @brief AD7798 power control switch bit configuration.
 *
 * @details This should be enable or disable the power control switch bit.
 *          This bit is set by the user to close the power switch PSW to GND.
 *          The power switch can  sink up to 30 mA. Cleared by user to open
 *          the power switch.
 *          0x01u : Close switch
 *          0x00u : Open switch
 *
 */
#define AD7798_PSW_CFG                 (0x0u)
/*!
 * @brief AD7798 coding mode configuration.
 *
 * @details This should be set using a coding mode value of #AD7798:CODING_MODE.
 *          0x00u :  Used to enable bipolar coding.
 *          0x01u :  Used to enable ubipolar coding.
 */
#define AD7798_CODING_MODE_CFG          (0x01u)
/*!
 * @brief AD7798 filter update rate configuration.
 *
 * @details This should be set using a filter update value of #AD7798:FILTER_RATE.
 *          0x01u : Update rate is 470 Hz.
 *          0x02u : Update rate is 242 Hz.
 *          0x03u : Update rate is 123 Hz.
 *          0x04u : Update rate is 62 Hz.
 *          0x05u : Update rate is 50 Hz.
 *          0x06u : Update rate is 39 Hz.
 *          0x07u : Update rate is 33.2 Hz.
 *          0x08u : Update rate is 19.6 Hz.
 *          0x09u : Update rate is 16.7 Hz with 80 db rejection.
 *          0x0Au : Update rate is 16.7 Hz with 65 db rejection.
 *          0x0Bu : Update rate is 12.5 Hz.
 *          0x0Cu : Update rate is 10 Hz.
 *          0x0Du : Update rate is 8.33 Hz.
 *          0x0Eu : Update rate is 6.25 Hz.
 *          0x0Fu : Update rate is 4.17 Hz.
 */
#define AD7798_FILTER_RATE_CFG           (0x05u)
/*!
 * @brief AD7798 reference detection function enable/disablen.
 *
 * @details This should be set using a filter update value of #AD7798:REF_DET.
 *          0x01u : Enable reference detect functioon
 *          0x00u : Disable reference detect functioon
 */
#define AD7798_REFERENCE_CFG             (0x01u)
/*!
 * @brief AD7798 burnout current enable bit configuration.
 *
 * @details This should be enable or disable the burnout current.
 *          When this bit is set to 1 by the user, the 100 nA current
 *          sources in the signal path are enabled. The burnout currents
 *          can be enabled only when the buffer or in-amp is active.
 *          0x01u : Enable burnout current
 *          0x00u : Disable burnout current
 *
 */
#define AD7798_BO_CFG                   (0x00u)
/*!
 * @brief Configure the AD7798 for buffered or unbuffered modes.
 *
 * @details If BUF is cleared, the ADC operates in unbuffered mode,
 *          lowering the power consumption of the device. If BUF is set,
 *          the ADC operates in buffered mode, allowing the user to place
 *          source impedances on the front end without contributing gain
 *          errors to the system. The buffer can be disabled when the gain
 *          equals 1 or 2. For higher gains, the buffer is automatically enabled.
 *          With the buffer disabled, the voltage on the analog input pins can
 *          range from 30 mV below GND to 30 mV above AVDD. When the buffer is enabled,
 *          it requires some headroom; therefore, the voltage on any input pin
 *          must be limited to 100 mV within the power supply rails.
 *          0x01u : Buffered mode
 *          0x00u : Unbuffered mode
 *
 */
#define AD7798_BUF_CFG                   (0x1u)

/****************** Macro Validation ******************/

#if (AD7798_SPI_BITRATE_CFG > (13000000u))
#error "Invalid spi bitrate configuration"
#endif

#if (AD7798_GAIN_CFG > (0x07u))
#error "Invalid gain configuration"
#endif

#if (AD7798_CODING_MODE_CFG > (0x01u))
#error "Invalid coding mode configuration"
#endif

#if (AD7798_FILTER_CFG > (0x0Fu))
#error "Invalid filter configuration"
#endif

#if (AD7798_REF_DET_EN_CFG > (0x01u))
#error "Invalid reference detection function configuration"
#endif

#if (AD7798_PSW_CFG > (0x01u))
#error "Invalid power switch bit configuration"
#endif

#if (AD7798_BO_CFG > (0x01u))
#error "Invalid burnout current bit configuration"
#endif

#if (AD7798_BUF_CFG > (0x01u))
#error "Invalid burnout current bit configuration"
#endif

#endif /* ADI_AD7798_CFG_H */
/* @} */
