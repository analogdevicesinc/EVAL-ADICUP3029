/*
 * i2c_extra.h
 *
 *  Created on: Jan 8, 2020
 *      Author: adrimbar
 */

#ifndef PLATFORM_INCLUDE_I2C_EXTRA_H_
#define PLATFORM_INCLUDE_I2C_EXTRA_H_

struct aducm_i2c_init_param {
	uint32_t id;
};

struct aducm_i2c_desc {
	void *instance;
	void *memory;
	uint32_t id;
};

#endif /* PLATFORM_INCLUDE_I2C_EXTRA_H_ */
