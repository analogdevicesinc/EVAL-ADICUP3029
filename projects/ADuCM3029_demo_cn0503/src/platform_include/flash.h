/*
 * flash.h
 *
 *  Created on: Feb 18, 2020
 *      Author: adrimbar
 */

#ifndef PLATFORM_INCLUDE_FLASH_H_
#define PLATFORM_INCLUDE_FLASH_H_

#include <stdint.h>

//todo: comment flash_dev
struct flash_dev {
	uint8_t id;
	void *extra;
};

//todo: comment flash_init_param
struct flash_init_param {
	uint8_t id;
};

/** Initialize flash controller. */
int32_t flash_init(struct flash_dev **device,
		   struct flash_init_param *init_param);

/** Free memory allocated by flash_init(). */
int32_t flash_remove(struct flash_dev *dev);

/** Write data in flash memory. */
int32_t flash_write(struct flash_dev *dev, uint32_t flash_addr,
		    uint32_t *array, uint32_t array_size);

/** Read data from the flash memory. */
void flash_read(struct flash_dev *dev, uint32_t flash_addr, uint32_t *array,
		uint32_t size);

#endif /* PLATFORM_INCLUDE_FLASH_H_ */
