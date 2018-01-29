/*
 * adi_cn0410.h
 *
 *  Created on: Jul 3, 2017
 *      Author: mcaprior
 */

#ifndef SENSORS_ADI_CN0410_H_
#define SENSORS_ADI_CN0410_H_

#include <drivers/gpio/adi_gpio.h>
#include <drivers/spi/adi_spi.h>
#include "cn0410_bindings.h"
#include "adi_cn0397.h"

extern uint8_t         m_spi_memory[ADI_SPI_MEMORY_SIZE];

namespace adi_sensor_swpack
{

#define SYNC_PORT	ADI_GPIO_PORT1 //this is the CS pin
#define SYNC_PIN	ADI_GPIO_PIN_12

#define LDAC_PORT	ADI_GPIO_PORT0
#define LDAC_PIN	ADI_GPIO_PIN_8

#define RESET_PORT	ADI_GPIO_PORT1
#define RESET_PIN	ADI_GPIO_PIN_11

	enum ad5686_commands
	{
		AD5686_NO_OPERATION = 0,
		AD5686_WRITE_LDAC,
		AD5686_UPDATE,
		AD5686_WRITE_UPDATE,
		AD5686_POWER,
		AD5686_LDAC_MASK,
		AD5686_RESET,
		AD5686_ITERNAL_REFERENCE,
		AD5686_SET_DCEN = 0x08,
		AD5686_SET_READBACK,
		AD5686_DAISY_CHAIN = 0x0F
	};

	enum ad5686_dac_channels
	{
		AD5686_DAC_A = 0x01,
		AD5686_DAC_B = 0x02,
		AD5686_DAC_C = 0x04,
		AD5686_DAC_D = 0x08
	};

	struct stLight_tolerance
	{
		float RedHighLux;
		float RedLowLux;
		float BlueHighLux;
		float BlueLowLux;
		float GreenHighLux;
		float GreenLowLux;
	};

	struct stLedCommands
	{
		uint16_t u16RedCommand;
		uint16_t u16BlueCommand;
		uint16_t u16GreenCommand;
	};



	class CN0410
	{
	public:
		CN0410(VisibleLight  * pLight);

		int8_t Init();
		void SetChannelLuxValue(float fpValue);
		void SetChannelValue(uint16_t u16Value);
		void Reset();
		void UpdateDAC();
		int8_t SendCommand(uint8_t u8Command, uint8_t u8Channel, uint16_t u16Value);
		uint32_t ReadBack(uint8_t u8DacChannelAddr);
		void Setup(void);
		void Controll(float red, float blue, float green);
		~CN0410();

	private:

		/*!< SPI handle.                     */
		ADI_SPI_HANDLE      m_spi_handle;
		/*!< SPI transceiver buffer.         */
		ADI_SPI_TRANSCEIVER m_transceive;

		VisibleLight   *pLight;

	};
}



#endif /* SENSORS_ADI_CN0410_H_ */
