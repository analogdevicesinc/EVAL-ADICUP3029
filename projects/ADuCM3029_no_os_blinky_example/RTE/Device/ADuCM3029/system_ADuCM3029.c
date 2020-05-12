/*!
 *****************************************************************************
 * @file:    system_ADuCM3029.c
 * @brief:   CMSIS Cortex-M3 Device Peripheral Access Layer Source File for
 *           ADuCM3029
 * @version  V3.10
 * @date     23. November 2012
 *-----------------------------------------------------------------------------
 *
 * @note     Modified January 11 2018 Analog Devices 
 *
******************************************************************************/
/* Copyright (c) 2012 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

   Portions Copyright (c) 2018 Analog Devices, Inc.
   ---------------------------------------------------------------------------*/

/*! \addtogroup SYS_Driver System Interfaces
 *  @{
 */

#include <stdint.h>
#include "system_ADuCM3029.h"
#include <adi_callback.h>
#include <adi_processor.h>
#include <rtos_map/adi_rtos_map.h>

#ifdef __ICCARM__
/*
* IAR MISRA C 2004 error suppressions.
*
* Pm073 (rule 14.7): a function should have a single point of exit.
* Pm143 (rule 14.7): a function should have a single point of exit at the end of the function.
*   Multiple returns are used for error handling.
*
* Pm140 (rule 11.3): a cast should not be performed between a pointer type and an integral type
*   The rule makes an exception for memory-mapped register accesses.
*/
#pragma diag_suppress=Pm073,Pm143,Pm140
#endif /* __ICCARM__ */


/*! \cond PRIVATE */

/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/

/* To enable the  cache. Please note that linker description  file need to 
   have appropriate memory mapping.  */
/* #define ENABLE_CACHE */


#ifdef ADI_DEBUG
/*! Low frequency clock frequency, not needed unless its debug mode 
    "lf_clk" coming out of LF mux */
uint32_t lfClock = 0u;
#endif

/*! "root_clk" output of HF mux */
uint32_t hfClock = 0u;  

 /*! external GPIO clock */  
uint32_t gpioClock = 0u;
#ifdef RELOCATE_IVT
extern void* __Vectors[];
#else
extern uint32_t __Vectors[];
#endif

/*----------------------------------------------------------------------------
  Security options
 *----------------------------------------------------------------------------*/

#if defined (__CC_ARM)
  __attribute__ ((at(0x00000180u)))
#elif defined (__GNUC__)
  __attribute__ ((section(".security_options")))
#elif defined (__ICCARM__)
  #pragma location=".security_options"
  __root
  __weak
#endif /* __ICCARM__ */
const ADI_ADUCM302X_SECURITY_OPTIONS adi_aducm302x_security_options
  = {
        { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu },
        0xA79C3203u,
        127u,
        0xFFFFFFFFu,
        0xFFFFFFFFu,
};

/*! \endcond  */


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/

/*! Variable to hold the system core clock value.  */
uint32_t SystemCoreClock = 0u;

/*----------------------------------------------------------------------------
  Clock functions
 *----------------------------------------------------------------------------*/

/*!
 * Update the clock. 
 *
 * @return none
 *
 * @brief  Updates the variable SystemCoreClock and must be called whenever 
           the core clock is changed during program execution.
 */
 void SystemCoreClockUpdate(void)
{
    uint32_t val, nDivisor, nMulfactor, div2, mul2;

#ifdef ADI_DEBUG
    /* "lfclock" is only used during debug checks... */
    /* LF clock is always 32k, whether osc or xtal */
    lfClock = __LFCLK;  /* for beep, wdt and lcd */
    if (lfClock == 0u)
    {
      while (1) {}
    }
#endif
    /* Update Core Clock sources */
    /* update the HF clock */
    switch (pADI_CLKG0_CLK->CTL0 & BITM_CLKG_CLK_CTL0_CLKMUX ) {

        case HFMUX_INTERNAL_OSC_VAL:
            hfClock = __HFOSC;
            break;

        case HFMUX_EXTERNAL_XTAL_VAL:
            hfClock = __HFXTAL;
            break;

        case HFMUX_SYSTEM_SPLL_VAL:
            /* Calculate System PLL output frequency */
            if ((pADI_CLKG0_CLK->CTL0 & BITM_CLKG_CLK_CTL0_SPLLIPSEL) != 0u) {
                /* PLL input from HFXTAL */
                val = __HFXTAL;
            } else {
                /* PLL input from HFOSC */
                val = __HFOSC;
            }

            /* PLL NSEL multiplier */
            nMulfactor = (pADI_CLKG0_CLK->CTL3 & BITM_CLKG_CLK_CTL3_SPLLNSEL) >> BITP_CLKG_CLK_CTL3_SPLLNSEL;
            /* PLL MSEL divider */
            nDivisor = (pADI_CLKG0_CLK->CTL3 & BITM_CLKG_CLK_CTL3_SPLLMSEL) >> BITP_CLKG_CLK_CTL3_SPLLMSEL;

            /* PLL NSEL multiplier */
            mul2 = (pADI_CLKG0_CLK->CTL3 & BITM_CLKG_CLK_CTL3_SPLLMUL2) >> BITP_CLKG_CLK_CTL3_SPLLMUL2;
            /* PLL MSEL divider */
            div2 = (pADI_CLKG0_CLK->CTL3 & BITM_CLKG_CLK_CTL3_SPLLDIV2) >> BITP_CLKG_CLK_CTL3_SPLLDIV2;
            
            val = ((val << mul2) * nMulfactor / nDivisor) >> div2;

            hfClock = val;
            break;

        case HFMUX_GPIO_VAL:
            hfClock = gpioClock;
            break;

        default:         
            return;
    } /* end switch */
 
    SystemCoreClock = hfClock;
 }

#ifdef __ARMCC_VERSION
/* We want a warning if semi-hosting libraries are used. */
#pragma import(__use_no_semihosting_swi)
#endif

#ifdef RELOCATE_IVT
/**
  A relocated IVT is requested.  Provision for IVT relocation
  to SRAM during startup.  This allows for dynamic interrupt
  vector patching required by RTOS.  Places the relocated IVT
  at the start of SRAM.  Note: the IVT placement is required
  to be next power-of-two of the vector table size.  So the
  IVT includes 72 programmable interrupts, 15 system exception
  vectors and the main stack pointer, therefore we need
  (72 + 15 + 1)*4 = 352 bytes, which rounds up to a 512 (0x200)
  address boundary (which address 0x20000000 satisfies).
*/
  
#define ADI_NUM_EXCEPTIONS        16
#define LENGTHOF_IVT              (NVIC_INTS + ADI_NUM_EXCEPTIONS)
#define RELOCATION_ADDRESS        (0x20000000)

/* Relocatable vector table is only supported in IAR */
#if defined (__ICCARM__)
/* reserve no-init aligned IVT space at top of RAM */
SECTION_PLACE(KEEP_VAR(__no_init uint32_t  __relocated_vector_table[LENGTHOF_IVT]), RELOCATION_ADDRESS);
#else
#warning "Relocated Interupt Vector Tables are not supported in this toolchain"
#endif
#endif
/*!
 * @brief  Sets up the microcontroller system.
 *         Initializes the System and updates the relocate vector table.
 * @return none
 *
 * @note This function is called by the start-up code and does not need to be
 *       called directly by applications
 */
void SystemInit (void)
{
    uint32_t IntStatus;
    
    /* Enable Bank1 and 2 SRAM retention. */
    adi_system_EnableRetention(ADI_SRAM_BANK_1 | ADI_SRAM_BANK_2, true);
                               
    
    /* To enable the instruction cache.  */    
#ifdef  ENABLE_CACHE
    adi_system_EnableCache(true);    
#endif    

    /* Switch the Interrupt Vector Table Offset Register
     * (VTOR) to point to the relocated IVT in SRAM.
     */

    /* Because SystemInit must not use global variables, the following
     * interrupt disabling code should not be replaced with critical region
     * code which uses global variables.
     */
    IntStatus = __get_PRIMASK();
    __disable_irq();
#ifdef RELOCATE_IVT
    /* Copy the IVT (avoid use of memcpy here so it does not become locked into flash). */
    size_t i;
    for (i = 0u; i < LENGTHOF_IVT; i++)
    {
        __relocated_vector_table[i] = (uint32_t )__Vectors[i];
    }
    SCB->VTOR = (uint32_t) &__relocated_vector_table;
#else
    /* Set the vector table address  */
    SCB->VTOR = (uint32_t) &__Vectors;
#endif /* RELOCATE_IVT */

    /* Set all three (USGFAULTENA, BUSFAULTENA, and MEMFAULTENA) fault enable bits
     * in the System Control Block, System Handler Control and State Register
     * otherwise these faults are handled as hard faults.
     */
    SCB->SHCSR = SCB_SHCSR_USGFAULTENA_Msk |
                 SCB_SHCSR_BUSFAULTENA_Msk |
                 SCB_SHCSR_MEMFAULTENA_Msk ;

    /* Flush instruction and data pipelines to insure assertion of new settings. */
    __ISB();
    __DSB();

    __set_PRIMASK(IntStatus);
	
#ifdef ADI_MAX_IRQ_PRIORITY
    adi_system_SetGlobalIrqPriority();
#endif
}
 
/*!
 * @brief  This enables or disables  the cache.
 * @param  bEnable : To specify whether to enable/disable cache.
 * \n              true : To enable cache.
 * \n
 * \n              false : To disable cache.
 * \n
 * @return none
 *
 */
void adi_system_EnableCache(bool bEnable)
{
    pADI_FLCC0_CACHE->KEY = CACHE_CONTROLLER_KEY;
    
    if(bEnable == true)
    {
        pADI_FLCC0_CACHE->SETUP |= BITM_FLCC_CACHE_SETUP_ICEN;
    }
    else
    {
        pADI_FLCC0_CACHE->SETUP &= ~BITM_FLCC_CACHE_SETUP_ICEN;
    }
}

/*!
* @brief  Enable/disable SRAM retention during hibernation for SRAM bank1 
 *         and/or bank2.
* @param eBank SRAM bank(s) to be retained/not retained when the processor 
 *              enters hibernation mode.
*              The only valid arguments are #ADI_SRAM_BANK_1, #ADI_SRAM_BANK_2
*              or a combination of both.
* @param bEnable Enable/disable the  retention for specified SRAM bank.
*             - true:  Enable retention during hibernation.
*             - false: Disable retention during hibernation.
* @return   0u : SRAM configured successfully.
* @return   1u : Incorrect bank passed as an argument  
 * @note The linker file in the application needs to support the requested 
 *       configuration. Only BANK1 and BANK2 of SRAM are valid.
*/
uint32_t adi_system_EnableRetention(ADI_SRAM_BANK eBank, bool bEnable)
{
#ifdef ADI_DEBUG
    if(eBank & (~(ADI_SRAM_BANK_1 | ADI_SRAM_BANK_2)))
    {
        return 1u;
    }

#endif
    pADI_PMG0->PWRKEY = PWRKEY_VALUE_KEY;
    if(bEnable == true)
    {
        pADI_PMG0->SRAMRET |= (uint32_t)eBank>>1;
    }
    else
    {
        pADI_PMG0->SRAMRET &= ~((uint32_t)eBank >> 1);    
    }

    return 0u;
}

/*!
 * @brief  This function sets the priority for all IRQ interrupts to the value 
 *         defined by ADI_MAX_IRQ_PRIORITY (if defined).
 *
 * @details  At reset the hardware default priority 0, which is the highest. For 
 *         any applications that use BASEPRI to mask interrupts, such as any
 *         application using FreeRTOS, then any interrupts using default priority
 *         will never be masked.  Defining ADI_MAX_IRQ_PRIORITY to a value
 *         will enable this function call at startup and set all IRQ interrupts
 *         to that priority value. 
 * @return none
 *
 */
 
void adi_system_SetGlobalIrqPriority(void)
{
#ifdef ADI_MAX_IRQ_PRIORITY
  uint32_t prio = (uint32_t)ADI_MAX_IRQ_PRIORITY;
#else
  uint32_t prio = (uint32_t)0u;
#endif
   
  /* NVIC supports up to 240 interrupt sources, but this part does not
   * use the full range. */ 
  for(uint8_t irq = 0u; irq < NVIC_INTS; irq++) {
      NVIC_SetPriority((IRQn_Type)irq, prio);
  }
}

/**@}*/
