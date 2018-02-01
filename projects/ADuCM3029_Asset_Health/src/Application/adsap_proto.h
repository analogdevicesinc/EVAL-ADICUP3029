/*!
********************************************************************************
* @file:        adsap_proto.h
* @brief        Commom variables, prototypes and definitions across project
* @version:     1.00 $Revision: 0328 $
* @date:        $Date: 2016-03-28 12:23:37 $
*-------------------------------------------------------------------------------
Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated Analog Devices
Software License Agreement.
*******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <drivers/i2c/adi_i2c.h>
#include <drivers/spi/adi_spi.h>
#include <drivers/gpio/adi_gpio.h>

typedef struct adsap_sensors
{
    uint32_t enabled;
    uint32_t detected;
    uint32_t configured;
    uint32_t data_read;
}adsap_sensors;

/*! \struct Sens_Data
* Sensor Data type */
typedef struct Sens_Data{
//#ifdef CLUMP
    float   ADXL362[3];	//Accel XYZ
    float   SHT25[2];	//Temp & RH
    float   MAX44009;	//ALS in Lux
    float   BMP280[2];	//Pressure & Temp
    float   ADXL355[3];	//Accel XYZ
    float   ADXL372[3];	//Accel XYZ
    float   ADT7320;	//Temp
    float   EKMB120;	//Motion
//#else
    float   ADXL345[3];
    float   ADT75;
    float   ADXL363[3];
    float   SHT21[2];
    float   ADT7310;
    float   ADT7420;
    float   AD7151;
    float   PIR;
    float   ALS;
    float   VOS[2];
    float   MPS;
//#endif
}Sens_Data;


/*!
 *****************************************************************************
 * \enum TIME_UNIT
 *
 * Time Units
 *****************************************************************************/
typedef enum {
    MICROSECONDS = 0,
    MILLISECONDS,
    SECONDS,
    MINUTES,
    HOURS,
}TIME_UNIT;

/*!
 *****************************************************************************
 * \enum ADSENSORAPP_RESULT_TYPE
 *
 *  result type
 *****************************************************************************/
typedef enum
{
  ADI_ADS_API_FAIL      = -1,
  ADI_ADS_API_SUCCESS   = 0
} ADSENSORAPP_RESULT_TYPE;

typedef enum
{
  I2C   = 0,
  SPI	= 1,
  ADC	= 2
} INTF_TYPE;

typedef enum
{
  REG_READ      = 0,
  REG_WRITE		= 1
} REG_RW_MODE;

#define COMMAND_NA	0x00

typedef enum
{
  TURN_OFF = 0x00,
  TURN_ON,
  TOGGLE
} LED_OnOffState;

#ifdef CLUMP
	#define LED_DS1 	0  //used in main.c
	#define LED_DS2  	1  //used in UDP Client.c and UDP Server.c
#else
	#define LED3  		0  //used in main.c
	#define LED4  		1  //used in UDP Client.c and UDP Server.c
	#define LED5  		2  //Not used
#endif


extern struct Sens_Data SensData;

void adsAPI_ADSensInit(void);
extern ADSENSORAPP_RESULT_TYPE adsAPI_RW_I2C_Sensor_Reg(REG_RW_MODE rw, uint8_t I2CM_DevNum, uint8_t RegAddr, uint8_t *Data, uint8_t Numbytes, bool RepeatStart);
extern ADSENSORAPP_RESULT_TYPE adsAPI_RW_SPI_Sensor_Reg(REG_RW_MODE rw_mode, uint8_t SPI_Dev, uint8_t cmd, uint8_t *RegAddr, uint8_t *RegData, uint32_t No_of_Bytes);


extern void ADSensorAppAdvOps();

extern void SystemInit(void);
extern int32_t adi_initpinmux(void);
extern void adsAPI_Init_Devices();
extern void adsAPI_UnInit_Devices();
extern ADI_I2C_RESULT adsAPI_Uninit_I2C_Port();
extern ADI_I2C_RESULT adsAPI_Init_I2C_Port();
extern void adsAPI_Read_ADSensorData();

extern void adsAPI_Delay(uint32_t value);

extern ADSENSORAPP_RESULT_TYPE ADXL372_Init();
extern ADSENSORAPP_RESULT_TYPE Detect_ADXL372_Sensor();
extern ADSENSORAPP_RESULT_TYPE Get_Data_From_ADXL372();
extern ADSENSORAPP_RESULT_TYPE ADXL372_Read_FIFO_Data();
extern ADSENSORAPP_RESULT_TYPE ADXL372_Set_Impact_Detection(void);

extern void Print_SensData_on_UART(ADSENSORAPP_RESULT_TYPE res);
extern void ftoa(float f,char *buf);

extern void Config_Ext_Ints();


extern void adsAPI_Setup_GPT0_Counter();
extern uint16_t TmrVal;


void LEDControl(int WhichLED, LED_OnOffState State);
void TurnON_All_LEDs(void);
void TurnOFF_All_LEDs(void);
ADI_GPIO_RESULT InitLEDs(void);
