/*!
 *****************************************************************************
  @file:   cn0357_app.cpp

  @brief:  Example application demonstrating how to use the CN0357 Carbon
           Monoxide shield with the ADICUP3029.
 -----------------------------------------------------------------------------
Copyright (c) 2017 Analog Devices, Inc.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Modified versions of the software must be conspicuously marked as such.
  - This software is licensed solely and exclusively for use with processors
    manufactured by or for Analog Devices, Inc.
  - This software may not be combined or merged with other code in any manner
    that would cause the software to become subject to terms and conditions
    which differ from those listed here.
  - Neither the name of Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.
  - The use of this software may or may not infringe the patent rights of one
    or more patent holders.  This license does not release you from the
    requirement that you obtain separate licenses from these patent holders
    to use this software.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-
INFRINGEMENT, TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF
CLAIMS OF INTELLECTUAL PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/
#include "cn0357_app.h"

#include <common.h>
#include <gas/adi_gas.h>
#include <drivers/pwr/adi_pwr.h>
#include <radio/adi_ble_radio.h>
#include <common/adi_timestamp.h>
#include <gas/cn0357/adi_cn0357.h>
#include <common/adi_error_handling.h>
#include <framework/noos/adi_ble_noos.h>
#include <base_sensor/adi_sensor_packet.h>
#include <base_sensor/adi_sensor_errors.h>

using namespace adi_sensor_swpack;

/* Peripheral Advertisement Mode */
#define PERIPHERAL_ADV_MODE ((ADI_BLE_GAP_MODE)(ADI_BLE_GAP_MODE_CONNECTABLE | ADI_BLE_GAP_MODE_DISCOVERABLE))

#if ADI_APP_USE_BLUETOOTH == 1u
/* Global Data */
static bool               gbConnected;
static ADI_BLE_GAP_MODE   geMode;
static ADI_BLER_CONN_INFO sConnInfo;
uint8_t *reg_flag = (uint8_t *)malloc(sizeof(uint8_t));

struct RegistrationPacket reg_pkt = {0x00, 1, 5, "CN0357"};
struct FieldNamePacket name_pkt0 = {0x01, 0, "CO Gas [ppm]"};
struct DataPacket data_pkt = {0x02, 0, 0, 0, 0};

/* Local Bluetooth Functions */
static void ApplicationCallback(void * pCBParam, uint32_t Event, void * pArg);
static void SetAdvertisingMode(void);
static void InitBluetooth(void);
static uint8_t GasSensorBluetoothMode(Gas *gas);
#else
static uint8_t GasSensorStandaloneMode(Gas *gas);
#endif

/* Local Functions */
static void InitSystem(void);

/* Defined in pinmux.c */
extern "C" int32_t adi_initpinmux(void);

int main(void)
{
	SENSOR_RESULT eSensorResult;

	CN0357 coSensor;
	Gas * gas = &coSensor;

	InitSystem();

	eSensorResult = gas->open();

	if(eSensorResult != SENSOR_ERROR_NONE) {
		PRINT_SENSOR_ERROR(DEBUG_MESSAGE, eSensorResult);
		return (1u);
	}

#if (ADI_APP_USE_BLUETOOTH == 1u)

	if(GasSensorBluetoothMode(gas) == 1u)
		return (1u);

#else

	if(GasSensorStandaloneMode(gas) == 1u)
		return (1u);

#endif

	eSensorResult = gas->close();

	if(eSensorResult != SENSOR_ERROR_NONE) {
		PRINT_SENSOR_ERROR(DEBUG_MESSAGE, eSensorResult);
		return (1u);
	}

	return (0u);
}

/*!
 * @brief      Hardware Initialization
 *
 * @details    Setups up the power subsystem and the pinmuxing.
 *
 */
static void InitSystem(void)
{
	ADI_PWR_RESULT  ePwr;

	/* Explicitly disable the watchdog timer */
	*pREG_WDT0_CTL = 0x0u;

	/* Pinmux */
	adi_initpinmux();

	/* Initialize clocks */
	ePwr = adi_pwr_Init();
	DEBUG_RESULT("Error initializing the power service.\r\n", ePwr,
		     ADI_PWR_SUCCESS);

	ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u);
	DEBUG_RESULT("Error configuring the core clock.\r\n", ePwr, ADI_PWR_SUCCESS);

	ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u);
	DEBUG_RESULT("Error configuring the peripheral clock.\r\n", ePwr,
		     ADI_PWR_SUCCESS);

	common_Init();

	DEBUG_MESSAGE("Starting CN0357 Demo application\r\n");

}

#if ADI_APP_USE_BLUETOOTH == 1u

/*!
 * @brief      Bluetooth C0 gas demo.
 *
 * @details    Bluetooth mode sends the CO ppm values to the Android or
 *             ios application.
 */
uint8_t GasSensorBluetoothMode(Gas *gas)
{
	ADI_BLER_RESULT eResult;
	float           fConcentration;
	SENSOR_RESULT   eSensorResult;

	InitBluetooth();

	while(1u) {
		/* Dispatch events for two seconds - they will arrive in the application callback */
		eResult = adi_ble_DispatchEvents(ADI_APP_DISPATCH_TIMEOUT);
		DEBUG_RESULT("Error dispatching events to the callback.\r\n", eResult,
			     ADI_BLER_SUCCESS);

		/* If connected, send data */
		if(gbConnected == true) {
			adi_ble_GetConnectionInfo(&sConnInfo);

			if(*reg_flag == 0)	{
				eResult = adi_radio_DE_SendData(sConnInfo.nConnHandle,
								sizeof(reg_pkt),(uint8_t*)&reg_pkt);

				eResult = adi_radio_DE_SendData(sConnInfo.nConnHandle,
								sizeof(name_pkt0),(uint8_t*)&name_pkt0);
			}

			/* Read gas sensor */
			eSensorResult = gas->getPPM(&fConcentration);

			if(eSensorResult != SENSOR_ERROR_NONE) {
				PRINT_SENSOR_ERROR(DEBUG_MESSAGE, eSensorResult);
				return (1u);
			}

			data_pkt.Sensor_Data1.fValue = fConcentration;

			/* Send to host */
			eResult = adi_radio_DE_SendData(sConnInfo.nConnHandle,
							sizeof(data_pkt),(uint8_t*)&data_pkt);
			if (eResult != ADI_BLER_SUCCESS) {
				DEBUG_MESSAGE("Error sending the data.\r\n");
			}
		}
		/* If disconnected switch to advertising mode */
		else {
			if(geMode != PERIPHERAL_ADV_MODE) {
				SetAdvertisingMode();
			}
		}
	}
}

/*!
 * @brief      Application Callback
 *
 * @details    Called by the framework layer (adi_ble_noos.c) when an event occurs that the application did NOT
 *             explicity expect by calling #adi_ble_WaitForEventWithTimeout.
 *
 * @param [in] pCBParam : Callback parameter (unused)
 *
 * @param [in] Event : Event of type #ADI_BLER_EVENT.
 *
 * @param [in] pArg : Callback argument (unused)
 *
 * @note       The application should NOT call other radio functions (adi_ble_radio.c) from this callback that issue
 *             a command to the radio. The application may call radio functions that simply extract data from the
 *             companion module, these are located below #adi_ble_GetEvent in adi_ble_radio.c. Ideally this callback
 *             should just be used for flags and semaphores.
 */
static void ApplicationCallback(void * pCBParam, uint32_t Event, void * pArg)
{
	switch(Event) {
	case GAP_EVENT_CONNECTED:
		DEBUG_MESSAGE("Connected!\r\n");
		gbConnected = true;
		break;

	case GAP_EVENT_DISCONNECTED:
		DEBUG_MESSAGE("Disconnected!\r\n");
		geMode      = ADI_BLE_GAP_MODE_NOTCONNECTABLE;
		gbConnected = false;
		*reg_flag = 0;
		break;

	case DATA_EXCHANGE_RX_EVENT:
		DEBUG_MESSAGE("Data received!\r\n");
		break;

	case DATA_EXCHANGE_TX_COMPLETE:
		DEBUG_MESSAGE("Data sent!\r\n");
		if (*reg_flag == 0)
			*reg_flag = 1;
		break;

	case GAP_EVENT_MODE_CHANGE:
		DEBUG_MESSAGE("GAP mode changed.\r\n");
		break;

	case GAP_EVENT_CONNECTION_UPDATED:
		DEBUG_MESSAGE("Connection interval updated.\r\n");
		break;

	case BLE_RADIO_ERROR_READING:
		/* If you want to enable this print statement, please be aware that the first
		 * packet sent from the radio on startup will cause this error. It is a known bug
		 * and will not have any adverse effects on the application.
		 *
		 *DEBUG_MESSAGE("Failed to read a packet from the radio.\r\n");
		 *
		 */
		*reg_flag = 0;
		break;

	case BLE_RADIO_ERROR_PARSING:
		DEBUG_MESSAGE("Failed to parse a packet from the radio.\r\n");
		*reg_flag = 0;
		break;

	case BLE_RADIO_ERROR_PROCESSING:
		DEBUG_MESSAGE("Failed to process a packet from the radio.\r\n");
		*reg_flag = 0;
		break;

	default:
		DEBUG_MESSAGE("Unexpected event received.\r\n");
		break;
	}
}


/*!
 * @brief      Set Advertising Mode
 *
 * @details    Helper function to avoid repeated code in main.
 *
 */
static void SetAdvertisingMode(void)
{
	ADI_BLER_RESULT eResult;

	eResult = adi_radio_SetMode(PERIPHERAL_ADV_MODE, 0u, 0u);
	DEBUG_RESULT("Error setting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

	eResult = adi_ble_WaitForEventWithTimeout(GAP_EVENT_MODE_CHANGE, 5000u);
	DEBUG_RESULT("Error waiting for GAP_EVENT_MODE_CHANGE.\r\n", eResult,
		     ADI_BLER_SUCCESS);

	eResult = adi_radio_GetMode(&geMode);
	DEBUG_RESULT("Error getting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

	if(geMode != PERIPHERAL_ADV_MODE) {
		DEBUG_MESSAGE("Error in SetAdvertisingMode.\r\n");
	}
}


/*!
 * @brief      Bluetooth Initialization and Configuration
 *
 * @details    Initializes the Bluetooth radio and applys the necessary GAP and profile
 *             settings.
 *
 * @note       This needs to be seperated from InitSystem such that we can enable the
 *             CN0357 between the two function calls. The CN0357 uses the GPIO driver
 *             for the AD5270 chip select, and the Bluetooth software uses the GPIO driver
 *             as well. Since the AD5270 is only used temporarily, it is opened and closed
 *             in a single CN0357 open call. The Bluetooth GPIO settings need to persist
 *             for the example lifetime, so they must come second.
 *
 */
static void InitBluetooth(void)
{
	ADI_BLER_RESULT eResult;
	uint8_t *       aDataExchangeName = (unsigned char *) "CN0357 Demo";

	/* Init timestamping */
	INIT_TIME();

	/* Initialize radio and framework layer */
	eResult = adi_ble_Init(ApplicationCallback, NULL);
	DEBUG_RESULT("Error initializing the radio.\r\n", eResult, ADI_BLER_SUCCESS);

	/* Configure radio */
	eResult = adi_radio_RegisterDevice(ADI_BLE_ROLE_PERIPHERAL);
	DEBUG_RESULT("Error registering the radio.\r\n", eResult, ADI_BLER_SUCCESS);

	eResult = adi_radio_SetLocalBluetoothDevName(aDataExchangeName,
			strlen((const char *) aDataExchangeName), 0u, 0u);
	DEBUG_RESULT("Error setting local device name.\r\n", eResult, ADI_BLER_SUCCESS);

	SetAdvertisingMode();

	/* Initialize data exchange profile */
	eResult = adi_radio_Register_DataExchangeServer();
	DEBUG_RESULT("Error registering data exchange server.\r\n", eResult,
		     ADI_BLER_SUCCESS);

	/* Now enter infinite loop waiting for connection and then data exchange events */
	DEBUG_MESSAGE("Waiting for connection. Initiate connection on central device please.\r\n");
}
#else
/*!
 * @brief      Standalone C0 gas demo.
 *
 * @details    Standalone mode just prints the CO ppm values to the console
 *             in debug mode and the terminal in release mode.
 */
uint8_t GasSensorStandaloneMode(Gas *gas)
{
	float         fConcentration;
	SENSOR_RESULT eSensorResult;


	while(1u) {
		/* Read gas sensor */
		eSensorResult = gas->getPPM(&fConcentration);

		if(eSensorResult != SENSOR_ERROR_NONE) {
			PRINT_SENSOR_ERROR(DEBUG_MESSAGE, eSensorResult);
			return (1u);
		}

		DEBUG_MESSAGE("Gas Concentration = %f PPM\r\n", fConcentration);

		for(volatile uint32_t i = 0u; i < 0xFFFFul; i++);
	}
}
#endif

