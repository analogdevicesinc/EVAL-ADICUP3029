/*!
********************************************************************************
* @file:        adsap_intf_api.c
* @brief        Contains sensor data read/write functions for sensors.
*               These functions are used when sensor loadable objects are not
*               available..
* @version:     1.00 $Revision: 0116 $
* @date:        $Date:
*-------------------------------------------------------------------------------

Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated Analog Devices
Software License Agreement.

*******************************************************************************/
//#define ADI_ADXL362_DEMO
#define ADI_ADXL372_DEMO

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <drivers/i2c/adi_i2c.h>
#include <drivers/spi/adi_spi.h>
#include <drivers/pwr/adi_pwr.h>
#include "adsap_def.h"
#include "adsap_proto.h"

extern ADI_I2C_HANDLE	hI2CMasterDev;
extern ADI_SPI_HANDLE 	hSPI0MasterDev;
extern ADI_SPI_HANDLE 	hSPI1MasterDev;
extern ADI_SPI_HANDLE 	hSPI2MasterDev;


ADI_SPI_RESULT adsAPI_Config_SPI(uint8_t SPI_Dev_Num, const uint32_t sclk, const ADI_SPI_CHIP_SELECT eChipSelect, const bool iCPOL, const bool iCPHA)
{
	ADI_SPI_HANDLE hDevice;
  	switch (SPI_Dev_Num)
	{
		case 0:
		  hDevice = hSPI0MasterDev;
		break;
		case 1:
		  hDevice = hSPI1MasterDev;
		break;
		case 2:
		  hDevice = hSPI2MasterDev;
		break;
		default:
		break;
	}
  	/* Set the SPI clock rate */
    if(adi_spi_SetBitrate(hDevice, sclk) != ADI_SPI_SUCCESS)
    {
        return ADI_SPI_FAILURE;
    }

    /* Set the chip select */
    if(adi_spi_SetChipSelect(hDevice, eChipSelect) != ADI_SPI_SUCCESS)
    {
        return ADI_SPI_FAILURE;
    }

	/* Set the SPI clock polarity */
    if(adi_spi_SetClockPolarity(hDevice, iCPOL) != ADI_SPI_SUCCESS)
    {
        return ADI_SPI_FAILURE;
    }

    /* Set the SPI clock phase */
    if(adi_spi_SetClockPhase(hDevice, iCPHA) != ADI_SPI_SUCCESS)
    {
        return ADI_SPI_FAILURE;
    }

    adi_spi_SetContinuousMode(hDevice, true);
    adi_spi_SetIrqmode(hDevice, true);

    return ADI_SPI_SUCCESS;
}

#ifdef ADI_ADXL372_DEMO
ADSENSORAPP_RESULT_TYPE adsAPI_RW_SPI_Sensor_Reg(REG_RW_MODE rw_mode, uint8_t SPI_Dev_Num, uint8_t cmd, uint8_t *RegAddr, uint8_t *RegData, uint32_t No_of_Bytes)
{
    ADI_SPI_RESULT eResult = ADI_SPI_SUCCESS;  /* assume the best */
    ADI_SPI_TRANSCEIVER  transceive;
	ADI_SPI_HANDLE hDevice;

  	switch (SPI_Dev_Num)
	{
		case 0:
		  	hDevice = hSPI0MasterDev;
		break;
		case 1:
		  	hDevice = hSPI1MasterDev;
		break;
		case 2:
		  	hDevice = hSPI2MasterDev;
		break;
		default:
			return ADI_ADS_API_FAIL;
		break;
	}

	if (rw_mode == REG_READ)
	{
		uint8_t Tx_Buf[2];
		if(cmd == 0x00)
		{
			Tx_Buf[0] = *RegAddr;
		}
		else
		{
			Tx_Buf[0] = cmd;
			Tx_Buf[1] = *RegAddr;
		}
		/* initialize data attributes */
	    transceive.pTransmitter = &Tx_Buf[0];
	    transceive.pReceiver = RegData;

	    /* link transceive data size to the remaining count */
	    transceive.TransmitterBytes = No_of_Bytes;
		transceive.ReceiverBytes = No_of_Bytes;

	    /* auto increment both buffers */
	    transceive.nTxIncrement = true;	//false
	    transceive.nRxIncrement = true;

	    transceive.bDMA = false;
	    transceive.bRD_CTL = false;
	}
	else
	{
		uint8_t Tx_Buf[7];
		uint8_t ui8Count = 0;
		if(cmd == 0x00)
		{
			for (ui8Count = 0; ui8Count < No_of_Bytes; ui8Count++)
			{
				Tx_Buf[ui8Count] = *(RegAddr + ui8Count);
			}
		}
		else
		{
			Tx_Buf[0] = cmd;
			for (ui8Count = 0; ui8Count < No_of_Bytes; ui8Count++)
			{
				Tx_Buf[ui8Count + 1] = *(RegAddr + ui8Count);
			}
		}
		/* initialize data attributes */
	    transceive.pTransmitter = &Tx_Buf[0];
	    transceive.pReceiver = NULL;

	    /* link transceive data size to the remaining count */
	    transceive.TransmitterBytes = No_of_Bytes;
		transceive.ReceiverBytes = 0u;

	    /* auto increment both buffers */
	    transceive.nTxIncrement = true;
	    transceive.nRxIncrement = false;

	    transceive.bDMA = false;
	    transceive.bRD_CTL = false;
	}

//    if(adi_spi_SetContinousMode(hDevice, true) != ADI_SPI_SUCCESS)
//    {
//         return ADI_ADS_API_FAIL;		//Check this later
//    }

#ifdef SPI_NONBLOCKING_MODE
	bool bComplete = 0;

	eResult = adi_spi_MasterTransfer(hDevice, &transceive);

	// poll on non-blocking tests...
	#ifdef DMA_ENABLE
        if (adi_SPI_GetDmaMode(hDevice))
        {
       		while (!adi_SPI_GetDmaTxComplete(hDevice) || !adi_SPI_GetDmaRxComplete(hDevice)) ;      // DMA polling loop...
       	}
        else
	#endif
        {
			while (!bComplete)
			  eResult = adi_spi_MasterComplete(hDevice, &bComplete);
       	}
#else
//	eResult = adi_spi_ReadWrite(hDevice, &transceive);						//Blocking mode call
        eResult=  adi_spi_MasterReadWrite(hDevice, &transceive);
#endif
    if (eResult == ADI_SPI_SUCCESS)
      return ADI_ADS_API_SUCCESS;
    else
      return ADI_ADS_API_FAIL;
}
#endif //ADI_ADXL372_DEMO

#ifdef ADI_ADXL362_DEMO

ADSENSORAPP_RESULT_TYPE adsAPI_RW_SPI_Sensor_Reg(REG_RW_MODE rw_mode, uint8_t SPI_Dev_Num, uint8_t cmd, uint8_t *RegAddr, uint8_t *RegData, uint32_t No_of_Bytes)
{
    ADI_SPI_RESULT eResult = ADI_SPI_SUCCESS;  /* assume the best */
    ADI_SPI_TRANSCEIVER  transceive;
	ADI_SPI_HANDLE hDevice;

  	switch (SPI_Dev_Num)
	{
		case 0:
		  	hDevice = hSPI0MasterDev;
		break;
		case 1:
		  	hDevice = hSPI1MasterDev;
		break;
		case 2:
		  	hDevice = hSPI2MasterDev;
		break;
		default:
			return ADI_ADS_API_FAIL;
		break;
	}

	if (rw_mode == REG_READ)
	{
		uint8_t Tx_Buf[2];

		Tx_Buf[0] = cmd;
		Tx_Buf[1] = *RegAddr;
		/* initialize data attributes */
	    transceive.pTransmitter = &Tx_Buf[0];
	    transceive.pReceiver = RegData;

	    /* link transceive data size to the remaining count */
	    transceive.TransmitterBytes = 2;
		transceive.ReceiverBytes = No_of_Bytes;

	    /* auto increment both buffers */
	    transceive.nTxIncrement = true;	//false
	    transceive.nRxIncrement = true;

	    transceive.bDMA = false;
	    transceive.bRD_CTL = true;
	}
	else
	{
		uint8_t Tx_Buf[7];
		uint8_t ui8Count = 0;

		Tx_Buf[0] = cmd;
		Tx_Buf[1] = RegAddr[0];
		Tx_Buf[2] = RegAddr[1];

		/* initialize data attributes */
	    transceive.pTransmitter = &Tx_Buf[0];
	    transceive.pReceiver = NULL;

	    /* link transceive data size to the remaining count */
	    transceive.TransmitterBytes = No_of_Bytes;
		transceive.ReceiverBytes = 0u;

	    /* auto increment both buffers */
	    transceive.nTxIncrement = true;
	    transceive.nRxIncrement = false;

	    transceive.bDMA = false;
	    transceive.bRD_CTL = false;
	}


	eResult=  adi_spi_MasterReadWrite(hDevice, &transceive);

    if (eResult == ADI_SPI_SUCCESS)
      return ADI_ADS_API_SUCCESS;
    else
      return ADI_ADS_API_FAIL;
}

#endif //ADI_ADXL362_DEMO

