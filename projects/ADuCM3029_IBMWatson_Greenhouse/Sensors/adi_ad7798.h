/*! 
 *****************************************************************************
  @file adi_ad7798.h
 
  @brief AD7798 class definition.
 
  @details
 -----------------------------------------------------------------------------
Copyright (c) 2017 Analog Devices, Inc.

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

/*! 
 * @defgroup sensor_macros Sensor Macros
 */

/*! @addtogroup ad7798_macros AD7798 Macros
 *  @ingroup sensor_macros
 *  @brief Non-configurable macros for the AD7798 class.
 *  @{
 */

#ifndef ADI_AD7798_H
#define ADI_AD7798_H

#include <adi_adc.h>
#include <adi_sensor_errors.h>
#include <adi_sensor.h>
#include <drivers/spi/adi_spi.h>

/********** Bit positions within registers (AD7798_REG_COMM) **********/
/*!< Write enable bit position.                                                                       */
#define AD7798_COMM_WEN_P                     (0x07u)
/*!< Read/write operation bit position.                                                               */
#define AD7798_COMM_READ_WRITE_P              (0x06u)
/*!< Register address bit position.                                                                   */
#define AD7798_COMM_REG_ADDR_P                (0x03u)
/*!< Continuous read operation bit position.                                                          */
#define AD7798_COMM_CONT_READ_P               (0x02u)

/********** Bit positions within registers (AD7798_REG_MODE) **********/
/*!< Filter update rate select bits position.                                                         */
#define AD7798_MODE_RATE_P                    (0x0Fu)
/*!< Operating mode bit position.                                                                     */
#define AD7798_MODE_SEL_P                     (0x0Du)
/*!< Power switch control bit position.                                                               */
#define AD7798_MODE_PSW_P                     (0x0Cu) 

/********** Bit positions within registers (AD7798_REG_CONF) **********/
/*!< Burnout current bit position.                                                                    */
#define AD7798_CONF_BO_P                      (0x0Du) 
/*!< Coding bit position.                                                                             */
#define AD7798_CONF_CODING_P                  (0x0Cu) 
/*!< Gain bit position.                                                                               */
#define AD7798_CONF_GAIN_P                    (0x08u) 
/*!< Reference detect function bit position.                                                          */
#define AD7798_CONF_REFDET_P                  (0x05u) 
/*!< Buffer mode bit position.                                                                        */
#define AD7798_CONF_BUF_P                     (0x04u) 
/*!< Channel select bit position.                                                                     */
#define AD7798_CONF_CHAN_P                    (0x00u)

/********** Bit positions within registers (AD7798_REG_IO) **********/
/*!< IO enable bit position.                                                                         */
#define AD7798_IO_IOEN_P                      (0x06u) 

/********** Status register masks (AD7798_REG_STAT) **********/
/*!< Ready mask.                                                                                      */
#define AD7798_STAT_RDY                       (1 << 0x07u) 
/*!< Error mask. Sources include overrange and underrange.                                            */
#define AD7798_STAT_ERR                       (1 << 0x06u) 
/*!< No reference mask. Indicates the reference voltage is below a specified threshold.               */
#define AD7798_STAT_NOREF                     (1 << 0x05u) 
/*!< Channel 3 mask. Indicates channel 3 is being converted by the ADC.                               */
#define AD7798_STAT_CH3                       (1 << 0x02u) 
/*!< Channel 2 mask. Indicates channel 2 is being converted by the ADC.                               */
#define AD7798_STAT_CH2                       (1 << 0x01u) 
/*!< Channel 1 mask. Indicates channel 1 is being converted by the ADC.                               */
#define AD7798_STAT_CH1                       (1 << 0x00u) 

/********** Communication register setters (AD7798_REG_COMM) **********/
/*!< Write enable setter.                                                                             */
#define AD7798_COMM_WEN                       (0 << AD7798_COMM_WEN_P)  
/*!< Write operation setter.                                                                          */      
#define AD7798_COMM_WRITE                     (0 << AD7798_COMM_READ_WRITE_P)     
/*!< Read operation setter.                                                                           */
#define AD7798_COMM_READ                      (1 << AD7798_COMM_READ_WRITE_P)     
/*!< Continuous read of data register setter.                                                         */
#define AD7798_COMM_CREAD                     (1 << AD7798_COMM_CONT_READ_P) 
/*!< Sets up a write operation.                                                                       */      
#define AD7798_COMM_WRITE_REG(regAddress)     (AD7798_COMM_WEN | AD7798_COMM_WRITE | (regAddress & 0x07u) << AD7798_COMM_REG_ADDR_P)
/*!< Sets up a read operation.                                                                        */     
#define AD7798_COMM_READ_REG(regAddress)      (AD7798_COMM_WEN | AD7798_COMM_READ | ((regAddress & 0x07u) << AD7798_COMM_REG_ADDR_P))
/*!< Sets up a continuous data read operation.                                                        */     
#define AD7798_COMM_READ_DATA_CONT            (AD7798_COMM_WEN | AD7798_COMM_READ | ((0x03u & 0x07u) << AD7798_COMM_REG_ADDR_P) | AD7798_COMM_CREAD_M)

/********** Mode register setters (AD7798_REG_MODE) **********/
/*!< Sets an operating mode.                                                                          */      
#define AD7798_MODE_SEL(x)                    (((x) & 0x07u) << AD7798_MODE_SEL_P) 
/*!< Sets the power switch control bit.                                                               */      
#define AD7798_MODE_PSW(x)                    (((x) & 0x01u) << AD7798_MODE_PSW_P)    
/*!< Sets the filter update rate select bits.                                                         */      
#define AD7798_MODE_RATE(x)                   (((x) & 0x0Fu) & AD7798_MODE_RATE_P)    

/********** Configuration register setters (AD7798_REG_CONF) **********/
/*!< Sets the burnout current bit.                                                                    */
#define AD7798_CONF_BO(x)                     (((x) & 0x01u) << AD7798_CONF_BO_P)
/*!< Sets unipolar/bipolar coding bit.                                                                */
#define AD7798_CONF_CODING(x)                 (((x) & 0x01u) << AD7798_CONF_CODING_P)
/*!< Sets the gain select bits.                                                                       */
#define AD7798_CONF_GAIN(x)                   (((x) & 0x07u) << AD7798_CONF_GAIN_P)  
/*!< Sets the reference detect function.                                                              */
#define AD7798_CONF_REFDET(x)                 (((x) & 0x01u) << AD7798_CONF_REFDET_P)  
/*!< Sets the buffered mode enable bit.                                                               */
#define AD7798_CONF_BUF(x)                    (((x) & 0x01u) << AD7798_CONF_BUF_P)      
/*!< Sets the channel select bits.                                                                    */
#define AD7798_CONF_CHAN(x)                   (((x) & 0x07u) << AD7798_CONF_CHAN_P)      

/********** IO (Excitation Current Sources) register setters (AD7798_REG_IO) **********/
/*!< Sets pins P1 and P1 as digital output pins.                                                      */
#define AD7798_IO_IOEN(x)                     (((x) & 0x01u) << AD7798_IO_IOEN_P)

/********** AD9370 Definitions **********/
/*!< ID value.                                                                                        */
#define AD7798_ID                           (0x08u)
/*!< ID bit mask.                                                                                     */
#define AD7798_ID_MASK                      (0x0Fu)
/*!< Reset Value.                                                                                     */
#define AD7798_RESET                        (0xFFu)
/*!< Format buffer data from a register read.                                                         */
#define AD7798_FORMAT_DATA(byte1, byte2)    ((byte2 | (byte1 << 0x08u)))
/*!< The maximum register size of the ADC in bytes. For the AD7798 this value is 2 bytes.             */
#define AD7798_MAX_REG_SIZE                 (2u)
/*!< The maximum timeout in cycles for an ADC transaction to wait on the RDY signal to go low.        */
#define AD7798_TRANSACTION_TIMEOUT          (20000000u)
/*!< AD7798 reference voltage.                                                                        */  
#define AD7798_REFERENCE_VOLTAGE            (3150.0)
/*!< 2^ADC Resolution = 2^16 as the AD7798 is a 16-bit ADC.                                           */
#define AD7798_2_TO_THE_RES                 (65535.0)

/*! @} */

extern uint8_t             m_spi_memory[ADI_SPI_MEMORY_SIZE];

namespace adi_sensor_swpack
{

  /*!
   * @class AD7798 
   * @brief 16-bit Sigma-Delta Analog-to-Digital Converter
   *
   **/
  #pragma pack(push)
  #pragma pack(4)
  class AD7798 : public ADC 
  {
      public:
    /*!
     *  @enum    ERROR_CODES
     *
     *  @brief   AD7798 error codes.
     *
     *  @details Error codes specific to the AD7798 class that will
     *           get packed into the ADC base class error code functions
     *           as a SENSOR_ERROR_ADC in the SENSOR_RESULT type.
     */
    typedef enum
    {
      ERROR_CODE_ID_MISMATCH = 0u, /*!< ID register does not match the expected value.  */
      ERROR_CODE_RDY_TIMEOUT = 1u, /*!< Timed out waiting for a conversion to complete. */
    } ERROR_CODES;

    /*!
     * @enum REGISTER
     * @brief AD7798 register values.
     *
     **/
    typedef enum
    {
      REGISTER_COMM_STAT         = 0x00u, /*!< Communications Register(WO, 8-bit) during a write operation Status Register (RO, 8-bit).      */
      REGISTER_MODE              = 0x01u, /*!< Mode Register          (RW, 16-bit).                                                          */
      REGISTER_CONF              = 0x02u, /*!< Configuration Register (RW, 16-bit).                                                          */
      REGISTER_DATA              = 0x03u, /*!< Data Register          (RO, 16-/24-bit).                                                      */
      REGISTER_ID                = 0x04u, /*!< ID Register            (RO, 8-bit).                                                           */
      REGISTER_IO                = 0x05u, /*!< IO Register            (RO, 8-bit).                                                           */
      REGISTER_OFFSET            = 0x06u, /*!< Offset Register        (RW, 16/24-bit).                                                       */
      REGISTER_FULLSCALE         = 0x07u, /*!< Full-Scale Register    (RW, 16/24-bit).                                                       */
    }REGISTER;

    /*!
     * @enum CODING_MODE
     * @brief AD7798 coding mode bit.
     *
     **/
    typedef enum
    {
      BIPOLAR          = 0x00u, /*!< Used to enable bipolar coding.  */
      UNIPOLAR         = 0x01u, /*!< Used to enable ubipolar coding. */
    }CODING_MODE;

    /*!
     * @enum OPERATING_MODE
     * @brief AD7798 operating modes.
     *
     **/
    typedef enum
    {
      OPERATING_MODE_CONT               = 0x00u, /*!< Continuous conversion mode.      */
      OPERATING_MODE_SINGLE             = 0x01u, /*!< Single conversion mode.          */
      OPERATING_MODE_IDLE               = 0x02u, /*!< Idle mode.                       */
      OPERATING_MODE_PWRDN              = 0x03u, /*!< Power-down mode.                 */
      OPERATING_MODE_CAL_INT_ZERO       = 0x04u, /*!< Internal zero-scale calibration. */
      OPERATING_MODE_CAL_INT_FULL       = 0x05u, /*!< Internal full-scale calibration. */
      OPERATING_MODE_CAL_SYS_ZERO       = 0x06u, /*!< System zero-scale calibration.   */
      OPERATING_MODE_CAL_SYS_FULL       = 0x07u, /*!< System full-scale calibration.   */
    }OPERATING_MODE;

    /*!
     * @enum GAIN
     * @brief Available gain settings for the AD7798.
     *
     **/
    typedef enum
    {
      GAIN_1               = 0x00u, /*!< Gain 1 input range is 2.5V.          */
      GAIN_2               = 0x01u, /*!< Gain 2 input range is 1.25V.         */
      GAIN_4               = 0x02u, /*!< Gain 4 input range is 625mV.         */
      GAIN_8               = 0x03u, /*!< Gain 8 input range is 312.5mV.       */
      GAIN_16              = 0x04u, /*!< Gain 16 input range is 156.2mV.      */
      GAIN_32              = 0x05u, /*!< Gain 32 input range is 78.125mV.     */
      GAIN_64              = 0x06u, /*!< Gain 64 input range is 39.06mV.      */
      GAIN_128             = 0x07u, /*!< Gain 128 input range is 19.53mV.     */
    }GAIN;

    /*!
     * @enum FILTER_RATE
     * @brief Filter update rate select bits.
     *
     **/
    typedef enum
    {
      FILTER_RATE_470              = 0x01u, /*!< Update rate is 470 Hz.                             */
      FILTER_RATE_242              = 0x02u, /*!< Update rate is 242 Hz.                             */
      FILTER_RATE_123              = 0x03u, /*!< Update rate is 123 Hz.                             */
      FILTER_RATE_62               = 0x04u, /*!< Update rate is 62 Hz.                              */
      FILTER_RATE_50               = 0x05u, /*!< Update rate is 50 Hz.                              */
      FILTER_RATE_39               = 0x06u, /*!< Update rate is 39 Hz.                              */
      FILTER_RATE_33_2             = 0x07u, /*!< Update rate is 33.2 Hz.                            */
      FILTER_RATE_19_6             = 0x08u, /*!< Update rate is 19.6 Hz.                            */
      FILTER_RATE_16_7_80          = 0x09u, /*!< Update rate is 16.7 Hz with 80 db rejection.       */
      FILTER_RATE_16_7_65          = 0x0Au, /*!< Update rate is 16.7 Hz with 65 db rejection.       */
      FILTER_RATE_12_5             = 0x0Bu, /*!< Update rate is 12.5 Hz.                            */
      FILTER_RATE_10               = 0x0Cu, /*!< Update rate is 10 Hz.                              */
      FILTER_RATE_8_33             = 0x0Du, /*!< Update rate is 8.33 Hz.                            */
      FILTER_RATE_6_25             = 0x0Eu, /*!< Update rate is 6.25 Hz.                            */
      FILTER_RATE_4_17             = 0x0Fu, /*!< Update rate is 4.17 Hz.                            */
    }FILTER_RATE;

    /*!
     * @enum REF_DET
     * @brief Enable or disable the reference detect function.
     *
     **/
    typedef enum
    {
      REF_DET_DIS             = 0x00u, /*!< Disable reference detect function.        */
      REF_DET_EN              = 0x01u, /*!< Enable reference detect function.         */
    }REF_DET;

    /*!
     * @enum CHANNEL
     * @brief Channel id used for enabling an analog input channel.
     *
     **/
    typedef enum
    {
      CHANNEL_AIN1P_AIN1M             = 0x00u, /*!< Channel 1 plus/minus.        */
      CHANNEL_AIN2P_AIN2M             = 0x01u, /*!< Channel 2 plus/minus.        */
      CHANNEL_AIN3P_AIN3M             = 0x02u, /*!< Channel 3 plus/minus.        */
      CHANNEL_AIN1M_AIN1M             = 0x03u, /*!< Channel 1 minus/minus.       */
      CHANNEL_AVDD_MONITOR            = 0x07u, /*!< AVdd monitor.                */
    }CHANNEL;

    /*!
     * @enum BURNOUT_CURRENT
     * @brief Enable or disable the burnout current.
     *
     **/
    typedef enum
    {
      BO_DIS             = 0x00u, /*!< Disable the burnout current.                             */
      BO_EN              = 0x01u, /*!< Enable the 100 nA current sources in the signal path.    */
    }BURNOUT_CURRENT;


     /*!
      * @brief  Constructor for the AD7798 class.
      *
      * @details This function initializes the SPI radio structure member variable. 
      *
      */          
      AD7798();

     /*!
      * @brief  Initializes the ADC.
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      * @details Init function initializes the ADC and underlying
      *          peripheral. This API has to be implemented by all ADC classes.
      *          A read of the ADC ID value is performed to confirm the ADC
      *          is responding correctly.
      * @note    An assumption is made that the SPI bus specified in the configuration 
      *          file will not be modified by another sensor. As a result, it will only
      *          be configured during this function and not on a transaction by transaction
      *          basis. 
      */
      virtual SENSOR_RESULT   init();

     /*!
      * @brief  Reset the ADC.
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      * @details This function resets all ADC registers to their default values.
      *          The user must wait 500 us after a reset before accessing an
      *          on chip register. This API has to be implemented by all ADC classes.
      */
      virtual SENSOR_RESULT   reset();

     /*!
      * @brief  Read a specified register on the ADC.
      *
      * @param  [in] regAddress : The address of the register to read from. 
      * @param  [in] size       : The number of bytes to read. The largest register
      *                           size is 16 bits so this needs to be less than 3 bytes. 
      * @param  [out] regValue  : The value read from the register.
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      virtual SENSOR_RESULT   readRegister(uint32_t regAddress, uint32_t size, uint8_t * regValue);

     /*!
      * @brief  Write to a specified register on the ADC.
      *
      * @param  [in] regAddress : The address of the register to write to. 
      * @param  [in] regValue   : The value to write to the register. 
      * @param  [in] size       : The number of bytes to write. The largest register
      *                           size is 16 bits so this needs to be less than 3 bytes.
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      virtual SENSOR_RESULT   writeRegister(uint32_t regAddress, uint32_t size, uint32_t regValue);

     /*!
      * @brief  Get ADC data using continuous conversion mode.
      *
      * @param  [out] data : Location where 2 byte data will be written. 
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      SENSOR_RESULT   getData(uint16_t * data);

     /*!
      * @brief  Set the ADC input channel.
      *
      * @param  [in] channel :  Value of the channel to set as an input to the ADC.
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      SENSOR_RESULT   setChannel(AD7798::CHANNEL channel);

     /*!
      * @brief  Set the operating mode of the ADC.
      *
      * @param  [in] mode : Operating mode to set.
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      SENSOR_RESULT   setOperatingMode(AD7798::OPERATING_MODE mode);

     /*!
      * @brief  Run a full-scale system calibration.
      *
      * @details A full-scale calibration is required each time the gain of a channel is changed. 
      *          The calibration is only run on the selected ADC input channel.  
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      SENSOR_RESULT   fullScaleSystemCalibration();

     /*!
      * @brief  Run a zero-scale system calibration.
      *
      * @details A zero-scale calibration is required each time the gain of a channel is changed. 
      *          The calibration is only run on the selected ADC input channel.  
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      SENSOR_RESULT   zeroScaleSystemCalibration();

     /*!
      * @brief  Set the coding mode of the configuration register.S
      *
      * @param  [in] mode : Value of the coding mode to set. This can either be
      *                     unipolar or bipolar. 
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      SENSOR_RESULT   setCodingMode(AD7798::CODING_MODE mode);

     /*!
      * @brief  Set the gain select bits of the configuration register.
      *
      * @param  [in] gain : Value of the gain to set.
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      SENSOR_RESULT   setGain(AD7798::GAIN gain);

     /*!
      * @brief  Set the filter update select bits of the ADC.
      *
      * @param  [in] rate : Value of the filter update rate to set.
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      SENSOR_RESULT   setFilter(AD7798::FILTER_RATE rate);

     /*!
      * @brief  Set the reference detect function of the ADC.
      *
      * @param  [in] reference :  Enable/disable the reference detect function.
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      * @details When reference detect function is set, the NOREF bit in the status register indicates
      *          when the external reference being used by the ADC is open circuit or less than 0.5 V.
      *
      */
      SENSOR_RESULT   setReference(AD7798::REF_DET reference);

      /*!
       * @brief  Get the gain select bits of the configuration register.
       *
       * @return #GAIN.
       *
       */
      AD7798::GAIN   getGain(void);


    private:

     /*!
      * @brief  Initialize SPI bus.
      *
      * @return ADI_SPI_RESULT, the return value of the spi transactions.
      *
      * @details Initialize the SPI bus based on the static configuration parameters
      *          specified in the configuration file adi_ad7798_cfg.h. 
      */
      ADI_SPI_RESULT  initSPI();


     /*!
      *
      * @brief  Apply static configuration.
      *
      * @return SENSOR_RESULT. This value can be set using #SET_SENSOR_ERROR(type, error).  
      *
      */
      SENSOR_RESULT applyStaticConfig(void);


      /*!< SPI handle.                     */
      ADI_SPI_HANDLE      m_spi_handle;
      /*!< SPI transceiver buffer.         */
      ADI_SPI_TRANSCEIVER m_transceive;
      /*!< SPI transmit buffer.            */
      uint8_t             m_txBuffer[4];
      /*!< SPI receive buffer.             */
      uint8_t             m_rxBuffer[2];
      /*!< Current gain select bits.       */
      AD7798::GAIN       m_gain;
      //Array for reading registers
      uint8_t au8Value[3];
  };
  #pragma pack(pop)
}

#endif /* ADI_AD7798_H */
