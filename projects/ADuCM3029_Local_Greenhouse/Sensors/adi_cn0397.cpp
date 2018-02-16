/*! 
 *****************************************************************************
  @file adi_cn0397.cpp
 
  @brief Defines the cn0397 visible light interface file.
 
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


#include "adi_ad7798.h"
#include"adi_cn0397.h"
#include "adi_cn0397_cfg.h"

namespace adi_sensor_swpack
{
    CN0397::CN0397()
    {       
    }
    
    SENSOR_RESULT CN0397::open()
    {   
        /* Initialize the CN0397 light intensity and concentration values */
        m_intensity_red = 0.0;
        m_intensity_green = 0.0;
        m_intensity_blue = 0.0;
        m_optimal_conc_red = CN0397_OPTIMAL_CONC_RED_CFG;
        m_optimal_conc_green = CN0397_OPTIMAL_CONC_GREEN_CFG;
        m_optimal_conc_blue = CN0397_OPTIMAL_CONC_BLUE_CFG;


        /* Initialize the AD7798 */
        return(m_ad778.init());
    }

    SENSOR_RESULT CN0397::start()
    {
        SENSOR_RESULT eSensorResult;
        uint8_t nChannel;

        /* Run a zero-scale system calibration on all three channels  */
        for(nChannel= 0x0u; nChannel < CN0397_NUM_CHANNELS; nChannel++)
        {
            if((eSensorResult = m_ad778.setChannel((AD7798::CHANNEL)nChannel)) == SENSOR_ERROR_NONE)
            {
                if((eSensorResult = m_ad778.zeroScaleSystemCalibration()) == SENSOR_ERROR_NONE)
                {
                    return(m_ad778.setOperatingMode(AD7798::OPERATING_MODE_CONT));
                }
            }
        }       
        return(eSensorResult);
    }
        
    SENSOR_RESULT CN0397::stop()
    {
        /* Put the AD7798 into powerdown mode when it is not being used */
        return(m_ad778.setOperatingMode(AD7798::OPERATING_MODE_PWRDN));
    }
        
    SENSOR_RESULT CN0397::close()
    {
        /* Nothing to clean up */
        return(SENSOR_ERROR_NONE);
    }

    SENSOR_RESULT CN0397::getLightIntensityRed(float *pLux)
    {
        SENSOR_RESULT eSensorResult; 
        uint16_t nData = 0;

            if((eSensorResult = m_ad778.setChannel(CN0397_CHANNEL_ID_RED)) == SENSOR_ERROR_NONE)
            {
                if((eSensorResult = m_ad778.getData(&nData)) == SENSOR_ERROR_NONE)
                {
                   *pLux = (float)(nData * LIGHT_INTENSITY_CONSTANT_RED);
                   m_intensity_red = *pLux;               
                }
            }
        
        return(eSensorResult);
    }

    SENSOR_RESULT CN0397::getLightIntensityGreen(float *pLux)
    {
        SENSOR_RESULT eSensorResult; 
        uint16_t nData = 0;

            if((eSensorResult = m_ad778.setChannel(CN0397_CHANNEL_ID_GREEN)) == SENSOR_ERROR_NONE)
            {
                if((eSensorResult = m_ad778.getData(&nData)) == SENSOR_ERROR_NONE)
                {
                   *pLux = (float)(nData * LIGHT_INTENSITY_CONSTANT_GREEN);  
                   m_intensity_green = *pLux;             
                }
            }
        
        return(eSensorResult);
    }

    SENSOR_RESULT CN0397::getLightIntensityBlue(float *pLux)
    {
        SENSOR_RESULT eSensorResult; 
        uint16_t nData = 0;

            if((eSensorResult = m_ad778.setChannel(CN0397_CHANNEL_ID_BLUE)) == SENSOR_ERROR_NONE)
            {
                if((eSensorResult = m_ad778.getData(&nData)) == SENSOR_ERROR_NONE)
                {
                   *pLux = (float)(nData * LIGHT_INTENSITY_CONSTANT_BLUE);
                   m_intensity_blue = *pLux;
                }
            }
        
        return(eSensorResult);
    }

    SENSOR_RESULT CN0397::getLightIntensity(float *pLux)
    {
        SENSOR_RESULT eSensorResult; 

            if((eSensorResult = getLightIntensityRed(&pLux[0])) == SENSOR_ERROR_NONE)
            {
                if((eSensorResult = getLightIntensityGreen(&pLux[1])) == SENSOR_ERROR_NONE)
                {
                    return(eSensorResult = getLightIntensityBlue(&pLux[2]));
                }
            }
        
        return(eSensorResult);
    }

    SENSOR_RESULT CN0397::getLightConcentrationRed(float *pConc)
    {
    	*pConc = (m_intensity_red * 100)/m_optimal_conc_red;
    	return(SENSOR_ERROR_NONE);
    }

    SENSOR_RESULT CN0397::getLightConcentrationGreen(float *pConc)
    {
    	*pConc = (m_intensity_green * 100)/m_optimal_conc_green;
    	return(SENSOR_ERROR_NONE);
    }

    SENSOR_RESULT CN0397::getLightConcentrationBlue(float *pConc)
    {
    	*pConc = (m_intensity_blue * 100)/m_optimal_conc_blue;
    	return(SENSOR_ERROR_NONE);
    }

    SENSOR_RESULT CN0397::getLightConcentration(float *pConc)
    {
        SENSOR_RESULT eSensorResult; 

            if((eSensorResult = getLightConcentrationRed(&pConc[0])) == SENSOR_ERROR_NONE)
            {
                if((eSensorResult = getLightConcentrationGreen(&pConc[1])) == SENSOR_ERROR_NONE)
                {
                    return(eSensorResult = getLightConcentrationBlue(&pConc[2]));
                }
            }
        
        return(eSensorResult);
    }

    SENSOR_RESULT CN0397::convertToVoltage(uint16_t adcValue, float * voltage)
    {
        *voltage = (float)(adcValue * AD7798_REFERENCE_VOLTAGE)/(float)(AD7798_2_TO_THE_RES * m_ad778.getGain());
        
        return(SENSOR_ERROR_NONE);
    }

    void CN0397::displayData()
    {
    }
}
