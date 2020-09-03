# Brief:  Class to expose the CN0503 basic functionalites
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

import cn0503.model_cn0503 as _model
from cn0503.RxThreads import MonitorRx
from time import sleep

class CN0503():
    class DISPLAY_MODE():
        RAW_DATA = 'CODE'
        ABSOLUTE_RATIO = 'ARAT'
        RELATIVE_RATIO = 'RRAT'
        INSTRUMENTATION_UNIT_1 = 'INS1'
        INSTRUMENTATION_UNIT_2 = 'INS2'
        ALL = [RAW_DATA, ABSOLUTE_RATIO, RELATIVE_RATIO,\
                INSTRUMENTATION_UNIT_1, INSTRUMENTATION_UNIT_2]

    class MEASURMENT_TYPES():
        FLUORESCENCE = 'FLUO'
        TURBIDITY = 'TURB'
        COLORIMETRY = 'COLO'
        ALL = [FLUORESCENCE, TURBIDITY, COLORIMETRY]

    class CHANNELS():
        CH_1 = 1
        CH_2 = 2
        CH_3 = 3
        CH_4 = 4
        ALL = [CH_1, CH_2, CH_3, CH_4]
        USABLE_FOR_TURBIDITY = [CH_1, CH_4]

    class Dirty_Value():
        def __init__(self, value):
            self._dirty = False
            self._value = value
        @property
        def value(self):
            return self._value
        @value.setter
        def value(self, value):
            self._value = value
            self._dirty = True
        def is_dirty(self):
            return self._dirty

    class Channel():
        def __init__(self, nb):
            self.nb = nb
            self.ratb = CN0503.Dirty_Value(0)
            self.poly1 = CN0503.Dirty_Value('')
            self.poly2 = CN0503.Dirty_Value('')

    def __init__(self):
        self.model = _model.model_CN0503(0)
        self.monitor = MonitorRx()
        self.monitor.update_list(self.model._rx_q, self.model)
        self.opened_channels = [0, 0, 0, 0]
        self.channels = {
            CN0503.CHANNELS.CH_1 : CN0503.Channel(CN0503.CHANNELS.CH_1),
            CN0503.CHANNELS.CH_2 : CN0503.Channel(CN0503.CHANNELS.CH_2),
            CN0503.CHANNELS.CH_3 : CN0503.Channel(CN0503.CHANNELS.CH_3),
            CN0503.CHANNELS.CH_4 : CN0503.Channel(CN0503.CHANNELS.CH_4)
        }

    def list_serial_ports(self):
        return self.model.list_serial_ports()

    def set_serial_port(self, port):
        self.serial_port = port
        self.model.connect_serial(port)

    def set_display_mode(self, value):
        if value in CN0503.DISPLAY_MODE.ALL:
            self.model.send_command_direct("MODE " + value)
        else:
            print("Unrecognized mode")

    def set_instrumentation_unit_polynomial(self, channel_nb, coefs, nb):
        if nb > 2 or nb < 1:
            print("Error: only INS1 and INS2 available")
        if len(coefs) > 6:
            print("Error: Too many coefs (Max 6)")
        coefs_str = " ".join(str(x) for x in coefs)
        if nb == 1:
            self.channels[channel_nb].poly1.value = coefs_str
        else:
            self.channels[channel_nb].poly2.value = coefs_str
        self.model.send_command_direct("DEF%d INS%d %s" % (channel_nb - 1,\
                nb, coefs_str))

    def set_instrumentation_unit_2_polynomial(self, channel_nb, coefs):
        self.set_instrumentation_unit_polynomial(channel_nb, coefs, 2)
    
    def set_instrumentation_unit_1_polynomial(self, channel_nb, coefs):
        self.set_instrumentation_unit_polynomial(channel_nb, coefs, 1)

    def read_data(self, nb_samples):
        self.model.start_streaming(nb_samples)
        self.model.data_list[0].clear()
        while (len(self.model.data_list[0]) < nb_samples):
            pass
        self.model.stop_streaming()
        data = []
        for i in CN0503.CHANNELS.ALL:
            data.append(list(self.model.data_list[i - 1]))

        return data

    #
    # CHANNEL ATTRIBUTES
    #

    def set_measurment_type(self, channel_id, type):
        if channel_id in CN0503.CHANNELS.ALL and\
            type in CN0503.MEASURMENT_TYPES.ALL:
            self.model.send_command_direct('CHANN%d %s' %(channel_id, type))
            sleep(1.0)
        else:
            print('Wrong input')
    
    def set_ratio_baseline(self, channel_id, value):
        if channel_id in CN0503.CHANNELS.ALL:
            self.model.send_command_direct('DEF%d RATB %s' %(channel_id - 1, str(value)))
            self.channels[channel_id].ratb.value = value
        else:
            print('Wrong input')
    
    def get_config(self):
        cfg = []
        for ch in self.channels.values():
            if ch.ratb.is_dirty():
                cfg.append('DEF%d RATB %d' % (ch.nb - 1, ch.ratb.value))
            if ch.poly1.is_dirty():
                cfg.append('DEF%d INS1 %s' % (ch.nb - 1, ch.poly1.value))
            if ch.poly2.is_dirty():
                cfg.append('DEF%d INS2 %s' % (ch.nb - 1, ch.poly2.value))

        return cfg

    def fl_write(self, cmd):
        self.model.send_command_direct('FL_WRITE ' + cmd)

    def fl_program(self):
        self.model.send_command_direct('FL_PROGRAM 0')

    def fl_dump(self):
        self.model.send_command_direct('FL_DUMP')
        ok = True
        old_size = 0
        while ok:
            sleep(1.0)
            new_size = len(self.model.dump_commands)
            if new_size == old_size:
                ok = False
            old_size = new_size

        return self.model.dump_commands
