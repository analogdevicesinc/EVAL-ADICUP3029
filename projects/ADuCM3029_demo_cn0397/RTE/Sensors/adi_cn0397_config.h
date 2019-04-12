/*!
 *****************************************************************************
  @file adi_cn0397_cfg.h

  @brief CN0397 visible light configuration file.

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


/*! @addtogroup cn0397_static_config CN0397 Static Configuration
 *  @ingroup static_config
 *  @brief Configurable macros for the CN0397 class.
 *  @{
 */
#ifndef ADI_CN0397_CFG_H
#define ADI_CN0397_CFG_H

namespace adi_sensor_swpack
{

    /*!
     *  @brief ID of sensor.
     *
     *  @details Unique identifier for this particular sensor. This is used
     *           in the Bluetooth data packet to alert the Android application
     *           of a particular instance of a sensor. This value is a 7-bit
     *           number.
     */
#define CN0397_SENSOR_ID_CFG            (0x01u)

    /*!
     *  @brief Optimal level of light for the red photodiode.
     *
     *  @details The default value is taken from the PAR(Photosynthetically Active Radiation)
     *           curve which plants react to.
     */
#define CN0397_OPTIMAL_CONC_RED_CFG      (26909)

    /*!
     *  @brief Optimal level of light for the green photodiode.
     *
     *  @details The default value is taken from the PAR(Photosynthetically Active Radiation)
     *           curve which plants react to.
     */
#define CN0397_OPTIMAL_CONC_GREEN_CFG      (8880)
    /*!
    *  @brief Optimal level of light for the blue photodiode.
    *
    *  @details The default value is taken from the PAR(Photosynthetically Active Radiation)
    *           curve which plants react to.
    */
#define CN0397_OPTIMAL_CONC_BLUE_CFG      (26909)
}

/****************** Macro Validation ******************/

#if (CN0397_SENSOR_ID > (0x127u))
#error "Invalid sensor id"
#endif

#endif /* ADI_CN0397_CFG_H */
/* @} */
