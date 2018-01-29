/*
 * adi_cn0410_cfg.h
 *
 *  Created on: Jul 3, 2017
 *      Author: mcaprior
 */

#ifndef SENSORS_ADI_CN0410_CFG_H_
#define SENSORS_ADI_CN0410_CFG_H_


#include <drivers/spi/adi_spi.h>
#include "adi_cn0410.h"

/****************** SPI Configuration ******************/

/*!
 * @brief SPI device number.
 * @details Choose the SPI bus that this device is connected to.
 */
#define CN0410_SPI_DEV_CFG       (0u)
/*!
 * @brief SPI CS configuration.
 *
 * @details Choose which of the hardware chip selects is
 *          used to communicate with the AD7798.
 *
 */
#define CN0410_SPI_CS_CFG        (ADI_SPI_CS_NONE)
/*!
 * @brief SPI bitrate configuration.
 *
 * @details Set this macro to the SPI0 bit rate in hertz
 *
 */
#define CN0410_SPI_BITRATE_CFG   (1000000u)
/*!
 * @brief SPI master mode configuration.
 *
 * @details If using the SPI in master mode set this macro to true. For slave mode set this macro to false.
 *
 */
#define CN0410_SPI_MASTER_CFG    (true)
/*!
 * @breif Configure DMA use.
 *
 * @details  When initializing the SPI, this flag will either enable or disable
 *           DMA use for all the transactions.
 *            true  : use DMA
 *            false : use PIO
 */
#define CN0410_SPI_DMA_CFG        (false)

/****************** ADC Configuration ******************/


/****************** Macro Validation ******************/

#if (CN0410_SPI_BITRATE_CFG > (13000000u))
#error "Invalid spi bitrate configuration"
#endif


#endif /* SENSORS_ADI_CN0410_CFG_H_ */
