/*******************************************************************************
*   @file     ADuCM3029_demo_cn0428_cn0429.h
*   @brief    Main project header file
*   @version  V0.1
*   @author   ADI
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

#ifndef __ADUCM3029_DEMO_CN0428_CN0429_H__
#define __ADUCM3029_DEMO_CN0428_CN0429_H__

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#define DISABLE_WATCHDOG	0x0u

/* UART SECTION */

#define UART_DEVICE_NUM 	0u
/* Memory required by the driver for bidirectional mode of operation. */
#define ADI_UART_MEMORY_SIZE	(ADI_UART_BIDIR_MEMORY_SIZE)
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


 */

#define UART_DIV_C_9600		22
#define UART_DIV_C_19200	11
#define UART_DIV_C_38400	17
#define UART_DIV_C_57600	7
#define UART_DIV_C_115200	7
#define UART_DIV_C_230400	7
#define UART_DIV_C_460800	7

#define UART_DIV_M_9600		3
#define UART_DIV_M_19200	3
#define UART_DIV_M_38400	1
#define UART_DIV_M_57600	2
#define UART_DIV_M_115200	2
#define UART_DIV_M_230400	2
#define UART_DIV_M_460800	2

#define UART_DIV_N_9600		1734
#define UART_DIV_N_19200	1735
#define UART_DIV_N_38400	501
#define UART_DIV_N_57600	31
#define UART_DIV_N_115200	31
#define UART_DIV_N_230400	31
#define UART_DIV_N_460800	31

#define UART_OSR_9600		3
#define UART_OSR_19200		3
#define UART_OSR_38400		3
#define UART_OSR_57600		3
#define UART_OSR_115200		2
#define UART_OSR_230400		1
#define UART_OSR_460800		0

/* ====================================================*/

EXTERNC void delay_ms(uint32_t mSec);
EXTERNC void delay_us(uint32_t uSec);

EXTERNC uint16_t streamTickCnt;
EXTERNC uint16_t streamTickCfg;

#endif /* __ADUCM3029_DEMO_CN0428_CN0429_H__ */
