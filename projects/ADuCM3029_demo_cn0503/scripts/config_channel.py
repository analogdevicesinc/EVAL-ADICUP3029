# Brief:  Script to configure a channel for the CN0503 board
# Author: Mihail Chindris (mihail.chindris@analog.com)
#
# Copyright 2020(c) Analog Devices, Inc.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  - Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  - Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#  - Neither the name of Analog Devices, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#  - The use of this software may or may not infringe the patent rights
#    of one or more patent holders.  This license does not release you
#    from the requirement that you obtain separate licenses from these
#    patent holders to use this software.
#  - Use of the software either in source or binary form, must be run
#    on or directly connected to an Analog Devices Inc. component.
#
# THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from cn0503 import CN0503
import numpy as np
from numpy.polynomial.polynomial import Polynomial
import matplotlib.pyplot as plt
from time import sleep
import argparse
from scipy.interpolate import lagrange

DEFAULT_NB_SAMPLES = 3

DEFUALT_SERIAL_PORT = 'COM4'

DEFAULT_INS1_NB_VALUES = 3
DEFAULT_INS1_UNITS  = [1, 2, 3]

DEFAULT_INS2_NB_VALUES = 3
DEFAULT_INS2_UNITS  = [1, 2, 3]

description_help='''Configure CN0503 optical channel

Note: When a prompt is shown user can answare with:
      Yes, No, y, n (case insensitive) or press enter to choose the default
      answare (in parentesis). Eg:
      - Do you want to configure relative ratio baseline ? (Yes) :
      By pressing enter Yes will be selected.

Examples:\n
    Start fluorimetry configuration for channel 1
    >python config_channel.py 1 FLUO
    
    Start colorimetry configuration for channel 4
    >python config_channel.py 4 COLO

    Start turbidity configuration for channel 3 and manual select of serial port
    >python config_channel.py 3 TURB -select_port

'''
measurment_type_help = 'FLUO (fluorimetry), COLO (colorimetry) or TURB (turbidity)'
select_port_help = '''If select_port option is set, a list of available serial\
ports will be shown.
Then the user can choose a port where the board is connected at.
Otherwise, the hardcoded DEFUALT_SERIAL_PORT value for the port will be used'''

config_file_help = '''File where to save the configuration commands
The saved file can be used to configure a new chip with load_cfg.py 
If the file exist a prompt will ask to append or overwrite it.
For example, you can call config_channel 1 FLUO -f file1.cfg . Do the
configuration and then do the same for the second channel and append the
configuration to file1.cfg. Also, configuration can be appended to default chip
configuration files.'''

def parse_input():
    parser = argparse.ArgumentParser(description=description_help,\
                            formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('channel_number', type=int, help='From 1 to 4')
    parser.add_argument('measurment_type', help=measurment_type_help)
    parser.add_argument('-f', metavar='config_file', help=config_file_help)
    parser.add_argument('-select_port', help=select_port_help, action='store_true')
    args = parser.parse_args()

    if not args.channel_number in CN0503.CHANNELS.ALL:
        print('Error: channel_number must be between 1 and 4')
        exit()

    if not args.measurment_type.upper() in CN0503.MEASURMENT_TYPES.ALL:
        print('Error: measurment_type should be one of the values:')
        print(CN0503.MEASURMENT_TYPES.ALL)
        exit()

    if args.measurment_type == CN0503.MEASURMENT_TYPES.TURBIDITY and \
      not args.channel_number in CN0503.CHANNELS.USABLE_FOR_TURBIDITY:
        print('Error: Only channel 1 and 4 can be used to measure turbidiy')
        exit()

    return (args.channel_number, args.measurment_type, args.select_port, args.f)

def choose_serial_port(dev):
    #Select connected port 
    ports = dev.list_serial_ports()
    print("Available Serial Ports: ", ports)
    return input("Enter Serial Port Number, e.g., COM3: ")

def read_samples(dev, channel_nb, nb_samples):
    input("Press enter when ready to start measurment ...")

    print("Reading %d samples (first is discarded)" % (nb_samples + 1))
    print("Ignoring first sample")
    data = dev.read_data(nb_samples + 1)[channel_nb - 1][1:]

    print("Readed: ")
    print(data)

    return data

def promt_yes_or_no(message, is_yes_default):
    if is_yes_default:
        ret = input(message + ' (Yes) :').lower().strip()
    else:
        ret = input(message + ' (No) :').lower().strip()
    if ret == '':
        if is_yes_default:
            return True
        else:
            return False

    if ret in ['yes', 'y']:
        return True
    else:
        return False

def plot_data(data, label):
        mean = np.mean(data)
        plt.plot(data, label='Data')
        plt.plot([mean] * len(data), label='Mean')
        plt.xlabel(label)
        plt.legend()
        plt.title(label + ' (close figure to continue ...)')
        plt.ticklabel_format(style='plain')
        plt.show()

def configure_relative_ratio_baseline(dev, channel_nb, measurment_type):
    print('Configuring Relative ratio')

    if measurment_type == CN0503.MEASURMENT_TYPES.TURBIDITY or\
           measurment_type == CN0503.MEASURMENT_TYPES.FLUORESCENCE:
        print("Ratio baseline for Turbidity and Fluorescence should be 1")
        print("Setting ratio baseline to 1")
        dev.set_ratio_baseline(channel_nb, 1)
        return

    print("Set display mode to absolut ratio")
    dev.set_display_mode(CN0503.DISPLAY_MODE.ABSOLUTE_RATIO)

    print("Please add destilled water")
    data = read_samples(dev, channel_nb, DEFAULT_NB_SAMPLES)
    mean = np.mean(data)
    print("Mean: %f" % (mean))

    print("Setting Ratio Baseline to %f" %(mean))
    dev.set_ratio_baseline(channel_nb, mean)

    print("Set display mode to RELATIVE_RATIO")
    dev.set_display_mode(CN0503.DISPLAY_MODE.RELATIVE_RATIO)

    print("Ready to read RELATIVE_RATIO data. Please add sample for measurment")
    data = read_samples(dev, channel_nb, DEFAULT_NB_SAMPLES)
    mean = np.mean(data)
    print("Mean: %f" % (mean))

    if promt_yes_or_no("Do you want current data to be plotted?", True):
        plot_data(data, 'Relativ ratio')
    
    print("Relative baseline ratio configuaration done")

def configure_instrumentation_measurment_unit(dev, channel_nb, is_ins1):
    default_nb_samples = 0
    default_units = []
    if is_ins1:
        print("Set display mode to relative ratio")
        dev.set_display_mode(CN0503.DISPLAY_MODE.RELATIVE_RATIO)
        default_nb_samples = DEFAULT_INS1_NB_VALUES
        default_units = DEFAULT_INS1_UNITS
    else:
        print("Set display mode to ins1")
        dev.set_display_mode(CN0503.DISPLAY_MODE.INSTRUMENTATION_UNIT_1)
        default_nb_samples = DEFAULT_INS2_NB_VALUES
        default_units = DEFAULT_INS2_UNITS

    nb_samples = input('How many samples do you want to use to calculate the '+\
                            'polinom for the instrumenatation unit? (%d) :' %\
                            default_nb_samples);
    if nb_samples == '':
        nb_samples = default_nb_samples
    else:
        nb_samples = int(nb_samples)
    
    ok = False
    while not ok:
        unit_values = input("Input the know unit values for the samples (%s) :"%\
                       " ".join(str(x) for x in default_units))
        if (unit_values == ''):
            unit_values = default_units
        else:
            unit_values = [int(val) for val in unit_values.split()]
        if len(unit_values) == nb_samples:
            ok = True

    mean = []
    for i in range(nb_samples):
        print("Please add sample number %d" % i)
        data = read_samples(dev, channel_nb, DEFAULT_NB_SAMPLES)
        mean.append(np.mean(data))
        print("Mean(%d): %f" % (i, mean[i]))
    
    x = np.array(mean)
    y = np.array(unit_values)
    poly = lagrange(x, y)
    coefs = poly.c.tolist()
    #Reverse list
    coefs = coefs[::-1]
    print("Coefficients for instrumentation measurment unit polynomial \n")
    print(coefs)
    
    print("Setting polinomyal coefficients")
    if is_ins1:
        dev.set_instrumentation_unit_1_polynomial(channel_nb, coefs)
    else:
        dev.set_instrumentation_unit_2_polynomial(channel_nb, coefs)

    print("Set display mode to instrumentation unit")
    if is_ins1:
        dev.set_display_mode(CN0503.DISPLAY_MODE.INSTRUMENTATION_UNIT_1)
    else:
        dev.set_display_mode(CN0503.DISPLAY_MODE.INSTRUMENTATION_UNIT_2)

    print("Ready to read instrumentation unit data. Please add sample for measurment")
    data = read_samples(dev, channel_nb, DEFAULT_NB_SAMPLES)
    mean = np.mean(data)
    print("Mean: %f" % (mean))
    nb = 2
    if is_ins1:
        nb = 1
    if promt_yes_or_no("Do you want current data to be plotted?", True):
        plot_data(data, 'Instrumentation unit %d' % (nb))

    print("Configure instrumentation measurment unit done")

def save_config(dev, file_name):
    if not file_name:
        file_name = input("Write the name of the file where to save:")
    f = None
    try:
        f = open(file_name, 'x')
    except FileExistsError:
        if promt_yes_or_no("File already exist. Do you want to append?", True):
            f = open(file_name, 'a')
        elif promt_yes_or_no("Do you want to overwrite?", True):
            f = open(file_name, 'w')
        else:
            return
    cfg = dev.get_config()
    while len(cfg) != 0:
        f.write(cfg.pop(0) + "\n")

def main():
    # Get user input to configure channel
    (channel_nb, measurment_type, select_port, config_file) = parse_input()
    print("Configuring channel %d to measure %s" % (channel_nb, measurment_type))

    dev = CN0503()

    #Configure comuniction port
    if select_port:
        port = choose_serial_port(dev)
    else:
        port = DEFUALT_SERIAL_PORT
    dev.set_serial_port(port)

    print("Set CN0503 measurment_type to %s" %(measurment_type))
    dev.set_measurment_type(channel_nb, measurment_type)
    
    config_question = 'Do you want to configure %s ?'
    repeat_question = 'Do you want to repeat %s ?'

    config = 'relative ratio baseline calibration'
    repeat = promt_yes_or_no(config_question % (config), True)
    while repeat:
        configure_relative_ratio_baseline(dev, channel_nb, measurment_type)
        repeat = promt_yes_or_no(repeat_question % (config), False)

    config = 'instrumentation measurment unit 1 calibration'
    repeat = promt_yes_or_no(config_question % (config), True)
    while repeat:
        configure_instrumentation_measurment_unit(dev, channel_nb, True)
        repeat = promt_yes_or_no(repeat_question % (config), False)

    config = 'instrumentation measurment unit 2 calibration'
    repeat = promt_yes_or_no(config_question % (config), True)
    while repeat:
        configure_instrumentation_measurment_unit(dev, channel_nb, False)
        repeat = promt_yes_or_no(repeat_question % (config), False)

    print('Done channel configuration')

    if promt_yes_or_no("Do you want to save configuration?", True):
        save_config(dev, config_file)

if __name__ == "__main__":
    main()
