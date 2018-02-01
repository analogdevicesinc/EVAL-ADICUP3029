/*!
********************************************************************************
* @file:        adsap_main.c
* @brief        The starting point for application. This will be the main
*               application when used with Radio modules.
* @version:     1.00 $Revision: 0116 $
* @date:        $Date:
*-------------------------------------------------------------------------------

Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated Analog Devices
Software License Agreement.

*******************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <drivers/i2c/adi_i2c.h>
#include <drivers/spi/adi_spi.h>
#include <drivers/pwr/adi_pwr.h>
#include <drivers/gpio/adi_gpio.h>

#include "common.h"
#include "adsap_def.h"
#include "adsap_proto.h"


#ifdef STANDALONE_APP
#define USE_PERIODIC_READING
#include <services/tmr/adi_tmr.h>
#ifdef USE_PERIODIC_READING
	//GPT0 Driver related
	#define TIMER_DEVICE_ID		0
	static uint8_t GPT0_Memory[ADI_TMR_MEMORY_SIZE];
	#define PRELOAD_VALUE     2048	//(6144u) //	For 3sec
	volatile uint8_t GPT0_Timeout = 0;
	uint32_t GPT0_cnt=0, time_elapsed=0;
	ADI_TMR_RESULT GPT0_Init();
#endif
#endif

struct Sens_Data SensData, SensData_temp;
struct adsap_sensors adsap_sensor_status;
extern ADI_SPI_HANDLE get_spi_handle();

uint8_t I2C_DevMem[ADI_I2C_MEMORY_SIZE];	/* Memory required for I2C driver */
uint8_t SPI0_DevMem[ADI_SPI_MEMORY_SIZE];	/* Memory required for SPI0 driver - Clump Sensors*/
//uint8_t SPI1_DevMem[ADI_SPI_MEMORY_SIZE];	/* Memory required for SPI0 driver - Clump Sensors*/
//uint8_t SPI2_DevMem[ADI_SPI_MEMORY_SIZE];	/* Memory required for SPI driver - EZKIT Sensors & Transceiver*/

/* SPI device handles */
ADI_SPI_HANDLE	hSPI0MasterDev;
ADI_SPI_HANDLE	hSPI1MasterDev;
ADI_SPI_HANDLE	hSPI2MasterDev;

/* I2C device handle */
ADI_I2C_HANDLE 	hI2CMasterDev;


#ifdef STANDALONE_APP		//This functions is main() for standalone app
int main(void)
#else
void adsAPI_ADSensInit()
#endif
{
	int result;

#ifdef STANDALONE_APP
    ADSENSORAPP_RESULT_TYPE result = ADI_ADS_API_SUCCESS;
    SystemInit();    		/* Clock initialization */
    adi_initpinmux();	    /* Pin Mux */
    test_Init();		    /* test system initialization */

    do
    {
        if(adi_pwr_Init()!= ADI_PWR_SUCCESS)
        {
            DEBUG_MESSAGE("\n Failed to intialize the power service \n");
            break;
        }

        if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_HCLK,1))
        {
            DEBUG_MESSAGE("Failed to set clock divider for HCLK\n");
            break;
        }

        if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_PCLK,1))
        {
            DEBUG_MESSAGE("Failed to set clock divider for PCLK\n");
            break;
        }
    }while(0);
#endif

    adsAPI_Init_Devices();	// Initializes the peripheral drivers used by the sensors

	result = Detect_ADXL372_Sensor();
	if(ADI_ADS_API_SUCCESS == result)
	{
		result = ADXL372_Init(); // This starts the sensor config process.
	}

#ifdef STANDALONE_APP
#ifdef USE_PERIODIC_READING
	if(ADI_TMR_SUCCESS != GPT0_Init())
	{
		DEBUG_MESSAGE("Failed to set GPT0 \n");
	}
#endif
    while(1)
    {
	  	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
#ifdef USE_PERIODIC_READING
	if (GPT0_Timeout)
#endif
	{
            if(ADI_ADS_API_SUCCESS == status)
            {
			#ifdef FIFO_READ
				status = ADXL372_Read_FIFO_Data();
			#else
			  	status = Get_Data_From_ADXL372();    //Reads data from all ADXL372 sensor and palces data in a global structure.
				Print_SensData_on_UART(status);
			#endif
            }
            ADSensorAppAdvOps();
			status = ADI_ADS_API_SUCCESS;
	  }
	}
#endif
}

void ADSensorAppAdvOps()
{
#ifdef USE_PERIODIC_READING
    GPT0_Timeout = 0;
	time_elapsed++;
#endif
}



/* Initializes SPI device */
static ADI_SPI_RESULT adsAPI_Init_SPI_Port(void)
{
    if(adi_spi_Open(SPI0_DEV_NUM,
                    &SPI0_DevMem[0],
                    ADI_SPI_MEMORY_SIZE,
                    &hSPI0MasterDev
                   ) != ADI_SPI_SUCCESS)
    {
		return ADI_SPI_FAILURE;
    }

    /* Enable underflow errors */
    if(adi_spi_SetTransmitUnderflow(hSPI0MasterDev, true) != ADI_SPI_SUCCESS)
    {
        return ADI_SPI_FAILURE;
    }

    /* Set the SPI clock rate */
    if(adi_spi_SetBitrate(hSPI0MasterDev, SPI2_CLK) != ADI_SPI_SUCCESS)
    {
        return ADI_SPI_FAILURE;
    }

    /* Set the chip select */
    if(adi_spi_SetChipSelect(hSPI0MasterDev, SPI2_CS1) != ADI_SPI_SUCCESS)
    {
        return ADI_SPI_FAILURE;
    }

	/* Set the SPI clock polarity */
    if(adi_spi_SetClockPolarity(hSPI0MasterDev, false) != ADI_SPI_SUCCESS)
    {
        return ADI_SPI_FAILURE;
    }

    /* Set the SPI clock phase */
    if(adi_spi_SetClockPhase(hSPI0MasterDev, false) != ADI_SPI_SUCCESS)
    {
        return ADI_SPI_FAILURE;
    }


    /* Set master mode */
    if(adi_spi_SetMasterMode(hSPI0MasterDev, true) != ADI_SPI_SUCCESS)
    {
        return ADI_SPI_FAILURE;
    }
    return ADI_SPI_SUCCESS;
}

static ADI_SPI_RESULT adsAPI_UnInit_SPI2_Port()
{
  	ADI_SPI_RESULT eResultSPI = adi_spi_Close(hSPI2MasterDev);
	return eResultSPI;
}

/*!
* @brief        Initializes ADuCM3029 peripherals
* @param[in]    NULL
* @param[out]   NULL
* @return       NULL
* @note         Master API to initialize the peripherals and other system resources
*/
void adsAPI_Init_Devices()
{
//    if(adsAPI_Init_I2C_Port() != ADI_I2C_SUCCESS)                           	// I2C device initialization
//        DEBUG_MESSAGE("I2C Init failed\n");

	if (adsAPI_Init_SPI_Port()!= ADI_SPI_SUCCESS)                           // SPI2 device initialization
        DEBUG_MESSAGE("SPI Init failed\n");
}

/*!
* @brief        Uninitializes ADuCM3029 peripherals
* @param[in]    NULL
* @param[out]   NULL
* @return       NULL
* @note         Master API to uninitialize the peripherals and other system resources
*/
void adsAPI_UnInit_Devices()
{
//    if(adsAPI_Uninit_I2C_Port() != ADI_I2C_SUCCESS)                           	// I2C device uninitialization
//        DEBUG_MESSAGE("I2C UnInit failed\n");

    if(adsAPI_UnInit_SPI2_Port()!= ADI_SPI_SUCCESS)                           	// SPI0 device uninitialization
        DEBUG_MESSAGE("SPI0 UnInit failed\n");
}


#ifdef STANDALONE_APP
/*!
* @brief        Prints Sensor Data on UART
* @param[in]    NULL
* @param[out]   NULL
* @return       NULL
* @note         Prints all sensor data on UART
*/
void Print_SensData_on_UART(ADSENSORAPP_RESULT_TYPE result)
{
    SensData_temp = SensData;
    char buffer [8];
    static uint32_t iteration=0;
    iteration++;
	DEBUG_MESSAGE("ADXL372 Accelerometer \n");
	DEBUG_MESSAGE("Iteration = %d,", iteration);
	if (result == ADI_ADS_API_FAIL)
		DEBUG_MESSAGE("Failed to get data \n");
	else{
		ftoa(SensData_temp.ADXL372[0],buffer);
		DEBUG_MESSAGE("X = %s mg,",buffer);
		ftoa(SensData_temp.ADXL372[1],buffer);
		DEBUG_MESSAGE("Y = %s mg,",buffer);
		ftoa(SensData_temp.ADXL372[2],buffer);
		DEBUG_MESSAGE("Z = %s mg\n",buffer);
	}
}


/***************************************************************************************************************************/
/*						TIMER														   */
/***************************************************************************************************************************/
#ifdef USE_PERIODIC_READING
static void GPTimer0Callback(void *pCBParam, uint32_t Event, void *pArg)
{
	switch(Event)
	{
		case ADI_TMR_EVENT_TIMEOUT:
        {
        	GPT0_Timeout = 1;
			GPT0_cnt++;
        }
 		break;
      	case ADI_TMR_EVENT_CAPTURED:
        break;
     	default:
        break;
  	}
}


ADI_TMR_RESULT GPT0_Init()
{
    ADI_TMR_RESULT result = ADI_TMR_SUCCESS;
    ADI_TMR_HANDLE hDevice;
    do
    {
		result = adi_tmr_Open(TIMER_DEVICE_ID,GPT0_Memory,ADI_TMR_MEMORY_SIZE,&hDevice);

		if (ADI_TMR_SUCCESS == result)
		{
			result = adi_tmr_RegisterCallback( hDevice, GPTimer0Callback ,hDevice);
		}

		if(ADI_TMR_SUCCESS == result)
		{
			result = adi_tmr_SetClockSource(hDevice, ADI_TMR_CLOCK_LFOSC);
		}

		if( ADI_TMR_SUCCESS == result )
		{
			result = adi_tmr_SetPrescaler(hDevice, ADI_GPT_PRESCALER_16);
		}

		if (ADI_TMR_SUCCESS == result)
		{
			result = adi_tmr_SetLoadValue( hDevice, PRELOAD_VALUE);
		}

		if (ADI_TMR_SUCCESS == result)
		{
			result = adi_tmr_SetRunMode( hDevice, ADI_TMR_PERIODIC_MODE);
		}
		if (ADI_TMR_SUCCESS == result)
		{
			result = adi_tmr_SetCountMode(hDevice, ADI_TMR_COUNT_DOWN);
		}

		if (ADI_TMR_SUCCESS == result)
		{
			result = adi_tmr_Enable(hDevice, true );
		}
    }while(0);
	return result;
}
#endif

#endif
