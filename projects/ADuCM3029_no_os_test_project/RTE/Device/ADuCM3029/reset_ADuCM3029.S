/*!
 *****************************************************************************
 * @file:    reset_ADuCM3029.S
 * @brief:   Reset entry for use with SRAM MODEs 2 or 3.
 *-----------------------------------------------------------------------------
 *
 Copyright (c) 2018 Analog Devices, Inc.

 All rights reserved.
 *****************************************************************************/
#include <ADuCM3029_def.h>

/*
When using SRAM memory modes 2 & 3, ISRAM is used as data memory.  On reset
ISRAM is enabled by default by the hardware, so any write to that data area 
will result in a fault.  

This file, and Reset_Handler entrypoint, is provided to ensure that SRAM is 
configured correctly before any data is accessed.  An assembly handler is used
as the STACK may be mapped to the ISRAM area - a C handler will push data onto 
that stack on entry and cause an immediate hard fault.

ISRAM is enabled by default and can be disabled by below macro.  This will define
Reset_Handler that will replace the weak alias in startup_*.c.

If you wish to use your own Reset_Handler definition, then you should not define
this macro, and if ISRAM is to be disabled then you should include a similar
code sequence to that below before you access the ISRAM data bank.
*/

/* ISRAM must be disabled here when using SRAM MODES 2 or 3 */
/*#define ADI_DISABLE_INSTRUCTION_SRAM */

#ifdef ADI_DISABLE_INSTRUCTION_SRAM
	.syntax	unified
	.arch	armv7e-m

	.text
	.thumb
	.thumb_func
	.align	2
	.global	Reset_Handler
	.type	Reset_Handler, %function
Reset_Handler:
	LDR R1, =REG_PMG0_TST_SRAM_CTL;             /* Load the SRAM control register */ 
	LDR R0, [R1] ;
	BIC R0, R0, BITM_PMG_TST_SRAM_CTL_INSTREN ; /* Clear the ISRAM enable bit */
	STR R0, [R1] ;                              /* Store ->  ISRAM memory can now be used as data. */ 
                                                
	LDR     R0, =Reset_Handler_C                /* Branch to the C version of the reset handler. */
    BX      R0;

	.pool
	.size	Reset_Handler, . - Reset_Handler
	.end
#endif
