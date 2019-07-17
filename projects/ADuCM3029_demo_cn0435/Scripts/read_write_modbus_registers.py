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

from colorama import init, Fore
import minimalmodbus
import dcs_cn0435_utilities as utilities

init(autoreset=True)


def run_selected_function(test_option: str) -> None:
    """Run selected function.

    Args:
        test_option: Option value (number or letter)

    Returns:
        None

    """
    options = ['1', '2', '3', '4', '5', 'q']
    if test_option in options:
        if test_option == '1':
            utilities.read_common_analog_input_regs(GLOBAL_DATA, debug=True)
        elif test_option == '2':
            utilities.read_common_output_holding_regs(GLOBAL_DATA, debug=True)
        elif test_option == '3':
            utilities.read_analog_input_regs_from_sys_config(GLOBAL_DATA)
        elif test_option == '4':
            utilities.read_output_holding_regs_from_sys_config(GLOBAL_DATA)
        elif test_option == '5':
            utilities.select_and_write_register(GLOBAL_DATA)
        elif test_option == 'q':
            print(Fore.CYAN + "\nExit... Good Bye!")
            exit(1)
        else:
            pass
    else:
        print(Fore.RED + 'No valid option selected!')


def test_options() -> str:
    """Print test options.

    Args:
        None

    Returns:
        Keyboard input

    """
    print(Fore.LIGHTMAGENTA_EX + "\nTest options:")
    print(Fore.LIGHTMAGENTA_EX + "1 - Read common analog input registers.")
    print(Fore.LIGHTMAGENTA_EX + "2 - Read common output holding registers.")
    print(Fore.LIGHTMAGENTA_EX + "3 - Read analog input registers.")
    print(Fore.LIGHTMAGENTA_EX + "4 - Read output holding registers.")
    print(Fore.LIGHTMAGENTA_EX + "5 - Write output holding register.")
    print(Fore.LIGHTMAGENTA_EX + "q - Quit.")
    print(Fore.GREEN + "\nEnter test option: ", end='')
    return input()


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
            NEW_DATA = utilities.switch_modbus_address(
                PORT_NUMBER, MODBUS_TIMEOUT)
            MODBUS_ADDRESS = NEW_DATA[0]
            VALID_CS, BOARDS = NEW_DATA[1], NEW_DATA[2]
