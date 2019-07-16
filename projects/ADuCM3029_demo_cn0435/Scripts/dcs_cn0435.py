"""PLC MODBUS module.

This module provide a simple terminal interface to easy interact with:
    - CN0414 - Analog input and HART for PLC/DCS systems
    - CN0418 - Analog output and HART for PLC/DCS systems
    - CN0416 - RS485 transceiver
Only one comunication port is used and a single MODBUS node.
This application also allows to switch between MODBUS nodes if are available.

"""

__version__ = '0.1'
__author__ = 'Mihai Ionut Suciu'
__status__ = 'Development'

from itertools import zip_longest
from colorama import init, Fore
import minimalmodbus
from tabulate import tabulate
import dcs_cn0435_utilities as utilities
import dcs_cn0414_utilities as cn0414
import dcs_cn0418_utilities as cn0418

init(autoreset=True)


def read_adc_data(test_option: str) -> None:
    """Read ADC data.

    Args:
        test_option: Option value (number or letter)

    Returns:
        None

    """
    if test_option == '1':
        cn0414.read_device_voltage_channel(GLOBAL_DATA)
    elif test_option == '2':
        cn0414.read_device_current_channel(GLOBAL_DATA)
    elif test_option == '3':
        cn0414.read_board_voltage_channels(GLOBAL_DATA)
    elif test_option == '4':
        cn0414.read_board_current_channels(GLOBAL_DATA)
    elif test_option == '5':
        cn0414.read_instrument_voltage_channels(GLOBAL_DATA)
    elif test_option == '6':
        cn0414.read_instrument_current_channels(GLOBAL_DATA)
    else:
        pass


def read_registers(test_option: str) -> None:
    """Read analog input or output holding registers.

    Args:
        test_option: Option value (number or letter)

    Returns:
        None

    """
    if test_option == 'o':
        utilities.read_common_analog_input_regs(GLOBAL_DATA, debug=True)
    elif test_option == 'p':
        utilities.read_common_output_holding_regs(GLOBAL_DATA, debug=True)
    elif test_option == 'r':
        utilities.read_analog_input_regs_from_sys_config(GLOBAL_DATA)
    elif test_option == 's':
        utilities.read_output_holding_regs_from_sys_config(GLOBAL_DATA)
    else:
        pass


def run_selected_function(test_option: str) -> None:
    """Run selected function.

    Args:
        test_option: Option value (number or letter)

    Returns:
        None

    """
    options = ['1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
               'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
               'p', 'r', 's', 't', 'q']
    if test_option in options:
        read_registers(test_option)
        set_adc_parametters(test_option)
        read_adc_data(test_option)
        write_dac_data(test_option)
        set_hart(test_option)
        if test_option == 't':
            utilities.scan_system_config(PORT_NUMBER, DELAY)
        elif test_option == 'q':
            print(Fore.CYAN + "\nExit... Good Bye!")
            exit(1)
        else:
            pass
    else:
        print(Fore.RED + 'No valid option selected!')


def set_adc_parametters(test_option: str) -> None:
    """Set ADC parametters.

    Args:
        test_option: Option value (number or letter)

    Returns:
        None

    """
    if test_option == '7':
        cn0414.set_adc_output_code(GLOBAL_DATA)
    elif test_option == '8':
        cn0414.set_adc_filter(GLOBAL_DATA)
    elif test_option == '9':
        cn0414.set_adc_postfilter(GLOBAL_DATA)
    elif test_option == 'a':
        cn0414.set_adc_output_data_rate(GLOBAL_DATA)
    elif test_option == 'b':
        cn0414.set_adc_open_wire_detection(GLOBAL_DATA)
    else:
        pass


def set_hart(test_option: str) -> None:
    """Send HART command 0 or select HART channel.

    Args:
        test_option: Option value (number or letter)

    Returns:
        None

    """
    if test_option == 'c':
        cn0414.send_hart_command_0(GLOBAL_DATA)
    elif test_option == 'd':
        cn0414.select_hart_channel(GLOBAL_DATA)
    elif test_option == 'm':
        cn0418.send_hart_command_0(GLOBAL_DATA)
    elif test_option == 'n':
        cn0418.select_hart_channel(GLOBAL_DATA)
    else:
        pass


def test_options() -> str:
    """Print test options.

    Args:
        None

    Returns:
        Keyboard input

    """
    cn0435_options = [
        Fore.GREEN + 'o - Read common analog input registers',
        Fore.GREEN + 'p - Read common output holding registers',
        Fore.GREEN + 'r - Read analog input registers',
        Fore.GREEN + 's - Read output holding registers',
        Fore.GREEN + 't - Detect system configuration',
        Fore.GREEN + '',
        Fore.GREEN + '',
        Fore.GREEN + '',
        Fore.GREEN + '',
        Fore.GREEN + '',
        Fore.GREEN + '',
        Fore.GREEN + '',
        Fore.YELLOW + 'q - Quit']

    cn0414_options = [
        Fore.GREEN + '1 - Read device voltage channel',
        Fore.GREEN + '2 - Read device current channel',
        Fore.GREEN + '3 - Read board voltage channels',
        Fore.GREEN + '4 - Read board current channels',
        Fore.GREEN + '5 - Read instrument voltage channels',
        Fore.GREEN + '6 - Read instrument current channels',
        Fore.GREEN + '7 - Set ADC output code',
        Fore.GREEN + '8 - Set ADC filter',
        Fore.GREEN + '9 - Set ADC postfilter',
        Fore.GREEN + 'a - Set ADC output data rate',
        Fore.GREEN + 'b - Set ADC open wire detection state',
        Fore.GREEN + 'c - Send HART command zero',
        Fore.GREEN + 'd - Select HART channel']

    cn0418_options = [
        Fore.GREEN + 'e - Set DAC channel 1 output',
        Fore.GREEN + 'f - Set DAC channel 2 output',
        Fore.GREEN + 'g - Set DAC channel 3 output',
        Fore.GREEN + 'h - Set DAC channel 4 output',
        Fore.GREEN + 'i - Set DAC channel 1 range',
        Fore.GREEN + 'j - Set DAC channel 2 range',
        Fore.GREEN + 'k - Set DAC channel 3 range',
        Fore.GREEN + 'l - Set DAC channel 4 range',
        Fore.GREEN + 'm - Send HART command zero',
        Fore.GREEN + 'n - Select HART channel']

    print(Fore.CYAN + "\nUse CTRL+C to end a process or switch between nodes.")
    print(tabulate(
        headers=[Fore.YELLOW + 'CN0414',
                 Fore.YELLOW + 'CN0418',
                 Fore.YELLOW + 'CN0435'],
        tabular_data=list(
            zip_longest(cn0414_options, cn0418_options, cn0435_options))))
    print(Fore.CYAN + "\nEnter Option: ", end='')

    return input()


def write_dac_data(test_option: str) -> None:
    """Write DAC data.

    Args:
        test_option: Option value (number or letter)

    Returns:
        None

    """
    if test_option == 'e':
        cn0418.select_channel_range(GLOBAL_DATA, 1)
    elif test_option == 'f':
        cn0418.select_channel_range(GLOBAL_DATA, 2)
    elif test_option == 'g':
        cn0418.select_channel_range(GLOBAL_DATA, 3)
    elif test_option == 'h':
        cn0418.select_channel_range(GLOBAL_DATA, 4)
    elif test_option == 'i':
        cn0418.set_channel_output(GLOBAL_DATA, 1)
    elif test_option == 'j':
        cn0418.set_channel_output(GLOBAL_DATA, 2)
    elif test_option == 'k':
        cn0418.set_channel_output(GLOBAL_DATA, 3)
    elif test_option == 'l':
        cn0418.set_channel_output(GLOBAL_DATA, 4)
    else:
        pass


if __name__ == "__main__":
    DATA = utilities.request_info()
    PORT_NUMBER, MODBUS_ADDRESS = DATA[0], DATA[1]
    MODBUS_TIMEOUT, DELAY = DATA[2], DATA[3]
    VALID_CS, BOARDS = DATA[4], DATA[5]

    while True:
        try:
            INSTRUMENT = minimalmodbus.Instrument(PORT_NUMBER, MODBUS_ADDRESS)
            INSTRUMENT.serial.timeout = MODBUS_TIMEOUT

            GLOBAL_DATA = {
                "INSTRUMENT": INSTRUMENT,
                "MODBUS_ADDRESS": MODBUS_ADDRESS,
                "DELAY": DELAY,
                "VALID_CS": VALID_CS,
                "BOARDS": BOARDS
            }

            while True:
                try:
                    run_selected_function(test_options())
                except OSError as error_message:
                    print(Fore.RED + 'Timeout error!', error_message)
                except ValueError as error_message:
                    print(Fore.RED + 'Register operation error!',
                          error_message)
        except KeyboardInterrupt:
            print(Fore.CYAN + "\nSwitch to a new MODBUS address... ")
            DATA = utilities.switch_modbus_address(PORT_NUMBER, MODBUS_ADDRESS)
            MODBUS_ADDRESS, VALID_CS, BOARDS = DATA[0], DATA[1], DATA[2]
