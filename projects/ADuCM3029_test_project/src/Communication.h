/**
******************************************************************************
*   @file     Communication.h
*   @brief    Header file for communication part
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
#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <drivers/uart/adi_uart.h>
#include <drivers/spi/adi_spi.h>
#include "drivers/i2c/adi_i2c.h"

/*
                    Baudrate divider for PCLK-26000000

+------------------------------------------------------------------------+
| CALCULATING UART DIV REGISTER VALUE FOR THE  INPUT CLOCK: 26000000     |
|------------------------------------------------------------------------|
|       BAUDRATE       DIV-C     DIV-M     DIV-N         OSR    DIFF     |
|------------------------------------------------------------------------|
|       00009600        0022      0003      1734        0003    0000     |
|------------------------------------------------------------------------|
|       00019200        0011      0003      1735        0003    0000     |
|------------------------------------------------------------------------|
|       00038400        0017      0001      0501        0003    0000     |
|------------------------------------------------------------------------|
|       00057600        0007      0002      0031        0003    0000     |
|------------------------------------------------------------------------|
|       00115200        0007      0002      0031        0002    0000     |
|------------------------------------------------------------------------|
|       00230400        0007      0002      0031        0001    0000     |
|------------------------------------------------------------------------|
|       00460800        0007      0002      0031        0000    0001     |
|------------------------------------------------------------------------|
|       00921600        0001      0001      1563        0002    0028     |
|------------------------------------------------------------------------|
|       01000000        0001      0001      1280        0002    0000     |
|------------------------------------------------------------------------|
|       01500000        0001      0002      0341        0001    0112     |
|------------------------------------------------------------------------|
|       03000000        0001      0002      0341        0000    0225     |
|------------------------------------------------------------------------|
|       04000000        0001      0001      1280        0000    0000     |
|------------------------------------------------------------------------|
|       05000000        0001      0001      0614        0000    0751     |
|------------------------------------------------------------------------|
|       06000000        0001      0001      0171        0000    0901     |
|------------------------------------------------------------------------|
|       06500000        0001      0001      0000        0000    0000     |
|------------------------------------------------------------------------|

*/
/* Select the baud rate divider for 9600 */
#define UART_DIV_C      22

#define UART_DIV_M      3

#define UART_DIV_N      1734

#define UART_OSR        3

/* Memory required by the driver for DMA mode of operation */
#define ADI_UART_MEMORY_SIZE    (ADI_UART_BIDIR_MEMORY_SIZE)

void UART_Init(void);
void SPI_Init(void);
void I2C_Init(void);

#endif /* COMMUNICATION_H_ */
