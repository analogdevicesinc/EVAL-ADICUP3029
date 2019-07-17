"""Base MODBUS functions.

This module provide base functionalities to easy interact with:
CN0414 - Analog input and HART for PLC/DCS systems
CN0418 - Analog output and HART for PLC/DCS systems
CN0416 - RS485 transceiver.
"""

__version__ = '0.1'
__author__ = 'Mihai Ionut Suciu'
__status__ = 'Development'

import sys
from time import sleep
from typing import Dict, Any, Tuple, List
from colorama import init, Fore
from tabulate import tabulate
import serial.tools.list_ports
import minimalmodbus
import dcs_cn0414_utilities as cn0414
import dcs_cn0418_utilities as cn0418

init(autoreset=True)


def detect_system_configuration(
        global_data: Dict[str, Any]) -> Tuple[List[int], List[int]]:
    """Detect boards connected.

    Args:
        global_data: Dictionary with global variables

    Returns:
        Tuple of two lists

    """
    registers = read_common_analog_input_regs(global_data)
    valid_addr, boards = [], []
    for index in range(1, 5):
        if registers[index] < 8:
            board_address = format(registers[index], '03b')[:2]
            board_type = format(registers[index], '03b')[2:]
            valid_addr.append(int(board_address, 2))
            if board_type == '0':
                print(Fore.GREEN + 'Analog input board at address:',
                      Fore.YELLOW + str(board_address))
                boards.append(0)
            elif board_type == '1':
                print(Fore.GREEN + 'Analog output board at address:',
                      Fore.YELLOW + str(board_address))
                boards.append(1)
            else:
                pass
    print()
    return valid_addr, boards


def generate_channel_register_address(
        channel_type: str, address: List[int]) -> int:
    """Generate channel register address.

    Args:
        channel_type:
            - If "V" voltage channel is selected
            - If "I" current channel is selected
        address: List of MODBUS address, CS and channel number

    Returns:
        Register address

    """
    if channel_type == 'V':
        offset = 5
    elif channel_type == 'I':
        offset = 13
    else:
        print('Invalid channel type option.')
    bin_modbus_addr = bin(address[0])[2:].zfill(4)
    bin_cs_addr = bin(address[1])[2:].zfill(4)
    bin_channel_addr = bin((2 * (address[2] - 1)) + offset)[2:].zfill(8)
    register_address = int(bin_cs_addr + bin_modbus_addr + bin_channel_addr, 2)
    return register_address


def generate_register_address(address: List[int]) -> int:
    """Generate register address from a list.

    Args:
        address: list used to generate the the registers address

    Returns:
        Return the registers address

    """
    bin_modbus_addr = bin(address[0])[2:].zfill(4)
    bin_cs_addr = bin(address[1])[2:].zfill(4)
    bin_offset_addr = bin(address[2])[2:].zfill(8)
    register_address = int(bin_cs_addr + bin_modbus_addr + bin_offset_addr, 2)
    return register_address


def print_table(register_address: int, register_count: int,
                labels: List[str], registers: List[int]) -> None:
    """Print data in a table.

    Print registers addresses, description and
        current value in a colored table format.

    Args:
        register_address: First register number
        register_count: Number of registers
        labels: Registers description
        registers: Registers values

    Returns:
        None

    """
    register_list = range(register_address, register_address + register_count)
    address_list = []
    for index, _ in enumerate(register_list):
        dec_address = Fore.GREEN + str(register_list[index]).zfill(4)
        hex_address = Fore.GREEN + ' (0x{0:04X})'.format(register_list[index])
        address_list.append(dec_address + hex_address)
    values = []
    for index, _ in enumerate(labels):
        values.append(Fore.GREEN + str(registers[index]))
    print(tabulate(headers=[Fore.CYAN + 'Address',
                            Fore.CYAN + 'Name',
                            Fore.CYAN + 'Value'],
                   tabular_data=list(zip(address_list, labels, values))))


def read_analog_input_regs_from_sys_config(
        global_data: Dict[str, Any]) -> None:
    """Read analog input registers from detected system configuration.

    Determine the number and type (analog input/output) of the boards detected
    in the curent system configuration and read all analog input registers.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    system_config = detect_system_configuration(global_data)
    boards_detected, boards_types = system_config[0], system_config[1]
    for count, _ in enumerate(boards_detected):
        cs_address = boards_detected[count]
        if boards_types[count] == 0:
            cn0414.read_analog_input_regs_cn0414(
                global_data, [global_data["MODBUS_ADDRESS"], cs_address, 5],
                debug=True)
        elif boards_types[count] == 1:
            cn0418.read_analog_input_regs_cn0418(
                global_data, [global_data["MODBUS_ADDRESS"], cs_address, 5],
                debug=True)
        else:
            pass


def read_common_analog_input_regs(
        global_data: Dict[str, Any], register_address: int = 0,
        registers_number: int = 5, debug: bool = False) -> List[int]:
    """Read common analog input registers.

    Read common analog input registers with function code 4.

    Args:
        global_data: Dictionary with global variables
        register_address: MODBUS register start address (default value = 0)
        registers_number: Number of MODBUS register read (default value = 5)
        debug: If True will print registers description and value
            in a colored table (default value = False)

    Returns:
        Return a list of registers values

    """
    sleep(global_data["DELAY"])
    registers = global_data["INSTRUMENT"].read_registers(
        register_address, registers_number, functioncode=4)
    if debug:
        print(Fore.LIGHTMAGENTA_EX + '\nCommon analog input registers:')
        labels = [
            Fore.YELLOW + 'Detected boards',
            Fore.YELLOW + 'First board data',
            Fore.YELLOW + 'Second board data',
            Fore.YELLOW + 'Third board data',
            Fore.YELLOW + 'Fourth board data']
        print_table(register_address, registers_number, labels, registers)
    return registers


def read_common_output_holding_regs(
        global_data: Dict[str, Any], register_address: int = 254,
        registers_number: int = 2, debug: bool = False) -> List[int]:
    """Read common output holding registers.

    Read common output holding registers with function code 3.

    Args:
        global_data: Dictionary with global variables
        register_address: MODBUS register start address (default value = 254)
        registers_number: Number of MODBUS register read (default value = 2)
        debug: If True will print registers description and value
            in a colored table (default value = False)

    Returns:
        Return a list of registers values

    """
    sleep(global_data["DELAY"])
    registers = global_data["INSTRUMENT"].read_registers(
        register_address, registers_number, functioncode=3)
    if debug:
        print(Fore.LIGHTMAGENTA_EX + '\nCommon output holding registers:')
        labels = [
            Fore.YELLOW + 'Update rate MSW',
            Fore.YELLOW + 'Update rate LSW']
        print_table(register_address, registers_number, labels, registers)
    return registers


def read_output_holding_regs_from_sys_config(
        global_data: Dict[str, Any]) -> None:
    """Read output holding registers from detected system configuration.

    Determine the number and type (analog input/output) of the boards detected
    in the curent system configuration and read all output holding registers.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    system_config = detect_system_configuration(global_data)
    boards_detected, boards_types = system_config[0], system_config[1]
    for count, _ in enumerate(boards_detected):
        cs_address = boards_detected[count]
        if boards_types[count] == 0:
            cn0414.read_output_holding_regs_cn0414(
                global_data, [global_data["MODBUS_ADDRESS"], cs_address, 0],
                debug=True)
        elif boards_types[count] == 1:
            cn0418.read_output_holding_regs_cn0418(
                global_data, [global_data["MODBUS_ADDRESS"], cs_address, 0],
                debug=True)
        else:
            pass


def request_cs_data(global_data: Dict[str, Any]) -> int:
    """Request CS address.

    Args:
        global_data: Dictionary with global variables

    Returns:
        CS address

    """
    while True:
        print(Fore.CYAN + "Enter CS address from this list",
              Fore.YELLOW + str(global_data["VALID_CS"]) + Fore.CYAN + ",",
              Fore.CYAN + "or press ENTER to use CS",
              Fore.YELLOW + str(global_data["VALID_CS"][0]) + Fore.CYAN + ": ",
              end='')
        input_data = input()
        if input_data == "":
            cs_addr = int(global_data["VALID_CS"][0])
            break
        elif int(input_data.split()[0]) not in global_data["VALID_CS"]:
            print(Fore.YELLOW + "Invalid option. Try again.")
        else:
            cs_addr = int(input_data.split()[0])
            break
    return cs_addr


def request_info() -> Tuple[str, int, float, float, List[int], List[str]]:
    """Request input data.

    List all serial devices detected in use and wait for the user to choose
    which port/serial device want to use.

    Args:
        None

    Returns:
        Return requested port name, modbus address,modbus timeout, delay
        between commands and devices address(es) for current MODBUS address

    """
    print(Fore.CYAN + "\nWelcome! ", end='')
    print(Fore.CYAN + "Use 'CTRL+C' to go back from the current menu or exit!")

    devices, ports = serial_devices()

    print(Fore.GREEN + '\nAvailable devices:',
          Fore.YELLOW + " ".join(str(element) for element in devices))

    default = Fore.GREEN + "or press ENTER to use " + ports[0] + ": "
    print(Fore.GREEN + "\nEnter detected device index,", default, end='')
    input_data = input()
    if input_data == '':
        port_index = 0
    else:
        port_index = int(input_data) - 1

    default = Fore.GREEN + "or press ENTER to use 0.1[s] timeout: "
    print(Fore.GREEN + "Enter MODBUS timeout (0.05[s] to inf),", default,
          end='')
    timeout = input()
    if timeout == '':
        returned_timeout = 0.1
    elif float(timeout) <= 0.05:
        returned_timeout = 0.05
    else:
        returned_timeout = float((timeout.split()[0]))

    modbus_address, current_cs_list, current_boards = switch_modbus_address(
        ports[port_index], returned_timeout)

    default = Fore.GREEN + "or press ENTER to use 0.1[s] delay: "
    print(Fore.GREEN + "Enter commands delay (0[s] to inf),", default, end='')
    delay = input()
    if delay == '':
        returned_delay = 0.1
    else:
        returned_delay = float((delay.split()[0]))

    return ports[port_index], modbus_address, returned_timeout, \
        returned_delay, current_cs_list, current_boards


def request_reconfig_data(global_data: Dict[str, Any]) -> Tuple[int, int]:
    """Request device CS address and register value.

    Args:
        global_data: Dictionary with global variables

    Returns:
        Return requested device address and register value

    """
    cs_addr = request_cs_data(global_data)

    value_msg = "Enter register value, or press ENTER to use value 0: "
    print(Fore.CYAN + value_msg, end='')
    input_data = input()
    if input_data == "":
        reg_val = 0
    else:
        reg_val = int(input_data.split()[0])

    return cs_addr, reg_val


def select_and_write_register(global_data: Dict[str, Any]) -> None:
    """Write any output holding register.

    Args:
        global_data: Dictionary with global variables

    Returns:
        None

    """
    cs_addr = request_cs_data(global_data)

    offset_msg = "Enter offset address, or press ENTER to use offset 0: "
    print(Fore.CYAN + offset_msg, end='')
    input_data = input()
    if input_data == "":
        offset_addr = 0
    else:
        offset_addr = int(input_data.split()[0])

    value_msg = "Enter register value, or press ENTER to use value 0: "
    print(Fore.CYAN + value_msg, end='')
    input_data = input()
    if input_data == "":
        reg_val = 0
    else:
        reg_val = int(input_data.split()[0])

    write_output_holding_reg(
        global_data, [global_data["MODBUS_ADDRESS"], cs_addr, offset_addr],
        reg_val)


def scan_system_config(
        port_name: str,
        delay: float) -> Tuple[List[int], List[List[int]], List[List[str]]]:
    """Detect system configuration for each possible MODBUS node.

    Create an instrument object for each MODBUS node and read 5 analog
    input registers with function code 4 starting from address 0.
    Print registers description and value in a colored table.
    Convert first 4 registers value in binary format and check if the least
    significant bit is 0 or 1. If 0 is found, an analog input board have been
    detected, else, if 1 is found, an analog output board have been detected.

    Args:
        port_name: COMPORT name

    Returns:
        None

    """
    valid_modbus_addresses, current_cs, all_valid_cs = [], [], []
    current_board_type, all_boards = [], []
    for modbus_index in range(1, 17):
        try:
            instrument = minimalmodbus.Instrument(port_name, modbus_index)
            registers = instrument.read_registers(0, 5, 4)
            valid_modbus_addresses.append(modbus_index)
            sleep(delay)
            print(Fore.GREEN + '\nBoards found at MODBUS address:',
                  Fore.YELLOW + str(modbus_index))
            labels = [
                Fore.YELLOW + 'Detected boards',
                Fore.YELLOW + 'First board data',
                Fore.YELLOW + 'Second board data',
                Fore.YELLOW + 'Third board data',
                Fore.YELLOW + 'Fourth board data']
            print_table(0, 5, labels, registers)
            for index in range(1, 5):
                if registers[index] < 8:
                    board_address = format(registers[index], '03b')[:2]
                    board_type = format(registers[index], '03b')[2:]
                    if board_type == '0':
                        print(Fore.GREEN + 'analog input board at address:',
                              Fore.YELLOW + str(board_address))
                        current_cs.append(int('0b' + board_address, 2))
                        current_board_type.append('0')
                    elif board_type == '1':
                        print(Fore.GREEN + 'analog output board at address:',
                              Fore.YELLOW + str(board_address))
                        current_cs.append(int('0b' + board_address, 2))
                        current_board_type.append('1')
                    else:
                        pass
            all_valid_cs.append(current_cs)
            all_boards.append(current_board_type)
            current_cs, current_board_type = [], []
        except OSError as os_error_message:
            print(Fore.LIGHTMAGENTA_EX + 'No boards at MODBUS address:',
                  Fore.YELLOW + str(modbus_index),
                  Fore.CYAN + str(os_error_message))
    return valid_modbus_addresses, all_valid_cs, all_boards


def serial_devices() -> Tuple[List[str], List[str]]:
    """List serial port names and description.

    Search for all serial devices detected in use and return
    their description and port name as a list with an index.

    Args:
        None

    Returns:
        Return a tuple of two lists, serial device(s) description(s)
        and serial device(s) port(s).

    """
    devices, ports = [], []
    all_ports = serial.tools.list_ports.comports()
    for index, current_port in enumerate(all_ports, 1):
        devices.append("\n" + str(index) + " -> " + current_port.description)
        ports.append(current_port.device)
    return devices, ports


def serial_ports() -> List[str]:
    """List serial port names.

    Args:
        None

    Returns:
        List of detected serial ports.

    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            serial_port = serial.Serial(port)
            serial_port.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    print(result)
    return result


def switch_modbus_address(selected_port: str,
                          timeout: float) -> Tuple[int, List[int], List[str]]:
    """Switch between MODBUS address.

    Args:
        selected_port: Port name
        timeout: MODBUS timeout

    Returns:
        Return a tuple of two lists, MODBUS address(es)
        and detected device(s) address(es).

    """
    modbus_list, all_valid_cs, all_boards = scan_system_config(
        selected_port, timeout)

    while True:
        print(Fore.GREEN + "\nEnter MODBUS address from this list",
              Fore.YELLOW + str(modbus_list) + Fore.GREEN + ",",
              Fore.GREEN + "or press ENTER to use MODBUS address",
              Fore.YELLOW + str(modbus_list[0]) + Fore.GREEN + ': ', end='')
        modbus_address = input()
        if modbus_address == '':
            returned_modbus_address = modbus_list[0]
            current_cs_list = all_valid_cs[0]
            current_boards_list = all_boards[0]
            break
        elif int((modbus_address.split()[0])) not in modbus_list:
            print(Fore.YELLOW + "Invalid option. Try again.")
        else:
            returned_modbus_address = int((modbus_address.split()[0]))
            current_cs_list = all_valid_cs[modbus_list.index(
                returned_modbus_address)]
            current_boards_list = all_boards[modbus_list.index(
                returned_modbus_address)]
            break
    return returned_modbus_address, current_cs_list, current_boards_list


def write_output_holding_reg(
        global_data: Dict[str, Any], address: List[int],
        registers_values: int) -> None:
    """Write output holding register.

    Write a single output holding register with function code 6.

    Args:
        instrument: Instrument object created by minimalmodbus
        delay: Delay between MODBUS commands
        address: MODBUS register address
        registers_values: New MODBUS register value to be write

    Returns:
        None

    """
    register_address = generate_register_address(address)
    sleep(global_data["DELAY"])
    global_data["INSTRUMENT"].write_register(
        register_address, registers_values, functioncode=6)
