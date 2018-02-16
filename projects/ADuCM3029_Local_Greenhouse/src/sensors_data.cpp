/**
******************************************************************************
*   @file     sensors_data.cpp
*   @brief    Source file for reading data from sensors.
*   @version  V0.1
*   @author   ADI
*   @date     November 2017
*   @par Revision History:
*  - V0.1, May 2017: initial version.
*
*
*******************************************************************************
* Copyright 2017(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*  - Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  - Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
*  - Neither the name of Analog Devices, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*  - The use of this software may or may not infringe the patent rights
*    of one or more patent holders.  This license does not release you
*    from the requirement that you obtain separate licenses from these
*    patent holders to use this software.
*  - Use of the software either in source or binary form, must be run
*    on or directly connected to an Analog Devices Inc. component.
*
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*********************************************************************************/

#include "sensors_data.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "adi_cn0398.h"
#include "adi_cn0397.h"

using namespace adi_sensor_swpack;

extern CN0398  * cn0398;
extern VisibleLight   *pLight;

ADI_VISUAL_LIGHT_DATA eIntensity;
ADI_VISUAL_LIGHT_DATA eConcentration;


/**
  * @brief  fill the payload with the sensor values
  * @param  none
  * @param sensor_data_t is the char pointer for the data to be filled
  * @retval 0 in case of success
  *         -1 in case of failure
  */
int PreparePayload(void *v) {

	sensors_data_t *sd = (sensors_data_t*)v;

	float temp;

	temp = cn0398->read_rtd();
	if ( (temp > 0) & (temp < 100) )
		sd->temperature = temp;

	sd->ph = cn0398->read_ph(sd->temperature);
	sd->humidity = cn0398->read_moisture();

	/* Get the red, green and blue visual light data and concentration. */
	pLight->getLightIntensity((float *)&eIntensity.fData_Red);
	pLight->getLightConcentration((float *)&eConcentration.fData_Red);

	sd->red_intensity = eIntensity.fData_Red;
	sd->green_intensity = eIntensity.fData_Green;
	sd->blue_intensity = eIntensity.fData_Blue;

	sd->red_concentration = eConcentration.fData_Red;
	sd->green_concentration = eConcentration.fData_Green;
	sd->blue_concentration = eConcentration.fData_Blue;

	if (sd->red_concentration > 100)
		sd->red_concentration = 100;

	if (sd->blue_concentration > 100)
		sd->blue_concentration = 100;

	if (sd->green_concentration > 100)
		sd->green_concentration = 100;

	return 0;
}

