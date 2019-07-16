"""Detect configuration module.

This module provide a simple way to detect the PLC/DCS configuration.
In this way the user can easy detect boards like:
    - CN0414 (Analog input and HART compatible)
    - CN0418 (Analog output and HART compatible)
The CN0416 (RS485 transceiver) needs to be used in a DCS configuration,
while for a PLC configuration is optional.

Only one comunication port is used and needs to be chosen by the user.
"""

__version__ = '0.1'
__author__ = 'Mihai Ionut Suciu'
__status__ = 'Development'

from time import sleep
from colorama import init, Fore
import minimalmodbus
import serial.tools.list_ports
from tabulate import tabulate

init(autoreset=True)


def request_info() -> str:
    """Request input data.

    List all serial devices detected in use and wait for the user to choose
    which port/serial device want to use.

    Args:
        None

    Returns:
        Return requested port name

    """
    print(Fore.CYAN + "\nWelcome! ", end='')
    print(Fore.CYAN + "Use 'CTRL+C' to go back from current menu or exit!")

    devices, ports = serial_devices()

    print(Fore.GREEN + '\nAvailable devices:',
          Fore.YELLOW + " ".join(str(element) for element in devices))

    default = Fore.GREEN + "or press ENTER to use " + ports[0] + ": "
    print(Fore.GREEN + "\nEnter detected device index,", default, end='')
    input_data = input()
    if input_data == '':
        port_number = 0
    else:
        port_number = int(input_data) - 1
    return ports[port_number]


def serial_devices() -> tuple:
    """Lists serial port names and description.

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


def search_serial_port_id(
        device_description: str, view_device_description: bool = False) -> str:
    """Search for a serial device description.

    Args:
        device_description: Device description string
        view_device_description: flag to display device description

    Returns:
        Device port name
    """
    devices, descriptions, device_port = [], [], None
    all_ports = serial.tools.list_ports.comports()
    for current_port in all_ports:
        devices.append(current_port.device)
        descriptions.append(current_port.description)
        for index, _ in enumerate(descriptions):
            if descriptions[index] == device_description:
                device_port = devices[index]
    if view_device_description:
        print(descriptions)
    return device_port


def print_table(
        register_address: int, register_count: int,
        labels: list, registers: list) -> None:
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


def scan_system_config(port_name: str) -> None:
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
    for modbus_index in range(1, 17):
        try:
            instrument = minimalmodbus.Instrument(port_name, modbus_index)
            registers = instrument.read_registers(0, 5, 4)
            sleep(0.05)
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
                        print(Fore.GREEN + 'Analog input board at address:',
                              Fore.YELLOW + str(board_address))
                    elif board_type == '1':
                        print(Fore.GREEN + 'Analog output board at address:',
                              Fore.YELLOW + str(board_address))
                    else:
                        pass
        except OSError as os_error_message:
            print(Fore.LIGHTMAGENTA_EX + 'No boards at MODBUS address:',
                  Fore.YELLOW + str(modbus_index),
                  Fore.CYAN + str(os_error_message))


if __name__ == "__main__":
    try:
        PORT = request_info()
        scan_system_config(PORT)
    except OSError as os_error_message:
        print(Fore.RED + 'Timeout error!', os_error_message)
    except ValueError as value_error_message:
        print(Fore.RED + 'Register operation error!', value_error_message)
