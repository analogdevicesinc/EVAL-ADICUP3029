# File: ad7799_py.py
# Description: AD7799
# Author: Antoniu Miclaus (antoniu.miclaus@analog.com)
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

sys.path.append('C:/amiclaus/pyadi-iio')  # add own path to python bindings
import adi



try:

    import tkinter as tk
    from tkinter import filedialog
    import tkinter.scrolledtext as tkscrolled

except:
    print("Please install tkinter")
try:
    import csv
except:
    print("Please install csv")
try:
    import pandas as pd
except:
    print("Please install pandas")
try:
    import numpy as np
except:
    print("Please install numpy")

try:
    import matplotlib.pyplot as plt
    from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
except:
    print("Please install matplotlib")

#Transciever settings  
def setup_device(dev):

    global gain, voltage0, voltage1, voltage2

    gain_val = dev.gain
    voltage0_val = dev.channel[0].value
    voltage1_val = dev.channel[1].value
    voltage2_val = dev.channel[2].value

    voltage0.set(voltage0_val)
    voltage1.set(voltage1_val)
    voltage2.set(voltage2_val)
    gain.set(gain_val)

    
#Start Button
def read_dev():
    global ad7799
    #setup device
    setup_device(ad7799)



#Create GUI
def gui():
    
    global dev, gain, voltage0, voltage1, voltage2

    root = tk.Tk()
    root.iconbitmap("favicon.ico")
    root.title("AD7799 (Analog Devices, Inc.)")

    gain = tk.StringVar()
    voltage0 = tk.StringVar()
    voltage1 = tk.StringVar()
    voltage2 = tk.StringVar()
    btn_text = tk.StringVar()

    btn_text.set("Read")

    fr1 = tk.Frame(root)
    fr1.pack(side = tk.LEFT, anchor = 'n', padx = 10, pady = 10)

    fr2 = tk.Frame(fr1)
    fr2.grid(row = 0, column = 0, pady = 10)

    label1 = tk.Label(fr2, text = "Gain ")
    label1.grid(row = 0, column = 0)

    entry1 = tk.Entry(fr2, textvariable=gain)
    entry1.grid(row = 0, column = 1)

    label2 = tk.Label(fr2, text = "Voltage0")
    label2.grid(row = 1, column = 0, pady = (0,5))

    entry2 = tk.Entry(fr2, textvariable=voltage0)
    entry2.grid(row = 1, column = 1)
    
    label3 = tk.Label(fr2, text = "Voltage1")
    label3.grid(row = 2, column = 0)

    check1 = tk.Entry(fr2, textvariable=voltage1)
    check1.grid(row = 2, column = 1)
    
    label4 = tk.Label(fr2, text = "Voltage2")
    label4.grid(row = 3, column = 0)

    entry3 = tk.Entry(fr2, textvariable=voltage2)
    entry3.grid(row = 3, column = 1)
    
    btn_sweep = tk.Button(fr2, text="Read", command=read_dev)
    btn_sweep.config(width = 13, height = 1,  bg = "orange")
    btn_sweep.grid(row = 8, column = 0, pady = (10,0))

    root.update_idletasks()
    
    root.mainloop()

#main function
def main():

    global ad7799

    ad7799 = adi.ad7799(uri="serial:COM23,115200")

    gui()

    del dev

main()