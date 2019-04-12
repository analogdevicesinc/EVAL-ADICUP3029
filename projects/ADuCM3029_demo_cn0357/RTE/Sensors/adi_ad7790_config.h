/*!
 *****************************************************************************
  @file adi_ad7790_cfg.h

  @brief AD7790 static configuration file.

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


/*!
 * @defgroup static_config Static Configuration
 */


/*! @addtogroup ad7790_static_config AD7790 Static Configuration
 *  @ingroup static_config
 *  @brief Configurable macros for the AD7790 class.
 *  @{
 */


#ifndef ADI_AD7790_CFG_H
#define ADI_AD7790_CFG_H


#include <drivers/spi/adi_spi.h>


/*!
 * @brief SPI CS configuration.
 *
 * @details Choose which of the hardware chip selects is
 *          used to communicate with the AD7790.
 *
 * @note  Since the AD7790 is passed a SPI handle when created,
 *        minimal SPI configuration is done within the AD7790
 *        driver itself. The creator must open the SPI driver
 *        with the proper bus number (0, 1, 2, etc.) and apply
 *        the static configuration.
 *
 *        The AD7790 has the following SPI parameters:
 *        - Master mode
 *        - Continuous mode
 *        - CPOL = 1 (clock idles high)
 *        - CPHA = 1 (data clocked on leading edge)
 */
#define AD7790_CFG_SPI_CS  (ADI_SPI_CS1)

/*!
 * @brief Measurement mode select bits.
 *
 * @details Choose from the following options:
 *          - 0 = Continuous (default)
 *          - 2 = Single
 *          - 3 = Power Down
 */
#define AD7790_CFG_MODE_SELECT (0x0u)

/*!
 * @brief Analog input range.
 *
 * @details Choose from the following options:
 *          - 0 = +/- VREF (default)
 *          - 1 = +/- VREF/2
 *          - 2 = +/- VREF/4
 *          - 3 = +/- VREF/8
 */
#define AD7790_CFG_MODE_RANGE  (0x0u)

/*!
 * @brief Buffering.
 *
 * @details Choose from the following options:
 *          - 0 = Unbuffered mode
 *          - 1 = Buffered mode (default)
 */
#define AD7790_CFG_MODE_BUFFERING (0x0u)

/*!
 * @brief Burnout current.
 *
 * @details Choose from the following options:
 *          - 0 = Burnout currents disabled (default)
 *          - 1 = Burnout currents enabled
 */
#define AD7790_CFG_MODE_BURNOUT_CURRENT (0x0u)

/*!
 * @brief Clock divider.
 *
 * @details Choose from the following options:
 *          - 0 = CLOCK  (default)
 *          - 1 = CLOCK / 2
 *          - 2 = CLOCK / 4
 *          - 3 = CLOCK / 8
 */
#define AD7790_CFG_FILTER_CLOCK_DIV (0x0u)

/*!
 * @brief Output data rate.
 *
 * @details Please see data sheet page 12 for possible options. Valid values between 0-7 (inclusive).
 *
 */
#define AD7790_CFG_FILTER_WORD_RATE (0x7u)


#if (AD7790_CFG_MODE_REGISTER != 0x0) && (AD7790_CFG_MODE_REGISTER != 0x2u) && (AD7790_CFG_MODE_REGISTER != 0x3u)
#error "Invalid setting of the mode select bits"
#endif

#if (AD7790_CFG_MODE_RANGE > 0x3u)
#error "Invalid setting of the range bits"
#endif

#if (AD7790_CFG_MODE_BUFFERING > 0x1u)
#error "Invalid setting of the buffering mode"
#endif

#if (AD7790_CFG_MODE_BURNOUT_CURRENT > 0x1u)
#error "Invalid setting of the burnout current mode"
#endif

#if (AD7790_CFG_FILTER_CLOCK_DIV > 0x3u)
#error "Invalid setting of the clock divider bits"
#endif

#if (AD7790_CFG_FILTER_WORD_RATE > 0x7u)
#error "Invalid setting of the word rate bits"
#endif


#endif /* ADI_AD7790_CFG_H */


/*! @} */
