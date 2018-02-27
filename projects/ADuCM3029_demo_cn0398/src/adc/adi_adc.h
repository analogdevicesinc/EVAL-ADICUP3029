/*!
 *****************************************************************************
  @file adi_adc.h

  @brief ADC class definition.

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


#ifndef ADI_BASE_ADC_H
#define ADI_BASE_ADC_H


#include <base_sensor/adi_sensor.h>


namespace adi_sensor_swpack
{
/*!
 *
 * @class ADC
 *
 * @brief Generic interface for all Analog-to-Digital Converters.
 *
 */
class ADC
{
public:

	/*!
	 * @brief  Initializes the ADC.
	 *
	 * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).
	 *
	 * @details Init function initializes the ADC and underlying peripherals. This API has
	 *          to be implemented by all adc classes.
	 */
	virtual SENSOR_RESULT   init() = 0;

	/*!
	 * @brief  Reset the ADC.
	 *
	 * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).
	 *
	 * @details Reset the ADC to the default state.
	 */
	virtual SENSOR_RESULT   reset() = 0;

	/*!
	 * @brief  Read a specified register on the ADC.
	 *
	 * @param  [in] regAddress : The address of the register to read from.
	 * @param  [in] size       : The number of bytes to read.
	 * @param  [out] regValue  : Value read from the register.
	 *
	 * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).
	 *
	 */
	virtual SENSOR_RESULT   readRegister(uint32_t regAddress, uint32_t size, uint8_t * regValue) = 0;

	/*!
	 * @brief  Write to a specified register on the ADC.
	 *
	 * @param  [in] regAddress : The address of the register to write to.
	 * @param  [in] regValue   : The value to write to the register.
	 * @param  [in] size       : The number of bytes to write.
	 *
	 * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).
	 *
	 */
	virtual SENSOR_RESULT   writeRegister(uint32_t regAddress, uint32_t size, uint32_t regValue) = 0;

	/*!
	 * @brief  This method is used to get last ADC hardware error.
	 *
	 * @return Value of the last hardware error that occured on the ADC.
	 *
	 */
	uint32_t getLastADCHwError()
	{
		return m_last_hw_error;
	}

protected:

	/*!
	  * @brief   This method is used to set last ADC hardware error.
	  *
	  * @param   [in] lastHwError : Value of the last hardware error that occured on the ADC.
	  *
	  * @details This method is used to save the ADC error. This should be used
	  *          if #SENSOR_RESULT has indicated that a hardware error occured.
	  */
	void setLastADCHwError(const uint32_t lastHwError)
	{
		m_last_hw_error = lastHwError;
	}

private:
	/*! Hold the ADC last hardware error value. */
	uint32_t    m_last_hw_error;
};
}

#endif /* ADI_BASE_ADC_H */
