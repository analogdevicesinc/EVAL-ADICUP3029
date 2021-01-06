/***************************************************************************//**
*   @file   aducm3029_adc.h
*   @brief  ADuCM3029 integrated ADC wrapper header.
*   @author Andrei Drimbarean (andrei.drimbarean@analog.com)
********************************************************************************
* Copyright 2020(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
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
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef ADUCM3029_ADC_H_
#define ADUCM3029_ADC_H_

#include <stdint.h>
#include <drivers/adc/adi_adc.h>

#define ADuCM3029_ADC_CHANN_0	ADI_ADC_CHANNEL_0
#define ADuCM3029_ADC_CHANN_1	ADI_ADC_CHANNEL_1
#define ADuCM3029_ADC_CHANN_2	ADI_ADC_CHANNEL_2
#define ADuCM3029_ADC_CHANN_3	ADI_ADC_CHANNEL_3
#define ADuCM3029_ADC_CHANN_4	ADI_ADC_CHANNEL_4
#define ADuCM3029_ADC_CHANN_5	ADI_ADC_CHANNEL_5
#define ADuCM3029_ADC_CHANN_6	ADI_ADC_CHANNEL_6
#define ADuCM3029_ADC_CHANN_7	ADI_ADC_CHANNEL_7

//TODO: comment aducm3029_adc_init_param
struct aducm3029_adc_init_param {
	uint8_t adc_id;
};

//TODO: comment aducm3029_adc_desc
struct aducm3029_adc_desc {
	uint8_t adc_id;
	void *memory;
	ADI_ADC_HANDLE handler;
};

//TODO: comment aducm3029_adc_init
int32_t aducm3029_adc_init(struct aducm3029_adc_desc **device,
			   struct aducm3029_adc_init_param *init_param);

//TODO: comment aducm3029_adc_remove
int32_t aducm3029_adc_remove(struct aducm3029_adc_desc *dev);

//TODO: comment aducm3029_adc_sample_once
int32_t aducm3029_adc_sample_once(struct aducm3029_adc_desc *dev,
				  uint32_t channel, uint16_t *code);

//TODO: comment aducm3029_adc_convert_sample
float aducm3029_adc_convert_sample(struct aducm3029_adc_desc *dev,
				   uint16_t code);

#endif /* ADUCM3029_ADC_H_ */
