"""Read and write one output holding register to change DAC channel voltage."""

import minimalmodbus

# declare an instrument object with port name, slave address as input arguments
INSTRUMENT = minimalmodbus.Instrument('COM12', 1)

# read a single register from address 4356 by using function code 3
INITIAL_DAC_CHANNEL_1_CODE = INSTRUMENT.read_register(
    registeraddress=4356, functioncode=3)

# result an integer
print(INITIAL_DAC_CHANNEL_1_CODE)

# wrie a single register from address 4356 by using function code 6
INSTRUMENT.write_register(registeraddress=4356, value=65535, functioncode=6)

# read a single register from address 4356 by using function code 3
FINAL_DAC_CHANNEL_1_CODE = INSTRUMENT.read_register(
    registeraddress=4356, functioncode=3)

# result an integer
print(FINAL_DAC_CHANNEL_1_CODE)
