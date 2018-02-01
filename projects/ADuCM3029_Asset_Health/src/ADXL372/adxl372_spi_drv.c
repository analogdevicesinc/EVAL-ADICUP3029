/*!
********************************************************************************
* @file:    ADXL372.c
* @brief:   ADXL372 sensor interface code and programming.
* @version: $Revision: 1805 $
* @date:    $Date: 2012-08-28 12:23:37 $
*-------------------------------------------------------------------------------
* Copyright (c) 2012 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc.
*
* Contains the routines required for programming & reading data from ADXL372 sensor.
*******************************************************************************/

#include "adsap_proto.h"
#include "adxl372.h"

uint32_t data_not_ready = 0;
uint8_t reset_done = 0;


ADXL_DRIVER_RESULT_TYPE adxl372_Set_Op_mode(ADXL372_OP_MODE mode)
{
    ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit;
    uint8_t TxD[2]={0}, RxD[2]={0};

    /* read the power control register value */
    rw_ctrl_bit = 1;	//read
    TxD[0] = (ADI_ADXL372_POWER_CTL << 1) | rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

    /* update the power control register with mode bits */
    rw_ctrl_bit = 0;	//write
    TxD[0] = (ADI_ADXL372_POWER_CTL << 1) | rw_ctrl_bit;            			// ((reg_addr << 1) | R/W_CONTROL_BIT)
    TxD[1] = (RxD[1] & PWRCTRL_OPMODE_MASK ) | mode;                                           // Current POWER_CONTROL register value | mode
    if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

	/* read the power control register value */
    rw_ctrl_bit = 1;	//read
    TxD[0] = (ADI_ADXL372_POWER_CTL << 1) | rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);
//
//	if((RxD[1] &PWRCTRL_OPMODE_MASK) == mode)

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Set_ODR(ADXL372_ODR odr)
{
    ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit;
    uint8_t TxD[2]={0}, RxD[2]={0};

    /* read the timing register value */
    rw_ctrl_bit = 1;	//read
    TxD[0] = (ADI_ADXL372_TIMING << 1) | rw_ctrl_bit;
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

    /* update the timing register with ODR bits */
    rw_ctrl_bit = 0;	//write
    TxD[0] = (ADI_ADXL372_TIMING << 1) | rw_ctrl_bit;            				// ((reg_addr << 1) | R/W_CONTROL_BIT)
    TxD[1] = (RxD[1] & TIMING_ODR_MASK ) | (odr << TIMING_ODR_POS);                                     // Current TIMING register value | (odr << 5)
    if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}


ADXL_DRIVER_RESULT_TYPE adxl372_Set_WakeUp_Rate(ADXL372_WUR wur)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit;
    uint8_t TxD[2]={0}, RxD[2]={0};

    /* read the timing register value */
    rw_ctrl_bit = 1;	//read
    TxD[0] = (ADI_ADXL372_TIMING << 1) | rw_ctrl_bit;
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

    /* update the timing register with WUR bits */
    rw_ctrl_bit = 0;	//write
    TxD[0] = (ADI_ADXL372_TIMING << 1) | rw_ctrl_bit;            				// ((reg_addr << 1) | R/W_CONTROL_BIT)
    TxD[1] = (RxD[1] & TIMING_WUR_MASK ) | (wur << TIMING_WUR_POS);        // Current TIMING register value | (wur << 2)
    if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}


ADXL_DRIVER_RESULT_TYPE adxl372_Set_BandWidth(ADXL372_BW bw)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit;
    uint8_t TxD[2]={0}, RxD[2]={0};

    /* read the Measurement Control value */
    rw_ctrl_bit = 1;	//read
    TxD[0] = (ADI_ADXL372_MEASURE << 1) | rw_ctrl_bit;
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

    /* update the Measurement Control with BW bits */
    rw_ctrl_bit = 0;	//write
    TxD[0] = (ADI_ADXL372_MEASURE << 1) | rw_ctrl_bit;            				// ((reg_addr << 1) | R/W_CONTROL_BIT)
    TxD[1] = (RxD[1] & MEASURE_BANDWIDTH_MASK) | bw; 		                                    // Current Measurement Control register value | bw
    if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}


ADXL_DRIVER_RESULT_TYPE adxl372_Set_Autosleep(bool enable)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit;
    uint8_t TxD[2]={0}, RxD[2]={0};

    /* read the Measurement Control value */
    rw_ctrl_bit = 1;	//read
    TxD[0] = (ADI_ADXL372_MEASURE << 1) | rw_ctrl_bit;
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

    /* update the Measurement Control with BW bits */
    rw_ctrl_bit = 0;	//write
    TxD[0] = (ADI_ADXL372_MEASURE << 1) | rw_ctrl_bit;            				// ((reg_addr << 1) | R/W_CONTROL_BIT)
    TxD[1] = (RxD[1] & MEASURE_AUTOSLEEP_MASK ) | (enable << MEASURE_AUTOSLEEP_POS) ; 	// Present Measurement Control register value | enable
    if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}


ADXL_DRIVER_RESULT_TYPE adxl372_Set_Act_Proc_Mode(ADXL372_ACT_PROC_MODE mode)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit;
    uint8_t TxD[2]={0}, RxD[2]={0};

    /* read the Measurement Control value */
    rw_ctrl_bit = 1;	//read
    TxD[0] = (ADI_ADXL372_MEASURE << 1) | rw_ctrl_bit;
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

    /* update the Measurement Control with BW bits */
    rw_ctrl_bit = 0;	//write
    TxD[0] = (ADI_ADXL372_MEASURE << 1) | rw_ctrl_bit;            				// ((reg_addr << 1) | R/W_CONTROL_BIT)
    TxD[1] = (RxD[1] & MEASURE_ACTPROC_MASK ) | (mode<<MEASURE_ACTPROC_POS); 		                                // Current Measurement Control register value | mode<<5
    if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}


ADXL_DRIVER_RESULT_TYPE adxl372_Get_DevID(uint8_t *DevID)
{
  	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit;
	uint8_t TxD[5]={0}, RxD[5]={0};

	/* Read Device ID registers */
	rw_ctrl_bit = 1;
	TxD[0]= (ADI_ADXL372_ADI_DEVID << 1) | rw_ctrl_bit;							// ((reg_addr << 1) | R/W_CONTROL_BIT)
	if (ADI_ADS_API_SUCCESS == status)
    {
		status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 5);
		DevID[0] = RxD[1];	DevID[1] = RxD[2];	DevID[2] = RxD[3];	DevID[3] = RxD[4];
	}

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Get_Status_Register(uint8_t *adxl_status)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit;
    uint8_t TxD[2]={0}, RxD[2]={0};

    /* read the status register */
    rw_ctrl_bit = 1;	//read
    TxD[0] = (ADI_ADXL372_STATUS_1 << 1) | rw_ctrl_bit;							// ((reg_addr << 1) | R/W_CONTROL_BIT)
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);
    *adxl_status = RxD[1];

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Get_ActivityStatus_Register(uint8_t *adxl_status2)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit;
    uint8_t TxD[2]={0}, RxD[2]={0};

    /* read the activity status register */
    rw_ctrl_bit = 1;	//read
    TxD[0] = (ADI_ADXL372_STATUS_2 << 1) | rw_ctrl_bit;							// ((reg_addr << 1) | R/W_CONTROL_BIT)
    status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);
    *adxl_status2 = RxD[1];

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Get_Highest_Peak_Accel_data(int16_t *max_peak)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit, SignBit;
    uint8_t TxD[7]={0}, RxD[7]={0};

    /* read the maximum peak data registers */
    rw_ctrl_bit = 1;
	TxD[0]= (ADI_ADXL372_X_MAXPEAK_H << 1) | rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 8);

	max_peak[0] = (int16_t) (((RxD[X_VALID_DATA_INDEX]<<8) | RxD[X_VALID_DATA_INDEX+1]) >> 4);
	SignBit = (max_peak[0] & (1 << 11)) != 0;
	if (SignBit)
		max_peak[0] = max_peak[0] | ~((1 << 12) - 1);

	max_peak[1] = (int16_t) (((RxD[Y_VALID_DATA_INDEX]<<8) | RxD[Y_VALID_DATA_INDEX+1]) >> 4);
	SignBit = (max_peak[1] & (1 << 11)) != 0;
	if (SignBit)
		max_peak[1] = max_peak[1] | ~((1 << 12) - 1);

	max_peak[2] = (int16_t) (((RxD[Z_VALID_DATA_INDEX]<<8) | RxD[Z_VALID_DATA_INDEX+1]) >> 4);
	SignBit = (max_peak[2] & (1 << 11)) != 0;
	if (SignBit)
		max_peak[2] = max_peak[2] | ~((1 << 12) - 1);

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}


ADXL_DRIVER_RESULT_TYPE adxl372_Get_Accel_data(int16_t *accel_data)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	#define ACCEL_DATA_BUF_SIZE		7
	uint8_t TxD[ACCEL_DATA_BUF_SIZE]={0}, RxD[ACCEL_DATA_BUF_SIZE]={0};

	uint8_t rw_ctrl_bit = 1, SignBit;
	TxD[0]= (ADI_ADXL372_STATUS_1 << 1) | rw_ctrl_bit;

	/* wait till status is data ready */
	while (!(RxD[1] & 0x01))	//Checking status register for bit-0; Indicates data is ready to read.
	{
		status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);
		adsAPI_Delay(ADI_Sensor_Delay);
	}

//	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);
	/* Read data once the status indicates data ready */
	if(RxD[1] & 0x01)
	{
		TxD[0]= (ADI_ADXL372_X_DATA_H << 1) | rw_ctrl_bit;
		status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], ACCEL_DATA_BUF_SIZE);

//		//Data validation incase of CS disconnected
//		/* Check if the data is valid */
//		if(RxD[0] == 0x00) return ADXL_DRV_FAIL;	// The first byte should be 0xFF always
//		uint8_t err=0;
//		for(uint8_t i=0;i<ACCEL_DATA_BUF_SIZE;i++)
//		{
//			if(RxD[i] == INVALID_SPI_DATA) err++;
//		}
//		if(err == ACCEL_DATA_BUF_SIZE) return ADXL_DRV_FAIL;

		accel_data[0] = (int16_t) ((((int16_t)RxD[X_VALID_DATA_INDEX]<<8) | (int16_t)RxD[X_VALID_DATA_INDEX+1]) >> 4);
		SignBit = (accel_data[0] & (1 << 11)) !=0;
		if (SignBit)
			accel_data[0] = accel_data[0] | (int16_t)(~((1 << 12) - 1));
//		accel_data[0] = (int16_t) ((RxD[X_VALID_DATA_INDEX]<<8) | RxD[X_VALID_DATA_INDEX+1]);
//		accel_data[0] = (~(accel_data[0])+ 1) >> 4;  //two's compliment


		accel_data[1] = (int16_t) (((int16_t)(RxD[Y_VALID_DATA_INDEX]<<8) | (int16_t)RxD[Y_VALID_DATA_INDEX+1]) >> 4);
		SignBit = (accel_data[1] & (1 << 11)) !=0;
		if (SignBit)
			accel_data[1] = accel_data[1] | ~((1 << 12) - 1);
//		accel_data[1] = (int16_t) ((RxD[Y_VALID_DATA_INDEX]<<8) | RxD[Y_VALID_DATA_INDEX+1]);
//		accel_data[1] = (~(accel_data[1])+ 1)>> 4;  //two's compliment


		accel_data[2] = (int16_t) ((((int16_t)RxD[Z_VALID_DATA_INDEX]<<8) | (int16_t)RxD[Z_VALID_DATA_INDEX+1]) >> 4);
		SignBit = (accel_data[2] & (1 << 11)) !=0;
		if (SignBit)
			accel_data[2] = accel_data[2] | ~((1 << 12) - 1);
//		accel_data[2] = (int16_t) ((RxD[Z_VALID_DATA_INDEX]<<8) | RxD[Z_VALID_DATA_INDEX+1]);
//		accel_data[2] = (~(accel_data[2])+ 1) >>4;  //two's compliment
	}
	else
	{
		data_not_ready++;
	}

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Reset(void)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	uint8_t TxD[2]={0}, RxD[2]={0};

  	/* Reset ADXL372*/
	uint8_t rw_ctrl_bit = 0;	//0 - for write; 1- for read
	TxD[0]= (ADI_ADXL372_SRESET << 1) | rw_ctrl_bit;            				//Reset register 	= 0x2F
	TxD[1]= 0x52;																//Reset value 		= 0x52
    status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 1);

	/* Give some delay after reset */
	adsAPI_Delay(10*ADI_Sensor_Delay);

	/* Read status registers */
	rw_ctrl_bit = 1;
	TxD[0]= (ADI_ADXL372_STATUS_1 << 1) | rw_ctrl_bit;            			// Status Resisters = 0x04, 0x05
	if (ADI_ADS_API_SUCCESS == status)
    {
		status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 3);
	}

    if(RxD[0] & 0xA0)   														//Checking status_1 register for SEU & USR_NVM_BUSY;
	{
	  	reset_done = 1;
	}

	if (status == ADI_ADS_API_SUCCESS)
	  	result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

fifo_config_t fifo_config;

ADXL_DRIVER_RESULT_TYPE adxl372_Configure_FIFO(uint16_t fifo_samples, ADXL372_FIFO_MODE fifo_mode, ADXL372_FIFO_FORMAT fifo_format)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit;
    uint8_t TxD[3]={0};//, RxD[3]={0};

	result = adxl372_Set_Op_mode(STAND_BY);

//    /* read the FIFO_SAMPLES & FIFO_CTL resgister values */
//    rw_ctrl_bit = 1;	//read
//    TxD[0] = (ADI_ADXL372_FIFO_SAMPLES << 1) | rw_ctrl_bit;
//	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 3);

//	/* update the FIFO_SAMPLES & FIFO_CTL resgister values */
//    rw_ctrl_bit = 0;	//write
//    TxD[0] = (ADI_ADXL372_FIFO_SAMPLES << 1) | rw_ctrl_bit;            			// ((reg_addr << 1) | R/W_CONTROL_BIT)
//	TxD[1] = (RxD[1]) | ((fifo_samples-1) & 0xFF); 		                        // FIFO Samples register value | fifo_samples
//	TxD[2] = (RxD[2] & 0xC7) | (fifo_format<<3); 		                        // FIFO Control register value | fifo_format<<3
//	TxD[2] = (RxD[2] & 0xF9) | (fifo_mode<<1); 		                        	// FIFO Control register value | fifo_mode<<1
//    if (fifo_samples > 0xFF)
//		TxD[2] = (TxD[2] & 0xFE) | 1; 		                        			// FIFO Control register value | (fifo_samples)>>8

//    if (ADI_ADS_API_SUCCESS == status)
//    	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 3);

	/* update the FIFO_SAMPLES resgister value */
    rw_ctrl_bit = 0;	//write
    TxD[0] = (ADI_ADXL372_FIFO_SAMPLES << 1) | rw_ctrl_bit;            			// ((reg_addr << 1) | R/W_CONTROL_BIT)
	TxD[1] = (fifo_samples-1) & 0xFF;			 		                        // FIFO Samples register value | fifo_samples[7:0]
    if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

	/* update the FIFO_CTL resgister value */
	TxD[0] = (ADI_ADXL372_FIFO_CTL << 1) | rw_ctrl_bit;            				// ((reg_addr << 1) | R/W_CONTROL_BIT)
	TxD[1] = 0;
	TxD[1] = (TxD[1] & 0xC7) | (fifo_format<<3); 		                        // FIFO Control register value | fifo_format<<3
	TxD[1] = (TxD[1] & 0xF9) | (fifo_mode<<1); 		                        	// FIFO Control register value | fifo_mode<<1
	if (fifo_samples > 0xFF)
		TxD[1] = (TxD[1] & 0xFE) | 1; 		                        			// FIFO Control register value | fifo_samples[8]
    if (ADI_ADS_API_SUCCESS == status)
    	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

    fifo_config.samples = fifo_samples;
    fifo_config.mode 	= fifo_mode;
    fifo_config.format 	= fifo_format;

	uint8_t TxD_m[4]={0}, RxD_m[4]={0};
    /* read the FIFO_SAMPLES & FIFO_CTL resgister values */
    rw_ctrl_bit = 1;	//read
    TxD_m[0] = (ADI_ADXL372_FIFO_SAMPLES << 1) | rw_ctrl_bit;
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD_m[0], &RxD_m[0], 4);

	if(TxD_m[0] == RxD_m[0]) result = ADXL_DRV_FAIL;

    if (status == ADI_ADS_API_SUCCESS)
            result = ADXL_DRV_SUCCESS;
    else
            result = ADXL_DRV_FAIL;
    return result;
}


ADXL_DRIVER_RESULT_TYPE adxl372_Get_FIFO_data(uint8_t *buff)
{
    ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
    ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
    uint8_t rw_ctrl_bit = 1;
	//uint8_t TxD[3]={0}, RxD[3]={0};
    uint8_t TxD[1]={0}, RxD[1030]={0};

    if(fifo_config.mode == BYPASSED) return ADXL_DRV_FAIL;

    /* read the FIFO_DATA resgister */
    rw_ctrl_bit = 1;	//read
	TxD[0] = (ADI_ADXL372_FIFO_DATA << 1) | rw_ctrl_bit;
//    for(uint16_t index = 0; index < 1030; index++)
//    {
//        TxD[index + 1] = TxD[index];
//    }
    status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], (uint8_t*)&buff[0], (fifo_config.samples*2)+1);

    /* read the FIFO_DATA resgister */
//    rw_ctrl_bit = 1;	//read
//    TxD[0] = (ADI_ADXL372_FIFO_DATA << 1) | rw_ctrl_bit;
//
//	for (uint16_t i=0; i<fifo_config.samples; i++)
//	{
//    	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 3);
//		buff[i*2]= RxD[1];buff[(i*2)+1]= RxD[2];
//	}

    if (status == ADI_ADS_API_SUCCESS)
            result = ADXL_DRV_SUCCESS;
    else
            result = ADXL_DRV_FAIL;
    return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Set_InstaOn_Thresh(ADXL_INSTAON_THRESH mode)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	uint8_t rw_ctrl_bit = 1;
	uint8_t TxD[2]={0}, RxD[2]={0};

	/*Set Instant on threshold */
	/* read the Measurement Control value */
	rw_ctrl_bit = 1;	//read
	TxD[0] = (ADI_ADXL372_POWER_CTL << 1) | rw_ctrl_bit;
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

	/* update the Measurement Control with BW bits */
	rw_ctrl_bit = 0;	//write
	TxD[0] = (ADI_ADXL372_POWER_CTL << 1) | rw_ctrl_bit;            				// ((reg_addr << 1) | R/W_CONTROL_BIT)
	TxD[1] = (RxD[1] & PWRCTRL_INSTON_THRESH_MASK ) | (mode<<INSTAON_THRESH_POS);
	if (ADI_ADS_API_SUCCESS == status)
		status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);


    if (status == ADI_ADS_API_SUCCESS)
            result = ADXL_DRV_SUCCESS;
    else
            result = ADXL_DRV_FAIL;
    return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Set_Activity_Threshold(void)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	uint8_t rw_ctrl_bit = 1;
	uint8_t TxD[7]={0}, RxD[7]={0};

	result = adxl372_Set_Op_mode(STAND_BY);

	/*Set activity threshold*/
	rw_ctrl_bit = 0; //write flag
	TxD[0] = (ADI_ADXL372_X_THRESH_ACT_H << 1) | rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	TxD[1] = ACT_VALUE >> 3;
	TxD[2] = ((ACT_VALUE << 5) & 0xFF ) | 0x1; //Set ACT_X_EN to X-axis used
	TxD[3] = TxD[1];
	TxD[4] = TxD[2];
	TxD[5] = TxD[1];
	TxD[6] = TxD[2];
	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 7);

	/* Read register value for verification */
	rw_ctrl_bit = 1;	//read
	TxD[0] = (ADI_ADXL372_X_THRESH_ACT_H << 1) | rw_ctrl_bit;
	if (status == ADI_ADS_API_SUCCESS)
		status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 7);

	if (status == ADI_ADS_API_SUCCESS)
		result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Set_Inactivity_Threshold(void)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	uint8_t rw_ctrl_bit = 1;
	uint8_t TxD[7]={0}, RxD[7]={0};

	/*Set activity threshold*/
	rw_ctrl_bit = 0; //write flag
	TxD[0] = (ADI_ADXL372_X_THRESH_INACT_H << 1) | rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	TxD[1] = INACT_VALUE >> 3;
	TxD[2] = ((INACT_VALUE << 5) & 0xFF ) | 0x1; //Set INACT_X_EN to X-axis used
	TxD[3] = TxD[1];
	TxD[4] = TxD[2];
	TxD[5] = TxD[1];
	TxD[6] = TxD[2];
	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 7);

	/* Read register value for verification */
	rw_ctrl_bit = 1;	//read
	TxD[0] = (ADI_ADXL372_X_THRESH_INACT_H << 1) | rw_ctrl_bit;
	if (status == ADI_ADS_API_SUCCESS)
		status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 7);

	if (status == ADI_ADS_API_SUCCESS)
		result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Set_Activity_Time(void)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	uint8_t rw_ctrl_bit = 1;
	uint8_t TxD[2]={0}, RxD[2]={0};

	/*Set activity threshold*/
	rw_ctrl_bit = 0; //write flag
	TxD[0] = (ADI_ADXL372_TIME_ACT << 1) | rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	TxD[1] = ACT_TIMER;
	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

	/* Read register value for verification */
	rw_ctrl_bit = 1;	//read
	TxD[0] = (ADI_ADXL372_TIME_ACT << 1) | rw_ctrl_bit;
	if (status == ADI_ADS_API_SUCCESS)
		status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

	if (status == ADI_ADS_API_SUCCESS)
		result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Set_Inactivity_Time(void)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	uint8_t rw_ctrl_bit = 1;
	uint8_t TxD[3]={0}, RxD[3]={0};

	/*Set activity threshold*/
	rw_ctrl_bit = 0; //write flag
	TxD[0] = (ADI_ADXL372_TIME_INACT_H << 1) | rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	TxD[1] = INACT_TIMER >> 8;
	TxD[2] = INACT_TIMER & 0xFF;
	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 3);

	/* Read register value for verification */
	rw_ctrl_bit = 1;	//read
	TxD[0] = (ADI_ADXL372_TIME_INACT_H << 1) | rw_ctrl_bit;
	if (status == ADI_ADS_API_SUCCESS)
		status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 3);

	if (status == ADI_ADS_API_SUCCESS)
		result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Set_Interrupts(void)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	uint8_t rw_ctrl_bit = 1;
	uint8_t TxD[2]={0}, RxD[2]={0};

	/* Set INT1_MAP values */
	rw_ctrl_bit = 0; //write flag
	TxD[0] = (ADI_ADXL372_INT1_MAP << 1) | rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	TxD[1] = 0x1 << 6; // Awake bit
//	TxD[1] = 0x1 << 5; //!!!Activity bit
	status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

	/* Read register value for verification */
	rw_ctrl_bit = 1;	//read
	TxD[0] = (ADI_ADXL372_INT1_MAP << 1) | rw_ctrl_bit;
	if (status == ADI_ADS_API_SUCCESS)
		status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

	if (status == ADI_ADS_API_SUCCESS)
		result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;
}

ADXL_DRIVER_RESULT_TYPE adxl372_Set_Filter_Settle(ADXL372_Filter_Settle mode)
{
	ADXL_DRIVER_RESULT_TYPE result = ADXL_DRV_SUCCESS;
	ADSENSORAPP_RESULT_TYPE status = ADI_ADS_API_SUCCESS;
	uint8_t rw_ctrl_bit;
	uint8_t TxD[2]={0}, RxD[2]={0};

	/* read the power control register value */
	rw_ctrl_bit = 1;	//read
	TxD[0] = (ADI_ADXL372_POWER_CTL << 1) | rw_ctrl_bit;						// ((reg_addr << 1) | R/W_CONTROL_BIT)
	status = adsAPI_RW_SPI_Sensor_Reg(REG_READ, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], &RxD[0], 2);

	/* update the power control register with mode bits */
	rw_ctrl_bit = 0;	//write
	TxD[0] = (ADI_ADXL372_POWER_CTL << 1) | rw_ctrl_bit;            			// ((reg_addr << 1) | R/W_CONTROL_BIT)
	TxD[1] = (RxD[1] & 0xEF ) | (mode << 4);                                           // Current POWER_CONTROL register value | mode
	if (ADI_ADS_API_SUCCESS == status)
		status = adsAPI_RW_SPI_Sensor_Reg(REG_WRITE, ADXL372_SPI_DEV, COMMAND_NA, &TxD[0], NULL, 2);

	if (status == ADI_ADS_API_SUCCESS)
		result = ADXL_DRV_SUCCESS;
	else
		result = ADXL_DRV_FAIL;
	return result;

}
