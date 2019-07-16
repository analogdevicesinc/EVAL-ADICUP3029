"""Read common analog input registers to determine PLC/DCS configuration."""

import minimalmodbus

# declare an instrument object with port name, slave address as input arguments
INSTRUMENT = minimalmodbus.Instrument('COM12', 1)

# read 5 registers starting from address 0 by using function code 4
COMMON_ANALOG_INPUT_REGISTERS = INSTRUMENT.read_registers(
    registeraddress=0, numberOfRegisters=5, functioncode=4)

# result a list with 5 elements in decimal format
print(COMMON_ANALOG_INPUT_REGISTERS)
