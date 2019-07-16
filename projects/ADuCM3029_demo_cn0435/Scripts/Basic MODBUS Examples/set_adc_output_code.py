"""Read and write one output holding register to change ADC output code."""

import minimalmodbus

# declare an instrument object with port name, slave address as input arguments
INSTRUMENT = minimalmodbus.Instrument('COM12', 1)

# read a single register from address 12544 by using function code 3
INITIAL_ADC_OUTPUT_CODE = INSTRUMENT.read_register(
    registeraddress=12544, functioncode=3)

# result an integer
print(INITIAL_ADC_OUTPUT_CODE)

# wrie a single register from address 12544 by using function code 6
INSTRUMENT.write_register(registeraddress=12544, value=1, functioncode=6)

# read a single register from address 12544 by using function code 3
FINAL_ADC_OUTPUT_CODE = INSTRUMENT.read_register(
    registeraddress=12544, functioncode=3)

# result an integer
print(FINAL_ADC_OUTPUT_CODE)
