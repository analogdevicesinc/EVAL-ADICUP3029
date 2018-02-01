/*!
********************************************************************************
* @file:    ADXL372_example.c
* @brief:   ADXL372 sensor example application.
* @version: $Revision: 1805 $
* @date:    $Date: 2012-08-28 12:23:37 $
*-------------------------------------------------------------------------------
* Copyright (c) 2012 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc.
*
* Contains example application to detect, configure & read sensor data.
*******************************************************************************/

#include "adsap_proto.h"
#include "adxl372.h"

#define ADXL372_DEBUG
//#define FIFO_READ

void adsAPI_Delay(uint32_t value)  {while(value--);}
uint32_t ADI_Sensor_Delay = 0x1000;//0x1F0;
extern ADI_SPI_HANDLE 	hSPIMasterDev;

#ifdef ADXL372_DEBUG
uint8_t TxD_temp=0, Reg_00_0D[15]={0}, Reg_15_1A[7]={0}, Reg_20_40[33]={0}, Reg_20_40_m[33]={0};
#endif

/*!
 * @brief  Detects the ADXL372 sensor mounted or not
 *
 * @return Status
 *                - #ADI_ADS_API_FAIL           On failure to detect the sensor.
 *                - #ADI_ADS_API_SUCCESS        On successfully detecting the sensor.
 *
 * Reads factory written device ID's from ADXL372 and verifies.
 */
ADSENSORAPP_RESULT_TYPE Detect_ADXL372_Sensor()
{
  	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	ADXL_DRIVER_RESULT_TYPE dResult = ADXL_DRV_SUCCESS;
	ADI_SPI_RESULT sResult = ADI_SPI_SUCCESS;
	uint8_t Dev_ID[4]={0}, ID_Check[4]={0};

	ID_Check[0]= ADI_ADXL372_ADI_DEVID_VAL;										//ADXL372_DEVID_AD Reg.Val 	= 0xAD
    ID_Check[1]= ADI_ADXL372_MST_DEVID_VAL;										//ADXL372_DEVID_MST Reg.Val = 0x1D
    ID_Check[2]= ADI_ADXL372_DEVID_VAL;											//ADXL372_PARTID Reg.Val 	= 0xFA
	ID_Check[3]= ADI_ADXL372_REVID_VAL;											//ADXL372_PARTID Reg.Val 	= 0x02


	adsAPI_Config_SPI(ADXL372_SPI_DEV, ADXL372_SPI_CLK, ADXL372_SPI_CS, false, false);
	dResult = adxl372_Reset();
	if (dResult == ADXL_DRV_SUCCESS)
		sResult = adsAPI_Config_SPI(ADXL372_SPI_DEV, ADXL372_SPI_CLK, ADXL372_SPI_CS, false, false);

	if (sResult == ADI_SPI_SUCCESS)
		dResult = adxl372_Get_DevID(&Dev_ID[0]);

	/* Cross check Device ID register values */
	if (dResult == ADXL_DRV_SUCCESS)
    {
		if((Dev_ID[0] == ID_Check[0])&&(Dev_ID[1] == ID_Check[1])&&(Dev_ID[2] == ID_Check[2])&&(Dev_ID[3] == ID_Check[3]))
			status = ADI_ADS_API_SUCCESS;
        else
          	status = ADI_ADS_API_FAIL;
    }
    return status;
}

/*!
 * @brief  Initializes and configures the ADXL372 operation
 *
 * @return Status
 *                - #ADI_ADS_API_FAIL           On failure to configure the sensor.
 *                - #ADI_ADS_API_SUCCESS        On successfully configuring the sensor.
 *
 * Configures the device by programming the rewuired registers.
 */
ADSENSORAPP_RESULT_TYPE ADXL372_Init()
{
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	ADXL_DRIVER_RESULT_TYPE dResult = ADXL_DRV_SUCCESS;
	ADI_SPI_RESULT sResult = ADI_SPI_SUCCESS;
	uint8_t rw_ctrl_bit = 0;	//0 - for write; 1- for read
//	uint8_t pwr_ctrl_config = 0x3F;

    sResult = adsAPI_Config_SPI(ADXL372_SPI_DEV, ADXL372_SPI_CLK, ADXL372_SPI_CS, false, false);

#ifdef FIFO_READ
	dResult = adxl372_Configure_FIFO(512, STREAMED, XYZ_FIFO);
#endif

#ifdef ADXL372_DEBUG
	/*Read back registers 0x00 to 0x0D values */
	rw_ctrl_bit = 1;
    TxD_temp = (ADI_ADXL372_ADI_DEVID << 1)|rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD_temp, &Reg_00_0D[0], 15);	//14 registers

	/*Read back registers 0x15 to 0x1A values */
	TxD_temp = (ADI_ADXL372_X_MAXPEAK_H << 1)|rw_ctrl_bit;
	if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD_temp, &Reg_15_1A[0], 7);	//6 registers

	/*Read back registers 0x20 to 0x40 values */
	TxD_temp = (ADI_ADXL372_OFFSET_X << 1)|rw_ctrl_bit;
	if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD_temp, &Reg_20_40[0], 33);	//32 registers
#endif

	dResult = adxl372_Set_Autosleep(false);
	dResult = adxl372_Set_BandWidth(BW_200Hz);
	dResult = adxl372_Set_ODR(ODR_400Hz);
	dResult = adxl372_Set_WakeUp_Rate(WUR_52ms);
	dResult = adxl372_Set_Act_Proc_Mode(DEFAULT);
	dResult = adxl372_Set_Op_mode(FULL_BW_MEASUREMENT);

	rw_ctrl_bit = 0;	//write
	uint8_t TxD[2]={0}, RxD[2]={0};
//  TxD[0]= (ADI_ADXL372_POWER_CTL << 1)|rw_ctrl_bit;            				// ((reg_addr << 1) | R/W_CONTROL_BIT)
//	TxD[1]= pwr_ctrl_config;	//0x1F; //0x3B;
//    if (ADI_SPI_SUCCESS == sResult)
//    	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

	rw_ctrl_bit = 1;	//read
	TxD[0]= (ADI_ADXL372_POWER_CTL << 1)|rw_ctrl_bit;            				//First register address to write	0x20
    if (status == ADI_ADS_API_SUCCESS)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

	/* Cross check if the configuration is written properly */
	if (status == ADI_ADS_API_SUCCESS)
//    {
//		if(RxD[1] & pwr_ctrl_config)
			status = ADI_ADS_API_SUCCESS;
        else
          	status = ADI_ADS_API_FAIL;
//    }
	return status;
}

/*!
 * @brief  Reads acceleration data on X,Y & Z axes from ADXL372 sensor
 *
 * @return Status
 *                - #ADI_ADS_API_FAIL           On failure to read data from sensor.
 *                - #ADI_ADS_API_SUCCESS        On successfully reading data from sensor.
 *
 * Reads acceleration data on X,Y & axes from ADXL372 sensor through SPI
 * Fills SensData.ADXL372[] with data.
 */
ADSENSORAPP_RESULT_TYPE Get_Data_From_ADXL372()
{
    ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	ADXL_DRIVER_RESULT_TYPE dResult = ADXL_DRV_SUCCESS;
	ADI_SPI_RESULT sResult = ADI_SPI_SUCCESS;
	int16_t accel_data[3];

	sResult = adsAPI_Config_SPI(ADXL372_SPI_DEV, ADXL372_SPI_CLK, ADXL372_SPI_CS, false, false);

	if (sResult == ADI_SPI_SUCCESS)
	{
#ifdef PEAK_MODE
		dResult = adxl372_Get_Highest_Peak_Accel_data(&accel_data[0]);
#else
		dResult = adxl372_Get_Accel_data(&accel_data[0]);
#endif

	}
	/* Check if the data is valid */
	uint8_t err=0;
	for(uint8_t i=0; i<3; i++)
	{
		if((accel_data[i] == 0) || (accel_data[i] == -1)) err++;
	}
	if(err == 3) return ADI_ADS_API_FAIL;

    if (dResult == ADXL_DRV_SUCCESS)
    {
        SensData.ADXL372[0] = (float)accel_data[0] * 100 / 1000;                              // 100mg/LSB
        SensData.ADXL372[1] = (float)accel_data[1] * 100 / 1000;                              // 100mg/LSB
        SensData.ADXL372[2] = (float)accel_data[2] * 100 / 1000;                              // 100mg/LSB
		status =  ADI_ADS_API_SUCCESS;
    }
    else
    {
        SensData.ADXL372[0] = 0;
        SensData.ADXL372[1] = 0;
        SensData.ADXL372[2] = 0;
		status = ADI_ADS_API_FAIL;
    }
	return status;
}



int16_t ADXL372_FIFO_Buf[520];
/*!
 * @brief  Reads FIFO data based on FIFO config from ADXL372 sensor
 *
 * @return Status
 *                - #ADI_ADS_API_FAIL           On failure to read data from sensor.
 *                - #ADI_ADS_API_SUCCESS        On successfully reading data from sensor.
 *
 * Reads acceleration data from FIFO from ADXL372 sensor through SPI
 * Fills ADXL372_FIFO_Buf[] with data.
 */
ADSENSORAPP_RESULT_TYPE ADXL372_Read_FIFO_Data()
{
    ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    ADXL_DRIVER_RESULT_TYPE dResult = ADXL_DRV_SUCCESS;
	ADI_SPI_RESULT sResult = ADI_SPI_SUCCESS;
	uint8_t status_reg_val = 0;

    sResult = adsAPI_Config_SPI(ADXL372_SPI_DEV, ADXL372_SPI_CLK, ADXL372_SPI_CS, false, false);
	if(sResult == ADI_SPI_SUCCESS)
	{
//		while(!(status_reg_val & 0x04)){										//Checking for FIFO full
//			dResult = adxl372_Get_Status_Register(&status_reg_val);
//		}
	}

	if (dResult == ADXL_DRV_SUCCESS)
    	dResult = adxl372_Get_FIFO_data((uint8_t *) &ADXL372_FIFO_Buf[0]);

    if (dResult == ADXL_DRV_SUCCESS)
    {
        status =  ADI_ADS_API_SUCCESS;
    }
    else
    {
        status = ADI_ADS_API_FAIL;
    }
    return status;
}

/*!
 * @brief  Reads FIFO data based on FIFO config from ADXL372 sensor
 *
 * @return Status
 *                - #ADI_ADS_API_FAIL           On failure to read data from sensor.
 *                - #ADI_ADS_API_SUCCESS        On successfully reading data from sensor.
 *
 * Reads acceleration data from FIFO from ADXL372 sensor through SPI
 * Fills ADXL372_FIFO_Buf[] with data.
 */
ADSENSORAPP_RESULT_TYPE ADXL372_Set_Impact_Detection(void)
{
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	ADXL_DRIVER_RESULT_TYPE dResult = ADXL_DRV_SUCCESS;
	ADI_SPI_RESULT sResult = ADI_SPI_SUCCESS;
	uint8_t status_reg_val = 0;
	uint8_t rw_ctrl_bit = 0;	//0 - for write; 1- for read
	uint8_t TxD[2];

	sResult = adsAPI_Config_SPI(ADXL372_SPI_DEV, ADXL372_SPI_CLK, ADXL372_SPI_CS, false, false);

	if(sResult == ADI_SPI_SUCCESS)
	{
		dResult =adxl372_Set_Op_mode(STAND_BY);

		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_Autosleep(false);

		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_BandWidth(BW_3200Hz);

		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_ODR(ODR_6400Hz);

		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_WakeUp_Rate(WUR_52ms);

		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_Act_Proc_Mode(LOOPED);

		/* Set Instant On threshold */
		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_InstaOn_Thresh(ADXL_INSTAON_LOW_THRESH); //Low threshold 10-15 G

		/*Put fifo in Peak Detect and Stream Mode */
		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Configure_FIFO(512, STREAMED, XYZ_PEAK_FIFO);

		/* Set activity/inactivity threshold */
		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_Activity_Threshold();
		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_Inactivity_Threshold();

		/* Set activity/inactivity time settings */
		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_Activity_Time();
		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_Inactivity_Time();

		/* Set instant-on interrupts and activity interrupts */
		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_Interrupts();

		/* Set filter settle time */
		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_Filter_Settle(FILTER_SETTLE_16);

		TxD[0] = (ADI_ADXL372_POWER_CTL << 1);
		TxD[1] = 0x1A;
		adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

		/* Set operation mode to Instant-On */
		if (dResult == ADXL_DRV_SUCCESS)
			dResult = adxl372_Set_Op_mode(INSTANT_ON);
	}

#ifdef ADXL372_DEBUG
	/*Read back registers 0x00 to 0x0D values */
	rw_ctrl_bit = 1;
    TxD_temp = (ADI_ADXL372_ADI_DEVID << 1)|rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD_temp, &Reg_00_0D[0], 15);	//14 registers

	/*Read back registers 0x15 to 0x1A values */
	TxD_temp = (ADI_ADXL372_X_MAXPEAK_H << 1)|rw_ctrl_bit;
	if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD_temp, &Reg_15_1A[0], 7);	//6 registers

	/*Read back registers 0x20 to 0x40 values */
	TxD_temp = (ADI_ADXL372_OFFSET_X << 1)|rw_ctrl_bit;
	if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD_temp, &Reg_20_40[0], 33);	//32 registers
#endif

	if (dResult == ADXL_DRV_SUCCESS)
	{
		status =  ADI_ADS_API_SUCCESS;
	}
	else
	{
		status = ADI_ADS_API_FAIL;
	}
	return status;
}
