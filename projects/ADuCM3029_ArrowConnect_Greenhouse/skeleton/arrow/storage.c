/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <arrow/storage.h>
#include <arrow/utf8.h>
#include <drivers/flash/adi_flash.h>
#include <stdio.h>
#include <string.h>

#define FLASH_MAGIC_NUMBER 0xdeadbeaf

uint8_t flash_memory[ADI_FEE_MEMORY_SIZE];
ADI_FEE_HANDLE   fee_handle;
ADI_FEE_TRANSACTION  fee_transaction;
uint32_t HwErrors;

/* Flash controller  Return code */
ADI_FEE_RESULT eResult = ADI_FEE_SUCCESS;

uint32_t nStartPage, nEndPage, nBlockNum;

/* Page Address to write the value */
#define PAGE_ADDR                (0x20000u)

/* Buffer Size */
#define BUFF_SIZE                (2048u)

flash_mem_t mem;

int init_flash(void)
{
	 ADI_FEE_RESULT eResult = ADI_FEE_SUCCESS;

	eResult = adi_fee_Open (0, flash_memory, ADI_FEE_MEMORY_SIZE, &fee_handle);

	/* Get the start page */
	eResult = adi_fee_GetPageNumber(fee_handle, PAGE_ADDR, &nStartPage);

	/* Get the start page */
	eResult = adi_fee_GetPageNumber(fee_handle, (PAGE_ADDR + BUFF_SIZE - 1), &nEndPage);

	adi_fee_GetBlockNumber (fee_handle, PAGE_ADDR, &nBlockNum);

	memset(&mem, 0, sizeof(mem));

	return 0;
}


int read_from_flash(uint32_t *pArray, uint32_t ulStartAddress, uint32_t u32Size)
{
	uint8_t* pDataInFlash = (uint8_t*)ulStartAddress;
	uint8_t * data = (uint8_t *)pArray;

	for (uint32_t i = 0; i < u32Size; i++)
	{
		*data++ = *pDataInFlash++;

	}

	return 0;

}

int restore_gateway_info(arrow_gateway_t *gateway) {


//	read_from_flash((uint32_t *)&mem, PAGE_ADDR, sizeof(mem));
//
//	if (mem.magic != FLASH_MAGIC_NUMBER)
//	{
//		return -1;
//	}
//	gateway->hid.value = mem.gateway_hid;
	return -1;
}

void save_gateway_info(const arrow_gateway_t *gateway) {

//	/* Clear the Pages */
//	adi_fee_PageErase (fee_handle, nStartPage, nEndPage, &HwErrors);
//
//	mem.magic = FLASH_MAGIC_NUMBER;
//	memcpy(mem.gateway_hid, P_VALUE(gateway->hid), P_SIZE(gateway->hid)+1);
//
//	fee_transaction.bUseDma = false;
//	fee_transaction.pWriteAddr = (uint32_t *)PAGE_ADDR;
//	fee_transaction.pWriteData = (uint32_t *)&mem;
//	fee_transaction.nSize = sizeof(mem);
//
//	eResult =  adi_fee_Write (fee_handle, &fee_transaction, &HwErrors);
//
//	mem.magic = 0;
}

int restore_device_info(arrow_device_t *device) {

	//read_from_flash((uint32_t *)&mem, PAGE_ADDR, sizeof(mem));

//	if (mem.magic != FLASH_MAGIC_NUMBER)
//	{
//		return -1;
//	}
//
//	P_VALUE(device->hid) = mem.device_hid;

	return -1;
}

void save_device_info(arrow_device_t *device) {

//	/* Clear the Pages */
//	adi_fee_PageErase (fee_handle, nStartPage, nEndPage, &HwErrors);
//
//	memcpy(mem.device_hid, P_VALUE(device->hid), P_SIZE(device->hid)+1);
//
//	fee_transaction.bUseDma = false;
//	fee_transaction.pWriteAddr = (uint32_t *)PAGE_ADDR;
//	fee_transaction.pWriteData = (uint32_t *)&mem;
//	fee_transaction.nSize = sizeof(mem);
//	eResult = adi_fee_Write (fee_handle, &fee_transaction, &HwErrors);
//
//	mem.magic = 0;

}

void save_wifi_setting(const char *ssid, const char *pass, int sec) {

//	fee_transaction.bUseDma = false;
//	fee_transaction.pWriteAddr = 0x200;
//	fee_transaction.pWriteData = device;
//	fee_transaction.nSize = sizeof(*device);
//	adi_fee_Write (fee_handle, fee_transaction, &HwErrors);
}

int restore_wifi_setting(char *ssid, char *pass, int *sec) {
    // FIXME restore this wifi settings
    return -1;
}
