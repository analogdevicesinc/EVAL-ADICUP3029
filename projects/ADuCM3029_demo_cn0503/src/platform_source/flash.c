/*
 * flash.c
 *
 *  Created on: Feb 18, 2020
 *      Author: adrimbar
 */

#include <drivers/flash/adi_flash.h>
#include <drivers/dma/adi_dma.h>
#include <stdlib.h>
#include <malloc.h>
#include "flash.h"
#include "flash_extra.h"
#include "error.h"

/**
 * Initialize flash controller.
 *
 * @param [out] device     - Pointer to the driver handler.
 * @param [in]  init_param - Pointer to the initialization structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
int32_t flash_init(struct flash_dev **device,
		   struct flash_init_param *init_param)
{
	int32_t ret;
	struct flash_dev *dev;
	struct adicup_flash_dev *adicup_extra;

	dev = (struct flash_dev *)calloc(1, sizeof(*dev));
	if (!dev)
		return FAILURE;
	dev->id = init_param->id;
	adicup_extra = (struct adicup_flash_dev *)calloc(1,
			sizeof(*adicup_extra));
	if (!adicup_extra)
		goto error_dev;
	dev->extra = adicup_extra;
	adicup_extra->dfp_memory = calloc(1, ADI_FEE_MEMORY_SIZE);
	if (!adicup_extra->dfp_memory)
		goto error_adicup;

	/* Open flash device */
	ret = adi_fee_Open(dev->id, adicup_extra->dfp_memory,
			   ADI_FEE_MEMORY_SIZE,
			   (ADI_FEE_HANDLE *)&adicup_extra->instance);
	if(ret != SUCCESS)
		goto error_dfp_mem;

	*device = dev;

	return SUCCESS;

error_dfp_mem:
	free(adicup_extra->dfp_memory);
error_adicup:
	free(adicup_extra);
error_dev:
	free(dev);

	return FAILURE;
}

/**
 * Free memory allocated by flash_init().
 *
 * @param [in] dev - Pointer to the driver handler.
 *
 * @return 0 in case of success, error code otherwise.
 */
int32_t flash_remove(struct flash_dev *dev)
{
	int32_t ret;
	struct adicup_flash_dev *adicup_extra;

	if (!dev)
		return FAILURE;

	adicup_extra = dev->extra;

	ret = adi_fee_Close(adicup_extra->instance);
	if (ret != SUCCESS)
		return FAILURE;

	free(adicup_extra->dfp_memory);
	free(adicup_extra);
	free(dev);

	return SUCCESS;
}

//todo: comment flash_write_page
static int32_t flash_write_page(struct flash_dev *dev, uint32_t flash_addr,
				uint32_t *array, uint32_t array_size)
{
	ADI_FEE_TRANSACTION transaction;
	struct adicup_flash_dev *adicup_extra = dev->extra;
	uint32_t page_nr, fee_hw_error;
	uint32_t temp_ptr[FLASH_PAGE_SIZE_WORDS] __attribute__ ((aligned (4)));
	int32_t ret, i;

	if ((flash_addr & 0x7) != 0)
		return FAILURE;
	if ((array_size % 2) != 0)
		return FAILURE;
	if (((flash_addr & 0x7FF) + (array_size * sizeof(uint32_t))) >
	    0x800)
		return FAILURE;

	flash_read(dev, (flash_addr & (~0x7FF)), temp_ptr,
		   FLASH_PAGE_SIZE_WORDS);

	for(i = 0; i < array_size; i++)
		temp_ptr[(i + (flash_addr & 0x7FF) / 4)] = array[i];

	transaction.bUseDma = true;
	transaction.nSize = FLASH_PAGE_SIZE_BYTES;
	transaction.pWriteAddr = (uint32_t *)(flash_addr & (~0x7FF));
	transaction.pWriteData = temp_ptr;

	/* Get the page number */
	ret = adi_fee_GetPageNumber(adicup_extra->instance, flash_addr,
				    &page_nr);
	if(ret != SUCCESS)
		return FAILURE;

	/* First erase page */
	ret = adi_fee_PageErase(adicup_extra->instance, page_nr, page_nr,
				&fee_hw_error);
	if(ret != SUCCESS)
		return FAILURE;

	/* Then write */
	ret = adi_fee_Write(adicup_extra->instance, &transaction,
			    &fee_hw_error);
	if(ret != SUCCESS)
		return FAILURE;

	return SUCCESS;
}

//todo: comment flash_write
int32_t flash_write(struct flash_dev *dev, uint32_t flash_addr,
		    uint32_t *array, uint32_t array_size)
{
	uint32_t data_index = 0, temp, initial_size = array_size;
	int32_t ret;

	do {
		temp = (FLASH_PAGE_SIZE_BYTES - (flash_addr & 0x7FF)) / 4;
		if (temp > array_size)
			temp = array_size;
		ret = flash_write_page(dev, flash_addr, (array + data_index),
				       temp);
		if (ret != SUCCESS)
			return FAILURE;
		flash_addr += temp;
		array_size -= temp;
		data_index += temp;
	} while (data_index < initial_size);

	return SUCCESS;
}

///**
// * Write data in flash memory.
// *
// * @param [in] array      - Pointer to the data to be written.
// * @param [in] array_size - Size of the written data.
// *
// * @return 0 in case of success, negative error code otherwise.
// */
//int32_t flash_write(struct flash_dev *dev, uint32_t flash_addr,
//		uint32_t *array, uint32_t array_size)
//{
//	ADI_FEE_TRANSACTION transaction;
//	uint32_t i, start_page, end_page, fee_hw_error;
//	int32_t ret;
//	struct adicup_flash_dev *adicup_extra = dev->extra;
//
//	if ((flash_addr & 0xFFFF) != 0)
//		return FAILURE;
//	if ((array_size % 2) != 0)
//		return FAILURE;
//
//	adicup_extra->write_mem = (uint32_t *)aligned_alloc(sizeof(uint32_t),
//			array_size * sizeof(*array));
//	if (!adicup_extra->write_mem)
//		return FAILURE;
//
//	transaction.bUseDma = true;
//	transaction.nSize = array_size * sizeof(*array);
//	transaction.pWriteAddr = flash_addr;
//	transaction.pWriteData = adicup_extra->write_mem;
//
//	for(i = 0; i < array_size; i++)
//		adicup_extra->write_mem[i] = array[i];
//
//	/* Get the start page */
//	ret = adi_fee_GetPageNumber(adicup_extra->instance, flash_addr,
//			&start_page);
//	if(ret != ADI_FEE_SUCCESS)
//		goto error;
//
//	/* Get the end page */
//	ret = adi_fee_GetPageNumber(adicup_extra->instance,
//				    (flash_addr + array_size - 1), &end_page);
//	if(ret != ADI_FEE_SUCCESS)
//		goto error;
//
//	/* First erase page */
//	ret = adi_fee_PageErase(adicup_extra->instance, start_page, end_page,
//				&fee_hw_error);
//	if(ret != SUCCESS)
//		goto error;
//
//	/* Then write */
//	ret = adi_fee_Write(adicup_extra->instance, &transaction,
//			&fee_hw_error);
//	if(ret != SUCCESS)
//		goto error;
//
//error:
//	free(adicup_extra->write_mem);
//
//	return ret;
//}

/**
 * Read data from the flash memory.
 *
 * @param [out] array - Pointer to the container for the read data.
 * @param [in]  size  - Size of the read data.
 *
 * @return void
 */
void flash_read(struct flash_dev *dev, uint32_t flash_addr, uint32_t *array,
		uint32_t size)
{
	uint32_t *data_in_flash = (uint32_t *)flash_addr;
	uint32_t *data		    = array;

	for (uint32_t i = 0; i < size; i++)
		*data++ = *data_in_flash++;
}
