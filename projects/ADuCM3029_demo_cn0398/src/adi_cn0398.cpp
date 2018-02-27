#include "adi_cn0398_cfg.h"
#include "adi_cn0398.h"
#include "adi_ad7124.h"
#include "Timer.h"
#include "math.h"
#include <drivers/gpio/adi_gpio.h>
#include "string.h"

#include <framework/noos/adi_wifi_noos.h>

volatile uint8_t ui8S[200];
volatile bool rec_flag = false;
extern uint8_t *payload_ptr;

namespace adi_sensor_swpack
{
#define RREF (5000.0)
#define TEMP_GAIN (16.0)
#define PT100_RESISTANCE_TO_TEMP(x) ((x-100.0)/(0.385))
#define _2_23 (1<<23)

#define ms_delay (1)

float temperature, pH, voltage[2], moisture;

uint8_t mem_gpio_handler[ADI_GPIO_MEMORY_SIZE];

int32_t adcValue[3];

ADI_WIFI_PUBLISH_CONFIG gPublishConfig = {
	.pMQTTData = NULL,
	.nMQTTDataSize = 0,
	.pTopic = (uint8_t*)"cn0398",
	.nLinkID = 5u,
	.nQos = 0u
};



CN0398::CN0398() : ad7124()
{
	offset_voltage = default_offset_voltage;
	calibration_ph[0][0] = default_calibration_ph[0][0];
	calibration_ph[0][1] = default_calibration_ph[0][1];
	calibration_ph[1][0] = default_calibration_ph[1][0];
	calibration_ph[1][1] = default_calibration_ph[1][1];
	solution0 = 0;
	solution1 = 0;
}

void CN0398::calibrate_ph_pt0(float temperature)
{
	uint8_t ui8S[200];

	set_digital_output(P2, true);
	int32_t data = read_channel(PH_CHANNEL);

	float volt = data_to_voltage_bipolar(data, 1, 3.3);

	if(temperature < 0) {
		calibration_ph[0][0] = ph_temp_lut[solution0][0];
	} else {
		for(uint8_t i = 1; i < NUMBER_OF_TEMPERATURE_ENTRIES; i++) {
			if(temperature > ph_temperatures[i - 1] &&
			   temperature <= ph_temperatures[i]) {
				calibration_ph[0][0] = ph_temp_lut[solution0][i];
				break;
			}
		}
	}

	calibration_ph[0][1] =  volt;
	set_digital_output(P2, false);

	sprintf((char *)ui8S,
			"\tCalibration solution1 pH = %.3f", calibration_ph[0][0]);
	gPublishConfig.pMQTTData = ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S,"\twith sensor voltage of %fV\n", volt);
	gPublishConfig.pMQTTData = ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
}
void CN0398::calibrate_ph_pt1(float temperature)
{
	uint8_t ui8S[200];
	set_digital_output(P2, true);
	int32_t data = read_channel(PH_CHANNEL);

	float volt = data_to_voltage_bipolar(data, 1, 3.3);

	if(temperature < 0) {
		calibration_ph[1][0] = ph_temp_lut[solution1][0];
	} else {
		for(uint8_t i = 1; i < NUMBER_OF_TEMPERATURE_ENTRIES; i++) {
			if(temperature > ph_temperatures[i - 1] &&
			   temperature <= ph_temperatures[i]) {
				calibration_ph[1][0] = ph_temp_lut[solution1][i];
				break;
			}
		}
	}

	calibration_ph[1][1] =  volt;
	set_digital_output(P2, false);

	sprintf((char *)ui8S,
			"\tCalibration solution2 pH = %.3f", calibration_ph[1][0]);
	gPublishConfig.pMQTTData = ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S,"\twith sensor voltage of %fV\n\r", volt);
	gPublishConfig.pMQTTData = ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
}

void CN0398::calibrate_ph_offset()
{
	uint8_t ui8S[200];
	set_digital_output(P2, true);

	int32_t data = read_channel(PH_CHANNEL);

	float volt = data_to_voltage_bipolar(data, 1, 3.3);
	offset_voltage = volt;

	sprintf((char *)ui8S,"\tOffset voltage is %fV\n\n", volt);
	gPublishConfig.pMQTTData = ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);

	set_digital_output(P2, false);
}

float CN0398::read_rtd()
{
	float temperature = 0;

	int32_t data;

	adcValue[RTD_CHANNEL] = data = read_channel(RTD_CHANNEL);

	float resistance = ((static_cast<float>(data) - _2_23) * RREF) /
					   (TEMP_GAIN * _2_23);

#ifdef USE_LINEAR_TEMP_EQ
	temperature = PT100_RESISTANCE_TO_TEMP(resistance);
#else

#define A (3.9083*pow(10,-3))
#define B (-5.775*pow(10,-7))
	if(resistance < R0)
		temperature = -242.02 + 2.228 * resistance + (2.5859 * pow(10, -3)) *
					  pow(resistance, 2) - (48260.0 * pow(10, -6)) *
					  pow(resistance, 3) - (2.8183 * pow(10, -3)) *
					  pow(resistance, 4) + (1.5243 * pow(10, -10)) *
					  pow(resistance, 5);
	else
		temperature = ((-A + sqrt(double(pow(A, 2) - 4 * B *
					  (1 - resistance / R0))) ) / (2 * B));
#endif
	return temperature;

}

int32_t CN0398::read_channel(uint8_t ch)
{
	uint8_t ui8S[200];
	enable_channel(ch);

	adi_gpio_SetLow(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN);
	start_single_conversion();


	if (ad7124.WaitForConvReady(10000) == -3) {
		sprintf((char *)ui8S,"TIMEOUT\n");
		gPublishConfig.pMQTTData = ui8S;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
		adi_wifi_radio_MQTTPublish(&gPublishConfig);
		return -1;
	}

	int32_t data;

	ad7124.ReadData(&data);

	convFlag = 0;
	adi_gpio_SetHigh(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN);

	disable_channel(ch);

	return data;

}

float CN0398::read_ph(float temperature)
{
	float ph = 0;

#ifdef PH_SENSOR_PRESENT
	int32_t data;

	set_digital_output(P2, true);

	adcValue[PH_CHANNEL] = data = read_channel(PH_CHANNEL);

	float volt = voltage[PH_CHANNEL - 1] =
				 data_to_voltage_bipolar(data, 1, 3.3);

	if(use_nernst) {
		ph  = PH_ISO -((volt - ZERO_POINT_TOLERANCE) /
			  ((2.303 * AVOGADRO * (temperature + KELVIN_OFFSET)) /
			   FARADAY_CONSTANT));
	} else {
		float m =  (calibration_ph[1][0] - calibration_ph[0][0]) /
				   (calibration_ph[1][1] - calibration_ph[0][1]);
		ph = m * (volt - calibration_ph[1][1] + offset_voltage) +
			 calibration_ph[1][0];
	}

	set_digital_output(P2, false);

#endif
	return ph;

}

float CN0398::read_moisture()
{
	float moisture = 0;
#ifdef MOISTURE_SENSOR_PRESENT

	adi_gpio_OutputEnable(ADP7118_PORT, ADP7118_PIN, true);
	adi_gpio_SetHigh(ADP7118_PORT, ADP7118_PIN);

	set_digital_output(P3, true);

	timer_sleep(SENSOR_SETTLING_TIME);
	int32_t data = adcValue[MOISTURE_CHANNEL]= read_channel(MOISTURE_CHANNEL);

	adi_gpio_SetLow(ADP7118_PORT, ADP7118_PIN);

	float volt = voltage[MOISTURE_CHANNEL - 1] = data_to_voltage_bipolar(data,
																		 1,
																		 3.3);

#ifdef USE_MANUFACTURER_MOISTURE_EQ
	if(volt <= 1.1) {
		moisture = 10 * volt - 1;
	} else if(volt > 1.1 && volt <= 1.3) {
		moisture = 25 * volt - 17.5;
	} else if(volt > 1.3 && volt <= 1.82) {
		moisture = 48.08 * volt - 47.5;
	} else if(volt > 1.82) {
		moisture = 26.32 * volt - 7.89;
	}
#else
	moisture = -1.18467 + 21.5371 * volt - 110.996 * (pow(volt, 2)) + 397.025 *
			   (pow(volt, 3)) - 666.986 * (pow(volt, 4)) + 569.236 *
			   (pow(volt, 5)) - 246.005 * (pow(volt, 6)) + 49.4867 *
			   (pow(volt, 7)) - 3.37077 * (pow(volt, 8));
#endif
	if(moisture > 100) moisture = 100;
	if(moisture < 0 ) moisture = 0;
#endif

	set_digital_output(P3, false);

	return moisture;
}

float CN0398::data_to_voltage_bipolar(uint32_t data, uint8_t gain, float VREF)
{
	data = data & 0xFFFFFF;
	return ((data / static_cast<float>(0xFFFFFF / 2)) - 1) * (VREF / gain);
}

float CN0398::data_to_voltage(uint32_t data, uint8_t gain, float VREF)
{
	data = data & 0xFFFFFF;
	return (data / static_cast<float>(0xFFFFFF)) * (VREF / gain);
}


void CN0398::enable_channel(int channel)
{
	/* Select ADC_Control register */
	AD7124::ad7124_registers regNr = static_cast<AD7124::ad7124_registers>
									 (AD7124::AD7124_Channel_0 + channel);
	uint32_t setValue = ad7124.ReadDeviceRegister(regNr);
	/* Enable channel0 */
	setValue |= (uint32_t) AD7124_CH_MAP_REG_CH_ENABLE;
	setValue &= 0xFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	timer_sleep(ms_delay);
}

void CN0398::disable_channel(int channel)
{
	/* Select ADC_Control register */
	AD7124::ad7124_registers regNr = static_cast<AD7124::ad7124_registers>
									 (AD7124::AD7124_Channel_0 + channel);
	uint32_t setValue = ad7124.ReadDeviceRegister(regNr);
	/* Disable channel */
	setValue &= (~(uint32_t) AD7124_CH_MAP_REG_CH_ENABLE);
	setValue &= 0xFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	timer_sleep(ms_delay);
}

void CN0398::enable_current_source0(int current_source_channel)
{
	/* Select ADC_Control register */
	AD7124::ad7124_registers regNr = AD7124::AD7124_IOCon1;
	uint32_t setValue = ad7124.ReadDeviceRegister(regNr);
	setValue &= ~(AD7124_IO_CTRL1_REG_IOUT_CH0(0xF));
	/* set IOUT0 current to 500uA */
	setValue |= AD7124_IO_CTRL1_REG_IOUT_CH0(current_source_channel);
	setValue &= 0xFFFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	timer_sleep(ms_delay);
}

void CN0398::enable_current_source1(int current_source_channel)
{
	/* Select ADC_Control register */
	AD7124::ad7124_registers regNr = AD7124::AD7124_IOCon1;
	uint32_t setValue = ad7124.ReadDeviceRegister(regNr);
	setValue &= ~(AD7124_IO_CTRL1_REG_IOUT_CH1(0xF));
	/* set IOUT0 current to 500uA */
	setValue |= AD7124_IO_CTRL1_REG_IOUT_CH1(current_source_channel);
	setValue &= 0xFFFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	timer_sleep(ms_delay);
}

void CN0398::set_digital_output(uint8_t p, bool state)
{
	uint32_t test;

	/* Select ADC_Control register */
	AD7124::ad7124_registers regNr = AD7124::AD7124_IOCon1;
	uint32_t setValue = ad7124.ReadDeviceRegister(regNr);
	if(state)
		setValue |= ((AD7124_8_IO_CTRL1_REG_GPIO_DAT1) << p);
	else
		setValue &= (~(AD7124_8_IO_CTRL1_REG_GPIO_DAT1 << p));
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	timer_sleep(ms_delay);

	test = ad7124.ReadDeviceRegister(regNr);
}


void CN0398::start_single_conversion()
{
	uint32_t test;
	/* Select ADC_Control register */
	AD7124::ad7124_registers regNr = AD7124::AD7124_ADC_Control;

	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, 0x0584);

	timer_sleep(100);
	test = ad7124.ReadDeviceRegister(regNr);
}


void CN0398::reset()
{
	uint8_t ui8S[200];

	ad7124.Reset();
	sprintf((char *)ui8S,"Reseted AD7124\r\n");
	gPublishConfig.pMQTTData = ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
}

void CN0398::setup()
{
	ad7124.Setup();
}


SENSOR_RESULT CN0398::init()
{
	SENSOR_RESULT result = 0;
	uint32_t gpio_mem_size = ADI_GPIO_MEMORY_SIZE;

	adi_gpio_Init(mem_gpio_handler, gpio_mem_size);

	/* Configure ADP7118 pin */
	adi_gpio_OutputEnable(ADP7118_PORT, ADP7118_PIN, true);
	/* Set INTACC_PORT as input */
	adi_gpio_InputEnable(ADP7118_PORT, ADP7118_PIN, false);
	adi_gpio_PullUpEnable(ADP7118_PORT, ADP7118_PIN, false);
	adi_gpio_SetLow(ADP7118_PORT, ADP7118_PIN);

	/* Configure CS pin */
	adi_gpio_OutputEnable(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN, true);
	/* Set INTACC_PORT as input */
	adi_gpio_InputEnable(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN, false);
	adi_gpio_PullUpEnable(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN, false);
	adi_gpio_SetHigh(AD7124_SPI_CS_PORT, AD7124_SPI_CS_PIN);


	result = ad7124.init();
	if(result != SET_SENSOR_ERROR(SENSOR_ERROR_SPI, ADI_SPI_SUCCESS))
		return SENSOR_ERROR_SPI;

	uint32_t setValue, test;
	enum AD7124::ad7124_registers regNr;

	/* Set Config_0 0x19 */
	/* Select Config_0 register - RTD */
	regNr = AD7124::AD7124_Config_0;
	test = ad7124.ReadDeviceRegister(regNr);
	setValue = 0;
	/* Select bipolar operation */
	setValue |= AD7124_CFG_REG_BIPOLAR;
	/* Burnout current source off */
	setValue |= AD7124_CFG_REG_BURNOUT(0);
	setValue |= AD7124_CFG_REG_REF_BUFP;
	setValue |= AD7124_CFG_REG_REF_BUFM;
	setValue |= AD7124_CFG_REG_AIN_BUFP;
	setValue |= AD7124_CFG_REG_AINN_BUFM;
	/* REFIN2(+)/REFIN2(âˆ’). */
	setValue |= AD7124_CFG_REG_REF_SEL(1);
	setValue |= AD7124_CFG_REG_PGA(4);
	setValue &= 0xFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	test = ad7124.ReadDeviceRegister(regNr);

	/* Select Config_1 register - pH */
	regNr = AD7124::AD7124_Config_1;
	test = ad7124.ReadDeviceRegister(regNr);
	setValue = 0;
	/* Select bipolar operation */
	setValue |= AD7124_CFG_REG_BIPOLAR;
	/* Burnout current source off */
	setValue |= AD7124_CFG_REG_BURNOUT(0);
	setValue |= AD7124_CFG_REG_REF_BUFP;
	setValue |= AD7124_CFG_REG_REF_BUFM;
	setValue |= AD7124_CFG_REG_AIN_BUFP;
	setValue |= AD7124_CFG_REG_AINN_BUFM;
	/* REFIN1(+)/REFIN1(-). */
	setValue |= AD7124_CFG_REG_REF_SEL(0);
	/* gain1 */
	setValue |= AD7124_CFG_REG_PGA(0);
	setValue &= 0xFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	test = ad7124.ReadDeviceRegister(regNr);

	/* Set Channel_0 register 0x09 */
	regNr = AD7124::AD7124_Channel_0;  /* RTD */
	test = ad7124.ReadDeviceRegister(regNr);
	setValue = 0;
	/* Select setup0 */
	setValue |= AD7124_CH_MAP_REG_SETUP(0);
	/* Set AIN9 as positive input */
	setValue |= AD7124_CH_MAP_REG_AINP(9);
	/* Set AIN10 as negative input */
	setValue |= AD7124_CH_MAP_REG_AINM(10);
	/* Disable channel */
	setValue &= (~(uint32_t) AD7124_CH_MAP_REG_CH_ENABLE);
	setValue &= 0xFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	test = ad7124.ReadDeviceRegister(regNr);

	regNr = AD7124::AD7124_Channel_1; /* pH */
	test = ad7124.ReadDeviceRegister(regNr);
	setValue = 0;
	/* Select setup2 */
	setValue |= AD7124_CH_MAP_REG_SETUP(1);
	/* Set AIN8 as positive input */
	setValue |= AD7124_CH_MAP_REG_AINP(6);
	/* Set gnd as negative input */
	setValue |= AD7124_CH_MAP_REG_AINM(7);
	 /* Disable channel */
	setValue &= (~(uint32_t) AD7124_CH_MAP_REG_CH_ENABLE);
	setValue &= 0xFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	test = ad7124.ReadDeviceRegister(regNr);

	regNr = AD7124::AD7124_Channel_2; /* moisture */
	test = ad7124.ReadDeviceRegister(regNr);
	setValue = 0;
	/* Select setup2 */
	setValue |= AD7124_CH_MAP_REG_SETUP(1);
	/* Set AIN8 as positive input */
	setValue |= AD7124_CH_MAP_REG_AINP(8);
	/* Set gnd as negative input */
	setValue |= AD7124_CH_MAP_REG_AINM(19);
	/* Disable channel */
	setValue &= (~(uint32_t) AD7124_CH_MAP_REG_CH_ENABLE);
	setValue &= 0xFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	test = ad7124.ReadDeviceRegister(regNr);

	/* Set IO_Control_1 0x03 */
	/* Select IO_Control_1 register */
	regNr = AD7124::AD7124_IOCon1;
	test = ad7124.ReadDeviceRegister(regNr);
	setValue = 0;
	/* enable AIN3 as digital output */
	setValue |= AD7124_8_IO_CTRL1_REG_GPIO_CTRL2;
	/* enable AIN4 as digital output */
	setValue |= AD7124_8_IO_CTRL1_REG_GPIO_CTRL3;
	/* source ain11 */
	setValue |= AD7124_IO_CTRL1_REG_IOUT_CH0(11);
	/* source ain12 */
	setValue |= AD7124_IO_CTRL1_REG_IOUT_CH1(12);
	/* set IOUT0 current to 500uA */
	setValue |= AD7124_IO_CTRL1_REG_IOUT0(0x4);
	/* set IOUT0 current to 500uA */
	setValue |= AD7124_IO_CTRL1_REG_IOUT1(0x4);
	setValue &= 0xFFFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	test = ad7124.ReadDeviceRegister(regNr);

	/* Set IO_Control_2 */
	/* Select IO_Control_2 register */
	regNr = AD7124::AD7124_IOCon2;
	test = ad7124.ReadDeviceRegister(regNr);
	setValue = 0;
	/* enable AIN3 as digital output */
	setValue |= AD7124_8_IO_CTRL2_REG_GPIO_VBIAS7;
	setValue &= 0xFFFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	test = ad7124.ReadDeviceRegister(regNr);


	/* Set ADC_Control 0x01 */
	/* Select ADC_Control register */
	regNr = AD7124::AD7124_ADC_Control;
	test = ad7124.ReadDeviceRegister(regNr);
	setValue = ad7124.ReadDeviceRegister(regNr);
	/* set data status bit in order to check on which channel the
	 * conversion is
	 * */
	setValue |= AD7124_ADC_CTRL_REG_DATA_STATUS;
	/* remove prev mode bits */
	setValue &= 0xFFC3;
	/* standby mode */
	setValue |= AD7124_ADC_CTRL_REG_MODE(2);
	setValue &= 0xFFFF;
	/* Write data to ADC */
	ad7124.WriteDeviceRegister(regNr, setValue);
	test = ad7124.ReadDeviceRegister(regNr);
	timer_sleep(ms_delay);

	return SENSOR_ERROR_NONE;
}

void CN0398::set_data(void)
{
	temperature = read_rtd();

	pH = read_ph(temperature);

	moisture = read_moisture();
}

void CN0398::display_data(void)
{
	uint8_t ui8S[200];

	sprintf((char *)ui8S, "Temperature = %f°C", temperature);
	gPublishConfig.pMQTTData = ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S, "pH = %f", pH);
	gPublishConfig.pMQTTData = ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S, "Moisture = %f%c\n\r", moisture, 37);
	gPublishConfig.pMQTTData = ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
}
void CN0398::calibrate_ph(void)
{
	ADI_WIFI_RESULT result;
	sprintf((char *)ui8S,"Do you want to calibrate offset voltage [y/N]?\n\r");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	while(!rec_flag)
		adi_wifi_DispatchEvents(1000);
	rec_flag = false;

	if(payload_ptr[0] == 'y' || payload_ptr[0] == 'Y') {
		sprintf((char *)ui8S,"Calibration step 0.");
		gPublishConfig.pMQTTData = (uint8_t*)ui8S;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
		result = adi_wifi_radio_MQTTPublish(&gPublishConfig);

		sprintf((char *)ui8S,"Short the pH probe and send any message to");
		gPublishConfig.pMQTTData = (uint8_t*)ui8S;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
		result = adi_wifi_radio_MQTTPublish(&gPublishConfig);

		sprintf((char *)ui8S,"calibrate.\r\n");
		gPublishConfig.pMQTTData = (uint8_t*)ui8S;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
		result = adi_wifi_radio_MQTTPublish(&gPublishConfig);
		while(!rec_flag)
			adi_wifi_DispatchEvents(1000);
		rec_flag = false;
		calibrate_ph_offset();
	}
	print_calibration_solutions();

	bool response_ok = false;
	while(response_ok == false) {
		sprintf((char *)ui8S,"Input calibration solution used for");
		gPublishConfig.pMQTTData = (uint8_t*)ui8S;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
		adi_wifi_radio_MQTTPublish(&gPublishConfig);
		sprintf((char *)ui8S,"first step [1-9][a-e]:\n\r");
		gPublishConfig.pMQTTData = (uint8_t*)ui8S;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
		adi_wifi_radio_MQTTPublish(&gPublishConfig);
		while(!rec_flag)
			adi_wifi_DispatchEvents(1000);
		rec_flag = false;
		if(payload_ptr[0] >= '0' && payload_ptr[0] <= '9') {
			response_ok = true;
			solution0 = payload_ptr[0] - '0';
		} else if(payload_ptr[0] >= 'A' && payload_ptr[0] <= 'E') {
			response_ok = true;
			solution0 = payload_ptr[0] - 'A' + 10;
		} else if(payload_ptr[0] >= 'a' && payload_ptr[0] <= 'e') {
			response_ok = true;
			solution0 = payload_ptr[0] - 'a' + 10;
		} else {
			response_ok = false;
		}
	}
	sprintf((char *)ui8S, "\t%s solution selected.", solutions[solution0]);
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S,
			"\tSolution pH at 25°C = %.3f \n\r", ph_temp_lut[solution0][11]);
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	float temperature = read_rtd();
	sprintf((char *)ui8S, "Calibration step 1.");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S, "Place pH probe in first calibration solution");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S, "and send any message to start calibration.\r\n");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	while(!rec_flag)
		adi_wifi_DispatchEvents(1000);
	rec_flag = false;
	calibrate_ph_pt0(temperature);

	response_ok = false;
	while(response_ok == false) {
		sprintf((char *)ui8S, "Input calibration solution used for");
		gPublishConfig.pMQTTData = (uint8_t*)ui8S;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
		adi_wifi_radio_MQTTPublish(&gPublishConfig);
		sprintf((char *)ui8S, "second step [1-9][a-e]:\n\r");
		gPublishConfig.pMQTTData = (uint8_t*)ui8S;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
		adi_wifi_radio_MQTTPublish(&gPublishConfig);
		while(!rec_flag)
			adi_wifi_DispatchEvents(1000);
		rec_flag = false;
		if(payload_ptr[0] >= '0' && payload_ptr[0] <= '9') {
			response_ok = true;
			solution1 = payload_ptr[0] - '0';
		} else if(payload_ptr[0] >= 'A' && payload_ptr[0] <= 'E') {
			response_ok = true;
			solution1 = payload_ptr[0] - 'A' + 10;
		} else if(payload_ptr[0] >= 'a' && payload_ptr[0] <= 'e') {
			response_ok = true;
			solution1 = payload_ptr[0] - 'a' + 10;
		} else {
			response_ok = false;
		}
	}
	sprintf((char *)ui8S, "\t%s solution selected.", solutions[solution1]);
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S,
			"\tSolution pH at 25°C = %.3f \n", ph_temp_lut[solution1][11]);
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);

	sprintf((char *)ui8S, "Calibration step 2.");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S, "Place pH probe in second calibration solution");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S, "and send any message to start calibration.\r\n");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);

	while(!rec_flag)
		adi_wifi_DispatchEvents(1000);
	rec_flag = false;

	calibrate_ph_pt1(temperature);
}
/**
 * @brief: Prints calibration solutions
 */
void CN0398::print_calibration_solutions()
{
	uint8_t ui8Temp[40];

	sprintf((char *)ui8S, "Calibration solutions available for");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);
	sprintf((char *)ui8S, "two point calibration:\n\r");
	gPublishConfig.pMQTTData = (uint8_t*)ui8S;
	gPublishConfig.nMQTTDataSize = strlen((char *)ui8S);
	adi_wifi_radio_MQTTPublish(&gPublishConfig);

	sprintf((char *)ui8S,"%x. %s\n\r", 0, solutions[0]);
	int i;
	for(i = 1; i < NUMBER_OF_SOLUTIONS; i++) {
		sprintf((char *)ui8Temp,"%x. %s", i, solutions[i]);
		gPublishConfig.pMQTTData = ui8Temp;
		gPublishConfig.nMQTTDataSize = strlen((char *)ui8Temp);
		adi_wifi_radio_MQTTPublish(&gPublishConfig);
	}
}

SENSOR_RESULT CN0398::open()
{
	this->setup();
	return this->init();
}
SENSOR_RESULT CN0398::start()
{
	return SENSOR_ERROR_NONE;
}
SENSOR_RESULT CN0398::stop()
{
	return SENSOR_ERROR_NONE;
}
SENSOR_RESULT CN0398::close()
{
	return SENSOR_ERROR_NONE;
}

}
