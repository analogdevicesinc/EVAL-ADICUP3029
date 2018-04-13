/*!
 *****************************************************************************
 * @file:    adi_cn0410_cfg.h
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

#ifndef SENSORS_ADI_CN0410_CFG_H_
#define SENSORS_ADI_CN0410_CFG_H_


#include <drivers/spi/adi_spi.h>
#include "adi_cn0410.h"

/****************** SPI Configuration ******************/

/*!
 * @brief SPI device number.
 * @details Choose the SPI bus that this device is connected to.
 */
#define CN0410_SPI_DEV_CFG       (0u)
/*!
 * @brief SPI CS configuration.
 *
 * @details Choose which of the hardware chip selects is
 *          used to communicate with the AD7798.
 *
 */
#define CN0410_SPI_CS_CFG        (ADI_SPI_CS_NONE)
/*!
 * @brief SPI bitrate configuration.
 *
 * @details Set this macro to the SPI0 bit rate in hertz
 *
 */
#define CN0410_SPI_BITRATE_CFG   (1000000u)
/*!
 * @brief SPI master mode configuration.
 *
 * @details If using the SPI in master mode set this macro to true. For slave mode set this macro to false.
 *
 */
#define CN0410_SPI_MASTER_CFG    (true)
/*!
 * @breif Configure DMA use.
 *
 * @details  When initializing the SPI, this flag will either enable or disable
 *           DMA use for all the transactions.
 *            true  : use DMA
 *            false : use PIO
 */
#define CN0410_SPI_DMA_CFG        (false)

/****************** ADC Configuration ******************/


/****************** Macro Validation ******************/

#if (CN0410_SPI_BITRATE_CFG > (13000000u))
#error "Invalid spi bitrate configuration"
#endif


#endif /* SENSORS_ADI_CN0410_CFG_H_ */
