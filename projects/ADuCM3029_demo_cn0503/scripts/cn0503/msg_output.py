"""
Copyright (c) 2020 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
"""

class msg_out():
    def __init__(self, console_level=2):
        self.console_level = console_level

    def write(self, msg, level = 1):
        print(msg)
        return
        
    def err(self, msg, level = 1, force_print=False):
        print(msg)
        return

