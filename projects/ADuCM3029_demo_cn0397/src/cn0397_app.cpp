/*!
 *****************************************************************************
   @file:    cn0397_app.cpp

   @brief:   CN0397 visible light sensor example

   @details: Example demonstrating the visible light sensor functionality
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

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF CLAIMS OF INTELLECTUAL
PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/

#include <sys/platform.h>
#include "cn0397_app.h"
#include "common.h"
#include <common/adi_error_handling.h>
#include "base_sensor/adi_sensor_errors.h"

using namespace adi_sensor_swpack;

/* Peripheral advertisement mode */
#define PERIPHERAL_ADV_MODE      ((ADI_BLE_GAP_MODE)(ADI_BLE_GAP_MODE_CONNECTABLE | ADI_BLE_GAP_MODE_DISCOVERABLE))

#if(ADI_APP_USE_BLUETOOTH == 1)
/* Static bluetooth data */
static bool               gbConnected;
static ADI_BLE_GAP_MODE   geMode;
uint8_t reg_flag;

 struct RegistrationPacket reg_pkt = {0x00, 3, 5, "CN0397"};
 struct FieldNamePacket name_pkt0 = {0x01, 0, "Red Light [lux]"};
 struct FieldNamePacket name_pkt1 = {0x01, 1, "Blue Light [lux]"};
 struct FieldNamePacket name_pkt2 = {0x01, 2, "Green Light [lux]"};
 struct DataPacket data_pkt = {0x02, 0, 0, 0, 0};

/* Local bluetooth functions */
static void InitBluetoothLowEnergy(void);
static void ApplicationCallback(void * pCBParam, uint32_t Event, void * pArg);
static void SetAdvertisingMode(void);
static void VisibleLightBluetoothMode(VisibleLight *pLight);

#else
#ifndef ADI_DEBUG
/* Static standalone data for release mode UART redirection*/
static  int  BarLine[3];

/* Static functions for release mode UART redirection*/
static void CN0397_SetBar(float conc, int *line);
#endif
/* Local standalone functions */
static void VisibleLightStandaloneMode(VisibleLight *pLight);

#endif

/* Local functions */
static void InitSystem(void);
static void Trap(void);

/*!
 * @brief      Main
 *
 * @details    Application entry point.
 *
 * @param [in] argc : Number of arguments (unused)
 *
 * @param [in] argv : Arguments (unused)
 *
 */
int main(int argc, char *argv[])
{
    CN0397          cn0397;
    VisibleLight   *pLight = &cn0397;
    SENSOR_RESULT   eSensorResult;

    /* Initialize the system */
    InitSystem();

    /* Open the visible light sensor */
    eSensorResult = pLight->open();

    if(eSensorResult != SENSOR_ERROR_NONE) {
        PRINT_SENSOR_ERROR(DEBUG_MESSAGE, eSensorResult);
        Trap();
    }

    DEBUG_MESSAGE("Please cover the sensor for the calibrations. \r\n");

    /* Delay for approx 5 seconds */
    for(volatile uint32_t i = 0; i < 12000000u; i++);


    /* Run calibrations */
    eSensorResult = pLight->start();

    if(eSensorResult != SENSOR_ERROR_NONE) {
        PRINT_SENSOR_ERROR(DEBUG_MESSAGE, eSensorResult);
        Trap();
    }

    DEBUG_MESSAGE("Calibration complete. You no longer need to cover the sensor.\r\n");

#if(ADI_APP_USE_BLUETOOTH == 1u)
    VisibleLightBluetoothMode(pLight);
#else
    VisibleLightStandaloneMode(pLight);
#endif
}

/*!
 * @brief      Initializes the system
 *
 * @details    This function is responsible for initializing the pinmuxing, power service
 *             and bluetooth subsystem. It also initializes the realtime clock for to timestamp
 *             the outgoing sensor data packets.
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
    DEBUG_RESULT("Error initializing the power service.\r\n", ePwr, ADI_PWR_SUCCESS);

    ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u);
    DEBUG_RESULT("Error configuring the core clock.\r\n", ePwr, ADI_PWR_SUCCESS);

    ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u);
    DEBUG_RESULT("Error configuring the peripheral clock.\r\n", ePwr, ADI_PWR_SUCCESS);

#ifndef ADI_DEBUG
    /* Initialize UART redirection in release mode only */
    common_Init();
#endif

    DEBUG_MESSAGE("Starting visual light sensor demo application\r\n");

}

/*!
 * @brief      Trap function
 *
 * @details    In case of catastrophic errors this function is called to block
 *             infinitely.
 */
static void Trap()
{
    while(1);
}

#if(ADI_APP_USE_BLUETOOTH == 1)

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
    switch(Event)
    {
        case GAP_EVENT_CONNECTED:
            DEBUG_MESSAGE("Connected!\r\n");
            gbConnected = true;
            break;

        case GAP_EVENT_DISCONNECTED:
            DEBUG_MESSAGE("Disconnected!\r\n");
            geMode      = ADI_BLE_GAP_MODE_NOTCONNECTABLE;
            gbConnected = false;
            reg_flag = 0;
            break;

        case DATA_EXCHANGE_RX_EVENT:
            DEBUG_MESSAGE("Data received!\r\n");
            break;

        case DATA_EXCHANGE_TX_COMPLETE:
        	DEBUG_MESSAGE(("Data sent!\r\n"));
        	if (reg_flag==0)
        		reg_flag=1;
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
        	reg_flag = 0;
            break;

        case BLE_RADIO_ERROR_PARSING:
        	DEBUG_MESSAGE(("Failed to parse a packet from the radio.\r\n"));
        	reg_flag = 0;
            break;

        case BLE_RADIO_ERROR_PROCESSING:
        	DEBUG_MESSAGE(("Failed to process a packet from the radio.\r\n"));
        	reg_flag = 0;
            break;

        default:
        	DEBUG_MESSAGE(("Unexpected event received.\r\n"));
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
    DEBUG_RESULT("Error waiting for GAP_EVENT_MODE_CHANGE.\r\n", eResult, ADI_BLER_SUCCESS);

    eResult = adi_radio_GetMode(&geMode);
    DEBUG_RESULT("Error getting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

    if(geMode != PERIPHERAL_ADV_MODE)
        DEBUG_MESSAGE("Error in SetAdvertisingMode.\r\n");
}

/*!
 * @brief      Initializes the bluetooth
 *
 * @details    Data Exchange profile is initialized to send
 *             data to the connected central device.
 */
static void InitBluetoothLowEnergy(void)
{
    ADI_BLER_RESULT eResult;
    uint8_t *       aDataExchangeName = (unsigned char *) "CN0397 Light Demo";

    /* Init timestamping */
    INIT_TIME();

    /* Initialize radio and framework layer */
    eResult = adi_ble_Init(ApplicationCallback, NULL);
    DEBUG_RESULT("Error initializing the radio.\r\n", eResult, ADI_BLER_SUCCESS);

    /* Configure radio */
    eResult = adi_radio_RegisterDevice(ADI_BLE_ROLE_PERIPHERAL);
    DEBUG_RESULT("Error registering the radio.\r\n", eResult, ADI_BLER_SUCCESS);

    eResult = adi_radio_SetLocalBluetoothDevName(aDataExchangeName, strlen((const char *) aDataExchangeName), 0u, 0u);
    DEBUG_RESULT("Error setting local device name.\r\n", eResult, ADI_BLER_SUCCESS);

    SetAdvertisingMode();

    /* Initialize data exchange profile */
    eResult = adi_radio_Register_DataExchangeServer();
    DEBUG_RESULT("Error registering data exchange server.\r\n", eResult, ADI_BLER_SUCCESS);

    /* Now enter infinite loop waiting for connection and then data exchange events */
    DEBUG_MESSAGE("Waiting for connection. Initiate connection on central device please.\r\n");
}

/*!
 * @brief      Bluetooth visible light demo.
 *
 * @details    Bluetooth mode sends the red, green and blue light intensity values to the Android or
 *             ios application.
 */
void VisibleLightBluetoothMode(VisibleLight *pLight)
{
    ADI_BLER_RESULT eResult;
    ADI_BLER_CONN_INFO sConnInfo;

    /* Initialize Bluetooth */
    InitBluetoothLowEnergy();

    /* WHILE(forever) */
    while(1u) {

        /* Dispatch events for two seconds - they will arrive in the application callback */
        eResult = adi_ble_DispatchEvents(ADI_APP_DISPATCH_TIMEOUT);
        DEBUG_RESULT("Error dispatching events to the callback.\r\n", eResult, ADI_BLER_SUCCESS);

        /* If connected, send data */
        if(gbConnected == true) {
            /* Get the connection info. The connection handle will be used to identify which remote device to send data to. */
            adi_ble_GetConnectionInfo(&sConnInfo);

            if(reg_flag == 0)	{
            	eResult = adi_radio_DE_SendData(sConnInfo.nConnHandle,
            			sizeof(reg_pkt),(uint8_t*)&reg_pkt);

            	eResult = adi_radio_DE_SendData(sConnInfo.nConnHandle,
            			sizeof(name_pkt0),(uint8_t*)&name_pkt0);

            	eResult = adi_radio_DE_SendData(sConnInfo.nConnHandle,
            			sizeof(name_pkt1),(uint8_t*)&name_pkt1);

            	eResult = adi_radio_DE_SendData(sConnInfo.nConnHandle,
            			sizeof(name_pkt2),(uint8_t*)&name_pkt2);
            }

            /* Get the red, green and blue visual light data */
            pLight->getLightIntensity((float *)&data_pkt.Sensor_Data1);

            if(reg_flag == 1)	{
            	/* Send the data packet to the remote device */
            	eResult = adi_radio_DE_SendData(sConnInfo.nConnHandle,
            									sizeof(data_pkt),(uint8_t*)&data_pkt);
            	 DEBUG_MESSAGE("Error sending the data.\r\n");
            }
        }
        /* If disconnected, switch to advertising mode */
        else
        {
            if (geMode != PERIPHERAL_ADV_MODE)
                SetAdvertisingMode();
        }

    } /* ENDWHILE */
}
#else
/*!
 * @brief      Standalone visible light demo.
 *
 * @details    Standalone mode just prints the red, green and blue values to the console
 *			   in debug mode and the terminal in release mode.
 */
void VisibleLightStandaloneMode(VisibleLight *pLight)
{
    /* WHILE(forever) */
    while(1u) {
        ADI_VISUAL_LIGHT_DATA eIntensity;
        ADI_VISUAL_LIGHT_DATA eConcentration;

        /* Get the red, green and blue visual light data and concentration. */
        pLight->getLightIntensity((float *)&eIntensity.fData_Red);
        pLight->getLightConcentration((float *)&eConcentration.fData_Red);

#ifndef ADI_DEBUG
        uint8_t i;

        /* Get the concentraion bar. This is used to draw a color line on the terminal or console. */
        CN0397_SetBar(eConcentration.fData_Red, &BarLine[0]);
        CN0397_SetBar(eConcentration.fData_Green, &BarLine[1]);
        CN0397_SetBar(eConcentration.fData_Blue, &BarLine[2]);

        /* Print the Red concentration bar */
        DEBUG_MESSAGE("\t%s channel:[", "RED");

        for(i = 0; i < BarLine[0] ; i++)
            DEBUG_MESSAGE("\033[2;%dm%c\033[0m", 31, 219);

        for(i = 0; i < (21 - BarLine[0]) ; i++)
            DEBUG_MESSAGE(" ");

        DEBUG_MESSAGE("]");
        DEBUG_MESSAGE("\t");

        /* Print the Green concentration bar */
        DEBUG_MESSAGE("\t%s channel:[", "Green");

        for(i = 0; i < BarLine[1] ; i++)
            DEBUG_MESSAGE("\033[2;%dm%c\033[0m", 32, 219);

        for(i = 0; i < (21 - BarLine[1]) ; i++)
            DEBUG_MESSAGE(" ");

        DEBUG_MESSAGE("]");
        DEBUG_MESSAGE("\t");

        /* Print the Blue concentration bar */
        DEBUG_MESSAGE("\t%s channel:[", "Blue");

        for(i = 0; i < BarLine[2] ; i++)
            DEBUG_MESSAGE("\033[2;%dm%c\033[0m", 34, 219);

        for(i = 0; i < (21 - BarLine[2]) ; i++)
            DEBUG_MESSAGE(" ");

        DEBUG_MESSAGE("]");
        DEBUG_MESSAGE("\t");

        DEBUG_MESSAGE("\r\n");
        DEBUG_MESSAGE(" ");
#else
        DEBUG_MESSAGE("\tRED   CHANNEL\t\t\t\t");
        DEBUG_MESSAGE("\tGREEN CHANNEL\t\t\t\t");
        DEBUG_MESSAGE("\tBLUE  CHANNEL\t\t\t\t");
        DEBUG_MESSAGE("\r\n");

#endif

        DEBUG_MESSAGE("\tLight Intensity = %.2f lux\t\t", (float)eIntensity.fData_Red);
        DEBUG_MESSAGE("\tLight Intensity = %.2f lux\t\t", (float)eIntensity.fData_Green);
        DEBUG_MESSAGE("\tLight Intensity = %.2f lux\t\t", (float)eIntensity.fData_Blue);
        DEBUG_MESSAGE("\r\n");

        DEBUG_MESSAGE("\tLight Concentration = %.2f %c\t\t", (float)eConcentration.fData_Red, 37);
        DEBUG_MESSAGE("\tLight Concentration = %.2f %c\t\t", (float)eConcentration.fData_Green, 37);
        DEBUG_MESSAGE("\tLight Concentration = %.2f %c\t\t", (float)eConcentration.fData_Blue, 37);
        DEBUG_MESSAGE("\r\n");
        DEBUG_MESSAGE("\r\n");

        /* Delay for approx 5 seconds*/
        for(volatile uint32_t i = 0; i < 12000000; i++);

    } /* ENDWHILE */
}

#ifndef ADI_DEBUG

/*!
 * @brief      Draw a bar to represent light concentration.
 *
 * @param [in] conc : Concentration value of the photodiode.
 * @param [out] line : Width of the line to draw for the concentration bar.
 *
 * @details    In debug mode this will print to the console and it release mode it will
 *             use UART to print to the terminal.
 */void CN0397_SetBar(float conc, int *line)
{
    float concLimit = 5.0;
    int i = 0, j;
    *line = 0;

    if(conc > 0.0) {
        i = 1;
        *line = i;
    }

    for(j = 0; j < 20; j++) {
        if(conc >= concLimit)
            *line = i + 1;

        concLimit += 5.0;
        i += 1;
    }
}
#endif
#endif
