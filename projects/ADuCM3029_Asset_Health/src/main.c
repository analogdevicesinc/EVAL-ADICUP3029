/***************************************************************************//**
 *   @file   main.c
 *   @brief  Implementation of main application file.
 *   @author Mircea Caprioru (mircea.caprioru@analog.com)
********************************************************************************
 * Copyright 2018(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/***************************** Include Files **********************************/
#include <sys/platform.h>
#include "adi_initialize.h"

#include <common/adi_timestamp.h>
#include "common.h"

#include "platform_drivers.h"
#include "adxl372.h"

#include "Communication.h"
#include "Timer.h"

#include "math.h"

/************************** Variable Definitions ******************************/
#define PEAK_ACCELERATION

#define GENERIC_SENSOR_TYPE 0
#define CURRENT_DATE_TIME 0 //25 May 2017 12:34 PM

uint8_t ui8Status2, ui8Status;
bool boInterruptFlag = false;
uint32_t LowPwrExitFlag;

static bool               gConnected, reg_flag;
ADI_BLER_CONN_INFO connInfo = {0};
ADI_BLER_EVENT BleEvent;

#ifdef PEAK_ACCELERATION
/*Structures for peak value acceleration*/
struct RegistrationPacket reg_pkt = {0x00, 3, 5, "ADXL372"};
struct FieldNamePacket name_pkt0 = {0x01, 0, "X axis [G]"};
struct FieldNamePacket name_pkt1 = {0x01, 1, "Y axis [G]"};
struct FieldNamePacket name_pkt2 = {0x01, 2, "Z axis [G]"};
struct DataPacket data_pkt = {0x02, 0, 0, 0, 0};

adxl372_init_param adxl372_default_init_param = {
	{GENERIC_SPI, 0, 10000000, SPI_MODE_0, ADI_SPI_CS1},    // spi_init
	1, 2,                // gpio_int1, gpio_int2
	ADXL372_BW_3200HZ,        // bw
	ADXL372_ODR_6400HZ,        // odr
	ADXL372_WUR_52ms,        // wur
	ADXL372_LOOPED,        // act_proc_mode
	ADXL372_INSTANT_ON_LOW_TH,    // th_mode
	{30, true, true},        // activity_th
	{0, false, false},        // activity2_th
	{30, true, true},        // inactivity_th
	0,                // activity_time
	0,                // inactivity_time
	ADXL372_FILTER_SETTLE_16,    // filter_settle
	{ADXL372_FIFO_OLD_SAVED, ADXL372_XYZ_PEAK_FIFO, 20},    // fifo_config
	/* data_rdy, fifo_rdy, fifo_full, fifo_ovr, inactivity, activity, awake, low_operation */
	{false, false, false, false, false, false, true, true},
	{false, false, false, false, false, false, false, false},
	ADXL372_FULL_BW_MEASUREMENT,    // op_mode
};

#else
/*Structures for full acceleration profile*/
struct RegistrationPacket reg_pkt_full = {0x04, 3, 5, "Full Accel"};
struct FieldNamePacket name_pkt0_full = {0x05, 0, "X axis [G]"};
struct FieldNamePacket name_pkt1_full = {0x05, 1, "Y axis [G]"};
struct FieldNamePacket name_pkt2_full = {0x05, 2, "Z axis [G]"};
struct DataPacket data_pkt_full = {0x06, 0, 0, 0, 0};

adxl372_init_param adxl372_default_init_param = {
	{GENERIC_SPI, 0, 10000000, SPI_MODE_0, ADI_SPI_CS1},    // spi_init
	1, 2,                // gpio_int1, gpio_int2
	ADXL372_BW_3200HZ,        // bw
	ADXL372_ODR_6400HZ,        // odr
	ADXL372_WUR_52ms,        // wur
	ADXL372_LOOPED,        // act_proc_mode
	ADXL372_INSTANT_ON_LOW_TH,    // th_mode
	{30, true, true},        // activity_th
	{0, false, false},        // activity2_th
	{30, true, true},        // inactivity_th
	0,                // activity_time
	0,                // inactivity_time
	ADXL372_FILTER_SETTLE_16,    // filter_settle
	{ADXL372_FIFO_OLD_SAVED, ADXL372_XYZ_FIFO, 20},    // fifo_config
	/* data_rdy, fifo_rdy, fifo_full, fifo_ovr, inactivity, activity, awake, low_operation */
	{false, false, false, false, false, false, true, true},
	{false, false, false, false, false, false, false, false},
	ADXL372_FULL_BW_MEASUREMENT,    // op_mode
};

#endif

/************************* Functions Definitions ******************************/
int16_t sign_extend16(uint16_t data);

/**
 * @brief Interrupt pin event Callback function.
 * @param pCBParam - pointer to parameters.
 * @param Port  - port that triggered the interrupt.
 * @param PinIntData  - pin interrupt data.
 * @return None.
 */
void pinIntCallback(void* pCBParam, uint32_t Port,  void* PinIntData)
{
	boInterruptFlag = true;
	LowPwrExitFlag++;
}

/**
 * @brief BLE callback function.
 * @param pParam - pointer to parameters.
 * @param Event  - event that is serviced by the callback.
 * @param pData  - data pointer.
 * @return None.
 */
void adi_DataExchange_Callback(void *pParam, uint32_t Event, void *pData)
{
	switch (Event) {
	case GAP_EVENT_DISCONNECTED:
		DEBUG_MESSAGE("Disconnected!\r\n");
		gGapMode      = ADI_BLE_GAP_MODE_NOTCONNECTABLE;
		gConnected = false;
		reg_flag = 0;
		break;
	case GAP_EVENT_CONNECTED:
		DEBUG_MESSAGE("Connected!\r\n");
		gConnected = true;
		break;
	case IMMEDIATE_ALERT_EVENT:
		break;
	case DATA_EXCHANGE_TX_COMPLETE:
		DEBUG_MESSAGE("Data sent!\r\n");
		if (reg_flag==0)
			reg_flag=1;
		break;
	case DATA_EXCHANGE_RX_EVENT:
		DEBUG_MESSAGE("Data received!\r\n");
		break;
	case BLE_RADIO_ERROR_READING:
		reg_flag = 0;
		break;
	case BLE_RESPONSE_FAILURE:
		reg_flag = 0;
		break;
	default:
		break;
	}

	return;
}

int main(int argc, char *argv[])
{
	ADI_BLER_RESULT      eResult;

	uint32_t u32RTCTime;

	adxl372_dev *adxl372 = malloc(sizeof(adxl372_dev));
	adxl372_xyz_accel_data max_peak;

	uint8_t status1;
	uint8_t status2;
	uint16_t fifo_entries;
	adxl372_xyz_accel_data fifo_samples[170];

	timer_start(); // Start timer

	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();

	init_gpio();

	adxl372_init(&adxl372, adxl372_default_init_param);
	timer_sleep(16);
	adxl372_set_op_mode(adxl372, ADXL372_INSTANT_ON);

	/* Initialize UART at 9600 baudrate */
	UART_Init();
	AppPrintf("UART IOT drivers test\n\r");

	/*Initialize RTC*/
	adi_RTCInit();

	configure_ble_radio();

	while(1) {
		eResult = adi_ble_DispatchEvents(500);
		DEBUG_RESULT("Error dispatching events to the callback.\r\n", eResult,
			     ADI_BLER_SUCCESS);

		if(gConnected) {
			adi_ble_GetConnectionInfo(&connInfo); //get connection handle

			if(reg_flag == 0)	{
				/*Accelerometer Registration packet*/
				timer_sleep(2000);
#ifdef PEAK_ACCELERATION
				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,
								sizeof(reg_pkt),(uint8_t*)&reg_pkt);
				timer_sleep(10);
				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,
								sizeof(name_pkt0),(uint8_t*)&name_pkt0);
				timer_sleep(10);
				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,
								sizeof(name_pkt1),(uint8_t*)&name_pkt1);
				timer_sleep(10);
				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,
								sizeof(name_pkt2),(uint8_t*)&name_pkt2);
				timer_sleep(10);
#else
				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,
								sizeof(reg_pkt_full),(uint8_t*)&reg_pkt_full);
				timer_sleep(10);
				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,
								sizeof(name_pkt0_full),(uint8_t*)&name_pkt0_full);
				timer_sleep(10);
				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,
								sizeof(name_pkt1_full),(uint8_t*)&name_pkt1_full);
				timer_sleep(10);
				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,
								sizeof(name_pkt2_full),(uint8_t*)&name_pkt2_full);
#endif
			}
		} else {
			/* If disconnected, switch to advertising mode */
			if(gGapMode != PERIPHERAL_MODE) {
				SetAdvertisingMode();
			}
		}

		/* Measurement mode */
		if (boInterruptFlag) {
			/*Read data from accelerometer*/
			timer_sleep (100);
			adxl372_get_status(adxl372, &status1, &status2, &fifo_entries);
			adxl372_get_highest_peak_data(adxl372, &max_peak);

			adxl372_get_fifo_xyz_data(adxl372, fifo_samples, fifo_entries);

			/*Print data over UART*/
			u32RTCTime = CURRENT_DATE_TIME + adi_GetRTCTime();

#ifdef PEAK_ACCELERATION
			data_pkt.Sensor_Data1.fValue =
				(float)sign_extend16(max_peak.x) * 0.1; //100 mg/LSB
			data_pkt.Sensor_Data2.fValue =
				(float)sign_extend16(max_peak.y) * 0.1;
			data_pkt.Sensor_Data3.fValue =
				(float)sign_extend16(max_peak.z) * 0.1;

			AppPrintf("x = % 5.2f G, y = % 5.2f G, z = % 5.2f G %d\n\r",
				  data_pkt.Sensor_Data1.fValue,
				  data_pkt.Sensor_Data2.fValue,
				  data_pkt.Sensor_Data3.fValue, u32RTCTime);
#endif
			if (gConnected) {
				BleEvent = adi_radio_GetEvent();

#ifdef PEAK_ACCELERATION
				eResult = adi_radio_DE_SendData(connInfo.nConnHandle,
								sizeof(data_pkt),(uint8_t*)&data_pkt);
				timer_sleep(10);
#else

				send_fifo_data(fifo_samples, 40);
				adxl372_configure_fifo(adxl372,
						       ADXL372_FIFO_BYPASSED,
						       adxl372_default_init_param.fifo_config.fifo_format,
						       adxl372_default_init_param.fifo_config.fifo_samples);
#endif
			}

			adxl372_configure_fifo(adxl372,
					       ADXL372_FIFO_OLD_SAVED,
					       adxl372_default_init_param.fifo_config.fifo_format,
					       adxl372_default_init_param.fifo_config.fifo_samples);
			adxl372_set_op_mode(adxl372, ADXL372_INSTANT_ON);

			boInterruptFlag = false;
			LowPwrExitFlag = 0;
		}

		/* Enter Flexi mode - low power */
		if(reg_flag) {
			adi_pwr_EnterLowPowerMode ( ADI_PWR_MODE_FLEXI,
						    &LowPwrExitFlag, 0u);
		}
	}
}

/**
 * @brief Function for sending the fifo data.
 * @param fifo_data - data extracted from the adxl372 fifo.
 * @param data_count - amount of data inside the fifo
 * @return int16_t - signed value.
 */
void send_fifo_data(adxl372_xyz_accel_data *fifo_data, uint16_t data_count)
{
#ifndef PEAK_ACCELERATION
	data_count /= 3;
	for(int i = 0; i < data_count; i++) {
		data_pkt_full.Sensor_Data1.fValue =
			(float)sign_extend16(fifo_data[i].x) * 0.1; //100 mg/LSB
		data_pkt_full.Sensor_Data2.fValue =
			(float)sign_extend16(fifo_data[i].y) * 0.1;
		data_pkt_full.Sensor_Data3.fValue =
			(float)sign_extend16(fifo_data[i].z) * 0.1;
		if(data_pkt_full.Sensor_Data1.fValue != 0)
			adi_radio_DE_SendData(connInfo.nConnHandle,
					      sizeof(data_pkt_full),(uint8_t*)&data_pkt_full);
		timer_sleep(10);
	}
#endif
}

/**
 * @brief Sign extension function.
 * @param data - data to extend sign.
 * @return int16_t - signed value.
 */
int16_t sign_extend16(uint16_t data)
{
	uint8_t SignBit;
	int16_t signed_data = data;

	SignBit = (data & (1 << 11)) != 0;
	if (SignBit)
		signed_data = (int16_t)signed_data | (int16_t)(~((1 << 12) - 1));

	return signed_data;
}

