# File: CN0548_simple_plot.py
# Description: CN0548 data logging and real-time plot
# Author: Harvey De Chavez (harveyjohn.dechavez@analog.com)
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

import sys
import time
from datetime import datetime
from typing import List
import matplotlib.animation as animation
import matplotlib.pyplot as plt
import numpy as np
import adi

global ad7799
while True:
    port = input('Input Serial line (e.g. if ADICUP3029 is connected to COM7, input \'COM7\' )\nSerial line:')

    try:
        context = "serial:" + port + ",115200"
        ad7799 = adi.ad7799(uri=context)
        print("\nCN0548 board detected.\nBegin setup query:\n")
        break
    except:
        print("Port not found\n")

while True:
    mode = input('CN0548 setting: (a/b) \n(a) Low Voltage High Current (absolute max: 15V, 9A)\n(b) High Voltage Low Current (absolute max: 80V, 1.5A)\nMode: ')
    if mode == 'a' or mode == 'b':
        break
    else:
        print("'a' or 'b' only")

if mode == 'a':
    v_gain = 0.004
else:
    v_gain = 0.02
i_gain = 0.005

global log
while True:
    log = input('\nLog data? (Y/N): ')
    if log == 'Y' or log == 'N' or log == 'y' or log == 'n':
        break
    else:
        print("'Y' or 'N' only")

if log == 'Y' or log == 'y':
    global filename
    dt_now = datetime.now()
    dt_format = dt_now.strftime("%d-%m-%Y_%H-%M")
    if mode == 'a':
        filename = 'LVHC_'+dt_format+'.csv'
    else:
        filename = 'HVLC_'+dt_format+'.csv'

while True:
    en = input('\nEnable plot? (Y/N): ')
    if en == 'Y' or en == 'N' or en == 'y' or en == 'n':
        break
    else:
        print("'Y' or 'N' only")

if en == 'Y' or en == 'y':
    enable_plot = True
else:
    enable_plot = False

global fs
while True:
    fs = input('\nSamples per second? (1-5): ')
    try:
        fs = int(fs)
    except:
        print('Please input an integer within the specified range')
        continue
    if fs < 1 or fs > 5:
        print('Please input an integer within the specified range')
    else:
        break

if enable_plot:
    global samples
    while True: 
        samples = input('\nInput number of samples to retain within the plot (>=5).\n***Note that this can affect the sampling rate of the device\nSamples to retain: ')
        try:
            samples = int(samples)
        except:
            print('Please input an integer within the specified range')
            continue
        if samples < 5:
            print('Please input an integer within the specified range')
        else:
            break

    # Create figure for plotting
    fig, ax1 = plt.subplots()
    ax2 = ax1.twinx()
    xs: List[float] = []
    ys: List[float] = []
    for i in range(samples):
        xs.append(0)
        ys.append(0)

    def animate(i,volt, curr):      # function for animation
    # Continuously adding streams of x and y to lists
        v_reading = round(v_gain*ad7799.channel[2].value,3) # get new voltage reading (mV)
        i_reading = round(i_gain*ad7799.channel[0].value,3) # get new current reading
        flag = len(volt)
        volt.append(v_reading)  
        curr.append(i_reading)  
        print('Voltage reading: ' + str(v_reading) + '\t\t\t' + 'Current reading: ' + str(i_reading))
        if log == 'Y' or log == 'y':
            output = open(filename,'a')
            output.write('Voltage reading:,' + str(v_reading) + ',' + 'Current reading:,' + str(i_reading)+'\n')
            output.close()  

    # number of recent samples to be retained
        volt = volt[-samples:]
        curr = curr[-samples:]
    #initialize/clear
        lower = 0
        upper = 0
    # Plot config
        ax1.clear()
        minimum = min(volt)
        if minimum >= 0:
            lower = int(minimum/5)
        else:
            lower = int(minimum/5) - 1
        maximum = max(volt)
        if maximum >= 0:
            upper = int(maximum/5)
        else:
            upper = int(maximum/5) - 1
        ax1.set_ylim([lower*5,(upper+1)*5])
        color = 'tab:orange'
        ax1.plot(volt, label="Voltage Reading", linewidth=2.0,color=color)
        ax1.set_ylabel("Volts (V)",color=color)
        ax1.tick_params(axis="x", which="both", bottom=True, top=False, labelbottom=False)
        ax1.legend(bbox_to_anchor=[0.5, -0.1], ncol=2, loc="upper left", frameon=False)
        
        ax2.clear()
        minimum = min(curr)
        if minimum >= 0:
            lower = int(minimum/5)
        else:
            lower = int(minimum/5) - 1
        maximum = max(curr)
        if maximum >= 0:
            upper = int(maximum/5)
        else:
            upper = int(maximum/5) - 1
        ax2.set_ylim([lower*0.5,(upper+1)*0.5])
        color = 'tab:blue'
        ax2.plot(curr, label="Current Reading", linewidth=2.0,color=color)
        ax2.set_ylabel("Amperes (A)",color=color)
        ax2.tick_params(axis="x", which="both", bottom=True, top=False, labelbottom=True)
        ax2.legend(bbox_to_anchor=[0.5, -0.1], ncol=2, loc="upper right", frameon=False)
        
        plt.title("Voltage and Current Sensing of CN0548", fontweight="bold")
        plt.subplots_adjust(bottom=0.30)

        
    # begin animation
    ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=(1000/fs))
    plt.show()
    
else:
    while True:
        v_reading = round(v_gain*ad7799.channel[2].value,3) # get new voltage reading
        i_reading = round(i_gain*ad7799.channel[0].value,3) # get new current reading  
        print('Voltage reading: ' + str(v_reading) + '\t\t\t' + 'Current reading: ' + str(i_reading) )
        time.sleep(1/fs)
        if log == 'Y' or log == 'y':
            output = open(filename,'a')
            output.write('Voltage reading:,' + str(v_reading) + ',' + 'Current reading:,' + str(i_reading)+'\n')
            output.close()


del ad7799
