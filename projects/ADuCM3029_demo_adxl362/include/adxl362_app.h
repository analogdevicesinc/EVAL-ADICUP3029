/*!
 *****************************************************************************
   @file:    adxl362_app.h

   @brief:   ADXL362 accelerometer example

   @details: Example demonstrating the accelerometer functionality
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


#ifndef ADXL362_APP_H
#define ADXL362_APP_H
#include <base_sensor/adi_sensor.h>
#include <base_sensor/adi_sensor_packet.h>
#include <base_sensor/adi_sensor_errors.h>
#include <axl/adxl362/adi_adxl362.h>

#include <adi_ble_config.h>
#include <drivers/pwr/adi_pwr.h>
#include <radio/adi_ble_radio.h>
#include <common/adi_error_handling.h>
#include <framework/noos/adi_ble_noos.h>

/*
 * Macro to enable or disable bluetooth functionality. If bluetooth is disabled the sensor
 * output will be directed to console.
 *
 *  ADI_APP_USE_BLUETOOTH 1 - Enables bluetooth and sensor data is sent to the iOS/Android app
 *
 *  ADI_APP_USE_BLUETOOTH 0 - Disables bluetooth and sensor data is printed to console with
 *                            debug build. In case of release build console output is redirected
 *                            to UART.
 */
#define ADI_APP_USE_BLUETOOTH   (1u)

/* Accelerometer instance ID */
#define ADI_ACCELEROMETER_ID    (1u)

/* BLE event processing dispatcher timeout. Waits for 2sec for BLE events */
#define ADI_APP_DISPATCH_TIMEOUT (2000)

/* Defined in pinmux.c */
extern "C" int32_t adi_initpinmux(void);

typedef union {
	uint8_t u8Value[4];
	float fValue;
} floatUnion_t;

#pragma pack(push)
#pragma pack(1)
struct RegistrationPacket {
	uint8_t pktTypeSensorId;
	uint8_t numFields;
	uint8_t dataType;
	uint8_t sensorName[17];
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct FieldNamePacket {
	uint8_t pktTypeSensorId;
	uint8_t fieldId;
	uint8_t fieldName[18];
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct DataPacket {
	uint8_t pktTypeSensorId;
	floatUnion_t Sensor_Data1;
	floatUnion_t Sensor_Data2;
	floatUnion_t Sensor_Data3;
	floatUnion_t Sensor_Data4;
};
#pragma pack(pop)

#endif /* ADXL362_APP_H */
