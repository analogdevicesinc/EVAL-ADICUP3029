/**
******************************************************************************
*   @file     Timer.c
*   @brief    Source file for the Timer part.
*   @version  V0.1
*   @author   ADI
*   @date     January 2017
*
*******************************************************************************
* Copyright 2017(c) Analog Devices, Inc.
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
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <drivers/pwr/adi_pwr.h>
#include "timer.h"
#include "common.h"

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/

static volatile  uint32_t timer_delayCount = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Power and SysTick timer initialization function.
 *
 * @params void
 *
 * @return 0 in case of success or negative error code otherwise.
 */
int32_t timer_start(void)
{
	int32_t ret;

	ret = adi_pwr_Init();
	if(ret != 0)
		return -1;

	ret = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1);
	if(ret != 0)
		return -1;

	ret = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1);
	if(ret != 0)
		return -1;

	/*
	 * SysTick initialization
	 */
	SysTick_Config(26000);

	return ret;
}

/**
 * Delay function of 1ms or more.
 *
 * Used for non-specific delays across the program.
 *
 * @params [in] ticks - Number of miliseconds to delay.
 *
 * @return void
 */
void timer_sleep(uint32_t ticks)
{
	timer_delayCount = ticks;

	/* Busy wait until the SysTick decrements the counter to zero. */
	while (timer_delayCount != 0u);
}

/**
 * ISR for SysTick timer.
 *
 * @params [in] pCBParam - Application passed pointer.
 * @params [in] nEvent   - Code of the event that triggered the interrupt.
 * @params [in] pArg     - Driver passed pointer.
 *
 * @return void
 */
void SysTick_Handler(void)
{
	/* Decrement to zero the counter used by the delay routine. */
	if (timer_delayCount != 0u)
		--timer_delayCount;
}
