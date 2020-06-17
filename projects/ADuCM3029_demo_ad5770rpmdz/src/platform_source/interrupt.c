/***************************************************************************//**
 *   @file   interrupt.c
 *   @author Andrei Drimbarean (Andrei.Drimbarean@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "interrupt.h"
#include "error.h"
#include <stdlib.h>

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

//todo: comment intr_register_callback
int32_t intr_enable_irq(uint32_t int_id, uint8_t mode)
{
	return adi_xint_EnableIRQ(int_id, mode);
}

//todo: comment intr_register_callback
int32_t intr_register_callback(uint32_t int_id, void *cb_function_pointer,
			       void *cb_parameter)
{
	return adi_xint_RegisterCallback(int_id,
					 (ADI_CALLBACK const)cb_function_pointer, cb_parameter);
}

//todo: comment intr_setup
int32_t intr_setup(struct intr_dev **device)
{
	int32_t ret;
	struct intr_dev *dev;

	dev = calloc(1, sizeof *dev);
	if(!dev)
		return -1;

	ret = adi_xint_Init(dev->mem_vector, ADI_XINT_MEMORY_SIZE);
	if(ret != SUCCESS)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

//todo: comment intr_remove
int32_t intr_remove(struct intr_dev *dev)
{
	int32_t ret;

	if(!dev)
		return -1;

	ret = adi_xint_UnInit();
	if(ret != SUCCESS)
		return ret;

	free(dev);

	return ret;
}
