/*! 
 *****************************************************************************
  @file adi_cn0397.h
 
  @brief CN0397 class definition.

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

#ifndef ADI_CN0397_H
#define ADI_CN0397_H

#include "adi_visible_light.h"
#include "adi_ad7798.h"


/*! @addtogroup cn0397_macros CN0397 Macros
 *  @ingroup sensor_macros
 *  @brief Non-configurable macros for the CN0397 class.
 *  @{
 */

/*!< Red photodiode channel id.                                                                       */
#define CN0397_CHANNEL_ID_RED           (AD7798::CHANNEL_AIN2P_AIN2M)
/*!< Green photodiode channel id.                                                                     */
#define CN0397_CHANNEL_ID_GREEN         (AD7798::CHANNEL_AIN1P_AIN1M)
/*!< Blue photodiode channel id.                                                                      */  
#define CN0397_CHANNEL_ID_BLUE          (AD7798::CHANNEL_AIN3P_AIN3M)
/*!< Number of channels that the visible light sensor has as inputs to the ADC. Red, green and blue.  */
#define CN0397_NUM_CHANNELS             (0x03u)
/*!< Full-scale, peak-to-peak op amp output swing at maximum diode output current.                    */
#define CN0398_FS_PP_VOLTAGE            (3.0)
/*!< Red light intensity constant.                                                                    */
#define LIGHT_INTENSITY_CONSTANT_RED    (2.122)
/*!< Green light intensity constant.                                                                  */
#define LIGHT_INTENSITY_CONSTANT_GREEN  (2.124)
/*!< Blue light intensity constant.                                                                   */
#define LIGHT_INTENSITY_CONSTANT_BLUE   (2.113)
/*! @} */

extern ADI_VISUAL_LIGHT_DATA eIntensity;
extern ADI_VISUAL_LIGHT_DATA eConcentration;
extern uint8_t u8SendTopic[20];

namespace adi_sensor_swpack
{
  /**
   * @class CN0397
   * @brief Ultralow power light recognition system for smart agriculture.
   *
   **/
  class CN0397 : public VisibleLight 
  {
      public:
           /*!
            * @brief  Constructor for the CN0397 class.
            *
            * @details This function sets the sensor type to a visible light sensor and 
            *          the sensor id to that configured by the user in the configuration file. 
            *
            * @return none
            */
//	  	    MQTTClient mqttClient;

            CN0397(/*MQTTClient mqttClient*/);

            SENSOR_RESULT open();
            SENSOR_RESULT start();
            SENSOR_RESULT stop();
            SENSOR_RESULT close();
            SENSOR_RESULT getLightIntensity(float  *pLux);
            SENSOR_RESULT getLightConcentration(float *pConc);
           /*!
            * @brief  Get the light intensity of the red photodiode.
            *
            * @param  [out] pLux : Light intensity of the red photodoide in lux.
            *
            * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
            *
            * @details Read from the data register on the AD7798 with channel 1 as the ADC input to
            *          get the current of the red photodiode. Then converts this value to light intensity.
            *          in lux. This must be called after start().
            */
            SENSOR_RESULT getLightIntensityRed(float *pLux);

           /*!
            * @brief  Get the light intensity of the green photodiode.
            *
            * @param  [out] pLux : Light intensity of the green photodoide in lux.
            *
            * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
            *
            * @details Read from the data register on the AD7798 with channel 2 as the ADC input to
            *          get the current of the green photodiode. Then converts this value to light intensity.
            *          in lux. This must be called after start().
            */
            SENSOR_RESULT getLightIntensityGreen(float *pLux);

           /*!
            * @brief  Get the light intensity of the blue photodiode.
            *
            * @param  [out] pLux : Light intensity of the blue photodoide in lux.
            *
            * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
            *
            * @details Read from the data register on the AD7798 with channel 3 as the ADC input to
            *          get the current of the blue photodiode. Then converts this value to light intensity.
            *          in lux. This must be called after start().
            */
            SENSOR_RESULT getLightIntensityBlue(float *pLux);

           /*!
            * @brief  Get the light concentration of the red photodiode.
            *
            * @param  [out] pConc : Light concentration of the red photodoide out of 100% of the optimal level.
            *
            * @return SENSOR_ERROR_NONE.  
            *
            * @details Uses the last received light intensity and compares it to the optimal 
            *          level which is defined in the user configuration #CN0397_OPTIMAL_CONC_RED_CFG.
            *          This must be called after start() and getLightIntensityRed().
            *           
            */
            SENSOR_RESULT getLightConcentrationRed(float *pConc);

           /*!
            * @brief  Get the light concentration of the green photodiode.
            *
            * @param  [out] pConc : Light concentration of the green photodoide out of 100% of the optimal level.
            *
            * @return SENSOR_ERROR_NONE.  
            *
            * @details Uses the last received light intensity and compares it to the optimal 
            *          level which is defined in the user configuration #CN0397_OPTIMAL_CONC_GREEN_CFG.
            *          This must be called after start() and getLightIntensityRed().
            *           
            */
            SENSOR_RESULT getLightConcentrationGreen(float *pConc);
           /*!
            * @brief  Get the light concentration of the blue photodiode.
            *
            * @param  [out] pConc : Light concentration of the blue photodoide out of 100% of the optimal level.
            *
            * @return SENSOR_ERROR_NONE. 
            *
            * @details Uses the last received light intensity and compares it to the optimal 
            *          level which is defined in the user configuration #CN0397_OPTIMAL_CONC_BLUE_CFG.
            *          This must be called after start() and getLightIntensityRed().
            *           
            */
            SENSOR_RESULT getLightConcentrationBlue(float *pConc);

           /*!
            * @brief  Convert ADC value from the AD7798 to a voltage. 
            *
            * @param  [in]  adcValue : Value read from the AD7798.
            * @param  [out] voltage  : Voltage calculated from adcValue.
            *
            * @return SENSOR_ERROR_NONE.  
            *           
            */             
            SENSOR_RESULT  convertToVoltage(uint16_t adcValue, float * voltage);

            void displayData(void);
      private:

            /*!< CN0397 member variable. The CN0397 uses the AD7798 to turn the current from the photodiodes into a digital value. */
            AD7798 m_ad778;
            /*!< CN0397 member variable. The last received intensity of the red photodiode.                                        */
            float  m_intensity_red;
            /*!< CN0397 member variable. The last received intensity of the green photodiode.                                      */
            float  m_intensity_green;
            /*!< CN0397 member variable. The last received intensity of the blue photodiode.                                       */
            float  m_intensity_blue;
            /*!< CN0397 member variable. The optimal light concentration of the red photodiode.                                    */
            float  m_optimal_conc_red;
            /*!< CN0397 member variable. The optimal light concentration of the green photodiode.                                  */
            float  m_optimal_conc_green;
            /*!< CN0397 member variable. The optimal light concentration of the blue photodiode.                                   */
            float  m_optimal_conc_blue;

  };
}

#endif /* ADI_CN0397_H */

