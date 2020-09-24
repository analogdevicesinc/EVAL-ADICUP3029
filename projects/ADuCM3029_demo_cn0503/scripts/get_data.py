# Brief:  Script to print data from a CN0503 channel
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
import argparse
import numpy as np
import matplotlib.pyplot as plt

DEFAULT_NB_SAMPLES = 3

DEFUALT_SERIAL_PORT = 'COM4'

description_help='''Get samples from a specific channel

Examples:\n
    Get %d(default) samples of fluorimetry data fron channel 1 displyed as
    instrumentation unit 1
    >python get_data.py 1 FLUO INS1
    
    Get 10 samples of data fron channel 4 displyed as relative ratio and
    selecting the communication port
    >python get_data.py 4 COLO RRAT -s 10 -select_port

'''%(DEFAULT_NB_SAMPLES)
measurment_type_help = '''Measurment type: FLUO (fluorimetry), COLO (colorimetry)\
 or TURB (turbidity)'''
display_type_help = '''Display type: ARAT (absolut ration), RRAT (relative ratio),
INS1/2 (instrumentation unit 1/2)'''
select_port_help = '''If select_port option is set, a list of available serial\
ports will be shown.
Then the user can choose a port where the board is connected at.
Otherwise, the hardcoded DEFUALT_SERIAL_PORT value for the port will be used'''

def parse_input():
    parser = argparse.ArgumentParser(description=description_help,\
                            formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('channel_number', choices=CN0503.CHANNELS.ALL,\
                            type=int, help='Channel number')
    parser.add_argument('measurment_type', choices=CN0503.MEASURMENT_TYPES.ALL,\
                            help=measurment_type_help)
    parser.add_argument('display_type', choices=CN0503.DISPLAY_MODE.ALL,\
                            help=display_type_help)
    parser.add_argument('-s', metavar='samples', default=DEFAULT_NB_SAMPLES,\
	    		 type=int, help="Number of samples to read. \
			Default is %d" % (DEFAULT_NB_SAMPLES))
    parser.add_argument('-select_port', help=select_port_help, action='store_true')
    args = parser.parse_args()

    if args.measurment_type == CN0503.MEASURMENT_TYPES.TURBIDITY and \
      not args.channel_number in CN0503.CHANNELS.USABLE_FOR_TURBIDITY:
        print('Error: Only channel 1 and 4 can be used to measure turbidiy')
        exit()

    return (args.channel_number, args.measurment_type, args.display_type,\
            args.s, args.select_port)

def choose_serial_port(dev):
    #Select connected port 
    ports = dev.list_serial_ports()
    print("Available Serial Ports: ", ports)
    return input("Enter Serial Port Number, e.g., COM3: ")

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

def read_samples(dev, channel_nb, nb_samples):

    print("Reading %d samples (first is discarded)" % (nb_samples + 1))
    print("Ignoring first sample")
    data = dev.read_data(nb_samples + 1)[channel_nb - 1][1:]

    print("Readed: ")
    print(data)

    return data

def plot_data(data, label):
        mean = np.mean(data)
        plt.plot(data, label='Data')
        plt.plot([mean] * len(data), label='Mean')
        plt.xlabel(label)
        plt.legend()
        plt.title(label + ' (close figure to continue ...)')
        plt.show()

def main():
    # Get user input to configure channel

    (channel_nb, measurment_type, display_type, nb_samples, select_port) = parse_input()
    print("Measuring %d samples of %s from channel %d displayed as %s" % \
        (nb_samples, measurment_type, channel_nb, display_type))

    dev = CN0503()

    #Configure comuniction port
    if select_port:
        port = choose_serial_port(dev)
    else:
        port = DEFUALT_SERIAL_PORT
    dev.set_serial_port(port)

    dev.set_measurment_type(channel_nb, measurment_type)
    dev.set_display_mode(display_type)

    data = read_samples(dev, channel_nb, nb_samples)
    print("Mean: %f" % np.mean(data))

    if promt_yes_or_no("Do you want current data to be plotted?", True):
        plot_data(data, '%s on channel %d' % (measurment_type, channel_nb))
    print('Done channel configuration')

if __name__ == "__main__":
    main()