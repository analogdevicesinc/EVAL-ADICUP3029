"""Base MODBUS functions.

This module provide base functionalities to easy interact with:
CN0414 - Analog input and HART for PLC/DCS systems
CN0418 - Analog output and HART for PLC/DCS systems
CN0416 - RS485 transceiver.
"""

__version__ = '0.1'
__author__ = 'Mihai Ionut Suciu'
__status__ = 'Development'

from time import sleep
from typing import Dict, Any, Tuple, List
from colorama import init, Fore
import dcs_cn0435_utilities as utilities

init(autoreset=True)


def read_analog_input_regs_cn0418(
        global_data: Dict[str, Any], address: List[int],
        registers_number: int = 30, debug: bool = False) -> List[int]:
    """Read analog input registers.

    Read analog input registers from CN0418 with function code 4.

    Args:
        global_data: Dictionary with global variables
        address: MODBUS register start address
        registers_number: Number of MODBUS register read (default value = 30)
        debug: If True will print registers description and value
            in a colored table (default value = False)

    Returns:
        Return a list of registers values

    """
    register_address = utilities.generate_register_address(address)
    registers = global_data["INSTRUMENT"].read_registers(
        register_address, registers_number, functioncode=4)
    sleep(global_data["DELAY"])
    if debug:
        print(Fore.LIGHTMAGENTA_EX + '\nAnalog input registers at',
              Fore.YELLOW + 'MODBUS address ' + str(address[1]))
        labels = [
            Fore.YELLOW + 'HART IN 01', Fore.YELLOW + 'HART IN 02',
            Fore.YELLOW + 'HART IN 03', Fore.YELLOW + 'HART IN 04',
            Fore.YELLOW + 'HART IN 05', Fore.YELLOW + 'HART IN 06',
            Fore.YELLOW + 'HART IN 07', Fore.YELLOW + 'HART IN 08',
            Fore.YELLOW + 'HART IN 09', Fore.YELLOW + 'HART IN 10',
            Fore.YELLOW + 'HART IN 11', Fore.YELLOW + 'HART IN 12',
            Fore.YELLOW + 'HART IN 13', Fore.YELLOW + 'HART IN 14',
            Fore.YELLOW + 'HART IN 15', Fore.YELLOW + 'HART IN 16',
            Fore.YELLOW + 'HART IN 17', Fore.YELLOW + 'HART IN 18',
            Fore.YELLOW + 'HART IN 19', Fore.YELLOW + 'HART IN 20',
            Fore.YELLOW + 'HART IN 21', Fore.YELLOW + 'HART IN 22',
            Fore.YELLOW + 'HART IN 23', Fore.YELLOW + 'HART IN 24',
            Fore.YELLOW + 'HART IN 25', Fore.YELLOW + 'HART IN 26',
            Fore.YELLOW + 'HART IN 27', Fore.YELLOW + 'HART IN 28',
            Fore.YELLOW + 'HART IN 29', Fore.YELLOW + 'HART IN 30']
        utilities.print_table(
            register_address, registers_number, labels, registers)
    return registers


def read_output_holding_regs_cn0418(
        global_data: Dict[str, Any], address: List[int],
        registers_number: int = 10, debug: bool = False) -> List[int]:
    """Read output holding registers.

    Read output holding registers from CN0418 with function code 3.

    Args:
        global_data: Dictionary with global variables
        address: MODBUS register start address
        registers_number: Number of MODBUS register read (default value = 10)
        debug: If True will print registers description and value
            in a colored table (default value = False)

    Returns:
        Return a list of registers values

    """
    register_address = utilities.generate_register_address(address)
    sleep(global_data["DELAY"])
    registers = global_data["INSTRUMENT"].read_registers(
        register_address, registers_number, functioncode=3)
    if debug:
        print(Fore.LIGHTMAGENTA_EX + '\nAnalog input registers at',
              Fore.YELLOW + 'MODBUS address ' + str(address[1]))
        labels = [
            Fore.YELLOW + 'Channel 1 range',
            Fore.YELLOW + 'Channel 2 range',
            Fore.YELLOW + 'Channel 3 range',
            Fore.YELLOW + 'Channel 4 range',
            Fore.YELLOW + 'Channel 1',
            Fore.YELLOW + 'Channel 2',
            Fore.YELLOW + 'Channel 3',
            Fore.YELLOW + 'Channel 4',
            Fore.YELLOW + 'HART cmd 0',
            Fore.YELLOW + 'HART CH select']
        utilities.print_table(
            register_address, registers_number, labels, registers)
    return registers


def request_dac_cs(global_data: Dict[str, Any]) -> int:
    """Request DAC CS.

    Args:
        global_data: Dictionary with global variables

    Returns:
        CS address

    """
    valid_dac_cs = []
    for index, _ in enumerate(global_data["BOARDS"]):
        if global_data["BOARDS"][index] == '1':
            valid_dac_cs.append(global_data["VALID_CS"][index])
    while True:
        print(Fore.CYAN + "Enter CS address from this list",
              Fore.YELLOW + str(valid_dac_cs) + Fore.CYAN + ",",
              Fore.CYAN + "or press ENTER to use CS",
              Fore.YELLOW + str(valid_dac_cs[0]) + Fore.CYAN + ": ", end='')
        input_data = input()
        if input_data == '':
            cs_address = int(valid_dac_cs[0])
            break
        elif int(input_data.split()[0]) not in valid_dac_cs:
            print(Fore.YELLOW + "Invalid option. Try again.")
        else:
            cs_address = int(input_data)
            break
    return cs_address


def request_dac_reconfig_data(global_data: Dict[str, Any]) -> Tuple[int, int]:
    """Request device CS address and register value.

    Args:
        global_data: Dictionary with global variables

    Returns:
        Return requested device address and register value

    """
    cs_addr = request_dac_cs(global_data)

    value_msg = "Enter register value, or press ENTER to use value 0: "
    print(Fore.CYAN + value_msg, end='')
    input_data = input()
    if input_data == "":
        reg_val = 0
    else:
        reg_val = int(input_data.split()[0])

    return cs_addr, reg_val


def select_channel_range(global_data: Dict[str, Any], channel: int) -> None:
    """Select DAC channel output range, voltage or current output.

    Args:
        global_data: Dictionary with global variables
        channel: Channel number

    Returns:
        None

    """
    if '1' not in global_data["BOARDS"]:
        print(Fore.RED + "No analog output board available at MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        print(Fore.CYAN + "Select channel range:")
        labels = ['0V to 5V', '0V to 10V', '-5V to 5V', '-10V to 10V',
                  '4mA to 20mA', '0mA to 20mA', '0mA to 24mA']
        for index, _ in enumerate(labels):
            print(Fore.YELLOW + '\t' + str(index) + ' for ' + labels[index])
        cs_addr, reg_val = request_dac_reconfig_data(global_data)
        utilities.write_output_holding_reg(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, channel - 1],
            reg_val)


def select_hart_channel(global_data: Dict[str, Any]) -> None:
    """Select HART channel.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '1' not in global_data["BOARDS"]:
        print(Fore.CYAN + "No analog output board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        print(Fore.CYAN + "Select HART channel:")
        labels = ['Channel 1', 'Channel 2', 'Channel 3', 'Channel 4']
        for index, _ in enumerate(labels):
            print(Fore.YELLOW + '\t' + str(index) + ' for ' + labels[index])
        cs_addr, reg_val = utilities.request_reconfig_data(global_data)
        utilities.write_output_holding_reg(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, 6], reg_val)


def send_hart_command_0(global_data: Dict[str, Any]) -> None:
    """Send HART command 0.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '1' not in global_data["BOARDS"]:
        print(Fore.CYAN + "No analog output board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        print(Fore.CYAN + "Send HART command zero:")
        print(Fore.YELLOW + '\t' + '1 to send HART command 0')
        cs_addr, reg_val = utilities.request_reconfig_data(global_data)
        utilities.write_output_holding_reg(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, 5], reg_val)


def set_channel_output(global_data: Dict[str, Any], channel: int) -> None:
    """Set DAC channel output value depending on current output range.

    Args:
        global_data: Dictionary with global variables
        channel: Channel number

    Returns:
        None

    """
    if '1' not in global_data["BOARDS"]:
        print(Fore.RED + "No analog output board available at MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        print(Fore.CYAN + "Select channel output value:")
        set_dac_output(global_data, channel)


def set_dac_output(global_data: Dict[str, Any], channel: int) -> None:
    """Request device CS address and register value.

    Args:
        global_data: Dictionary with global variables
        channel: Channel number

    Returns:
        Return requested device address and register value

    """
    cs_addr = request_dac_cs(global_data)

    detected_range = global_data["INSTRUMENT"].read_register(
        utilities.generate_register_address(
            [global_data["MODBUS_ADDRESS"], cs_addr, 0]))

    if detected_range in range(4):
        ranges = ['0V to 5V', '0V to 10V', '-5V to 5V', '-10V to 10V']
        print(Fore.CYAN + "Enter a voltage value in [V] from range",
              Fore.YELLOW + str(ranges[detected_range]),
              Fore.CYAN + "or press ENTER to set",
              Fore.YELLOW + "0V: ", end='')
        input_data = input()
        if input_data == "":
            voltage_value = 0.0
        else:
            voltage_value = float(input_data.split()[0])
        write_voltage(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, channel],
            voltage_value, detected_range)
    else:
        ranges = ['4mA to 20mA', '0mA to 20mA', '0mA to 24mA']
        print(Fore.CYAN + "Enter a current value in [mA] from range",
              Fore.CYAN + str(ranges[detected_range - 4]),
              Fore.CYAN + "or press ENTER to set",
              Fore.YELLOW + "0mA: ", end='')
        input_data = input()
        if input_data == "":
            current_value = 0.0
        else:
            current_value = float(input_data.split()[0]) / 1000
        write_current(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, channel],
            current_value, detected_range - 4)


def write_current(
        global_data: Dict[str, Any], channel_address: List[int],
        channel_value: float, output_range: int) -> None:
    """Write current channel.

    Args:
        global_data: Dictionary with global variables
        channel_address: List of MODBUS address, CS and channel number
        channel_value: Channel current value
        output_range: Current range

    Returns:
        None

    """
    channel_address[2] += 3
    if output_range == 0:
        dac_code = int(((channel_value - 4) / 16) * 2**16)
    elif output_range == 1:
        dac_code = int((channel_value / 20) * 2**16)
    elif output_range == 2:
        dac_code = int((channel_value / 24) * 2**16)
    else:
        pass
    utilities.write_output_holding_reg(global_data, channel_address, dac_code)


def write_voltage(
        global_data: Dict[str, Any], channel_address: List[int],
        channel_value: float, output_range: int) -> None:
    """Write voltage channel.

    Args:
        global_data: Dictionary with global variables
        channel_address: List of MODBUS address, CS and channel number
        channel_value: Channel voltage value
        output_range: Voltage range

    Returns:
        None

    """
    channel_address[2] += 3
    if output_range == 0:
        dac_code = int((channel_value / 5.0) * 2**16)
    elif output_range == 1:
        dac_code = int((channel_value / 10.0) * 2**16)
    elif output_range == 2:
        dac_code = int(((channel_value + 5.0) / 10.0) * 2**16)
    elif output_range == 3:
        dac_code = int(((channel_value + 10.0) / 20.0) * 2**16)
    else:
        pass
    utilities.write_output_holding_reg(global_data, channel_address, dac_code)
