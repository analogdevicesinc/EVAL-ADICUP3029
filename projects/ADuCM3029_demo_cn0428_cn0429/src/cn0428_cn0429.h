/*******************************************************************************
*   @file     cn0428_cn0429.h
*   @brief    header file for CN0429/CN0428 support functions
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

#ifndef CN0428_CN0429_H_
#define CN0428_CN0429_H_

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include <common.h>

#include <sys/platform.h>
#include <adi_processor.h>
#include <drivers/pwr/adi_pwr.h>
#include <drivers/tmr/adi_tmr.h>
#include <drivers/gpio/adi_gpio.h>
#include <drivers/rtc/adi_rtc.h>
#include <drivers/uart/adi_uart.h>
#include "adi_m355_gas_sensor.h"

using namespace adi_sensor_swpack;

/* I2C addresses */
#define ADI_CFG_I2C_SENSOR1_ADDR (0x0Au)
#define ADI_CFG_I2C_SENSOR2_ADDR (0x0Bu)
#define ADI_CFG_I2C_SENSOR3_ADDR (0x0Cu)
#define ADI_CFG_I2C_SENSOR4_ADDR (0x0Du)

#define _CR			13	/* <ENTER> */
#define _LF			10	/* <New line> */
#define _SPC			32	/* <Space> */
#define _BS			8	/* <Backspace> */
#define _EOS			"\0"	/* <End of String */

#define CYCLES_PER_MS		0xA28
#define CYCLES_PER_US		0x03

#define NUM_SENSORS		4

/* CN0428 command for how many bytes to read from slave */
#define BYTES_TO_READ		0x61

/* Generic result enum */
typedef enum {
	Failure = 0,
	Success = 1
} GResult;

typedef void (*cmdFunc)(uint8_t *);

EXTERNC uint8_t cmdInString[64];

EXTERNC struct RingBuf RX, TX;

typedef enum {
	INIT = 0,
	COMMAND,
	STREAM_GAS,
	OVERRIDE,
	WATER
} eFSM_State;
/*
 *	CN0429 Function Declarations
 */
void CN0429_CmdHelp(uint8_t *args);
void CN0429_CmdPrompt(void);
void DoNothing(uint8_t *args);
void CN0429_CmdHelp(uint8_t *args);
uint8_t *FindArgv(uint8_t *args);
void GetArgv(char *dst, uint8_t *args);
void CmdProcess(void);
cmdFunc FindCommand(char *cmd);
void CN0429_SetDefaultSns(uint8_t *args);
void CN0429_SnsConnected(uint8_t *args);
void CN0429_RdCfgs(uint8_t *args);
void CN0429_RdTemp(uint8_t *args);
void CN0429_RdHum(uint8_t *args);
void CN0429_CfgMeasTime(uint8_t *args);
void CN0429_StartMeas(uint8_t *args);
void CN0429_StopMeas(uint8_t *args);
void CN0429_CfgRtia(uint8_t *args);
void CN0429_CfgRload(uint8_t *args);
void CN0429_CfgVbias(uint8_t *args);
void CN0429_CfgSens(uint8_t *args);
void CN0429_RunEIS(uint8_t *args);
void CN0429_RdEIS(uint8_t *args);
void CN0429_RdEISfull(uint8_t *args);
void CN0429_RdRcal(uint8_t *args);
void CN0429_RunPulse(uint8_t *args);
void CN0429_ReadPulse(uint8_t *args);
void CN0429_SetPulseAmpl(uint8_t *args);
void CN0429_SetPulseDuration(uint8_t *args);
void CN0429_RdSensors(uint8_t *args);
void CN0429_StopRd(uint8_t *args);
void CN0429_CfgUpdateRate(uint8_t *args);
void CN0429_StreamData(void);

/*
 *	CN0428 Function Declarations
 */
void CN0428_RdSensorBuff(uint8_t continuous);
void CN0428_Setup(void);
void CN0428_SwitchSensor(char siteNum);
void CN0428_CommandLoop(void);

/*
*	Other Support Function Declarations
*/
SENSOR_RESULT SensorInitwithI2CAddr(uint8_t new_sensor_address, uint8_t site);
bool any_sensor_inited(void);
void flushBuff(uint8_t *buff, uint16_t len);
uint8_t *int2binString(int32_t a, uint8_t *buff, uint8_t bufSize);
double Ieee754ConvertToDouble(uint8_t s[32]);

EXTERNC void delay_ms(uint32_t mSec);
EXTERNC void delay_us(uint32_t uSec);

EXTERNC GResult UART_TX(const char *initialcmd);
EXTERNC GResult UART_TX_DIR(const char *initialcmd);

#endif /* CN0428_CN0429_H_ */
