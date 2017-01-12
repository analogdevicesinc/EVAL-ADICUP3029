/**
******************************************************************************
*   @file     Test.h
*   @brief    Header file for Test project
*   @version  V0.1
*   @author   ADI
*   @date     January 2017
*
*******************************************************************************
* Copyright 2017(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
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
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*******************************************************************************
**/

#ifndef TEST_H_
#define TEST_H_

#include <stddef.h>
#include <services/gpio/adi_gpio.h>

uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE];

/* Handle for UART device */
extern ADI_UART_HANDLE          hUartDevice __attribute__ ((aligned (4)));

/* Master SPI device handle */
extern ADI_SPI_HANDLE hMSpiDevice;
ADI_SPI_TRANSCEIVER MSpitransceive;

/* Master I2C device handle */
extern ADI_I2C_HANDLE masterI2cDev;
/* Slave I2C device handle */
extern ADI_I2C_HANDLE slaveI2cDev;

typedef enum {
   UART = 1,
   SPI,
   I2C,
   GPIO
} enTestFunc;

typedef enum {
   SPI_ARDUINO,	// SPI0 - used for ARDUINO connector on ADICUP3029 board
   SPI_PMOD,	// SPI1 - used for PMOD connector on ADICUP3029 board
   SPI_BLE		// SPI2 - used to send BLE commands to EM9304
} enSpiChannel;

void Test_Init(void);
void Port_Init(void);
void Test_Port(void);
void Test_UART(void);
void Test_SPI(void);
void Test_I2C(void);
void Test_Periph(void);

#endif /* TEST_H_ */
