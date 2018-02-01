/*!
********************************************************************************
* @file:        adsap_def.h
* @brief        Sensor registers definitions
* @version:     1.00 $Revision: 0911 $
* @date:        $Date: 2011-09-12 12:23:37 $
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

/*
Definitions for Sensors used in this project.
[Currently, these are being taken care from IAR project options under [C/C++ Compiler], [Preprocessor] options
*/
//#define ADT75_Sensor
//#define ADXL345_Sensor
//#define SHT21_Sensor
//#define AD7151_Sensor
//#define ADT7310_Sensor
//#define PIR_Sensor
//#define ALS_Sensor
//#define PmodJSTK_Sensor
//#define ADXL362_Sensor
//#define EVAL_POT_Sensor

extern void adsAPI_Delay(uint32_t value);
extern uint32_t ADI_Sensor_Delay;

//I2C Driver related
#define I2C0_DEVNUM   	(0u)        /* I2C device number */
#define I2C0_CLK      	(100000u)   /* Hz */
#define BITRATE       	(100u)      /* kHz */
#define I2C0_DUTYCYCLE 	(50u)       /* percent */
#define PRESCALEVALUE 	(125u/10u)  /* fSCLK/10MHz */

//SPI Driver related
#define SPI2_CLK		4000000
#define SPI2_CS0		ADI_SPI_CS0
#define SPI2_CS1		ADI_SPI_CS1
#define SPI2_CS2		ADI_SPI_CS2
#define SPI2_CS3		ADI_SPI_CS3

#define SPI0_DEV_NUM				0
#define SPI0_CS0_NUM				ADI_SPI_CS0
#define SPI0_CS1_NUM				ADI_SPI_CS1

#define SPI1_DEV_NUM				1
#define SPI1_CS0_NUM				ADI_SPI_CS0

#define SPI2_DEV_NUM				2
#define SPI2_CS0_NUM				ADI_SPI_CS0
#define SPI2_CS1_NUM				ADI_SPI_CS1

#define eADS_CFG_SPI_DEF_CLK		4000000

#define ADXL372_SPI_CLK				4000000
#define ADXL372_SPI_CS				SPI2_CS1_NUM
#define ADXL372_SPI_DEV				SPI2_DEV_NUM




/**********************   Sensor Masks    ********************** */
#define LED_SENSOR_MASK               	0x00000001		//0
#define ADT75_SENSOR_MASK             	0x00000002		//1
#define ADXL362_SENSOR_MASK           	0x00000004		//2
#define ADXL345_SENSOR_MASK           	0x00000008		//3
#define SHT21_SENSOR_MASK             	0x00000010		//4
#define AD7151_SENSOR_MASK            	0x00000020		//5
#define ALS_SENSOR_MASK           		0x00000040		//6
#define PIR_SENSOR_MASK               	0x00000080		//7
#define EVAL_POT_SENSOR_MASK          	0x00000100		//8
#define ADT7310_SENSOR_MASK           	0x00000200		//9
#define PMODJSTK_SENSOR_MASK          	0x00000400		//10
#define VOS_SENSOR_MASK               	0x00000800		//11
#define ADIS16480_SENSOR_MASK         	0x00001000		//12
#define BMP280_SENSOR_MASK            	0x00002000		//13
#define ADXL363_SENSOR_MASK         	0x00004000		//14
#define ADT7420_SENSOR_MASK      	    0x00008000		//15
#define ADXL355_SENSOR_MASK          	0x00010000		//16
#define SHT25_SENSOR_MASK				0x00020000		//17
#define ADXL372_SENSOR_MASK          	0x00040000		//18
#define EKMB120_SENSOR_MASK          	0x00080000		//19					//Motion
#define ADT7320_SENSOR_MASK          	0x00100000		//20					//Temp
#define ADXRS290_SENSOR_MASK          	0x00200000		//21					//Accel
#define ADIS16209_SENSOR_MASK          	0x00400000		//22					//Tilt
#define MAX44009_SENSOR_MASK			0x00800000		//23					//ALS

/**********************   Sensor Index	 **********************/
#define LED_SENSOR                  0
#define ADT75_SENSOR            	1
#define ADXL362_SENSOR          	2
#define ADXL345_SENSOR          	3
#define SHT21_SENSOR            	4
#define AD7151_SENSOR           	5
#define ALS_SENSOR              	6
#define PIR_SENSOR              	7
#define EVAL_POT_SENSOR         	8
#define ADT7310_SENSOR          	9
#define PMODJSTK_SENSOR         	10
#define VOS_SENSOR              	11
#define ADIS16480_SENSOR        	12
#define BMP280_SENSOR           	13
#define ADXL363_SENSOR        	    14
#define ADT7420_SENSOR     	        15
#define ADXL355_SENSOR         	    16
#define SHT25_SENSOR		        17
#define ADXL372_SENSOR         	    18
#define EKMB120_SENSOR         	    19					//Motion
#define ADT7320_SENSOR         	    20					//Temp
#define ADXRS290_SENSOR         	21					//Accel
#define ADIS16209_SENSOR         	22
#define MAX44009_SENSOR				23


/**
*********************************************************************************************************************************
*    SHT25 Definitions                                                                                                          *
********************************************************************************************************************************/
#define SHT25_DEV_ADDR	        	        	0x40	//Current Sensirion Device Addr =0x40

#define RH_MEAS_CONST                           (-6)
#define T_MEAS_CONST                            (-46.85)
#define RH_MEAS_MULT_CONST12                    0.03051 // (125/2^RES); RES=12-bit for RH
#define T_MEAS_MULT_CONST14                     0.01054 // (172.72/2^RES); RES=14-bit for Temp
#define SHT25_I2C_WRITE_ADDR	                0x80	// (SHT25_DEV_ADDR<<1)|0
#define SHT25_I2C_READ_ADDR	                	0x81	// (SHT25_DEV_ADDR<<1)|1

#define SHT25_Delay		                		0x1F //0x1FFF  //how much?????????????

/* SHT25_Sensor Register Address Definitions	*/
    #define SHT25_TRIG_T_MEAS_HOLDMASTER		0xE3
    #define SHT25_TRIG_RH_MEAS_HOLDMASTER		0xE5
    #define SHT25_WRITE_USER_REGISTER	        0xE6
    #define SHT25_READ_USER_REGISTER	        0xE7
    #define SHT25_TRIG_T_MEAS_NO_HOLDMASTER		0xF3	//POLL MODE
    #define SHT25_TRIG_RH_MEAS_NO_HOLDMASTER    0xF5	//POLL MODE
    #define SHT25_SOFT_RESET_REGISTER	        0xFE

/* Set bit resolution to measure RH & T in the right way as per spec. (bit#7 & bit#0), from table 5.6   */
    #define SHT25_RESOLUTION_RH12BIT_T14BIT     0x00   //12-bit RH & 14-bit Temperature (DEFAULT)
    #define SHT25_RESOLUTION_RH8BIT_T12BIT      0x01   //8-bit RH & 12-bit Temperature
    #define SHT25_RESOLUTION_RH10BIT_T13BIT     0x80   //10-bit RH & 13-bit Temperature
    #define SHT25_RESOLUTION_RH11BIT_T11BIT     0x81   //12-bit RH & 14-bit Temperature

/* End of battery (EoB) life? -> bit#6  */
    #define SHT25_END_OF_BATTERY_ON             0x40


/**
*********************************************************************************************************************************
*    ADT7320 Definitions                                                                                                        *
********************************************************************************************************************************/
/*      ADT7320_Sensor Register Address Definitions      */
#define ADT7320_STATUS_REG                      0x00
#define ADT7320_CONFIG_REG                      0x01
#define ADT7320_TEMPVAL_REG                     0x02
#define ADT7320_ID_REG                          0x03
#define ADT7320_CRIT_TEMP_SET_REG               0x04
#define ADT7320_HYST_TEMP_SET_REG               0x05
#define ADT7320_HIGH_TEMP_SET_REG               0x06
#define ADT7320_LOW_TEMP_SET_REG                0x07

/*      ADT7320_Sensor Register Init Values      */
#define ADT7320_STATUS_DEFVAL                   0x80
#define ADT7320_CONFIG_DEFVAL                   0xC0   //0x80  //0xA0
#define ADT7320_TEMPVAL_DEFVAL                  0x0000
#define ADT7320_ID_DEFVAL                       0xC3
#define ADT7320_CRIT_TEMP_SET_DEFVAL            0x4980
#define ADT7320_HYST_TEMP_SET_DEFVAL            0x05
#define ADT7320_HIGH_TEMP_SET_DEFVAL            0x2000
#define ADT7320_LOW_TEMP_SET_DEFVAL             0x0500

/**
*********************************************************************************************************************************
*    ADXL362 Definitions                                                                                                        *
********************************************************************************************************************************/

/* --- command --- */
    #define ADI_ADXL362_REG_WRITE   		0x0a
    #define ADI_ADXL362_REG_READ    		0x0b
    #define ADI_ADXL362_FIFO_READ   		0x0d

/* ------- Register names ------- */
/* -- part ID constants -- */
    #define ADI_ADXL362_DEVID_AD           0x00
    #define ADI_ADXL362_DEVID_MST          0x01
    #define ADI_ADXL362_PARTID             0x02
    #define ADI_ADXL362_REVID              0x03
    #define ADI_ADXL362_XID                0x04

/* -- RO data -- */
    #define ADI_ADXL362_XDATA8             0x08
    #define ADI_ADXL362_YDATA8             0x09
    #define ADI_ADXL362_ZDATA8             0x0A
    #define ADI_ADXL362_STATUS             0x0B
    #define ADI_ADXL362_FIFO_ENTRIES_L     0x0C
    #define ADI_ADXL362_FIFO_ENTRIES_H     0x0D
    #define ADI_ADXL362_XDATAL             0x0e
    #define ADI_ADXL362_XDATAH             0x0f
    #define ADI_ADXL362_YDATAL             0x10
    #define ADI_ADXL362_YDATAH             0x11
    #define ADI_ADXL362_ZDATAL             0x12
    #define ADI_ADXL362_ZDATAH             0x13
    #define ADI_ADXL362_TEMPL              0x14
    #define ADI_ADXL362_TEMPH              0x15
    #define ADI_ADXL362_X_ADCL             0x16
    #define ADI_ADXL362_X_ADCH             0x17

/* -- Control and Config -- */
    #define ADI_ADXL362_SOFT_RESET         0x1f
    #define ADI_ADXL362_THRESH_ACTL        0x20
    #define ADI_ADXL362_THRESH_ACTH        0x21
    #define ADI_ADXL362_TIME_ACT           0x22
    #define ADI_ADXL362_THRESH_INACTL      0x23
    #define ADI_ADXL362_THRESH_INACTH      0x24
    #define ADI_ADXL362_TIME_INACTL        0x25
    #define ADI_ADXL362_TIME_INACTH        0x26
    #define ADI_ADXL362_ACT_INACT_CTL      0x27
    #define ADI_ADXL362_FIFO_CONTROL       0x28
    #define ADI_ADXL362_FIFO_SAMPLES       0x29
    #define ADI_ADXL362_INTMAP1            0x2a
    #define ADI_ADXL362_INTMAP2            0x2b
    #define ADI_ADXL362_FILTER_CTL         0x2c
    #define ADI_ADXL362_POWER_CTL          0x2d
    #define ADI_ADXL362_SELF_TEST          0x2e

/*----------------------------------------------------------------------
Bit field definitions and register values
----------------------------------------------------------------------*/
/* register values for DEVID                                            */
/* The device ID should always read this value, The customer does not need to use
this value but it can be samity checked to check that the device can communicate  */
    #define ADI_ADXL362_ID              0xe2

/* Key Value in soft reset                                              */
    #define ADI_ADXL362_SOFT_RESET_KEY      0x52

/* Registers THRESH_ACTL through TIME_INACTH just take values in lsbs or samples.
There are no specific bit fields in these registers                  */

/* Bit values in ACT_INACT_CTL  (6 bits allocated)                      */
    #define ADI_ADXL362_ACT_ENABLE      0x01
    #define ADI_ADXL362_ACT_DISABLE     0x00
    #define ADI_ADXL362_ACT_AC          0x02
    #define ADI_ADXL362_ACT_DC          0x00
    #define ADI_ADXL362_INACT_ENABLE    0x04
    #define ADI_ADXL362_INACT_DISABLE   0x00
    #define ADI_ADXL362_INACT_AC        0x08
    #define ADI_ADXL362_INACT_DC        0x00
    #define ADI_ADXL362_ACT_INACT_LINK  0x10
    #define ADI_ADXL362_ACT_INACT_LOOP  0x20

/* Bit values in FIFO_CTL  (4 bits allocated)                            */
    #define ADI_ADXL362_FIFO_MODE_OFF     0x00
    #define ADI_ADXL362_FIFO_MODE_FIFO    0x01
    #define ADI_ADXL362_FIFO_MODE_STREAM  0x02
    #define ADI_ADXL362_FIFO_MODE_TRIGGER 0x03
    #define ADI_ADXL362_FIFO_TEMP         0x04
    #define ADI_ADXL362_FIFO_SAMPLES_AH   0x08

/* bit values in INTMAP1 and INTMAP2 (and status)                        */
    #define ADI_ADXL362_INT_DATA_READY     0x01
    #define ADI_ADXL362_INT_FIFO_READY     0x02
    #define ADI_ADXL362_INT_FIFO_WATERMARK 0x04
    #define ADI_ADXL362_INT_FIFO_OVERRUN   0x08
    #define ADI_ADXL362_INT_ACT            0x10
    #define ADI_ADXL362_INT_INACT          0x20
    #define ADI_ADXL362_INT_AWAKE          0x40
    #define ADI_ADXL362_INT_LOW            0x80

/* Bit values in FILTER_CTL  (8 bits allocated)                            */
    #define ADI_ADXL362_RATE_400        0x05
    #define ADI_ADXL362_RATE_200        0x04
    #define ADI_ADXL362_RATE_100        0x03  /* default */
    #define ADI_ADXL362_RATE_50         0x02
    #define ADI_ADXL362_RATE_25         0x01
    #define ADI_ADXL362_RATE_12_5       0x00

    #define ADI_ADXL362_EXT_TRIGGER     0x08

    #define ADI_ADXL362_AXIS_X          0x00
    #define ADI_ADXL362_AXIS_Y          0x10
    #define ADI_ADXL362_AXIS_Z          0x20

    #define ADI_ADXL362_RANGE_2G        0x00
    #define ADI_ADXL362_RANGE_4G        0x40
    #define ADI_ADXL362_RANGE_8G        0x80

/* Bit values in POWER_CTL  (8 bits allocated)                            */
    #define ADI_ADXL362_STANDBY           0x00
    #define ADI_ADXL362_MEASURE_1D        0x01
    #define ADI_ADXL362_MEASURE_3D        0x02
    #define ADI_ADXL362_AUTO_SLEEP        0x04
    #define ADI_ADXL362_SLEEP             0x08
    #define ADI_ADXL362_WAKEUP            0x0A

    #define ADI_ADXL362_LOW_POWER         0x00
    #define ADI_ADXL362_LOW_NOISE1        0x10
    #define ADI_ADXL362_LOW_NOISE2        0x20
    #define ADI_ADXL362_LOW_NOISE3        0x30

    #define ADI_ADXL362_EXT_CLOCK         0x40
    #define ADI_ADXL362_EXT_ADC           0x80

    /* Bit values in SELF_TEST  (1 bit allocated)                            */
    #define ADI_ADXL362_SELFTEST_ON       0x01
    #define ADI_ADXL362_SELFTEST_OFF      0x00



/* ==================================================== */
/*						ADXL363							*/
/* ==================================================== */

/* --- command --- */
#define ADI_ADXL363_REG_WRITE   		0x0A
#define ADI_ADXL363_REG_READ    		0x0B
#define ADI_ADXL363_FIFO_READ   		0x0D

/* Register address */
/*! \cond PRIVATE */
#define ADI_ADXL363_ADI_DEVID           0x00u   /* Analog Devices, Inc., accelerometer ID */
#define ADI_ADXL363_MEMS_DEVID          0x01u   /* Analog Devices MEMS device ID */
#define ADI_ADXL363_DEVID               0x02u   /* Device ID */
#define ADI_ADXL363_REVID               0x03u   /* product revision ID*/
#define ADI_ADXL363_XDATA               0x08u   /* X-axis acceleration data*/
#define ADI_ADXL363_YDATA               0x09u   /* Y-axis acceleration data*/
#define ADI_ADXL363_ZDATA               0x0Au   /* Z-axis acceleration data*/
#define ADI_ADXL363_STATUS              0x0Bu   /* Status register */
#define ADI_ADXL363_FIFO_ENTRIES_L      0x0Cu   /* Valid data samples in the FIFO-L */
#define ADI_ADXL363_FIFO_ENTRIES_H      0x0Du   /* Valid data samples in the FIFO-H */
#define ADI_ADXL363_DATAX_L             0x0Eu   /* X-axis acceleration data (lower part of 12 bit)*/
#define ADI_ADXL363_DATAX_H             0x0Fu   /* X-axis acceleration data (Higher part of 12 bit) */
#define ADI_ADXL363_DATAY_L             0x10u   /* Y-axis acceleration data (lower part of 12 bit)*/
#define ADI_ADXL363_DATAY_H             0x11u   /* Y-axis acceleration data (Higher part of 12 bit) */
#define ADI_ADXL363_DATAZ_L             0x12u   /* Z-axis acceleration data (lower part of 12 bit)*/
#define ADI_ADXL363_DATAZ_H             0x13u   /* Z-axis acceleration data (Higher part of 12 bit) */

#define ADI_ADXL363_TEMP_DATA_L         0x14u   /* Temperature sensor output data LSBs*/
#define ADI_ADXL363_TEMP_DATA_H         0x15u   /* Temperature sensor output data MSBs*/
#define ADI_ADXL363_DATAADC_L           0x16u   /* ADC-axis acceleration data (lower part of 12 bit)*/
#define ADI_ADXL363_DATAADC_H           0x17u   /* ADC-axis acceleration data (Higher part of 12 bit) */

#define ADI_ADXL363_SRESET              0x1Fu   /* Z axis offset */
#define ADI_ADXL363_THRESH_ACT_L        0x20u   /* Activity threshold LSBs*/
#define ADI_ADXL363_THRESH_ACT_H        0x21u   /* Activity threshold  MSBs*/
#define ADI_ADXL363_TIME_ACT            0x22u   /* Activity time */
#define ADI_ADXL363_THRESH_INACT_L      0x23u   /* Inactivity threshold LSBs */
#define ADI_ADXL363_THRESH_INACT_H      0x24u   /* Inactivity threshold MSBs*/
#define ADI_ADXL363_TIME_INACT_L        0x25u   /* Inctivity time LSBs*/
#define ADI_ADXL363_TIME_INACT_H        0x26u   /* Inctivity time MSBs*/
#define ADI_ADXL363_ACT_INACT_CTL       0x27u   /* Act/Inact control  */
#define ADI_ADXL363_FIFO_CTL            0x28u   /* FIFO control */
#define ADI_ADXL363_FIFO_SAMPLES        0x29u   /* FIFO samples */
#define ADI_ADXL363_INT1_MAP            0x2Au   /* Interrupt-1 mapping control */
#define ADI_ADXL363_INT2_MAP            0x2Bu   /* Interrupt-2 mapping control */
#define ADI_ADXL363_FILTER_CTL          0x2Cu   /* Filter control*/
#define ADI_ADXL363_POWER_CTL           0x2Du   /* Power control */
#define ADI_ADXL363_SELF_TEST           0x2Eu   /* Self test */



/* ==================================================== *
     Register fields position, masks and enumerations
 * ==================================================== */
#define ADXL363_SOFT_RESET_KEY      0x52

/* ACT_INACT_CTL register */
#define BITP_ADXL363_ACT_EN             0x00u   /* Bit position for Activity Enable.*/
#define BITM_ADXL363_ACT_EN             0x01u   /* Bit mask for Activity Enable. */
#define BITP_ADXL363_ACT_REF            0x01u   /* Bit position for activity absolute/reference*/
#define BITM_ADXL363_ACT_REF            0x02u   /* Bit mask for activity absolute/reference*/
#define BITP_ADXL363_INACT_EN           0x02u   /* DC or AC coupled activity detection operation */
#define BITM_ADXL363_INACT_EN           0x04u   /* Bit mask for Activity ACDC */
#define BITP_ADXL363_INACT_REF          0x03u   /* Bit position for activity absolute/reference */
#define BITM_ADXL363_INACT_REF          0x08u   /* Bit mask for Inactivity absolute/reference */
#define BITP_ADXL363_LINK_LOOP          0x04u   /* Bit position for Link-Loop mode */
#define BITM_ADXL363_LINK_LOOP          0x30u   /* Bit mask for Link-Loop mode */

#define BITP_ADXL363_FIFO_MODE          0x00u   /* Bit position for FIFO mode */
#define BITM_ADXL363_FIFO_MODE          0x03u   /* Bit mask for for FIFO mode  */
#define BITP_ADXL363_TEMP_FIFO_EN       0x02u   /* Bit position for enabling storing temperature data in the FIFO */
#define BITM_ADXL363_TEMP_FIFO_EN       0x04u   /* Bit mask for enabling storing temperature data in the FIFO  */
#define BITP_ADXL363_FIFO_HALF          0x03u   /* Bit position for enabling storing temperature data in the FIFO */
#define BITM_ADXL363_FIFO_HALF          0x08u   /* Bit mask for enabling storing temperature data in the FIFO  */


#define BITP_ADXL363_ODR                0x00u   /* Bit position for Output Data Rate. */
#define BITM_ADXL363_ODR                0x07u   /* Bit mask for Output Data Rate. */
#define BITP_ADXL363_EXT_SAMLING_EN     0x03u   /* Bit position for External Sampling Trigger */
#define BITM_ADXL363_EXT_SAMLING_EN     0x08u   /* Bit mask for External Sampling Trigger */
#define BITP_ADXL363_HALF_BW_EN         0x04u   /* Bit position for Halved Bandwidth */
#define BITM_ADXL363_HALF_BW_EN         0x10u   /* Bit mask for Halved Bandwidth */
#define BITP_ADXL363_RNAGE              0x06u   /* Bit position for Measurement Range */
#define BITM_ADXL363_RANGE              0xC0u   /* Bit mask for Measurement Range */

#define BITP_ADXL363_MEASURE_EN         0x00u   /* Bit position for Measurement Mode */
#define BITM_ADXL363_MEASURE_EN         0x03u   /* Bit mask  for Measurement Mode*/
#define BITP_ADXL363_AUTO_SLEEP         0x02u   /* Bit position for Autosleep. */
#define BITM_ADXL363_AUTO_SLEEP         0x04u   /* Bit mask for Autosleep. */
#define BITP_ADXL363_WAKE_UP            0x03u   /* Bit position forWake-Up Mode. */
#define BITM_ADXL363_WAKE_UP            0x08u   /* Bit mask forWake-Up Mode. */
#define BITP_ADXL363_LOW_NOISE          0x04u   /* Bit position for Power vs. Noise level  */
#define BITM_ADXL363_LOW_NOISE          0x30u   /* Bit mask for Power vs. Noise level */
#define BITP_ADXL363_EXT_CLK_EN         0x06u   /* Bit position for External Clock */
#define BITM_ADXL363_EXT_CLK_EN         0x40u   /* Bit mask for External Clock */
#define BITP_ADXL363_ADC_EN             0x07u   /* Bit position for ADC Enable. */
#define BITM_ADXL363_ADC_EN             0x80u   /* Bit mask for ADC Enable. */

#define BITP_ADXL363_INT_DATA_READY     0x00u   /* Bit position for Data Ready Interrupt. */
#define BITM_ADXL363_INT_DATA_READY     0x01u   /* Bit mask  for for Data Ready Interrupt.*/
#define BITP_ADXL363_INT_FIFO_READY     0x01u   /* Bit position  for FIFO Ready Interrupt*/
#define BITM_ADXL363_INT_FIFO_READY     0x02u   /* Bit mask  for FIFO Ready Interrupt*/
#define BITP_ADXL363_INT_FIFO_WATERMART 0X02u   /* Bit position for FIFO Watermark Interrupt */
#define BITM_ADXL363_INT_FIFO_WATERMART 0X04u   /* Bit mask for FIFO Watermark Interrupt */
#define BITP_ADXL363_INT_FIFO_OVERRUN   0x03u   /* Bit position for FIFO Overrun Interrupt. */
#define BITM_ADXL363_INT_FIFO_OVERRUN   0x08u   /* Bit mask for for FIFO Overrun Interrupt. */
#define BITP_ADXL363_INT_ACTIVITY       0x04u   /* Bit position for Activity Interrupt. */
#define BITM_ADXL363_INT_ACTIVITY       0x10u   /* Bit mask for for Activity Interrupt. */
#define BITP_ADXL363_INT_INACTIVITY     0x05u   /* Bit position for Inactivity Interrupt. */
#define BITM_ADXL363_INT_INACTIVITY     0x20u   /* Bit mask for Inactivity Interrupt. */
#define BITP_ADXL363_INT_AWAKE          0x06u   /* Bit position for Awake Interrupt. */
#define BITM_ADXL363_INT_AWAKE          0x40u   /* Bit mask for Awake Interrupt. */
#define BITP_ADXL363_INT_LOW            0x07u   /* Bit position for Interrupt Active Low. */
#define BITM_ADXL363_INT_LOW            0x80u   /* Bit mask for Interrupt Active Low. */

#define BITP_ADXL363_SELFTEST_EN        0x07u   /* Bit position for Self Test. */
#define BITM_ADXL363_SELFTEST_EN        0x80u   /* Bit mask for Self Test. */



/* ==================================================== */
/*						ADXL355							*/
/* ==================================================== */
#define ADXL355_DEV_ADDR	0x1D

/* Register address */
/*! \cond PRIVATE */
#define ADI_ADXL355_ADI_DEVID           0x00u   /* Analog Devices, Inc., accelerometer ID */
#define ADI_ADXL355_MST_DEVID          	0x01u   /* Analog Devices MEMS device ID */
#define ADI_ADXL355_DEVID               0x02u   /* Device ID */
#define ADI_ADXL355_REVID               0x03u   /* product revision ID*/
#define ADI_ADXL355_STATUS              0x04u   /* Status register */
#define ADI_ADXL355_FIFO_ENTRIES		0x05u   /* Valid data samples in the FIFO */
#define ADI_ADXL355_TEMP_DATA_2         0x06u   /* Temperature sensor output data MSBs [11:8]*/
#define ADI_ADXL355_TEMP_DATA_1         0x07u   /* Temperature sensor output data LSBs [7:0]*/
#define ADI_ADXL355_XDATA3              0x08u   /* X-axis acceleration data [19:12]*/
#define ADI_ADXL355_XDATA2              0x09u   /* X-axis acceleration data [11:4]*/
#define ADI_ADXL355_XDATA1              0x0Au   /* X-axis acceleration data [3:0] | LSBs reserved*/
#define ADI_ADXL355_YDATA3				0x0Bu   /* Y-axis acceleration data [19:12]*/
#define ADI_ADXL355_YDATA2             	0x0Cu   /* Y-axis acceleration data [11:4]*/
#define ADI_ADXL355_YDATA1             	0x0Du   /* Y-axis acceleration data [3:0] | LSBs reserved*/
#define ADI_ADXL355_ZDATA3             	0x0Eu   /* Z-axis acceleration data [19:12]*/
#define ADI_ADXL355_ZDATA2             	0x0Fu   /* Z-axis acceleration data [11:4]*/
#define ADI_ADXL355_ZDATA1             	0x10u   /* Z-axis acceleration data [3:0] | LSBs reserved*/
#define ADI_ADXL355_OFFSET_X_H          0x1Eu   /* X axis offset [15:8] */
#define ADI_ADXL355_OFFSET_X_L          0x1Fu   /* X axis offset [7:0] */
#define ADI_ADYL355_OFFSET_Y_H          0x20u   /* Y axis offset [15:8] */
#define ADI_ADYL355_OFFSET_Y_L          0x21u   /* Y axis offset [7:0] */
#define ADI_ADZL355_OFFSET_Z_H          0x22u   /* Z axis offset [15:8] */
#define ADI_ADZL355_OFFSET_Z_L          0x23u   /* Z axis offset [7:0] */
#define ADI_ADXL355_ACT_EN				0x24u   /* Act Control  */
#define ADI_ADXL355_ACT_THRESH_H        0x25u   /* Activity threshold MSBs*/
#define ADI_ADXL355_ACT_THRESH_L        0x26u   /* Activity threshold LSBs*/
#define ADI_ADXL355_ACT_COUNT	        0x27u   /* Activity Count */
#define ADI_ADXL355_FILTER		        0x28u   /* Filter Options */
#define ADI_ADXL355_FIFO_SAMPLES        0x29u   /* FIFO samples */
#define ADI_ADXL355_INT_MAP             0x2Au   /* Interrupt mapping control */
#define ADI_ADXL355_SYNC	            0x2Bu   /* Sync */
#define ADI_ADXL355_RANGE				0x2Cu   /* Capture Range */
#define ADI_ADXL355_POWER_CTL           0x2Du   /* Power control */
#define ADI_ADXL355_SRESET              0x2Fu   /* Z axis offset */


/* ==================================================== */
/*						ADXL372							*/
/* ==================================================== */
#define ADXL372_DEV_ADDR	0x1D		//0x53

/* Register address */
/*! \cond PRIVATE */
#define ADI_ADXL372_ADI_DEVID           0x00u   /* Analog Devices, Inc., accelerometer ID */
#define ADI_ADXL372_MST_DEVID          	0x01u   /* Analog Devices MEMS device ID */
#define ADI_ADXL372_DEVID               0x02u   /* Device ID */
#define ADI_ADXL372_REVID               0x03u   /* product revision ID*/
#define ADI_ADXL372_STATUS_1            0x04u   /* Status register 1 */
#define ADI_ADXL372_STATUS_2            0x05u   /* Status register 2 */
#define ADI_ADXL372_FIFO_ENTRIES_2		0x06u   /* Valid data samples in the FIFO */
#define ADI_ADXL372_FIFO_ENTRIES_1		0x07u   /* Valid data samples in the FIFO */
#define ADI_ADXL372_X_DATA_H            0x08u   /* X-axis acceleration data [11:4] */
#define ADI_ADXL372_X_DATA_L            0x09u   /* X-axis acceleration data [3:0] | dummy LSBs */
#define ADI_ADXL372_Y_DATA_H            0x0Au   /* Y-axis acceleration data [11:4] */
#define ADI_ADXL372_Y_DATA_L			0x0Bu   /* Y-axis acceleration data [3:0] | dummy LSBs */
#define ADI_ADXL372_Z_DATA_H            0x0Cu   /* Z-axis acceleration data [11:4] */
#define ADI_ADXL372_Z_DATA_L            0x0Du   /* Z-axis acceleration data [3:0] | dummy LSBs */
#define ADI_ADXL372_X_MAXPEAK_H         0x15u   /* X-axis MaxPeak acceleration data [15:8] */
#define ADI_ADXL372_X_MAXPEAK_L         0x16u   /* X-axis MaxPeak acceleration data [7:0] */
#define ADI_ADXL372_Y_MAXPEAK_H        	0x17u   /* X-axis MaxPeak acceleration data [15:8] */
#define ADI_ADXL372_Y_MAXPEAK_L         0x18u   /* X-axis MaxPeak acceleration data [7:0] */
#define ADI_ADXL372_Z_MAXPEAK_H         0x19u   /* X-axis MaxPeak acceleration data [15:8] */
#define ADI_ADXL372_Z_MAXPEAK_L         0x1Au   /* X-axis MaxPeak acceleration data [7:0] */
#define ADI_ADXL372_OFFSET_X 	        0x20u   /* X axis offset */
#define ADI_ADXL372_OFFSET_Y    		0x21u   /* Y axis offset */
#define ADI_ADXL372_OFFSET_Z	        0x22u   /* Z axis offset */
#define ADI_ADXL372_X_THRESH_ACT_H      0x23u   /* X axis Activity Threshold [15:8] */
#define ADI_ADXL372_X_THRESH_ACT_L		0x24u   /* X axis Activity Threshold [7:0] */
#define ADI_ADXL372_Y_THRESH_ACT_H      0x25u   /* Y axis Activity Threshold [15:8] */
#define ADI_ADXL372_Y_THRESH_ACT_L      0x26u   /* Y axis Activity Threshold [7:0] */
#define ADI_ADXL372_Z_THRESH_ACT_H	    0x27u   /* Z axis Activity Threshold [15:8] */
#define ADI_ADXL372_Z_THRESH_ACT_L		0x28u   /* Z axis Activity Threshold [7:0] */
#define ADI_ADXL372_TIME_ACT	        0x29u   /* Activity Time */
#define ADI_ADXL372_X_THRESH_INACT_H    0x2Au   /* X axis Inactivity Threshold [15:8] */
#define ADI_ADXL372_X_THRESH_INACT_L	0x2Bu   /* X axis Inactivity Threshold [7:0] */
#define ADI_ADXL372_Y_THRESH_INACT_H    0x2Cu   /* Y axis Inactivity Threshold [15:8] */
#define ADI_ADXL372_Y_THRESH_INACT_L    0x2Du   /* Y axis Inactivity Threshold [7:0] */
#define ADI_ADXL372_Z_THRESH_INACT_H	0x2Eu   /* Z axis Inactivity Threshold [15:8] */
#define ADI_ADXL372_Z_THRESH_INACT_L	0x2Fu   /* Z axis Inactivity Threshold [7:0] */
#define ADI_ADXL372_TIME_INACT_H        0x30u   /* Inactivity Time [15:8] */
#define ADI_ADXL372_TIME_INACT_L        0x31u   /* Inactivity Time [7:0] */
#define ADI_ADXL372_X_THRESH_ACT2_H     0x32u   /* X axis Activity2 Threshold [15:8] */
#define ADI_ADXL372_X_THRESH_ACT2_L		0x33u   /* X axis Activity2 Threshold [7:0] */
#define ADI_ADXL372_Y_THRESH_ACT2_H     0x34u   /* Y axis Activity2 Threshold [15:8] */
#define ADI_ADXL372_Y_THRESH_ACT2_L     0x35u   /* Y axis Activity2 Threshold [7:0] */
#define ADI_ADXL372_Z_THRESH_ACT2_H	    0x36u   /* Z axis Activity2 Threshold [15:8] */
#define ADI_ADXL372_Z_THRESH_ACT2_L		0x37u   /* Z axis Activity2 Threshold [7:0] */
#define ADI_ADXL372_HPF 	            0x38u   /* High Pass Filter */
#define ADI_ADXL372_FIFO_SAMPLES        0x39u   /* FIFO Samples */
#define ADI_ADXL372_FIFO_CTL	        0x3Au   /* FIFO Control */
#define ADI_ADXL372_INT1_MAP            0x3Bu   /* Interrupt 1 mapping control */
#define ADI_ADXL372_INT2_MAP            0x3Cu   /* Interrupt 2 mapping control */
#define ADI_ADXL372_TIMING	            0x3Du   /* Timing */
#define ADI_ADXL372_MEASURE				0x3Eu   /* Measure */
#define ADI_ADXL372_POWER_CTL           0x3Fu   /* Power control */
#define ADI_ADXL372_SELF_TEST           0x40u   /* Self Test */
#define ADI_ADXL372_SRESET              0x2Fu   /* Reset */
#define ADI_ADXL372_FIFO_DATA			0x2Fu   /* FIFO Data */

#define ADI_ADXL372_ADI_DEVID_VAL       0xADu   /* Analog Devices, Inc., accelerometer ID */
#define ADI_ADXL372_MST_DEVID_VAL       0x1Du   /* Analog Devices MEMS device ID */
#define ADI_ADXL372_DEVID_VAL           0xFAu   /* Device ID */
#define ADI_ADXL372_REVID_VAL           0x02u   /* product revision ID*/


/* ==================================================== */
/*						MAX44009						*/
/* ==================================================== */
#define MAX44009_DEV_ADDR			0x4A		//0x4B

#define MAX44009_INT_STATUS			0x00		//Interrupt Status
#define MAX44009_INT_ENABLE			0x01		//Interrupt Enable
#define MAX44009_CONFIG				0x02		//Configuration
#define MAX44009_LUX_HIGH			0x03		//Lux Reading High
#define MAX44009_LUX_LOW			0x04		//Lux Reading Low
#define MAX44009_UPPER_THRESHOLD	0x05		//Lux Reading High
#define MAX44009_LOWER_THRESHOLD	0x06		//Lux Reading High
#define MAX44009_THRESHOLD_TIMER	0x07		//Lux Reading High

#define MAX44009_LUX_LSB_VALUE      0.045 		//LSB = 0.045Lux


/* ==================================================== */
/*						BMP280							*/
/* ==================================================== */
#define BMP280_DEV_ADDR			0x76		//0x77
/************************************************/
/**\name	REGISTER ADDRESS DEFINITION       */
/***********************************************/
#define BMP280_CHIP_ID_REG                   (0xD0)  /*Chip ID Register */
#define BMP280_RST_REG                       (0xE0)  /*Softreset Register */
#define BMP280_STAT_REG                      (0xF3)  /*Status Register */
#define BMP280_CTRL_MEAS_REG                 (0xF4)  /*Ctrl Measure Register */
#define BMP280_CONFIG_REG                    (0xF5)  /*Configuration Register */
#define BMP280_PRESSURE_MSB_REG              (0xF7)  /*Pressure MSB Register */
#define BMP280_PRESSURE_LSB_REG              (0xF8)  /*Pressure LSB Register */
#define BMP280_PRESSURE_XLSB_REG             (0xF9)  /*Pressure XLSB Register */
#define BMP280_TEMPERATURE_MSB_REG           (0xFA)  /*Temperature MSB Reg */
#define BMP280_TEMPERATURE_LSB_REG           (0xFB)  /*Temperature LSB Reg */
#define BMP280_TEMPERATURE_XLSB_REG          (0xFC)  /*Temperature XLSB Reg */

/************************************************/
/**\name	CALIBRATION PARAMETERS DEFINITION       */
/***********************************************/
/*calibration parameters */
#define BMP280_TEMPERATURE_CALIB_DIG_T1_LSB_REG             (0x88)
#define BMP280_TEMPERATURE_CALIB_DIG_T1_MSB_REG             (0x89)
#define BMP280_TEMPERATURE_CALIB_DIG_T2_LSB_REG             (0x8A)
#define BMP280_TEMPERATURE_CALIB_DIG_T2_MSB_REG             (0x8B)
#define BMP280_TEMPERATURE_CALIB_DIG_T3_LSB_REG             (0x8C)
#define BMP280_TEMPERATURE_CALIB_DIG_T3_MSB_REG             (0x8D)
#define BMP280_PRESSURE_CALIB_DIG_P1_LSB_REG                (0x8E)
#define BMP280_PRESSURE_CALIB_DIG_P1_MSB_REG                (0x8F)
#define BMP280_PRESSURE_CALIB_DIG_P2_LSB_REG                (0x90)
#define BMP280_PRESSURE_CALIB_DIG_P2_MSB_REG                (0x91)
#define BMP280_PRESSURE_CALIB_DIG_P3_LSB_REG                (0x92)
#define BMP280_PRESSURE_CALIB_DIG_P3_MSB_REG                (0x93)
#define BMP280_PRESSURE_CALIB_DIG_P4_LSB_REG                (0x94)
#define BMP280_PRESSURE_CALIB_DIG_P4_MSB_REG                (0x95)
#define BMP280_PRESSURE_CALIB_DIG_P5_LSB_REG                (0x96)
#define BMP280_PRESSURE_CALIB_DIG_P5_MSB_REG                (0x97)
#define BMP280_PRESSURE_CALIB_DIG_P6_LSB_REG                (0x98)
#define BMP280_PRESSURE_CALIB_DIG_P6_MSB_REG                (0x99)
#define BMP280_PRESSURE_CALIB_DIG_P7_LSB_REG                (0x9A)
#define BMP280_PRESSURE_CALIB_DIG_P7_MSB_REG                (0x9B)
#define BMP280_PRESSURE_CALIB_DIG_P8_LSB_REG                (0x9C)
#define BMP280_PRESSURE_CALIB_DIG_P8_MSB_REG                (0x9D)
#define BMP280_PRESSURE_CALIB_DIG_P9_LSB_REG                (0x9E)
#define BMP280_PRESSURE_CALIB_DIG_P9_MSB_REG                (0x9F)

struct bmp280_calib_param_t {
	uint16_t dig_T1;/**<calibration T1 data*/
	int16_t dig_T2;/**<calibration T2 data*/
	int16_t dig_T3;/**<calibration T3 data*/
	uint16_t dig_P1;/**<calibration P1 data*/
	int16_t dig_P2;/**<calibration P2 data*/
	int16_t dig_P3;/**<calibration P3 data*/
	int16_t dig_P4;/**<calibration P4 data*/
	int16_t dig_P5;/**<calibration P5 data*/
	int16_t dig_P6;/**<calibration P6 data*/
	int16_t dig_P7;/**<calibration P7 data*/
	int16_t dig_P8;/**<calibration P8 data*/
	int16_t dig_P9;/**<calibration P9 data*/

	int32_t t_fine;/**<calibration t_fine data*/
};

/* numeric definitions */
#define	BMP280_PRESSURE_TEMPERATURE_CALIB_DATA_LENGTH	    (24)
#define	BMP280_GEN_READ_WRITE_DATA_LENGTH			(1)
#define	BMP280_TEMPERATURE_DATA_LENGTH				(3)
#define	BMP280_PRESSURE_DATA_LENGTH					(3)
#define	BMP280_ALL_DATA_FRAME_LENGTH				(6)
#define	BMP280_INIT_VALUE					(0)
#define	BMP280_INVALID_DATA					(0)

#define BMP280_ALL_DATA_FRAME_LENGTH	(6)
#define	BMP280_DATA_FRAME_SIZE			(6)

/* right shift definitions*/
#define BMP280_SHIFT_BIT_POSITION_BY_01_BIT				(1)
#define BMP280_SHIFT_BIT_POSITION_BY_02_BITS			(2)
#define BMP280_SHIFT_BIT_POSITION_BY_03_BITS			(3)
#define BMP280_SHIFT_BIT_POSITION_BY_04_BITS			(4)
#define BMP280_SHIFT_BIT_POSITION_BY_05_BITS			(5)
#define BMP280_SHIFT_BIT_POSITION_BY_08_BITS			(8)
#define BMP280_SHIFT_BIT_POSITION_BY_11_BITS			(11)
#define BMP280_SHIFT_BIT_POSITION_BY_12_BITS			(12)
#define BMP280_SHIFT_BIT_POSITION_BY_13_BITS			(13)
#define BMP280_SHIFT_BIT_POSITION_BY_14_BITS			(14)
#define BMP280_SHIFT_BIT_POSITION_BY_15_BITS			(15)
#define BMP280_SHIFT_BIT_POSITION_BY_16_BITS			(16)
#define BMP280_SHIFT_BIT_POSITION_BY_17_BITS			(17)
#define BMP280_SHIFT_BIT_POSITION_BY_18_BITS			(18)
#define BMP280_SHIFT_BIT_POSITION_BY_19_BITS			(19)
#define BMP280_SHIFT_BIT_POSITION_BY_25_BITS			(25)
#define BMP280_SHIFT_BIT_POSITION_BY_31_BITS			(31)
#define BMP280_SHIFT_BIT_POSITION_BY_33_BITS			(33)
#define BMP280_SHIFT_BIT_POSITION_BY_35_BITS			(35)
#define BMP280_SHIFT_BIT_POSITION_BY_47_BITS			(47)

/****************************************************/
/**\name	DEFINITIONS FOR ARRAY SIZE OF DATA   */
/***************************************************/
#define	BMP280_TEMPERATURE_DATA_SIZE	(3)
#define	BMP280_PRESSURE_DATA_SIZE		(3)
#define	BMP280_DATA_FRAME_SIZE			(6)
#define	BMP280_CALIB_DATA_SIZE			(24)

#define	BMP280_TEMPERATURE_MSB_DATA		(0)
#define	BMP280_TEMPERATURE_LSB_DATA		(1)
#define	BMP280_TEMPERATURE_XLSB_DATA	(2)

#define	BMP280_PRESSURE_MSB_DATA		(0)
#define	BMP280_PRESSURE_LSB_DATA		(1)
#define	BMP280_PRESSURE_XLSB_DATA	    (2)

#define	BMP280_DATA_FRAME_PRESSURE_MSB_BYTE	    (0)
#define	BMP280_DATA_FRAME_PRESSURE_LSB_BYTE		(1)
#define	BMP280_DATA_FRAME_PRESSURE_XLSB_BYTE	(2)
#define	BMP280_DATA_FRAME_TEMPERATURE_MSB_BYTE	(3)
#define	BMP280_DATA_FRAME_TEMPERATURE_LSB_BYTE	(4)
#define	BMP280_DATA_FRAME_TEMPERATURE_XLSB_BYTE	(5)

/****************************************************/
/**\name	ARRAY PARAMETER FOR CALIBRATION     */
/***************************************************/
#define	BMP280_TEMPERATURE_CALIB_DIG_T1_LSB		(0)
#define	BMP280_TEMPERATURE_CALIB_DIG_T1_MSB		(1)
#define	BMP280_TEMPERATURE_CALIB_DIG_T2_LSB		(2)
#define	BMP280_TEMPERATURE_CALIB_DIG_T2_MSB		(3)
#define	BMP280_TEMPERATURE_CALIB_DIG_T3_LSB		(4)
#define	BMP280_TEMPERATURE_CALIB_DIG_T3_MSB		(5)
#define	BMP280_PRESSURE_CALIB_DIG_P1_LSB       (6)
#define	BMP280_PRESSURE_CALIB_DIG_P1_MSB       (7)
#define	BMP280_PRESSURE_CALIB_DIG_P2_LSB       (8)
#define	BMP280_PRESSURE_CALIB_DIG_P2_MSB       (9)
#define	BMP280_PRESSURE_CALIB_DIG_P3_LSB       (10)
#define	BMP280_PRESSURE_CALIB_DIG_P3_MSB       (11)
#define	BMP280_PRESSURE_CALIB_DIG_P4_LSB       (12)
#define	BMP280_PRESSURE_CALIB_DIG_P4_MSB       (13)
#define	BMP280_PRESSURE_CALIB_DIG_P5_LSB       (14)
#define	BMP280_PRESSURE_CALIB_DIG_P5_MSB       (15)
#define	BMP280_PRESSURE_CALIB_DIG_P6_LSB       (16)
#define	BMP280_PRESSURE_CALIB_DIG_P6_MSB       (17)
#define	BMP280_PRESSURE_CALIB_DIG_P7_LSB       (18)
#define	BMP280_PRESSURE_CALIB_DIG_P7_MSB       (19)
#define	BMP280_PRESSURE_CALIB_DIG_P8_LSB       (20)
#define	BMP280_PRESSURE_CALIB_DIG_P8_MSB       (21)
#define	BMP280_PRESSURE_CALIB_DIG_P9_LSB       (22)
#define	BMP280_PRESSURE_CALIB_DIG_P9_MSB       (23)

/************************************************/
/**\name	POWER MODE DEFINITION       */
/***********************************************/
/* Sensor Specific constants */
#define BMP280_SLEEP_MODE                    (0x00)
#define BMP280_FORCED_MODE                   (0x01)
#define BMP280_NORMAL_MODE                   (0x03)
#define BMP280_SOFT_RESET_CODE               (0xB6)
/************************************************/
/**\name	STANDBY TIME DEFINITION       */
/***********************************************/
#define BMP280_STANDBY_TIME_1_MS              (0x00)
#define BMP280_STANDBY_TIME_63_MS             (0x01)
#define BMP280_STANDBY_TIME_125_MS            (0x02)
#define BMP280_STANDBY_TIME_250_MS            (0x03)
#define BMP280_STANDBY_TIME_500_MS            (0x04)
#define BMP280_STANDBY_TIME_1000_MS           (0x05)
#define BMP280_STANDBY_TIME_2000_MS           (0x06)
#define BMP280_STANDBY_TIME_4000_MS           (0x07)
/************************************************/
/**\name	OVERSAMPLING DEFINITION       */
/***********************************************/
#define BMP280_OVERSAMP_SKIPPED          (0x00)
#define BMP280_OVERSAMP_1X               (0x01)
#define BMP280_OVERSAMP_2X               (0x02)
#define BMP280_OVERSAMP_4X               (0x03)
#define BMP280_OVERSAMP_8X               (0x04)
#define BMP280_OVERSAMP_16X              (0x05)
/************************************************/
/**\name	WORKING MODE DEFINITION       */
/***********************************************/
#define BMP280_ULTRA_LOW_POWER_MODE          (0x00)
#define BMP280_LOW_POWER_MODE	             (0x01)
#define BMP280_STANDARD_RESOLUTION_MODE      (0x02)
#define BMP280_HIGH_RESOLUTION_MODE          (0x03)
#define BMP280_ULTRA_HIGH_RESOLUTION_MODE    (0x04)

#define BMP280_ULTRALOWPOWER_OVERSAMP_PRESSURE          BMP280_OVERSAMP_1X
#define BMP280_ULTRALOWPOWER_OVERSAMP_TEMPERATURE       BMP280_OVERSAMP_1X

#define BMP280_LOWPOWER_OVERSAMP_PRESSURE	             BMP280_OVERSAMP_2X
#define BMP280_LOWPOWER_OVERSAMP_TEMPERATURE	         BMP280_OVERSAMP_1X

#define BMP280_STANDARDRESOLUTION_OVERSAMP_PRESSURE     BMP280_OVERSAMP_4X
#define BMP280_STANDARDRESOLUTION_OVERSAMP_TEMPERATURE  BMP280_OVERSAMP_1X

#define BMP280_HIGHRESOLUTION_OVERSAMP_PRESSURE         BMP280_OVERSAMP_8X
#define BMP280_HIGHRESOLUTION_OVERSAMP_TEMPERATURE      BMP280_OVERSAMP_1X

#define BMP280_ULTRAHIGHRESOLUTION_OVERSAMP_PRESSURE       BMP280_OVERSAMP_16X
#define BMP280_ULTRAHIGHRESOLUTION_OVERSAMP_TEMPERATURE    BMP280_OVERSAMP_2X
/************************************************/
/**\name	FILTER DEFINITION       */
/***********************************************/
#define BMP280_FILTER_COEFF_OFF               (0x00)
#define BMP280_FILTER_COEFF_2                 (0x01)
#define BMP280_FILTER_COEFF_4                 (0x02)
#define BMP280_FILTER_COEFF_8                 (0x03)
#define BMP280_FILTER_COEFF_16                (0x04)
/************************************************/
/**\name	DELAY TIME DEFINITION       */
/***********************************************/
#define T_INIT_MAX							(20)
/* 20/16 = 1.25 ms */
#define T_MEASURE_PER_OSRS_MAX				(37)
/* 37/16 = 2.3125 ms*/
#define T_SETUP_PRESSURE_MAX				(10)
/* 10/16 = 0.625 ms */

/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/*																			SENSORS NOT MOUNTED ON CLUMP																   */
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************/

////ADXL363 Config in ADuCM3029 Example code
///*
////Default Registers
//	ADI_ADXL363_THRESH_ACT_L,	 ADXL363_CFG_ACTIVITY_THRESHOLD&0XFF,
//    ADI_ADXL363_THRESH_ACT_H,	(ADXL363_CFG_ACTIVITY_THRESHOLD&0X700)>>8,
//    ADI_ADXL363_TIME_ACT, 		 ADXL363_CFG_ACTIVITY_TIME&0XFF,
//    ADI_ADXL363_THRESH_INACT_L,  ADXL363_CFG_INACTIVITY_THRESHOLD&0XFF,
//    ADI_ADXL363_THRESH_INACT_H, (ADXL363_CFG_INACTIVITY_THRESHOLD&0X700)>>8,
//    ADI_ADXL363_TIME_INACT_L,	 ADXL363_CFG_INACTIVITY_TIME&0XFF,
//    ADI_ADXL363_TIME_INACT_H,	(ADXL363_CFG_INACTIVITY_TIME&0X700)>>8,
//    ADI_ADXL363_ACT_INACT_CTL,	(ADXL363_CFG_ENABLE_ACTIVITY    << BITP_ADXL363_ACT_EN    |
//                                 ADXL363_CFG_ACTIVITY_MODE      << BITP_ADXL363_ACT_REF   |
//                                 ADXL363_CFG_ENABLE_INACTIVITY  << BITP_ADXL363_INACT_EN  |
//                                 ADXL363_CFG_INACTIVITY_MODE    << BITP_ADXL363_INACT_REF |
//                                 ADXL363_CFG_LINK_LOOP_MODE     << BITP_ADXL363_LINK_LOOP ),
//
//    ADI_ADXL363_FIFO_CTL, 		(ADXL363_CFG_FIFO_MODE    			<< BITP_ADXL363_FIFO_MODE    |
//                              	 ADXL363_CFG_ENABLE_TEMPERATURE_FIFO   << BITP_ADXL363_TEMP_FIFO_EN),
//
//    ADI_ADXL363_FIFO_SAMPLES,	(ADXL363_CFG_FIFO_SIZE&0X0FF),
//    ADI_ADXL363_INT1_MAP,		(ADXL363_CFG_INT1_MAP&0X0FF),
//    ADI_ADXL363_INT2_MAP,		(ADXL363_CFG_INT2_MAP&0X0FF),
//    ADI_ADXL363_FILTER_CTL,		(ADXL363_CFG_OUTPUT_DATARATE <<BITP_ADXL363_ODR   	|
//                              	 ADXL363_CFG_FILTER_BW << BITP_ADXL363_HALF_BW_EN 	|
//                              	 ADXL363_CFG_MEASUREMENT_RANGE <<BITP_ADXL363_RNAGE),
//
//    ADI_ADXL363_POWER_CTL, (ADXL363_CFG_ENABLE_MEASUREMENT 	<<  BITP_ADXL363_MEASURE_EN |
//                             ADXL363_CFG_ENABLE_AUTOSLEEP   <<  BITP_ADXL363_AUTO_SLEEP |
//                             ADXL363_CFG_ENABLE_WAKEUP_MODE <<  BITP_ADXL363_WAKE_UP    |
//                             ADXL363_CFG_NOISE_MODE         <<  BITP_ADXL363_LOW_NOISE ),
//*/
//
//
/*!
*********************************************************************************************************************************
*    ADT75 Definitions                                                                                                          *
********************************************************************************************************************************/
#define ADT75_DEV_ADDR			0x4F  //J1, J2 & J3 in ADT75 Sensor shorted to A.

/*	ADT75_Sensor Register Address Definitions	*/
#define ADT75_TEMPVAL_REG_ADDR		0x00
#define ADT75_CONFIG_REG_ADDR      	0x01
#define ADT75_HYST_REG_ADDR	  	    0x02
#define ADT75_OVERTEMP_REG_ADDR	    0x03
#define ADT75_ONESHOT_REG_ADDR		0x04

/*	ADT75_Sensor Register Init Values	*/
#define ADT75_TEMPVAL_REG_DEFVAL	0x00
#define ADT75_CONFIG_REG_DEFVAL     0x00
#define ADT75_HYST_REG_DEFVAL	  	0x4B00  //0x1900
#define ADT75_OVERTEMP_REG_DEFVAL	0x5000
#define ADT75_ONESHOT_REG_DEFVAL	0x00


/*!
*********************************************************************************************************************************
*    ADT7420 Definitions                                                                                                          *
********************************************************************************************************************************/
#define ADT7420_DEV_ADDR					0x48  		// A1 and A0 are locked to GND

/*	ADT7420_Sensor Register Address Definitions	*/
#define ADT7420_TEMPVAL_MSB_ADDR			0x00
#define ADT7420_TEMPVAL_LSB_ADDR			0x01
#define ADT7420_STATUS_REG_ADDR     		0x02
#define ADT7420_CONFIG_REG_ADDR     		0x03
#define ADT7420_T_HIGH_SET_POINT_MSB_ADDR	0x04
#define ADT7420_T_HIGH_SET_POINT_LSB_ADDR	0x05
#define ADT7420_T_LOW_SET_POINT_MSB_ADDR	0x06
#define ADT7420_T_LOW_SET_POINT_LSB_ADDR	0x07
#define ADT7420_T_CRIT_SET_POINT_MSB_ADDR	0x08
#define ADT7420_T_CRIT_SET_POINT_LSB_ADDR	0x09
#define ADT7420_T_HYST_SET_POINT_ADDR		0x0A
#define ADT7420_ID_ADDR						0x0B
#define ADT7420_SOFT_RESET_ADDR				0x2F


/*	ADT7420_Sensor Register Init Values	*/
#define ADT7420_TEMPVAL_MSB_VAL				0x00
#define ADT7420_TEMPVAL_LSB_VAL				0x00
#define ADT7420_STATUS_REG_VAL     			0x00
#define ADT7420_CONFIG_REG_VAL     			0x00
#define ADT7420_T_HIGH_SET_POINT_MSB_VAL	0x20
#define ADT7420_T_HIGH_SET_POINT_LSB_VAL	0x00
#define ADT7420_T_LOW_SET_POINT_MSB_VAL		0x05
#define ADT7420_T_LOW_SET_POINT_LSB_VAL		0x00
#define ADT7420_T_CRIT_SET_POINT_MSB_VAL	0x49
#define ADT7420_T_CRIT_SET_POINT_LSB_VAL	0x80
#define ADT7420_T_HYST_SET_POINT_VAL		0x05
#define ADT7420_ID_VAL						0xCB
#define ADT7420_SOFT_RESET_VAL				0xFF


/**
*********************************************************************************************************************************
*    ADXL345 Definitions                                                                                                        *
********************************************************************************************************************************/
#define ADXL345_DEV_ADDR	                	0x1D    //SDO is shorted to Vcc in WSN-ADXL345. This is default address
                                                    //... if ALTADDRESS is chosen (pin12 to VCC), then address = 0x53
/*	ADXL345_Sensor Register Address Definitions	*/
    #define ADXL345_DEV_ID_REG_ADDR				0x00
    #define ADXL345_TAP_THRESH_REG_ADDR         0x1D
    #define ADXL345_X_OFFSET_REG_ADDR	        0x1E
    #define ADXL345_Y_OFFSET_REG_ADDR	        0x1F
    #define ADXL345_Z_OFFSET_REG_ADDR	        0x20
    #define ADXL345_TAP_DURATION_REG_ADDR		0x21
    #define ADXL345_TAP_LATENCY_REG_ADDR		0x22
    #define ADXL345_TAP_WINDOW_REG_ADDR         0x23
    #define ADXL345_THRESH_ACT_REG_ADDR	        0x24
    #define ADXL345_THRESH_INACT_REG_ADDR		0x25
    #define ADXL345_TIME_INACT_REG_ADDR	        0x26
    #define ADXL345_ACT_INACT_CTL_REG_ADDR		0x27
    #define ADXL345_FREEFALL_THRESH_REG_ADDR    0x28
    #define ADXL345_FREEFALL_TIME_REG_ADDR		0x29
    #define ADXL345_TAP_AXIS_REG_ADDR	        0x2A
    #define ADXL345_ACT_TAP_STATUS_REG_ADDR		0x2B
    #define ADXL345_BW_RATE_REG_ADDR	        0x2C
    #define ADXL345_PWR_CTL_REG_ADDR	        0x2D
    #define ADXL345_INT_ENA_REG_ADDR	        0x2E
    #define ADXL345_INT_MAP_REG_ADDR	        0x2F
    #define ADXL345_INT_SOURCE_REG_ADDR	        0x30
    #define ADXL345_DATA_FORMAT_REG_ADDR		0x31
    #define ADXL345_DATA_X0_REG_ADDR	        0x32
    #define ADXL345_DATA_X1_REG_ADDR	        0x33
    #define ADXL345_DATA_Y0_REG_ADDR	        0x34
    #define ADXL345_DATA_Y1_REG_ADDR	        0x35
    #define ADXL345_DATA_Z0_REG_ADDR	        0x36
    #define ADXL345_DATA_Z1_REG_ADDR	        0x37
    #define ADXL345_FIFO_CTL_REG_ADDR	        0x38
    #define ADXL345_FIFO_STATUS_REG_ADDR		0x39

/*	ADXL345_Sensor Register Init Values	*/
    #define ADXL345_DEV_ID_DEF_VAL				0xE5
    #define ADXL345_TAP_THRESH_DEF_VAL          15		//0x00
    #define ADXL345_X_OFFSET_DEF_VAL	        0x00
    #define ADXL345_Y_OFFSET_DEF_VAL	        0x00
    #define ADXL345_Z_OFFSET_DEF_VAL	        0x00
    #define ADXL345_TAP_DURATION_DEF_VAL		30		//0x00
    #define ADXL345_TAP_LATENCY_DEF_VAL	        100		//0x00
    #define ADXL345_TAP_WINDOW_DEF_VAL	        100		//0x00
    #define ADXL345_THRESH_ACT_DEF_VAL	        0x04	//0x00
    #define ADXL345_THRESH_INACT_DEF_VAL		0x02	//0x00
    #define ADXL345_TIME_INACT_DEF_VAL	        0x03	//0x00
    #define ADXL345_ACT_INACT_CTL_DEF_VAL		0xFF	//0x00
    #define ADXL345_FREEFALL_THRESH_DEF_VAL     0x06	//0x00
    #define ADXL345_FREEFALL_TIME_DEF_VAL		0x30	//0x00
    #define ADXL345_TAP_AXIS_DEF_VAL	        0x07	//0x00
    #define ADXL345_ACT_TAP_STATUS_DEF_VAL		0x0A	//0x00
#ifdef LOWPOWER_EVENT_MODE
    #define ADXL345_BW_RATE_DEF_VAL				0x07    //0x1A    //0x0A
#else
    #define ADXL345_BW_RATE_DEF_VAL				0x17    //0x1A    //0x0A
#endif
    #define ADXL345_PWR_CTL_DEF_VAL				0x0B	//0x00
    #define ADXL345_INT_ENA_DEF_VAL				0x10 //(0x88)	//0x00
    #define ADXL345_INT_MAP_DEF_VAL				0xFF	//0x00
    #define ADXL345_INT_SOURCE_DEF_VAL	        0x02
    #define ADXL345_DATA_FORMAT_DEF_VAL	        0x2B//For normal Data out  //0x2D- To Left justified Data Output
    #define ADXL345_DATA_X0_DEF_VAL				0x00
    #define ADXL345_DATA_X1_DEF_VAL				0x00
    #define ADXL345_DATA_Y0_DEF_VAL				0x00
    #define ADXL345_DATA_Y1_DEF_VAL				0x00
    #define ADXL345_DATA_Z0_DEF_VAL				0x00
    #define ADXL345_DATA_Z1_DEF_VAL				0x00
    #define ADXL345_FIFO_CTL_DEF_VAL            0x00
    #define ADXL345_FIFO_STATUS_DEF_VAL	        0x00



/**
*********************************************************************************************************************************
*    SHT21 Definitions                                                                                                          *
********************************************************************************************************************************/
#define SHT21_DEV_ADDR	        	        	0x40	//Current Sensirion Device Addr =0x40

#define RH_MEAS_CONST                           (-6)
#define T_MEAS_CONST                            (-46.85)
#define RH_MEAS_MULT_CONST12                    0.03051 // (125/2^RES); RES=12-bit for RH
#define T_MEAS_MULT_CONST14                     0.01054 // (172.72/2^RES); RES=14-bit for Temp
#define SHT21_I2C_WRITE_ADDR	                0x80	// (SHT21_DEV_ADDR<<1)|0
#define SHT21_I2C_READ_ADDR	                	0x81	// (SHT21_DEV_ADDR<<1)|1

#define SHT21_Delay		                		0x1F //0x1FFF  //how much?????????????

/* SHT21_Sensor Register Address Definitions	*/
    #define SHT21_TRIG_T_MEAS_HOLDMASTER		0xE3
    #define SHT21_TRIG_RH_MEAS_HOLDMASTER		0xE5
    #define SHT21_WRITE_USER_REGISTER	        0xE6
    #define SHT21_READ_USER_REGISTER	        0xE7
    #define SHT21_TRIG_T_MEAS_NO_HOLDMASTER		0xF3	//POLL MODE
    #define SHT21_TRIG_RH_MEAS_NO_HOLDMASTER    0xF5	//POLL MODE
    #define SHT21_SOFT_RESET_REGISTER	        0xFE

/* Set bit resolution to measure RH & T in the right way as per spec. (bit#7 & bit#0), from table 5.6   */
    #define SHT21_RESOLUTION_RH12BIT_T14BIT     0x00   //12-bit RH & 14-bit Temperature (DEFAULT)
    #define SHT21_RESOLUTION_RH8BIT_T12BIT      0x01   //8-bit RH & 12-bit Temperature
    #define SHT21_RESOLUTION_RH10BIT_T13BIT     0x80   //10-bit RH & 13-bit Temperature
    #define SHT21_RESOLUTION_RH11BIT_T11BIT     0x81   //12-bit RH & 14-bit Temperature

/* End of battery (EoB) life? -> bit#6  */
    #define SHT21_END_OF_BATTERY_ON             0x40


/**
*********************************************************************************************************************************
*    ADT7310 Definitions                                                                                                        *
********************************************************************************************************************************/
/*      ADT7310_Sensor Register Address Definitions      */
#define ADT7310_STATUS_REG                      0x00
#define ADT7310_CONFIG_REG                      0x01
#define ADT7310_TEMPVAL_REG                     0x02
#define ADT7310_ID_REG                          0x03
#define ADT7310_CRIT_TEMP_SET_REG               0x04
#define ADT7310_HYST_TEMP_SET_REG               0x05
#define ADT7310_HIGH_TEMP_SET_REG               0x06
#define ADT7310_LOW_TEMP_SET_REG                0x07

/*      ADT7310_Sensor Register Init Values      */
#define ADT7310_STATUS_DEFVAL                   0x80
#define ADT7310_CONFIG_DEFVAL                   0xC0   //0x80  //0xA0
#define ADT7310_TEMPVAL_DEFVAL                  0x0000
#define ADT7310_ID_DEFVAL                       0xC3
#define ADT7310_CRIT_TEMP_SET_DEFVAL            0x4980
#define ADT7310_HYST_TEMP_SET_DEFVAL            0x05
#define ADT7310_HIGH_TEMP_SET_DEFVAL            0x2000
#define ADT7310_LOW_TEMP_SET_DEFVAL             0x0500


/**
*********************************************************************************************************************************
*    AD7151 Definitions                                                                                                         *
********************************************************************************************************************************/
#define AD7151_DEV_ADDR			        		0x48
#define AD7151_DEV_ID			        		0x52

/*	AD7151_Sensor Register Address Definitions	*/
    #define AD7151_STATUS_REG_ADDR				0x00
    #define AD7151_DATA_HIGH_REG_ADDR   		0x01
    #define AD7151_DATA_LOW_REG_ADDR	  		0x02
    #define AD7151_AVG_HIGH_REG_ADDR	    	0x05
    #define AD7151_AVG_LOW_REG_ADDR				0x06
    #define AD7151_SENS_THD_HIGH_REG_ADDR		0x09
    #define AD7151_TIMEOUT_THD_LOW_REG_ADDR		0x0A
    #define AD7151_SETUP_REG_ADDR				0x0B
    #define AD7151_CONFIG_REG_ADDR				0x0F
    #define AD7151_PWR_DOWN_TMR_REG_ADDR		0x10
    #define AD7151_CAPDAC_REG_ADDR				0x11
    #define AD7151_SERIAL_NO_3_REG_ADDR			0x13
    #define AD7151_SERIAL_NO_2_REG_ADDR			0x14
    #define AD7151_SERIAL_NO_1_REG_ADDR			0x15
    #define AD7151_SERIAL_NO_0_REG_ADDR			0x16
    #define AD7151_CHIP_ID_REG_ADDR				0x17

/*	AD7151_Sensor Register Init Values	*/
    #define AD7151_STATUS_REG_VAL				0x00
    #define AD7151_DATA_HIGH_REG_VAL   			0x00
    #define AD7151_DATA_LOW_REG_VAL	  			0x00
    #define AD7151_AVG_HIGH_REG_VAL	    		0x00
    #define AD7151_AVG_LOW_REG_VAL				0x00
    #define AD7151_SENS_THD_HIGH_REG_VAL		0x08
    #define AD7151_TIMEOUT_THD_LOW_REG_VAL		0x86
    #define AD7151_SETUP_REG_VAL				0x0B
#ifdef LOWPOWER_EVENT_MODE
    #define AD7151_CONFIG_REG_VAL				0x11  //0x12  //0x13
#else
    #define AD7151_CONFIG_REG_VAL				0x11  //0x13  //0x12    //This can't set to 0x12 as the part goes to idle mode as soon as the conversion is finished.
#endif
    #define AD7151_PWR_DOWN_TMR_REG_VAL	  		0x00
    #define AD7151_CAPDAC_REG_VAL				0x00
    #define AD7151_SERIAL_NO_3_REG_VAL			0x00
    #define AD7151_SERIAL_NO_2_REG_VAL			0x00
    #define AD7151_SERIAL_NO_1_REG_VAL			0x00
    #define AD7151_SERIAL_NO_0_REG_VAL			0x00
    #define AD7151_CHIP_ID_REG_VAL				0x00


/******************************************************************************/
/*******************        VOS COMMAND DEFINITIONS       *********************/
/******************************************************************************/

#define COMMAND_SIZE                    4
#define DATA_LEN_SIZE                   1
#define COMMAND_SIZE                    4

/* Commands from master */
#define COMMAND_FIRSTBYTE              0x00
#define COMMAND_SECONDTBYTE            0x00
#define COMMAND_THIRDBYTE              0x00

#define COMMAND_START_MONITER          0x01
#define COMMAND_STOP_MONITER           0x0F
#define COMMAND_SEND_OCCUPANCY         0x02
#define COMMAND_SEND_COREUSAGE         0x04
#define COMMAND_SEND_DATAPKT           0x03

#define COMMAND_SLAVE_OCCUPANCY        0x81
#define COMMAND_SLAVE_CPUUSAGE         0x82

#define OCCUPANCY_ABSENT                  0
#define OCCUPANCY_PRESENT                 1
#define OCCUPANCY_UNDEFINED               2
/******************************************************************************/
