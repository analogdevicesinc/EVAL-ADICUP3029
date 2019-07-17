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
from typing import Dict, Any, List
from colorama import init, Fore
import dcs_cn0435_utilities as utilities

init(autoreset=True)


def read_analog_input_regs_cn0414(
        global_data: Dict[str, Any], address: List[int],
        registers_number: int = 50, debug: bool = False) -> List[int]:
    """Read analog input registers.

    Read analog input registers from CN0414 with function code 4.

    Args:
        instrument: Instrument object created by minimalmodbus
        delay: Delay between MODBUS commands
        address: MODBUS register start address
        registers_number: Number of MODBUS register read (default value = 50)
        debug: If True will print registers description and value
            in a colored table (default value = False)

    Returns:
        Return a list of registers values

    """
    register_address = utilities.generate_register_address(address)
    sleep(global_data["DELAY"])
    registers = global_data["INSTRUMENT"].read_registers(
        register_address, registers_number, functioncode=4)

    if debug:
        print(Fore.LIGHTMAGENTA_EX + '\nAnalog input registers at',
              Fore.YELLOW + 'MODBUS address ' + str(address[1]))
        labels = [
            Fore.YELLOW + 'Channel 1 MSW', Fore.YELLOW + 'Channel 1 LSW',
            Fore.YELLOW + 'Channel 2 MSW', Fore.YELLOW + 'Channel 2 LSW',
            Fore.YELLOW + 'Channel 3 MSW', Fore.YELLOW + 'Channel 3 LSW',
            Fore.YELLOW + 'Channel 4 MSW', Fore.YELLOW + 'Channel 4 LSW',
            Fore.YELLOW + 'Channel 5 MSW', Fore.YELLOW + 'Channel 5 LSW',
            Fore.YELLOW + 'Channel 6 MSW', Fore.YELLOW + 'Channel 6 LSW',
            Fore.YELLOW + 'Channel 7 MSW', Fore.YELLOW + 'Channel 7 LSW',
            Fore.YELLOW + 'Channel 8 MSW', Fore.YELLOW + 'Channel 8 LSW',
            Fore.YELLOW + 'Channel 1 OWD', Fore.YELLOW + 'Channel 2 OWD',
            Fore.YELLOW + 'Channel 3 OWD', Fore.YELLOW + 'Channel 4 OWD',
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


def read_board_current_channels(global_data: Dict[str, Any]) -> None:
    """Read a single board current channels.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        cs_address = request_adc_cs(global_data)
        while True:
            try:
                currents = read_current_channels(global_data, cs_address)
                print(Fore.GREEN + " ".join(
                    str('{:+.5f} A'.format(element)) for element in currents))
            except KeyboardInterrupt:
                break
            except OSError as error_message:
                print(Fore.RED + 'Timeout error!', error_message)
            except ValueError as error_message:
                print(Fore.RED + 'Register operation error!', error_message)


def read_board_voltage_channels(global_data: Dict[str, Any]) -> None:
    """Read a single board voltage channels.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        cs_address = request_adc_cs(global_data)
        while True:
            try:
                voltages = read_voltage_channels(global_data, cs_address)
                print(Fore.GREEN + " ".join(
                    str('{:+.5f} V'.format(element)) for element in voltages))
            except KeyboardInterrupt:
                break
            except OSError as error_message:
                print(Fore.RED + 'Timeout error!', error_message)
            except ValueError as error_message:
                print(Fore.RED + 'Register operation error!', error_message)


def read_current(global_data: Dict[str, Any], channel_address: List[int],
                 coding_type: int) -> float:
    """Read current channels.

    Args:
        global_data: Dictionary with global variables
        channel_address: List of MODBUS address, CS and channel number
        coding_type: Bipolar or Unipolar

    Returns:
        Measured current value

    """
    register_address = utilities.generate_channel_register_address(
        'I', channel_address)
    analog_input_registers = global_data["INSTRUMENT"].read_registers(
        register_address, 2, functioncode=4)
    sleep(global_data["DELAY"])
    code = hex(analog_input_registers[0])[2:].zfill(4) + \
        hex(analog_input_registers[1])[2:].zfill(4)
    if coding_type == 0:
        current = (((int(code, 16) / 2**23) - 1) * 2.5) / 50
    elif coding_type == 1:
        current = (int(code, 16) * 2.5) / (2**24 * 50)
    else:
        print(Fore.RED + "Invalid ADC coding type!")
    return current


def read_current_channels(
        global_data: Dict[str, Any], board_address: int) -> List[float]:
    """Read current channels from an address.

    Args:
        global_data: Dictionary with global variables
        board_address: CS value

    Returns:
        Measured current

    """
    values = []
    for index in range(1, 5):
        coding_type = read_output_holding_regs_cn0414(
            global_data, [global_data["MODBUS_ADDRESS"], board_address, 0])[0]
        values.append(read_current(
            global_data, [global_data["MODBUS_ADDRESS"], board_address, index],
            coding_type))
    return values


def read_device_current_channel(global_data: Dict[str, Any]) -> None:
    """Read current continuously from selected channel.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        cs_address = request_adc_cs(global_data)
        print(Fore.CYAN + "Select current channel this list [1, 2, 3, 4],",
              Fore.CYAN + "or press ENTER to use channel 1: ", end='')
        input_data = input()
        if input_data == '':
            channel = 1
        else:
            channel = int(input_data)
        coding_type = read_output_holding_regs_cn0414(
            global_data, [global_data["MODBUS_ADDRESS"], cs_address, 0])[0]
        while True:
            try:
                current = read_current(
                    global_data,
                    [global_data["MODBUS_ADDRESS"], cs_address, channel],
                    coding_type)
                print(Fore.YELLOW + '{:+.9f} A'.format(current))
            except KeyboardInterrupt:
                break
            except OSError as os_error_message:
                print(Fore.RED + 'Timeout error!', os_error_message)
            except ValueError as value_error_message:
                print(Fore.RED + 'Register operation error!',
                      value_error_message)


def read_device_voltage_channel(global_data: Dict[str, Any]) -> None:
    """Read voltage continuously from selected channel.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        cs_address = request_adc_cs(global_data)
        print(Fore.CYAN + "Select voltage channel this list [1, 2, 3, 4],",
              Fore.CYAN + "or press ENTER to use channel 1: ", end='')
        input_data = input()
        if input_data == '':
            channel = 1
        else:
            channel = int(input_data)
        coding_type = read_output_holding_regs_cn0414(
            global_data, [global_data["MODBUS_ADDRESS"], cs_address, 0])[0]

        while True:
            try:
                voltage = read_voltage(
                    global_data,
                    [global_data["MODBUS_ADDRESS"], cs_address, channel],
                    coding_type)
                print(Fore.YELLOW + "{:+.9f} V".format(voltage))
            except KeyboardInterrupt:
                break
            except OSError as error_message:
                print(Fore.RED + 'Timeout error!', error_message)
            except ValueError as error_message:
                print(Fore.RED + 'Register operation error!', error_message)


def read_instrument_current_channels(global_data: Dict[str, Any]) -> None:
    """Read curent selected instrument current channels.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address ",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        while True:
            try:
                currents = []
                for _, cs_address in enumerate(global_data["VALID_CS"]):
                    board_currents = read_current_channels(
                        global_data, cs_address)
                    for index, _ in enumerate(board_currents):
                        currents.append(board_currents[index])
                print(Fore.GREEN + " ".join(
                    str('{:+.5f} A'.format(element)) for element in currents))
            except KeyboardInterrupt:
                break
            except OSError as error_message:
                print(Fore.RED + 'Timeout error!', error_message)
            except ValueError as error_message:
                print(Fore.RED + 'Register operation error!', error_message)


def read_instrument_voltage_channels(global_data: Dict[str, Any]) -> None:
    """Read curent selected instrument voltage channels.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        while True:
            try:
                voltages = []
                for _, cs_address in enumerate(global_data["VALID_CS"]):
                    board_voltages = read_voltage_channels(
                        global_data, cs_address)
                    for index, _ in enumerate(board_voltages):
                        voltages.append(board_voltages[index])
                print(Fore.GREEN + " ".join(
                    str('{:+.5f} V'.format(element)) for element in voltages))
            except KeyboardInterrupt:
                break
            except OSError as error_message:
                print(Fore.RED + 'Timeout error!', error_message)
            except ValueError as error_message:
                print(Fore.RED + 'Register operation error!', error_message)


def read_output_holding_regs_cn0414(
        global_data: Dict[str, Any], address: List[int],
        registers_number: int = 7, debug: bool = False) -> List[int]:
    """Read output holding registers.

    Read output holding registers from CN0414 with function code 3.

    Args:
        instrument: Instrument object created by minimalmodbus
        delay: Delay between MODBUS commands
        address: MODBUS register start address
        registers_number: Number of MODBUS register read (default value = 7)
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
        print(Fore.LIGHTMAGENTA_EX + '\nOutput holding registers at',
              Fore.YELLOW + 'MODBUS address ' + str(address[1]))
        labels = [
            Fore.YELLOW + 'ADC Output Code',
            Fore.YELLOW + 'ADC Filter',
            Fore.YELLOW + 'ADC Postfilter',
            Fore.YELLOW + 'ADC ODR',
            Fore.YELLOW + 'ADC OWD EN',
            Fore.YELLOW + 'HART cmd 0',
            Fore.YELLOW + 'HART CH select']
        utilities.print_table(
            register_address, registers_number, labels, registers)
    return registers


def read_voltage(global_data: Dict[str, Any], channel_address: List[int],
                 coding_type: int) -> float:
    """Read voltage channels.

    Args:
        global_data: Dictionary with global variables
        channel_address: List of MODBUS address, CS and channel number
        coding_type: Bipolar or Unipolar

    Returns:
        Measured voltage value

    """
    register_address = utilities.generate_channel_register_address(
        'V', channel_address)
    analog_input_registers = global_data["INSTRUMENT"].read_registers(
        register_address, 2, functioncode=4)
    sleep(global_data["DELAY"])
    code = hex(analog_input_registers[0])[2:].zfill(4) + \
        hex(analog_input_registers[1])[2:].zfill(4)
    if coding_type == 0:
        voltage = (((int(code, 16) / 2**23) - 1) * 2.5) / 0.1
    elif coding_type == 1:
        voltage = (int(code, 16) * 2.5) / (2**24 * 0.1)
    else:
        print(Fore.RED + "Invalid ADC coding type!")
    return voltage


def read_voltage_channels(
        global_data: Dict[str, Any], board_address: int) -> List[float]:
    """Read voltage channels from an address.

    Args:
        global_data: Dictionary with global variables
        board_address: CS value

    Returns:
        Measured voltage

    """
    values = []
    for index in range(1, 5):
        coding_type = read_output_holding_regs_cn0414(
            global_data, [global_data["MODBUS_ADDRESS"], board_address, 0])[0]
        values.append(read_voltage(
            global_data, [global_data["MODBUS_ADDRESS"], board_address, index],
            coding_type))
    return values


def request_adc_cs(global_data: Dict[str, Any]) -> int:
    """Request ADC CS.

    Args:
        global_data: Dictionary with global variables

    Returns:
        CS address

    """
    valid_adc_cs = []
    for index, _ in enumerate(global_data["BOARDS"]):
        if global_data["BOARDS"][index] == '0':
            valid_adc_cs.append(global_data["VALID_CS"][index])
    while True:
        print(Fore.CYAN + "Enter CS address from this list",
              Fore.YELLOW + str(valid_adc_cs) + Fore.CYAN + ",",
              Fore.CYAN + "or press ENTER to use CS",
              Fore.YELLOW + str(valid_adc_cs[0]) + Fore.CYAN + ": ", end='')
        input_data = input()
        if input_data == '':
            cs_address = int(valid_adc_cs[0])
            break
        elif int(input_data.split()[0]) not in valid_adc_cs:
            print(Fore.YELLOW + "Invalid option. Try again.")
        else:
            cs_address = int(input_data)
            break
    return cs_address


def select_hart_channel(global_data: Dict[str, Any]) -> None:
    """Select HART channel.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.CYAN + "No analog input board available at",
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
    if '0' not in global_data["BOARDS"]:
        print(Fore.CYAN + "No analog input board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        print(Fore.CYAN + "Send HART command zero:")
        print(Fore.YELLOW + '\t' + '1 to send HART command 0')
        cs_addr, reg_val = utilities.request_reconfig_data(global_data)
        utilities.write_output_holding_reg(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, 5], reg_val)
        if reg_val == 1:
            sleep(2)
            registers = read_analog_input_regs_cn0414(
                global_data, [global_data["MODBUS_ADDRESS"], cs_addr, 25], 30)
            response = ''
            for index, _ in enumerate(registers):
                response += str(hex(registers[index]).upper()[2:]) + " "
            print(Fore.GREEN + "DEVICE RESPONSE: " + Fore.YELLOW + response)


def set_adc_filter(global_data: Dict[str, Any]) -> None:
    """Select ADC intrnal filter configuration.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        print(Fore.CYAN + "Set ADC filter:")
        labels = ['s5+s1', 's3']
        for index, _ in enumerate(labels):
            print(Fore.YELLOW + '\t' + str(index) + ' for ' + labels[index])
        cs_addr, reg_val = utilities.request_reconfig_data(global_data)
        utilities.write_output_holding_reg(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, 1], reg_val)


def set_adc_open_wire_detection(global_data: Dict[str, Any]) -> None:
    """Set ADC open wire detection feature.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address ",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        print(Fore.CYAN + "Set ADC output data rate:")
        labels = ['disable open wire detection', 'enable open wire detection']
        for index, _ in enumerate(labels):
            print(Fore.YELLOW + '\t' + str(index) + ' for ' + labels[index])
        cs_addr, reg_val = utilities.request_reconfig_data(global_data)
        utilities.write_output_holding_reg(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, 4], reg_val)


def set_adc_output_code(global_data: Dict[str, Any]) -> None:
    """Set ADC output data coding.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        print(Fore.CYAN + "Set ADC output code:")
        labels = ['bipolar coding', 'unipolar coding']
        for index, _ in enumerate(labels):
            print(Fore.YELLOW + '\t' + str(index) + ' for ' + labels[index])
        cs_addr, reg_val = utilities.request_reconfig_data(global_data)
        utilities.write_output_holding_reg(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, 0], reg_val)


def set_adc_output_data_rate(global_data: Dict[str, Any]) -> None:
    """Set ADC output data rate.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        print(Fore.CYAN + "Set ADC output data rate:")
        labels = [
            '31250 SPS', '31250 SPS', '31250 SPS', '31250 SPS', '31250 SPS',
            '31250 SPS', '15625 SPS', '10417 SPS', '5208 SPS', '2597 SPS',
            '1007 SPS', '503.8 SPS', '381 SPS', '200.3 SPS', '100.5 SPS',
            '59.52 SPS', '49.68 SPS', '20.01 SPS', '16.63 SPS', '10 SPS',
            '5 SPS', '2.5 SPS', '1.25 SPS']
        for index, _ in enumerate(labels):
            print(Fore.YELLOW + '\t' + str(index) + ' for ' + labels[index])
        cs_addr, reg_val = utilities.request_reconfig_data(global_data)
        utilities.write_output_holding_reg(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, 3], reg_val)


def set_adc_postfilter(global_data: Dict[str, Any]) -> None:
    """Set ADC postfilter.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    if '0' not in global_data["BOARDS"]:
        print(Fore.LIGHTMAGENTA_EX + "No analog input board available at",
              Fore.CYAN + "MODBUS address",
              Fore.CYAN + str(global_data["MODBUS_ADDRESS"]))
    else:
        print(Fore.CYAN + "Set ADC postfilter:")
        labels = ['27 SPS, 47 dB rejection, 36.7 ms settling',
                  '25 SPS, 62 dB rejection, 40 ms settling',
                  '20 SPS, 86 dB rejection, 50 ms settling',
                  '16.67 SPS, 92 dB rejection, 60 ms settling',
                  'disable postfilter']
        for index, _ in enumerate(labels):
            print(Fore.YELLOW + '\t' + str(index) + ' for ' + labels[index])
        cs_addr, reg_val = utilities.request_reconfig_data(global_data)
        utilities.write_output_holding_reg(
            global_data, [global_data["MODBUS_ADDRESS"], cs_addr, 2], reg_val)
