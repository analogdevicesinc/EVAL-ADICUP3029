"""Read analog input registers to determine ADC channels code."""

import minimalmodbus

# declare an instrument object with port name, slave address as input arguments
INSTRUMENT = minimalmodbus.Instrument('COM12', 1)

# read 16 registers starting from address 12549 by using function code 4
ADC_CHANNELS_CODES = INSTRUMENT.read_registers(
    registeraddress=12549, numberOfRegisters=16, functioncode=4)

# result a list with 16 elements in decimal format
print(ADC_CHANNELS_CODES)
