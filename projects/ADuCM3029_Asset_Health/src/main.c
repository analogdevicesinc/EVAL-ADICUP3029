/*****************************************************************************
 * ADuCM3029_ADXL372_demo.c
 *****************************************************************************/
#define ADI_ADXL372_DEMO
//#define ADI_ADXL362_DEMO

#include <sys/platform.h>
#include "adi_initialize.h"
#include "Communication.h"
#include "adsap_proto.h"
#include "Timer.h"
#include <drivers/gpio/adi_gpio.h>
#include <radio/adi_ble_radio.h>
#include <drivers/xint/adi_xint.h>
#include <common/adi_error_handling.h>
#include <common/adi_timestamp.h>
#include "math.h"
#include "adxl372.h"

//#define FIFO_READ
#define ADXL372_DEBUG

/*LCD backlight pin*/
#define BLLCD_PORT ADI_GPIO_PORT1
#define BLLCD_PIN  ADI_GPIO_PIN_12

/*DS3/DS4 led pin*/
#define DS3_PORT ADI_GPIO_PORT2
#define DS3_PIN  ADI_GPIO_PIN_0
#define DS4_PORT ADI_GPIO_PORT1
#define DS4_PIN  ADI_GPIO_PIN_15

#define PERIPHERAL_ADV_MODE      ((ADI_BLE_GAP_MODE)(ADI_BLE_GAP_MODE_NOTCONNECTABLE |  \
			                             ADI_BLE_GAP_MODE_DISCOVERABLE))

#define GENERIC_SENSOR_TYPE 0
#define CURRENT_DATE_TIME 1495715651 //25 May 2017 12:34 PM

uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE];
uint8_t xintMemory[ADI_XINT_MEMORY_SIZE];
uint8_t ui8Status2, ui8Status;
bool boInterruptFlag = false;
uint32_t LowPwrExitFlag;

static ADI_BLE_GAP_MODE   gGapMode;
static bool               gConnected, reg_flag;


__packed GenericReg_t generic_reg_pkt = {0x01, GENERIC_SENSOR_TYPE, "Magnitude[G]"};
__packed GenericReg_t generic1_reg_pkt = {0x02, GENERIC_SENSOR_TYPE, "Accel X Data[G]"};
__packed GenericReg_t generic2_reg_pkt = {0x03, GENERIC_SENSOR_TYPE, "Accel Y Data[G]"};
__packed GenericReg_t generic3_reg_pkt = {0x04, GENERIC_SENSOR_TYPE, "Accel Z Data[G]"};

__packed DataPacket_t generic_data_pkt = {0x81,GENERIC_SENSOR_TYPE,{0,0,0,0},0.123};
__packed DataPacket_t generic1_data_pkt = {0x82,GENERIC_SENSOR_TYPE,{0,0,0,0},0.123};
__packed DataPacket_t generic2_data_pkt = {0x83,GENERIC_SENSOR_TYPE,{0,0,0,0},0.125};
__packed DataPacket_t generic3_data_pkt = {0x84,GENERIC_SENSOR_TYPE,{0,0,0,0},0.125};

/*
 * GPIO event Callback function
 */
static void pinIntCallback(void* pCBParam, uint32_t Port,  void* PinIntData)
{
	boInterruptFlag = true;
	LowPwrExitFlag++;
}

/*!
 * @brief      Set Advertising Mode
 *
 * @details    Sets the bluetooth radio mode to discoverable and connectable mode.
 *             In this mode bluetooth device is visible to other central devices.
 *
 */
static void SetAdvertisingMode(void)
{
    ADI_BLER_RESULT eResult;

    eResult = adi_radio_SetMode(PERIPHERAL_ADV_MODE, 0u, 0u);
    PRINT_ERROR("Error setting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

    eResult = adi_ble_WaitForEventWithTimeout(GAP_EVENT_MODE_CHANGE, 5000u);
    PRINT_ERROR("Error waiting for GAP_EVENT_MODE_CHANGE.\r\n", eResult, ADI_BLER_SUCCESS);

    eResult = adi_radio_GetMode(&gGapMode);
    PRINT_ERROR("Error getting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

    if (gGapMode != PERIPHERAL_ADV_MODE)
    {
        PRINT_MESSAGE("Error in SetAdvertisingMode.\r\n");
    }
}

void adi_DataExchange_Callback(void *pParam, uint32_t Event, void *pData)
{
	switch (Event)
	{
	case BLE_PROFILE_TASK_CREATE:
		//adi_DataExchange_Create(pParam);
		break;

	case BLE_PROFILE_TASK_INIT:
		//adi_DataExchange_Init(pParam);
		break;

	case BLE_PROFILE_TASK_DESTROY:
		//adi_DataExchange_Destroy(pParam);
		break;

	case GAP_EVENT_DISCONNECTED:
	{
		//DEBUG_MSG("*************************\r\n");
		//DEBUG_MSG("ADICUP3029 - BLE Disconnect *******\r\n");
		gConnected = false;
		//toggle_red();
	}
	break;

	case GAP_EVENT_CONNECTED:
	{
		//DEBUG_MSG("ADICUP3029 - BLE Connection successful\r\n");
		//DEBUG_MSG( "\n");
		gConnected = true;
		//toggle_green();
	}
	break;

	case IMMEDIATE_ALERT_EVENT:
	{
		//DEBUG_MSG(" Data Exchange Server Alert Event\r\n");
		//DEBUG_MSG( "\n");
	}
	break;


	case DATA_EXCHANGE_TX_COMPLETE:
	{
		//DEBUG_MSG(" - BLE Tx Success \r\n");
		if (reg_flag==0)
		{
			reg_flag=1;
		}
	}
	break;

	case DATA_EXCHANGE_RX_EVENT:
	{
		//DEBUG_MSG("Data Exchange Server RX Exchange Event\r\n");
		//DEBUG_MSG( "\n");
	}
	break;
	case BLE_RADIO_ERROR_READING:
	{
		reg_flag = 0;
		gConnected = false;
	}
	break;
	case BLE_RESPONSE_FAILURE:
	{
		gConnected = false;
		reg_flag = 0;
	}
	break;
	default:
		break;

	}
	return;
}

int main(int argc, char *argv[])
{
	ADSENSORAPP_RESULT_TYPE enResult;
    ADI_BLER_RESULT      eResult;
	ADI_BLE_GAP_MODE     eMode;
	ADI_BLER_EVENT       eEvent;
	uint8_t *       pDeviceName = (unsigned char *) "ADI_ASSET_HEALTH_MONITOR_JW";
	uint32_t u32RTCTime;
	uint8_t u8FirstTimeConn = 0;


	timer_start(); // Start timer

	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	adi_initpinmux();

	adsAPI_Init_Devices();

	/* Initialize UART at 9600 baudrate */
	UART_Init();
	AppPrintf("UART IOT drivers test\n\r");

	/*Initialize RTC*/
	adi_RTCInit();

	/*Verify the interface between ADICUP3029 and ADXL372*/
	enResult = Detect_ADXL372_Sensor();

	if (enResult != ADI_ADS_API_SUCCESS)
	{
		AppPrintf("Error communicating to ADXL372\n\r");
	}

	/* Set interrupt pin */
	adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE); //initialize gpio
	adi_gpio_OutputEnable(INTACC_PORT, INTACC_PIN, false);
	adi_gpio_InputEnable(INTACC_PORT, INTACC_PIN, true);    // Set INTACC_PORT as input
	adi_gpio_PullUpEnable(INTACC_PORT, INTACC_PIN, false); 	// Disable pull-up resistors
	adi_gpio_SetGroupInterruptPolarity(INTACC_PORT, INTACC_PIN);
	adi_gpio_SetGroupInterruptPins(INTACC_PORT,  SYS_GPIO_INTA_IRQn, INTACC_PIN);
	adi_gpio_RegisterCallback (SYS_GPIO_INTA_IRQn, pinIntCallback, NULL );

	/* Turn off LCD backlight */
	adi_gpio_OutputEnable(BLLCD_PORT, BLLCD_PIN, true);
	adi_gpio_SetLow(BLLCD_PORT, BLLCD_PIN);

	/*Turn off leds DS3 and DS4*/
	adi_gpio_OutputEnable(DS3_PORT, DS3_PIN, true);
	adi_gpio_SetLow(DS3_PORT, DS3_PIN);
	adi_gpio_OutputEnable(DS4_PORT, DS4_PIN, true);
	adi_gpio_SetLow(DS4_PORT, DS4_PIN);

	/* Configure radio */
	eResult = adi_ble_Init(adi_DataExchange_Callback, NULL);
	PRINT_ERROR("Error initializing the radio.\r\n", eResult, ADI_BLER_SUCCESS);

	eResult = adi_radio_RegisterDevice(ADI_BLE_ROLE_PERIPHERAL);
	PRINT_ERROR("Error registering the radio.\r\n", eResult, ADI_BLER_SUCCESS);

	eResult = adi_radio_SetLocalBluetoothDevName((uint8_t* const)pDeviceName, strlen((const char*)pDeviceName), 0u, 0u);
	PRINT_ERROR("Error setting local device name.\r\n", eResult, ADI_BLER_SUCCESS);

	eResult = adi_radio_Register_DataExchangeServer();
	PRINT_ERROR("adi_radio_Register_DataExchange\r\n",eResult,ADI_BLER_SUCCESS);

	/* Start advertising */
	 eResult  = adi_radio_SetMode((ADI_BLE_GAP_MODE)(ADI_BLE_GAP_MODE_CONNECTABLE |ADI_BLE_GAP_MODE_DISCOVERABLE),0,0);
	 PRINT_ERROR("adi_radio_SetMode\r\n",eResult,ADI_BLER_SUCCESS);

	/* Set impact detection mode */
	ADXL372_Set_Impact_Detection();

	ADI_BLER_CONN_INFO connInfo = {0};
	ADI_BLER_EVENT BleEvent;
	while(1)
	{
		if (!gConnected)
				{
					BleEvent = adi_radio_GetEvent();
					eResult  = adi_radio_GetMode(&gGapMode);
					if (gGapMode != (ADI_BLE_GAP_MODE_CONNECTABLE | ADI_BLE_GAP_MODE_DISCOVERABLE))
					{
						eResult  = adi_radio_SetMode((ADI_BLE_GAP_MODE)(ADI_BLE_GAP_MODE_CONNECTABLE |ADI_BLE_GAP_MODE_DISCOVERABLE),0,0);
						PRINT_ERROR("adi_radio_SetMode",eResult,ADI_BLER_SUCCESS);
						u8FirstTimeConn = 0;
						reg_flag = 0;
					}
				}
				else
				{

					eResult  = adi_radio_GetMode(&gGapMode);
					BleEvent = adi_radio_GetEvent();


					adi_ble_GetConnectionInfo(&connInfo);

					adi_ble_GetConnectionInfo(&connInfo); //get connection handle

					if(reg_flag==0)
					{
						/*Accelerometer Registration packet*/
						timer_sleep(1000);
						eResult = adi_radio_DE_SendData(connInfo.nConnHandle,sizeof(generic_reg_pkt),(uint8_t*)&generic_reg_pkt);
						eResult = adi_radio_DE_SendData(connInfo.nConnHandle,sizeof(generic1_reg_pkt),(uint8_t*)&generic1_reg_pkt);
						eResult = adi_radio_DE_SendData(connInfo.nConnHandle,sizeof(generic2_reg_pkt),(uint8_t*)&generic2_reg_pkt);
						eResult = adi_radio_DE_SendData(connInfo.nConnHandle,sizeof(generic3_reg_pkt),(uint8_t*)&generic3_reg_pkt);
					}

				}

		/* Measurement mode */
		if (boInterruptFlag) {
			/*Clear interrupt on ADXL and reenter instant-on mode*/
			adxl372_Get_Status_Register(&ui8Status);
			adxl372_Get_ActivityStatus_Register(&ui8Status2);

			/*Read data from accelerometer*/
			timer_sleep (50);
			Get_Data_From_ADXL372();

			/*Print data over UART*/
			u32RTCTime = CURRENT_DATE_TIME + adi_GetRTCTime();
			AppPrintf("x = % 5.2f G, y = % 5.2f G, z = % 5.2f G %d\n\r", (float)SensData.ADXL372[0], (float)SensData.ADXL372[1], (float)SensData.ADXL372[2], u32RTCTime);

			memcpy(&(generic_data_pkt.rtcTimestamp), &u32RTCTime, 4);
			memcpy(&(generic2_data_pkt.rtcTimestamp), &u32RTCTime, 4);
			memcpy(&(generic3_data_pkt.rtcTimestamp), &u32RTCTime, 4);

			generic_data_pkt.Sensor_Data = sqrt(SensData.ADXL372[0]*SensData.ADXL372[0] + SensData.ADXL372[1]*SensData.ADXL372[1] + SensData.ADXL372[2]*SensData.ADXL372[2]);
			generic1_data_pkt.Sensor_Data = SensData.ADXL372[0];
			generic2_data_pkt.Sensor_Data = SensData.ADXL372[1];
			generic3_data_pkt.Sensor_Data = SensData.ADXL372[2];

			if (gConnected)
			{
				BleEvent = adi_radio_GetEvent();
				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,sizeof(generic_data_pkt),(uint8_t*)&generic_data_pkt);

				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,sizeof(generic1_data_pkt),(uint8_t*)&generic1_data_pkt);

				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,sizeof(generic2_data_pkt),(uint8_t*)&generic2_data_pkt);

				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,sizeof(generic3_data_pkt),(uint8_t*)&generic3_data_pkt);
				timer_sleep (100);
			}

			adxl372_Set_Op_mode(INSTANT_ON);
			boInterruptFlag = false;
			LowPwrExitFlag = 0;
		}

		/* Enter Flexi mode - low power */
		if(reg_flag)
		{
			adi_pwr_EnterLowPowerMode ( ADI_PWR_MODE_FLEXI, &LowPwrExitFlag, 0u);
		}
	}
}

