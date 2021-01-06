/***************************************************************************//**
*   @file   aducm3029_adc.c
*   @brief  ADuCM3029 integrated ADC wrapper source.
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

#include <stdlib.h>
#include <math.h>
#include "aducm3029_adc.h"
#include "error.h"
#include "delay.h"

//TODO: comment aducm3029_adc_init
int32_t aducm3029_adc_init(struct aducm3029_adc_desc **device,
			   struct aducm3029_adc_init_param *init_param)
{
	int32_t ret;
	struct aducm3029_adc_desc *dev;
	bool flg;

	dev = (struct aducm3029_adc_desc *)calloc(1, sizeof(*dev));
	if (!dev)
		return FAILURE;
	dev->memory = calloc(1, ADI_ADC_MEMORY_SIZE);
	if (!dev->memory)
		goto error_dev;
	dev->adc_id = init_param->adc_id;

	ret = adi_adc_Open(dev->adc_id, dev->memory, ADI_ADC_MEMORY_SIZE,
			   &dev->handler);
	if (ret != ADI_ADC_SUCCESS)
		goto error_mem;

	ret = adi_adc_PowerUp(dev->handler, true);
	if (ret != ADI_ADC_SUCCESS)
		goto error_drv;

	ret = adi_adc_SetVrefSource(dev->handler, ADI_ADC_VREF_SRC_INT_2_50_V);
	if (ret != ADI_ADC_SUCCESS)
		goto error_drv;

	ret = adi_adc_EnableADCSubSystem(dev->handler, true);
	if (ret != ADI_ADC_SUCCESS)
		goto error_drv;
	mdelay(5);

	ret = adi_adc_StartCalibration(dev->handler);
	if (ret != ADI_ADC_SUCCESS)
		goto error_drv;

	do {
		ret = adi_adc_IsCalibrationDone(dev->handler, &flg);
		if (ret != ADI_ADC_SUCCESS)
			goto error_drv;
	} while (!flg);

	*device = dev;

	return SUCCESS;
error_drv:
	adi_adc_Close(dev->handler);
error_mem:
	free(dev->memory);
error_dev:
	free(dev);

	return FAILURE;
}

//TODO: comment aducm3029_adc_remove
int32_t aducm3029_adc_remove(struct aducm3029_adc_desc *dev)
{
	int32_t ret;

	ret = adi_adc_EnableADCSubSystem(dev->handler, false);
	if (ret != ADI_ADC_SUCCESS)
		return FAILURE;

	ret = adi_adc_PowerUp(dev->handler, false);
	if (ret != ADI_ADC_SUCCESS)
		return FAILURE;

	ret = adi_adc_Close(dev->handler);
	if (ret != ADI_ADC_SUCCESS)
		return FAILURE;

	free(dev->memory);
	free(dev);

	return SUCCESS;
}

//TODO: comment aducm3029_adc_sample_once
int32_t aducm3029_adc_sample_once(struct aducm3029_adc_desc *dev,
				  uint32_t channel, uint16_t *code)
{
	int32_t ret;
	uint8_t buff[2];
	ADI_ADC_BUFFER buffer, *adcbuffer;

	buffer.nBuffSize = 2;
	buffer.nChannels = channel;
	buffer.nNumConversionPasses = 1;
	buffer.pDataBuffer = buff;

	ret = adi_adc_SubmitBuffer(dev->handler, &buffer);
	if (ret != ADI_ADC_SUCCESS)
		return FAILURE;

	ret = adi_adc_Enable(dev->handler, true);
	if (ret != ADI_ADC_SUCCESS)
		return FAILURE;

	ret = adi_adc_GetBuffer(dev->handler, &adcbuffer);
	if (ret != ADI_ADC_SUCCESS)
		return FAILURE;

	ret = adi_adc_Enable(dev->handler, false);
	if (ret != ADI_ADC_SUCCESS)
		return FAILURE;

	*code = ((uint8_t *)adcbuffer->pDataBuffer)[0];
	*code |= ((uint8_t *)adcbuffer->pDataBuffer)[1] << 8;

	return SUCCESS;
}

//TODO: comment aducm3029_adc_convert_sample
float aducm3029_adc_convert_sample(struct aducm3029_adc_desc *dev,
				   uint16_t code)
{
	const float dac_lsb = 2.5 / (pow(2, 12) - 1);

	return (code * dac_lsb);
}
