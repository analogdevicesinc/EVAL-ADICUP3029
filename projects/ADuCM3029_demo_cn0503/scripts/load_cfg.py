# Brief:  Script to load a configuration file to the CN0503
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

DEFUALT_SERIAL_PORT = 'COM4'

description_help='''Load config file on CN0503
Examples:\n
    >python load_cfg.py -h
    >python load_cfg.py config_file
    >python load_cfg.py config_file -select_port
'''
config_file_help = '''File with commands to configure the chip.
Commands will be appended as they are to the FL_WRITE CLI command'''
select_port_help = '''If select_port option is set, a list of available serial\
ports will be shown.
Then the user can choose a port where the board is connected at.
Otherwise, the hardcoded DEFUALT_SERIAL_PORT value for the port will be used'''

def parse_input():
    parser = argparse.ArgumentParser(description=description_help,\
                            formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('config_file', help=config_file_help)
    parser.add_argument('-select_port', help=select_port_help, action='store_true')
    args = parser.parse_args()

    return (args.config_file, args.select_port)

def choose_serial_port(dev):
    #Select connected port 
    ports = dev.list_serial_ports()
    print("Available Serial Ports: ", ports)
    return input("Enter Serial Port Number, e.g., COM3: ")

def main():
    # Get user input to configure channel
    (config_file, select_port) = parse_input()

    dev = CN0503()

    #Configure comuniction port
    if select_port:
        port = choose_serial_port(dev)
    else:
        port = DEFUALT_SERIAL_PORT
    dev.set_serial_port(port)

    with open(config_file) as f:
        line = f.readline()
        while line:
            cmd_line = line.split('#', 1)[0].strip()
            if cmd_line.upper().find('ARAT') != -1:
               cmd_line = line.strip()

            if cmd_line:
                if cmd_line[0].isnumeric():
                    cmd_line = 'REG ' + cmd_line
                dev.fl_write(cmd_line)
            
            line = f.readline()
    
    #Write all previous commands to flash memory
    dev.fl_program()

if __name__ == "__main__":
    main()