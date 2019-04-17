/*! *****************************************************************************
 * @file:    adi_uart_def.h
 * @brief:   UART Device Driver definition for processor
 -----------------------------------------------------------------------------
Copyright (c) 2010-2016 Analog Devices, Inc.

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

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-
INFRINGEMENT, TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF
CLAIMS OF INTELLECTUAL PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/
/*! \cond PRIVATE */
#ifndef DEF_UART_DEF_H
#define DEF_UART_DEF_H

#define INTR_UART0_EVT					INTR_UART_EVT
#define UART0_Int_Handler(void)			UART_Int_Handler(void)	

/*!
 *****************************************************************************
 * \struct  ADI_UART_DEVICE_INFO
 *  Structure for storing basic device information.
 *****************************************************************************/

typedef struct _ADI_UART_DEVICE_INFO
{ 
    IRQn_Type              eIRQn;              /*!<  UART interrupt ID.                  */   
    ADI_UART_TypeDef      *pUartRegs;          /*!<  Base address of the UART registers. */       
    ADI_UART_HANDLE        hDevice;            /*!<  Handle for the device instance.     */

}ADI_UART_DEVICE_INFO;


/*!
 *****************************************************************************
 * \struct  ADI_UART_DEVICE
 *  Structure for managing the UART device. 
 *****************************************************************************/

typedef struct _ADI_UART_DEVICE
{
    ADI_UART_DEVICE_INFO       *pUartInfo;                       /*!< Access to device information about the uart instance.           */
    volatile ADI_UART_TypeDef  *pUARTRegs;                       /*!< Access to UART Memory Mapped Registers.                         */     
    ADI_CALLBACK                pfCallback;                      /*!< Callback function.                                              */
    volatile uint32_t           nHwError;                        /*!< Stores any hardware errors.                                     */
    volatile uint32_t           nUserReadIndex;                  /*!< Index points to where the application has last read from.  	  */
    volatile uint32_t           nUartReadIndex;                  /*!< Index points to where the driver has last read from.            */
    volatile uint8_t            nInternalOverrunFlag;            /*!< Flag to signify if an internal overrun has occured.             */
    uint8_t                     pRxBuffer[ADI_UART_BUFFER_SIZE]; /*!< Internal Rx software buffer.                                    */
} ADI_UART_DEVICE;


/*!
 *****************************************************************************
 * \struct  ADI_UART_CONFIG
 *  Structure for initializing the static config.
 *****************************************************************************/

typedef struct _ADI_UART_CONFIG
{
    uint16_t                    LCR;                 /*!< UART_COMLCR Register. */    

    uint16_t                    DIV;                 /*!< UART_COMDIV Register. */ 

    uint16_t                    FBR;                 /*!< UART_COMFBR Register. */ 

    uint16_t                    LCR2;                /*!< UART_COMLCR2 Register.*/ 

    uint16_t                    FCR;                 /*!< UART_COMFCR Register. */ 

    uint16_t                    RSC;                 /*!< UART_COMRSC Register. */              

    uint16_t                    IEN;                 /*!< UART_COMIEN Register .*/ 

} ADI_UART_CONFIG;


/******************************************************************************
 * UART Device internal API function prototypes
 *****************************************************************************/

/*
 * UART device initialization helper function.
*/
static void uart_init(ADI_UART_CONST_HANDLE const hDevice, uint32_t const nDeviceNum);

/* 
 * Handle Validation function
*/
#ifdef ADI_DEBUG
static ADI_UART_RESULT  ValidateHandle(ADI_UART_CONST_HANDLE hDevice);
#endif /* ADI_DEBUG */

#endif  /* end of ifndef DEF_UART_DEF_H */
/*! \endcond */


