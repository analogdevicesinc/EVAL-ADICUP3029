/*! *****************************************************************************
 * @file     adi_uart.c
 * @brief    Uart device driver implementation.
 * @details  This file contains the UART device driver functions.
 -----------------------------------------------------------------------------
Copyright (c) 2010-2017 Analog Devices, Inc.

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

/** @addtogroup UART_Driver
 *  @{
 *  @brief UART Driver
 */

/*!  \cond PRIVATE */
#include <adi_uart.h>
#include "adi_uart_def.h"
#include <adi_cyclecount.h>


#ifdef __ICCARM__
/*
* IAR MISRA C 2004 error suppressions.
*
* Pm50: (MISRA C 2004 rule 14.3): a null statement shall only occur on a line by itself,
*                                 and shall not have any other text on the same line
*                                 Some Macros, such as ISR_PROLOGUE, may not have any expansion
*                                 resulting in just the terminating ';'.
*
* Pm073 (rule 14.7): A function should have a single point of exit.
* Pm143 (rule 14.7): A function should have a single point of exit at the end of the function.
*                    Multiple returns are used for error handling.
*
* Pm088 (rule 17.4): Pointer arithmetic should not be used.
*                    Relying on pointer arithmetic for buffer handling.
*
* Pm123 (rule 18.5): There shall be no definition of objects in a header file.
*
* Pm140 (rule 11.3): A cast should not be performed between a pointer type and an integral type.
*                    MMR addresses are defined as simple constants. Accessing the MMR requires casting to a pointer type.
*
* Pm152 (rule 17.4): Array indexing shall only be applied to objects defined as an array type.
*                    Relying on pointer arithmetic for buffer handling and
*                    Accessing the DMA descriptors, which are defined in the system as a pointer to an array of descriptors.
*
* Pm008: Code should not be commented out.
         This code was commented out to show what the autobaud equations would look like if there were floating point precision.
         Ideally this would be the case but for the sake of footprint size we will leave it at single point precision.
*/
#pragma diag_suppress=Pm050,Pm073,Pm088,Pm123,Pm140,Pm143,Pm152,Pm008
#endif /* __ICCARM__ */




/**********************************************************
 * UART Data
 **********************************************************/
static  ADI_UART_DEVICE_INFO uart_device_info[ ] =
{
  {
    (IRQn_Type)INTR_UART0_EVT,   /*!< UART0 interrupt ID.              */
    pADI_UART0,                  /*!< Start address of UART0 MMRs.     */
    NULL                         /*!< Device Handle for UART0.         */
  },
};

static const ADI_UART_CONFIG gUARTCfg[ ] =
{
    {
      /* Line control register.                                         */
      ((ADI_UART0_CFG_WORD_LENGTH          << BITP_UART_LCR_WLS)  |
       (ADI_UART0_CFG_STOP_BIT             << BITP_UART_LCR_STOP) |
       (ADI_UART0_CFG_ENABLE_PARITY        << BITP_UART_LCR_PEN)  |
       (ADI_UART0_CFG_PARITY_SELECTION     << BITP_UART_LCR_EPS)  |
       (ADI_UART0_CFG_ENABLE_STICKY_PARITY << BITP_UART_LCR_SP)),

      /* Div-C in baudrate divider register.                            */
      ADI_UART0_CFG_DIVC,

      /* Div-M and Div-N in fractional baudrate Register.               */
      (((uint32_t)ADI_UART0_CFG_DIVN       << BITP_UART_FBR_DIVN) |
       ((uint32_t)ADI_UART0_CFG_DIVM       << BITP_UART_FBR_DIVM) |
       ((uint32_t)BITM_UART_FBR_FBEN)),

      /* Over sample rate in second line control register.              */
      ADI_UART0_CFG_OSR,

      /* FIFO control register. Force FIFO enabled.                     */
      ((1         						   << BITP_UART_FCR_FIFOEN)|
       (0                                  << BITP_UART_FCR_RFTRIG)),

      /* Half duplex control register.                                  */
      ((0                                  << BITP_UART_RSC_OENP)  |
       (0                                  << BITP_UART_RSC_OENSP) |
       (0                                  << BITP_UART_RSC_DISRX) |
       (0                                  << BITP_UART_RSC_DISTX)),

      /* Interrupt enable register.                                     */
      ((0                                  << BITP_UART_IEN_EDSSI) |
       (1                                  << BITP_UART_IEN_ELSI))
    },
};

/*! Number of UARTs available on the chip. */
#define ADI_UART_NUM_DEVICES (sizeof(uart_device_info)/sizeof(ADI_UART_DEVICE_INFO))

/*! Override "weak" default binding in startup.c */
extern void UART0_Int_Handler(void);
/*! \endcond */

/**********************************************************
 * General UART APIs
 **********************************************************/

/*!
 * @brief       Initialization function for the UART device.
 * @details     Opens the specified UART device. This function must be called before operating any UART device.
 *
 *
 * @param [in]  nDeviceNum     UART device instance to be opened.
 * @param [in]  pMemory        Pointer to a 32 bit aligned buffer the size of #ADI_UART_BIDIR_MEMORY_SIZE
 * @param [out] phDevice       The caller's device handle pointer for storing the initialized device instance data pointer.
 *
 * @return      Status
 *              - #ADI_UART_SUCCESS                     Successfully initialized UART device.
 *              - #ADI_UART_INVALID_DEVICE_NUM      [D] Device instance is invalid.
 *              - #ADI_UART_DEVICE_IN_USE           [D] Device is already open.
 *
 * @sa  adi_uart_Close()
 *
 * @note: Memory supplied by the API will be used by the driver for managing the UART device. This memory can be reused once
 *        device is closed.
 *
 */
ADI_UART_RESULT adi_uart_Open(
						   	 uint32_t            const nDeviceNum,
							 ADI_UART_DIRECTION  const eDirection,
							 void                     *pMemory,
							 uint32_t            const nMemSize,
							 ADI_UART_HANDLE    *const phDevice
						     )
{
#ifdef ADI_DEBUG
    /* Check if the given device number is within the range of UART
     * devices present in the processor. There are two devices present for
     * the ADuCM4050 so this can be a 0 or 1 and only 0 for ADuCM302x.
     */
    if(nDeviceNum >= ADI_UART_NUM_DEVICES)
    {
        return(ADI_UART_INVALID_DEVICE_NUM);
    }

    /* Verify the device is not already open. */
    if(uart_device_info[nDeviceNum].hDevice != NULL)
    {
        return(ADI_UART_DEVICE_IN_USE);
    }
#endif /* ADI_DEBUG */

    /* Link the ADI_UART_HANDLE to the ADI_UART_DEVICE structure. */
     ADI_UART_HANDLE hDevice = pMemory;

    /* Zero the device handle memory so we do not have to explicitely initialize
       the structure members to 0.
    */
    memset(pMemory, 0, ADI_UART_BIDIR_MEMORY_SIZE);

    /* Set the device information. */
    hDevice->pUartInfo = &uart_device_info[nDeviceNum];

    /* Set the base of the UART register address. We do this to minimize
       the cycle count when accessing the UART registers.
    */
    hDevice->pUARTRegs = uart_device_info[nDeviceNum].pUartRegs;

    /* Increment the device handle with the size of the UART device structure
       so we can set the channel data next without overwriting
       the #ADI_UART_DEVICE data.
    */
    pMemory = ((uint8_t *)pMemory +(sizeof(ADI_UART_DEVICE)));

    /* Initialize the device with the static config values.*/
    uart_init(hDevice, nDeviceNum);

    /* Write the device data pointer to the application's handle. */
    *phDevice = hDevice;

    /* Store the device handle. */
    uart_device_info[nDeviceNum].hDevice = hDevice;

    /* Enable UART Interrupt. */
    NVIC_ClearPendingIRQ(hDevice->pUartInfo->eIRQn);
    NVIC_EnableIRQ(hDevice->pUartInfo->eIRQn);

    /* Enable RX interrupts. We do this right away to prevent any overrun errors. */
    hDevice->pUARTRegs->IEN |= (1 << BITP_UART_IEN_ERBFI);

    /* Return SUCCESS */
    return(ADI_UART_SUCCESS);
}

/*!
 * @brief       Un-initialize the memory for the specified UART instance.
 *
 * @param [in]  hDevice   UART device handle whose operation is to be closed. This handle was obtained when the UART
 *                        device instance was opened successfully.
 *
 * @return      Status
 *    - #ADI_UART_SUCCESS               Successfully closed the UART device instance.
 *    - #ADI_UART_INVALID_HANDLE    [D] Invalid UART device handle.
 *
 * @details     Closes the operation of specified UART device. Device needs to be opened again for any further use.
 *
 * @sa  adi_uart_Open()
 *
 * @note: It is the user's responsibility to free/reuse the memory supplied during the opening of the device.
 */
ADI_UART_RESULT adi_uart_Close(
                               ADI_UART_HANDLE const hDevice
                              )
{
#ifdef ADI_DEBUG
    /* Validate the given handle. */
    if(ValidateHandle(hDevice) != ADI_UART_SUCCESS)
    {
        return(ADI_UART_INVALID_HANDLE);
    }
#endif /* ADI_DEBUG */

    /* Disable UART status interrupts. */
    hDevice->pUARTRegs->IEN = 0x00u;

    /* Disable UART event interrupt. */
    NVIC_DisableIRQ(hDevice->pUartInfo->eIRQn);

    /* Free up the device memory. */
    hDevice->pUartInfo->hDevice = NULL;

    return(ADI_UART_SUCCESS);
}


/*!
 * @brief       Blocking UART write using PIO mode. Call to this function will not return until the entire buffer is transmitted.
 *
 * @param [in]  hDevice    Device handle obtained from adi_uart_Open().
 * @param [in]  pBuffer    Pointer to data supplied by the API that is to be transmitted.
 * @param [in]  nBufSize   Size of the buffer(in bytes).
 * @param [out] pHwError   Pointer to an integer that correlates with #ADI_UART_HW_ERRORS, containg the hardware status.
 *                         If there is no hardware event, this will be 0.
 *
 * @return      Status
 *    - #ADI_UART_SUCCESS                             Successfully transmitted the data from the submitted buffer.
 *    - #ADI_UART_HW_ERROR_DETECTED                   Hardware error(s) detected. "pHwError" can be checked for the specific error code(s).
 *    - #ADI_UART_INVALID_HANDLE                  [D] Invalid UART device handle.
 *    - #ADI_UART_INVALID_POINTER                 [D] The pointer to the buffer being submitted is a NULL.
 *
 * @sa  adi_uart_ReadChar()
 *
 * @note: This function is a blocking function which means that the function returns only after the completion of
 *        buffer transmission. DMA is not supported.
*/
ADI_UART_RESULT adi_uart_Write(
                               ADI_UART_HANDLE const hDevice,
							   uint8_t        *const pBuffer,
                               uint32_t        const nBufSize,
                               uint32_t             *pHwError
                              )
{

	uint32_t nTimeout = 0;
	uint32_t nCounter;

#ifdef ADI_DEBUG
        /* Validate the given handle. */
        if(ValidateHandle(hDevice) != ADI_UART_SUCCESS)
        {
            return(ADI_UART_INVALID_HANDLE);
        }

        /* Validate the pointer to the buffer memory. */
        if(pBuffer == NULL)
        {
            return(ADI_UART_INVALID_POINTER);
        }
#endif /* ADI_DEBUG */

  /* Loop through all of the data in the buffer. */
  for(nCounter = 0; nCounter < nBufSize; nCounter++)
  {
  	  /* Write the data to the transmit shift register to send it out. */
      hDevice->pUARTRegs->COMTX = (uint16_t)pBuffer[nCounter];

      /* Wait for the data to leave the transmit shift register. */
      while ((hDevice->pUARTRegs->LSR & BITM_UART_LSR_TEMT) != BITM_UART_LSR_TEMT)
      {
      	  /* Timeout to prevent getting stuck in this busy loop. */
    	  if(nTimeout == 0xFFFFFFFu)
    	  {
    		  return(ADI_UART_WRITE_TIMEOUT);
    	  }
    	  nTimeout++;
      }
  }
 
  /* If there are hardware errors, return failure. */
  if(hDevice->nHwError != 0u)
  {
      /* Save the hardware error detected. This will be passed back to the API. */
      *pHwError = hDevice->nHwError;

      /* Clear any hardware errors detected. */
      hDevice->nHwError = 0u;

      return(ADI_UART_HW_ERROR_DETECTED);
  }

  return(ADI_UART_SUCCESS);
}

/*!
 * @brief       Single byte blocking read using PIO mode. A call to this function will not return until the entire buffer is filled up. 
 *
 * @param [in]  hDevice    Device handle obtained from adi_uart_Open().
 * @param [in]  pBuffer    Pointer to buffer from that will be filled by the driver when data has been received.
 * @param [out] pHwError   Pointer to an integer that correlates with #ADI_UART_HW_ERRORS, containg the hardware status.
 *                         If there is no hardware event, this will be 0.
 *
 * @return      Status
 *    - #ADI_UART_SUCCESS                        Successfully submitted the buffer for receiving data.
 *    - #ADI_UART_HW_ERROR_DETECTED              Hardware error(s) detected. "pHwError" can be checked for the specific error code(s).
 *    - #ADI_UART_INVALID_HANDLE             [D] Invalid UART device handle.
 *    - #ADI_UART_INVALID_POINTER            [D] Pointer to the buffer being submitted is NULL.
 *    - #ADI_UART_READ_TIMEOUT 					 Timed out waiting for a byte to read.
 *
 * @sa  adi_uart_Write()
 *
 * @note: This function is a blocking function which means that the function returns only after the completion of
 *        data receive. DMA is not supported. 
*/
ADI_UART_RESULT adi_uart_ReadChar(
                              ADI_UART_HANDLE const hDevice,
                              uint8_t        *const pBuffer,
                              uint32_t             *pHwError
                             )
{
	uint32_t nTimeout = 0;

#ifdef ADI_DEBUG
     /* Validate the given handle. */
     if(ValidateHandle(hDevice) != ADI_UART_SUCCESS)
     {
         return(ADI_UART_INVALID_HANDLE);
     }

    /* Validate the pointer to the buffer memory. */
    if(pBuffer == NULL)
    {
        return(ADI_UART_INVALID_POINTER);
    }
#endif /* ADI_DEBUG */

  /* Wait for data to become available for reading. There are two cases where we know there is new data in
  	 the uart driver software buffer to be read by the user. The first is when the indices are not the same. 
  	 This means that the uart index has gone ahead and begun filling the uart driver software buffer. 
  	 The second is when the indices are the same, but the uart index has "lapped" the user index and an overrun 
  	 the uart driver software buffer. In this case, even though the indices are the same, the nInternalOverrunFlag
     will be set to show that there is valid data in the uart driver software buffer.*/  
  while((hDevice->nUserReadIndex == hDevice->nUartReadIndex) && (hDevice->nInternalOverrunFlag == 0u))
  {
	  if(nTimeout == 0xFFFFFFu)
	  {
		  return(ADI_UART_READ_TIMEOUT);
	  }
	  nTimeout++;
  }

  /* Clear the overrun flag if it was set. */
  hDevice->nInternalOverrunFlag = 0u;

  /* Read the uart driver software buffer to get the next data. */
  pBuffer[0] = hDevice->pRxBuffer[hDevice->nUserReadIndex];

  /* The internal software buffer for the UART is treated as a circular buffer so increment it as so. */
  if(hDevice->nUserReadIndex < (ADI_UART_BUFFER_SIZE - 1))
  {
      hDevice->nUserReadIndex++;
  }
  else
  {
      hDevice->nUserReadIndex = 0;
  }

  /* If there are hardware errors, return failure. */
  if(hDevice->nHwError != 0u)
  {
      /* Save the hardware error detected. This will be passed back to the API. */
      *pHwError = hDevice->nHwError;

      /* Clear any hardware errors detected. */
      hDevice->nHwError = 0u;

      return(ADI_UART_HW_ERROR_DETECTED);
  }


  return(ADI_UART_SUCCESS);


}

/*!
 * @brief       Registering a callback function.
 *
 * @param [in]  hDevice       Device handle obtained from adi_uart_Open().
 * @param [in]  pfCallback    Function pointer to callback. Passing a NULL pointer will unregister
 *                            the callback function.
 *
 * @return      Status
 *    - #ADI_UART_SUCCESS                Successfully registered callback function.
 *    - #ADI_UART_INVALID_HANDLE     [D] Invalid UART device handle.
 *
*/
ADI_UART_RESULT adi_uart_RegisterCallback(
                                          ADI_UART_HANDLE const hDevice,
                                          const ADI_CALLBACK    pfCallback
                                         )
{

#ifdef ADI_DEBUG
    /* Validate the given handle. */
    if(ValidateHandle(hDevice) != ADI_UART_SUCCESS)
    {
        return(ADI_UART_INVALID_HANDLE);
    }
#endif /* ADI_DEBUG */

    /* Set the device callback. */
    hDevice->pfCallback = pfCallback;

    return(ADI_UART_SUCCESS);
}


/*!
 * @brief        Configuration of UART data.
 *
 * @details      Sets the configuration parameters for the specified UART device such as word length, whether to
 *               enable/disable the parity, and the number of stop bits.
 *
 * @param [in]  hDevice      Device handle obtained from adi_uart_Open().
 * @param [in]  eParity      Specify the type of parity check for the UART device.
 * @param [in]  eStopBits    Specify the stop-bits for the UART device.
 * @param [in]  eWordLength  Specify the word size of the data for the UART device.
 *
 * @return      Status
 *    - #ADI_UART_SUCCESS                Successfully set the data configuration.
 *    - #ADI_UART_INVALID_HANDLE     [D] Invalid UART device handle.
 *
*/
ADI_UART_RESULT adi_uart_SetConfiguration(
                                         ADI_UART_HANDLE   const hDevice,
                                         ADI_UART_PARITY   const eParity,
                                         ADI_UART_STOPBITS const eStopBits,
                                         ADI_UART_WORDLEN  const eWordLength
                                         )
{
#ifdef ADI_DEBUG
    /* Validate the given handle. */
    if(ValidateHandle(hDevice) != ADI_UART_SUCCESS)
    {
        return(ADI_UART_INVALID_HANDLE);
    }
#endif /* ADI_DEBUG */

    /* Clear all the fields.  */
    uint16_t nDataCfg = hDevice->pUARTRegs->LCR & (uint16_t)(~(BITM_UART_LCR_WLS |BITM_UART_LCR_STOP |BITM_UART_LCR_PEN));

    /* Construct the configuration word. */
    nDataCfg |= (uint16_t)(((uint16_t)((uint16_t)eWordLength |(uint16_t)eStopBits) |(uint16_t)eParity));

    /* Write to the register */
    hDevice->pUARTRegs->LCR = nDataCfg;

    /* Return Success */
    return(ADI_UART_SUCCESS);
}

/*!
 * @brief       Flush the Rx channel and fifo.
 *
 * @param [in]  hDevice      Device handle obtained from adi_uart_Open().
 *
 * @return      Status
 *    - #ADI_UART_SUCCESS                Successfully set the data configuration.
 *    - #ADI_UART_INVALID_HANDLE     [D] Invalid UART device handle.
 *
*/
ADI_UART_RESULT adi_uart_FlushRxChannel(ADI_UART_HANDLE   const hDevice)
{
	ADI_INT_STATUS_ALLOC();

#ifdef ADI_DEBUG
    /* Validate the given handle. */
    if(ValidateHandle(hDevice) != ADI_UART_SUCCESS)
    {
        return(ADI_UART_INVALID_HANDLE);
    }
#endif /* ADI_DEBUG */

    /* Clear any data in the Rx Fifo. */
    hDevice->pUARTRegs->FCR |=  BITM_UART_FCR_RFCLR;

    /* Reset the index for the UART internal buffer. We do not need to memset it to 0 as the
     * current data will be overwritten by the next read operation. Also reset the overrun flag
     * as we are starting from scratch and any prior overruns will be irrelevant now.
     */
    ADI_ENTER_CRITICAL_REGION();
    hDevice->nUserReadIndex = 0u;
    hDevice->nUartReadIndex = 0u;
    hDevice->nInternalOverrunFlag = 0u;
    ADI_EXIT_CRITICAL_REGION();

    /* Return success */
    return(ADI_UART_SUCCESS);
}
/*!
 * @brief        Set baudrate by configuring the fractional dividers.
 *
 * @details      Baudrate is calculated as per below equation.
 *
 *               Baudrate = (UARTCLK / (nDivM + nDivN/2048)*pow(2,nOSR+2)* nDivC)).
 *
 * @param [in]  hDevice      Device handle obtained from adi_uart_Open().
 * @param [in]  nDivC        Specify the "nDivC" in the above equation.
 * @param [in]  nDivM        Specify the "nDivM" in the above equation.
 * @param [in]  nDivN        Specify the "nDivN" in the above equation.
 * @param [in]  nOSR         Specify the "nOSR" " in the above equation.
 *
 * @return      Status
 *    - #ADI_UART_SUCCESS                 Successfully set the baudrate for the device.
 *    - #ADI_UART_INVALID_HANDLE     [D]  Invalid UART device handle.
 *    - #ADI_UART_INVALID_PARAMETER  [D]  Input for baud rate values are out of range.
 *
 *   @sa  adi_uart_GetBaudRate()
 *   @sa  adi_uart_EnableAutobaud();
 *
 *   @note It is expected that initialization of the power management
 *         driver is done before calling this function.
 *
 */
ADI_UART_RESULT adi_uart_ConfigBaudRate(
                                       ADI_UART_HANDLE const hDevice,
                                       uint16_t        const nDivC,
                                       uint8_t         const nDivM,
                                       uint16_t        const nDivN,
                                       uint8_t         const nOSR
                                       )
{
	ADI_INT_STATUS_ALLOC();

#ifdef ADI_DEBUG
    /* Validate the given handle */
    if(ValidateHandle(hDevice) != ADI_UART_SUCCESS)
    {
        return(ADI_UART_INVALID_HANDLE);
    }

    /* Check if the given baudrate is valid */
    if( (nDivM < 1u) || (nDivM > 3u)|| (nDivN > 2047u ) || (nOSR > 3u))
    {
        return ADI_UART_INVALID_PARAMETER;
    }

#endif /* ADI_DEBUG */

    /* Write back the register contents for baudrate detection in the hardware. */
    ADI_ENTER_CRITICAL_REGION();
    hDevice->pUARTRegs->DIV  =  nDivC;
    hDevice->pUARTRegs->FBR  =  (uint16_t)((uint16_t)nDivN | (uint16_t)((uint16_t)nDivM <<BITP_UART_FBR_DIVM)) | (uint16_t)BITM_UART_FBR_FBEN;
    hDevice->pUARTRegs->LCR2 = nOSR;
    ADI_EXIT_CRITICAL_REGION();

    return(ADI_UART_SUCCESS);
}

/*! \cond PRIVATE */
/* UART interrupt handler */
void UART0_Int_Handler(void)
{
    ISR_PROLOG();
    ADI_UART_HANDLE hDevice = (ADI_UART_HANDLE)uart_device_info[0].hDevice;

    switch(hDevice->pUARTRegs->IIR & BITM_UART_IIR_STAT)
    {
        /* Tx buffer empty interrupt. This means that the data has successfully left the holding register and is
           now in transmit shift register or has completed its transfer.
        */
        case ENUM_UART_IIR_STAT_ETBEI:
          break;

        /* Rx buffer full interrupt. This means that the RX buffer has received a byte of data. */
        case ENUM_UART_IIR_STAT_ERBFI:

        /* Rx buffer FIFO timeout interrupt. This means that we have data in the RX FIFO
           but there is not enough data to trigger an interrupt so we will process this data here.
        */
        case ENUM_UART_IIR_STAT_RFTOI:

          /* Read the UART data into the uart device software buffer. */
          hDevice->pRxBuffer[hDevice->nUartReadIndex] = hDevice->pUARTRegs->RX;

          /* Increment the uart read index for the "circular" uart device internal buffer. */
          if(hDevice->nUartReadIndex < (ADI_UART_BUFFER_SIZE - 1))
          {
              hDevice->nUartReadIndex++;
          }
          else
          {
              hDevice->nUartReadIndex = 0;
          }

          /* If the uart has filled the buffer "lapped" the user notify that user that we overrunning
           * the internal buffer.
           */
          if(hDevice->nUartReadIndex == hDevice->nUserReadIndex)
          {
        	  hDevice->nInternalOverrunFlag = 1u;
          }

          /* Notify the application we have received data in the uart device software buffer. */
          if(hDevice->pfCallback != NULL)
          {
        	  /* Notify the application there is data to read. */
              hDevice->pfCallback(0, ADI_UART_EVENT_RX_DATA, NULL);
          }

          break;         

        /* Line status interrupt. */
        case ENUM_UART_IIR_STAT_RLSI:
        {
            /* Initialze the line status event to 0. */
            uint32_t nEvent = 0u;

            /* Get the interrupts status. */
            uint16_t  nStatus = hDevice->pUARTRegs->LSR;

            /* If a break signal is detected.. */
            if((BITM_UART_LSR_BI & nStatus) == BITM_UART_LSR_BI)
            {
                /* Dummy read to flush the RX register. We do this because
                   we do not actaully want to do anything with this data as it
                   is only a break indicator. */
                hDevice->pUARTRegs->RX;

                /* Set the event to a break interrupt. */
                nEvent  = (uint32_t)ADI_UART_BREAK_INTERRUPT;
            }

            /* Ignore the framing error if the break is asserted.
               We do this because a break can trigger a false framing error.
            */
            else if((BITM_UART_LSR_FE & nStatus) == BITM_UART_LSR_FE)
            {
                /* Set the event to show a framing error has been detected. */
                nEvent |= (uint32_t)ADI_UART_HW_ERR_FRAMING;
            }
            else
            {
              /* Do nothing. This is required for MISRA. */
            }

            if((BITM_UART_LSR_PE & nStatus) == BITM_UART_LSR_PE)
            {
                /* Set the event to show a parity error has been detected. */
                nEvent  |=  (uint32_t)ADI_UART_HW_ERR_PARITY;
            }
            if((BITM_UART_LSR_OE & nStatus) == BITM_UART_LSR_OE)
            {
                /* Set the event to show a hardware overrun error has been detected, meaning receive data has
                   been overwritten.
                */
                nEvent |= (uint32_t)ADI_UART_HW_ERR_OVERRUN;
            }

            /* If there was an event, notify the API. */
            if(nEvent != 0u)
            {
                if(hDevice->pfCallback != NULL)
                {
                    hDevice->pfCallback(0u, ADI_UART_EVENT_HW_ERROR_DETECTED, (void*) nEvent);
                }
                else
                {
	                /* Set the UART device hw error bit field. This will allow us to return the
	                   specific failure to the application once we return from this ISR.
	                */
	                hDevice->nHwError |= nEvent;
                }
            }
            break;
         }
    }

#if defined(ADI_CYCLECOUNT_UART_ISR_ENABLED) && (ADI_CYCLECOUNT_UART_ISR_ENABLED == 1u)
    ADI_CYCLECOUNT_STORE(ADI_CYCLECOUNT_ISR_UART);    
#endif
    ISR_EPILOG();
    return;
}

/*
 * @brief       Initialize the UART instance to the default values specified in "adi_uart_config.h".
 *
 * @param [in]  hDevice    Device handle obtained from adi_uart_Open().
 * @param [in]  nDeviceNum  UART device number
*/

static void uart_init(ADI_UART_CONST_HANDLE const hDevice, uint32_t const nDeviceNum)
{

    ADI_UART_CONFIG const* pUARTCfg = &gUARTCfg[nDeviceNum];

    /* Line Control Register. */
    hDevice->pUARTRegs->LCR = pUARTCfg->LCR;

    /* Div-C in Baudrate divider register. */
    hDevice->pUARTRegs->DIV = pUARTCfg->DIV;

    /* Div-M and Div-N in Fractional Baudrate register. */
    hDevice->pUARTRegs->FBR = pUARTCfg->FBR;

    /* Second line control register. */
    hDevice->pUARTRegs->LCR2 = pUARTCfg->LCR2;

    /* FIFO control register. */
    hDevice->pUARTRegs->FCR  = pUARTCfg->FCR;

    /* Half Duplex Control Register. */
    hDevice->pUARTRegs->RSC  = pUARTCfg->RSC;

    /* Interrupt enable register. */
    hDevice->pUARTRegs->IEN  = pUARTCfg->IEN;
}

#ifdef ADI_DEBUG
/*
 * @brief       Validate the device handle.
 *
 * @param [in]  hDevice    Device handle obtained from adi_uart_Open().
 *
 * @return      Status
 *    - #ADI_UART_SUCCESS          Specified handle is valid.
 *    - #ADI_UART_INVALID_HANDLE   Specified handle is invalid.
 *
*/

static ADI_UART_RESULT ValidateHandle(ADI_UART_CONST_HANDLE hDevice)
{
    uint32_t i;


    for(i = 0U; i <  ADI_UART_NUM_DEVICES; i++)
    {

      if((hDevice == uart_device_info[i].hDevice) && (hDevice != NULL))
       {
           return(ADI_UART_SUCCESS);
       }
    }
    return(ADI_UART_INVALID_HANDLE);
}
#endif /* ADI_DEBUG */
/*! \endcond */
/*@}*/
