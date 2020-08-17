/*
 * flash_extra.h
 *
 *  Created on: Feb 18, 2020
 *      Author: adrimbar
 */

#ifndef PLATFORM_INCLUDE_FLASH_EXTRA_H_
#define PLATFORM_INCLUDE_FLASH_EXTRA_H_

#include <stdint.h>

#define FLASH_PAGE_SIZE_BYTES 2048
#define FLASH_PAGE_SIZE_WORDS 512

//todo: comment adicup_flash_dev
struct adicup_flash_dev {
	void *instance;
	void *dfp_memory;
};

//todo: comment adicup_flash_init_param
struct adicup_flash_init_param {

};

#endif /* PLATFORM_INCLUDE_FLASH_EXTRA_H_ */
