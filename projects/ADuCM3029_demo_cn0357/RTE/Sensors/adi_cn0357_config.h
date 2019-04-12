/*! 
 *****************************************************************************
  @file adi_cn0357_cfg.h
 
  @brief CN0357 static configuration file.
 
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


/*! @addtogroup cn0357_static_config CN0357 Static Configuration 
 *  @ingroup static_config
 *  @brief Configurable macros for the CN0357 class.
 *  @{
 */


#ifndef ADI_CN0357_CFG_H 
#define ADI_CN0357_CFG_H 


#include <drivers/gpio/adi_gpio.h>


/*! Sensor ID. */
#define CN0357_CFG_ID                  (0x00000357ul)

/*! Sensor version. */
#define CN0357_CFG_VERSION             (0x00000001ul)

/*! SPI bus. */
#define CN0357_CFG_SPI_DEV_NUMBER      (0u)

/*! SPI CS port. */
#define CN0357_CFG_AD5270_CS_GPIO_PORT (ADI_GPIO_PORT1)

/*! SPI CS pin. */
#define CN0357_CFG_AD5270_CS_GPIO_PIN  (ADI_GPIO_PIN_11)


#endif /* ADI_CN0357_CFG_H */


/* @} */
