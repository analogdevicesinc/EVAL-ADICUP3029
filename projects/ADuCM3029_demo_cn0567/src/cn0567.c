/***************************************************************************//**
 *   @file   cn0567.c
 *   @brief  CN0567 Source file
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
********************************************************************************
 * Copyright 2021(c) Analog Devices, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "cn0567.h"
#include "cn0567_config.h"
#include "no-os/error.h"
#include "no-os/gpio.h"
#include "aducm3029_gpio.h"
#include "no-os/delay.h"
#include "no-os/util.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Set device to get timestamp for low frequency oscillator calibration.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0567_calibrate_lfo_set_ts(struct cn0567_dev *dev)
{
	int32_t ret;
	uint16_t reg_data;

	/** Enable clock calibration circuitry */
	if(dev->chip_id == 0x02c2) {
		ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
					&reg_data);
		if (IS_ERR_VALUE(ret))
			return ret;
		reg_data |= BITM_OSC1M_OSC_CLK_CAL_ENA;
		ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
					 reg_data);
	}

	/** Enable GPIO0 input */
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_GPIO_CFG,
				&reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;

	reg_data |= (1 & BITM_GPIO_CFG_GPIO_PIN_CFG0);

	ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_GPIO_CFG,
				 reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;

	/** Enable GPIO0 as time stamp input */
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_GPIO_EXT,
				&reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;

	reg_data |= ((0 << BITP_GPIO_EXT_TIMESTAMP_GPIO) &
		     BITM_GPIO_EXT_TIMESTAMP_GPIO);

	return adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_GPIO_EXT,
				  reg_data);
}

/**
 * @brief Get time stamp for low frequency oscillator calibration.
 * @param [in] dev - The device structure.
 * @param [out] ts_val - Pointer to the timestamp value container.
 * @param [in] ts_gpio - Descriptor for the counter start/stop GPIO.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0567_calibrate_lfo_get_timestamp(struct cn0567_dev *dev,
		uint32_t *ts_val, struct gpio_desc *ts_gpio)
{
	int32_t ret;
	uint16_t reg_data;

	/** Start time stamp calibration */
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC32K,
				&reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;
	reg_data |= BITM_OSC32K_CAPTURE_TIMESTAMP;
	ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_OSC32K,
				 reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;

	/** Give first time stamp trigger */
	ret = gpio_set_value(ts_gpio, GPIO_HIGH);
	if (IS_ERR_VALUE(ret))
		return ret;
	mdelay(1);
	ret = gpio_set_value(ts_gpio, GPIO_LOW);
	if (IS_ERR_VALUE(ret))
		return ret;

	/** Start time stamp calibration */
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC32K,
				&reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;
	reg_data |= BITM_OSC32K_CAPTURE_TIMESTAMP;
	ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_OSC32K,
				 reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;

	mdelay(10);

	ret = gpio_set_value(ts_gpio, GPIO_HIGH);
	if (IS_ERR_VALUE(ret))
		return ret;

	mdelay(1);

	ret = gpio_set_value(ts_gpio, GPIO_LOW);
	if (IS_ERR_VALUE(ret))
		return ret;

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC32K,
				&reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;

	if(reg_data & BITM_OSC32K_CAPTURE_TIMESTAMP)
		return ret;

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_STAMP_H,
				&reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;

	*ts_val = (reg_data << 16) & 0xFFFF0000;

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_STAMP_L,
				&reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;

	*ts_val |= reg_data;

	return SUCCESS;
}

/**
 * @brief Do low frequency oscillator calibration with respect to an external
 *        reference.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
static int32_t cn0567_calibrate_lfo(struct cn0567_dev *dev)
{
	int32_t ret;
	uint32_t ts_val_current, ts_val_last = 0, ts_val;
	uint16_t reg_data, cal_value;
	int8_t rdy = 0;
	struct gpio_desc *ts_gpio;
	struct gpio_init_param ts_param;

	ret = cn0567_calibrate_lfo_set_ts(dev);
	if (IS_ERR_VALUE(ret))
		return ret;

	/** Setup platform GPIO for time stamp trigger */
	ts_param.number = 15;
	ts_param.platform_ops = &aducm_gpio_ops;
	ts_param.extra = NULL;

	ret = gpio_get(&ts_gpio, &ts_param);
	if (IS_ERR_VALUE(ret))
		return ret;

	ret = gpio_direction_output(ts_gpio, GPIO_LOW);
	if (IS_ERR_VALUE(ret))
		return ret;

	/** Delay to correctly initialize GPIO circuitry in device. */
	mdelay(1);

	while (1) {
		ret = cn0567_calibrate_lfo_get_timestamp(dev, &ts_val_current,
				ts_gpio);
		if (IS_ERR_VALUE(ret))
			return ret;

		if(ts_val_current < ts_val_last) {
			ts_val_last = 0;
			continue;
		}
		ts_val = ts_val_current - ts_val_last;
		ts_val_last = ts_val_current;

		ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
					&reg_data);
		if (IS_ERR_VALUE(ret))
			return ret;

		cal_value = reg_data & BITM_OSC1M_OSC_1M_FREQ_ADJ;
		if(ts_val < (10000 - (10000 * 0.005)))
			cal_value++;
		else if(ts_val > (10000 + (10000 * 0.005)))
			cal_value--;
		else
			rdy = 1;
		if(rdy == 1)
			break;
		if((cal_value == 0) || (cal_value == BITM_OSC1M_OSC_1M_FREQ_ADJ))
			break;

		reg_data &= ~BITM_OSC1M_OSC_1M_FREQ_ADJ;
		reg_data |= cal_value & BITM_OSC1M_OSC_1M_FREQ_ADJ;

		ret = adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
					 reg_data);
		if (IS_ERR_VALUE(ret))
			return ret;
	};

	ret = gpio_remove(ts_gpio);
	if (IS_ERR_VALUE(ret))
		return ret;

	if(rdy == 1)
		return SUCCESS;
	else
		return ret;
}

/**
 * @brief Do high frequency oscillator calibration with respect to the low
 *        frequency oscillator.
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0567_calibrate_hfo(struct cn0567_dev *dev)
{
	int32_t ret;
	uint16_t reg_data;

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC32M_CAL,
				&reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;

	reg_data |= BITM_OSC32M_CAL_OSC_32M_CAL_START;

	do {
		ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC32M_CAL,
					&reg_data);
		if (IS_ERR_VALUE(ret))
			return ret;
	} while(reg_data & BITM_OSC32M_CAL_OSC_32M_CAL_START);

	/** Disable clock calibration circuitry */
	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
				&reg_data);
	if (IS_ERR_VALUE(ret))
		return ret;

	reg_data &= ~BITM_OSC1M_OSC_CLK_CAL_ENA;

	return adpd410x_reg_write(dev->adpd4100_handler, ADPD410X_REG_OSC1M,
				  reg_data);
}

/**
 * @brief Initial process of the application.
 * @param [out] device - Pointer to the application handler.
 * @param [in] init_param - Pointer to the application initialization structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0567_init(struct cn0567_dev **device)
{
	int32_t ret;
	struct cn0567_dev *dev;
	int8_t i;
	uint16_t data;

	dev = (struct cn0567_dev *)calloc(1, sizeof *dev);
	if(!dev)
		return FAILURE;

	ret = adpd410x_setup(&dev->adpd4100_handler,
			     &adpd4100_param);
	if (IS_ERR_VALUE(ret))
		goto error_cn;

	ret = adpd410x_reg_read(dev->adpd4100_handler, ADPD410X_REG_CHIP_ID,
				&dev->chip_id);
	if (IS_ERR_VALUE(ret))
		goto error_cn;

	ret = adpd410x_set_sampling_freq(dev->adpd4100_handler,
					 CN0567_CODE_ODR_DEFAULT);
	if (IS_ERR_VALUE(ret))
		goto error_cn;

	ret = adpd410x_set_last_timeslot(dev->adpd4100_handler,
					 ADPD410X_ACTIVE_TIMESLOTS - 1);
	if (IS_ERR_VALUE(ret))
		goto error_cn;

	for (i = 0; i < ADPD410X_ACTIVE_TIMESLOTS; i++) {
		ret = adpd410x_timeslot_setup(dev->adpd4100_handler, i,
					      ts_init_tab + i);
		if (IS_ERR_VALUE(ret))
			goto error_cn;

		/** Precondition VC1 and VC2 to TIA_VREF+250mV */
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_CATHODE(i), &data);
		if (IS_ERR_VALUE(ret))
			goto error_cn;
		data &= ~(BITM_CATHODE_A_VC2_SEL | BITM_CATHODE_A_VC1_SEL);
		data |= (2 << BITP_CATHODE_A_VC2_SEL) & BITM_CATHODE_A_VC2_SEL;
		data |= (2 << BITP_CATHODE_A_VC1_SEL) & BITM_CATHODE_A_VC1_SEL;
		ret = adpd410x_reg_write(dev->adpd4100_handler,
					 ADPD410X_REG_CATHODE(i), data);
		if (IS_ERR_VALUE(ret))
			goto error_cn;

		/** Set the two channels trim option */
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_AFE_TRIM(i), &data);
		if (IS_ERR_VALUE(ret))
			goto error_cn;
		data |= (1 << BITP_AFE_TRIM_A_CH1_TRIM_INT |
			 1 << BITP_AFE_TRIM_A_CH2_TRIM_INT);
		ret = adpd410x_reg_write(dev->adpd4100_handler,
					 ADPD410X_REG_AFE_TRIM(i), data);
		if (IS_ERR_VALUE(ret))
			goto error_cn;

		/**
		 * Set to ~32us integrator offset to line up zero crossing of
		 * BPF
		 */
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_INTEG_OFFSET(i),
					&data);
		if (IS_ERR_VALUE(ret))
			goto error_cn;
		data = 0x03FC & BITM_INTEG_OFFSET_A_INTEG_OFFSET;
		ret = adpd410x_reg_write(dev->adpd4100_handler,
					 ADPD410X_REG_INTEG_OFFSET(i), data);
		if (IS_ERR_VALUE(ret))
			goto error_cn;

		/** Set to ~32us LED offset */
		ret = adpd410x_reg_read(dev->adpd4100_handler,
					ADPD410X_REG_LED_PULSE(i), &data);
		if (IS_ERR_VALUE(ret))
			goto error_cn;
		data = 0x0220;
		ret = adpd410x_reg_write(dev->adpd4100_handler,
					 ADPD410X_REG_LED_PULSE(i), data);
		if (IS_ERR_VALUE(ret))
			goto error_cn;
	}

	ret = cn0567_calibrate_lfo(dev);
	if (IS_ERR_VALUE(ret))
		goto error_cn;

	ret = cn0567_calibrate_hfo(dev);
	if (IS_ERR_VALUE(ret))
		goto error_cn;

	for (i = 0; i < 47; i++) {
		ret = adpd410x_reg_write(dev->adpd4100_handler, reg_config_default[i][0],
					 reg_config_default[i][1]);
		if (IS_ERR_VALUE(ret))
			return ret;
	}

	*device = dev;

	return ret;

error_cn:
	free(dev);

	return FAILURE;
}

/**
 * @brief Free memory allocated by cn0567_init().
 * @param [in] dev - The device structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t cn0567_remove(struct cn0567_dev *dev)
{
	int32_t ret;

	if(!dev)
		return FAILURE;

	ret = adpd410x_remove(dev->adpd4100_handler);
	if (IS_ERR_VALUE(ret))
		return ret;

	free(dev);

	return SUCCESS;
}
