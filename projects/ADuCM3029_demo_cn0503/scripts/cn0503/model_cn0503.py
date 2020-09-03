"""CN0503 Model and Methods Module.

This module contains CN0503 classes and methods to interact with
the CN0503 via Python.

Copyright (c) 2020 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
"""
from PyQt5 import QtCore
from PyQt5.QtCore import QObject, QThreadPool, pyqtSlot, pyqtSignal
from multiprocessing import queues
import time, csv
import queue
import numpy
from collections import deque

from cn0503.uart_server import uart_server
import serial
from cn0503.RxThreads import MonitorRx, ThreadWorker, MonitorConnection
from cn0503.msg_output import msg_out


class DataUpdatedSignal(QObject):
    """Helper class to signal to a GUI if the data or model has been updated."""
    # New data in the measurement data container (e.g., to update graph or log)
    DataUpdated = QtCore.pyqtSignal()
    # Changes to the model instance variables (e.g., to update a ui)
    ModelUpdated = QtCore.pyqtSignal()


class Channel:
    """Model of a single channel or optical path in CN0503

    Stores channel-specific configuration info.
    number - channel number argument (1, 2, 3, 4 for standard CN0503)
    """

    meas_types = ('Absorption', 'Fluorescence', 'Turbidity')

    _default_names = ('Quinine', 'pH', 'Nitrate', 'Turbidity')
    _default_ratios = ('A1#2048-A2#2048-/', 'B1#2048-B2#2048-/', 'C1#2048-C2#2048-/', 'D2#2048-D1#2048-/')
    _default_wavelength = ('365.0', '430.0', '615.0', '530.0')
    _default_baselines = (1.0, 1.003, 1.329, 0.425)
    _default_meas_type_ind = (1, 0, 0, 2)

    def __init__(self, channel_number):
        """Initialize the channel"""
        self.number = channel_number
        self.name = self._default_names[self.number - 1]

        # Calibration settings defined in firmware
        # Ratio expressions up to 128 characters, RPN notation with # character to denote constant
        self.ratio_expression = self._default_ratios[self.number - 1]
        self.baseline_ratio = self._default_baselines[self.number - 1]
        # 5th order max: a0, a1, a2, a3, a4, a5
        self.ins1_polynomial = [0, 1, 0, 0, 0, 0]
        self.ins2_polynomial = [0, 1, 0, 0, 0, 0]
        # Firmware has channel-specific LPF
        self.lpf_cutoff = 0.5

        # Calibration pieces that are outside of firmware
        # cal_points stored as list of [[RRAT_value, INS1_value, INS2_value]]
        # and used in polynomial calculations
        self.cal_points = {}
        # Units up to 32 characters
        self.ins1_unit = ""
        self.ins2_unit = ""
        self.excitation_wavelength = self._default_wavelength[self.number - 1]
        self.emission_wavelength = 0.0        # fluorescence only
        self.calibration_changes = []

        # Channel measurement type
        # 0:Absorption/Colorimetry, 1:Fluorescence, 2:Turbidity
        self.meas_type_index = self._default_meas_type_ind[self.number - 1]
        # Typically relative ratio mode (RRAT) is (1 - absolute_ratio/baseline_ratio),
        # which is computed in firmware by setting MODE to RRAT.
        # For Fluorescence and Turbidity measurements, the "1 -" should be removed
        # in that equation because a baseline ratio measurement could read 0.
        # subtract_enabled (defn SUBE) determines whether or not to include the "1 -"
        # Then RATB can be set to 1 and the polynomials can be done for scaling
        if self.meas_type_index == 0:
            self.subtract_enabled = 1
        else:
            self.subtract_enabled = 0
        # Note: ARAT equation differs for Turbidity (see above). Not handled in init.

        '''By default, we can only output codes. Additional modes require more info/calibration:
            ARAT: requires ratio_expression
            RRAT: requires baseline_ratio (blank) in addition to above
            INS1: requires ins1_polynomial in addition to above
            INS2: requires ins2_polynomial in addition to above'''
        self.available_modes = model_CN0503.modes  # todo: start with [CN0503.modes[0], CN0503.modes[1]] and add more after cal

    @property
    def lpf_cutoff(self):
        return self.__lpf_cutoff

    @lpf_cutoff.setter
    def lpf_cutoff(self, lpf):
        """Setter for lpf so we can validate cutoff"""
        if lpf < 0.01 or lpf > 5:
            return

        self.__lpf_cutoff = lpf


class model_CN0503():
    """Model of the CN0503 system

    using_event_loop - optional argument to indicate whether the
    top-level is a pyqt app or a python script. If it is the latter,
    override with using_event_loop=0 and include RxThreads.py in the
    top-level for monitoring threads.

    Communications are handled in uart_server, which is an instance
    of uart_server.py that processes the _tx_q and services the _rx_q
    RxThreads.py has a monitor for received data to process in _rx_q
    and a lost connection monitor for the serial port

    When text is received from the RxMonitor(), the _dispatcher
    parses the response and either updates the model or updates
    the data and sends the appropriate signal
    (signaler.ModelUpdated or signaler.DataUpdated)

    Data containers:
        start_time: holds the time.time() when streaming was started
        time_list: deque of time.time() when each data sample was
                    received by the parser (time not given by CN0503)
        data_list: list of 16 deques of channel data since streaming
                    started. This data is cleared when mode changes
                    or when a new data streaming session starts.
                    Get channel data from list with data_list[channel]
                    where channel is 0 to 15 (and all modes except
                    CODE mode only use 0 to 3 corresponding to each
                    optical path)
        max_data_points: how many data to allow in the list (int)

    Attributes:
        signaler: instance of Signaler() class to emit signals
        uart_server: instance of uart_server class for device comms
        channels[]: list of Channel() objects in cn0503
        num_ratios: number of channels/ratios. Typically 4.
        code_mode_columns: number of data columns in CODE mode
        current_data_width: how many data should be populating the
                            list. Typically 8 in CODE mode or 4 in
                            other modes.
        odr: output data rate value (0.01Hz to 5Hz)
        mode_index: current mode index in modes tuple
        state_index: current state index in states tuple

    Methods:
        connect_serial(port_number): connect to device
        disconnect_serial(): disconnect from device
        serial_connected(): return whether device is connected
        list_serial_ports(): return list of available serial ports
        clear_graph_data(): clear data from time_list and data_list
        get_config(optional apply_default): read device config and
                                             update model
        load_config(file_name): load config from file
        start_streaming(optional samples): start streaming data
        stop_streaming(): stop streaming
        send_command(cmd, args): verify and send a command
                                  valid commands in uart_commands
        send_def_command(cmd, chan, args): verify and send DEF command
                                            valid commands in
                                            def_commands tuple
        send_flash_command(cmd, optional args): verify and send a flash
                                                 command. Valid commands
                                                 in flash_commands
        send_command_direct(msg): Send command without verification

    """

    states = ('Idle', 'Streaming')
    uart_commands = ('REG', 'MODE', 'STREAM', 'IDLE', 'DEF', 'ALRM',
                     'BOOT', 'ODR', 'NUMRAT', "PCB-LED")
    def_commands = ('ARAT', 'RFLT', 'ALRM', 'RATB', 'INS1', 'INS2', 'SUBE')
    flash_commands = ('FL_CLEARBUF', 'FL_LOAD', 'FL_PROGRAM', 'FL_ERASE',
                      'FL_APPLY', 'FL_WRITE', 'FL_READ')
    modes = ('CODE', 'ARAT', 'RRAT', 'INS1', 'INS2')

    dump_commands = []

    def __init__(self, using_event_loop=1):
        """Initialize CN0503 Attributes"""
        self.signaler = DataUpdatedSignal()

        # Init uart, communications, and logging
        self.uart_server = None
        self._vrb = msg_out()
        self._tx_q = queue.Queue(300)
        self._rx_q = queue.Queue(800)
        #self.dispatch_thread = None

        # Init data container
        self.start_time = 0.0 # time.time() of start_streaming (seconds since epoch)
        self.time_list = deque()
        self._max_columns = 16
        self.data_list = [deque() for _ in range(self._max_columns)]   # 16 column list of measured data
        # Append data to list with data_list[channel].append(channel_data)
        # Get channel data from list with data_list[channel] where channel is 0 to 16
        # Only CODE mode may use full 16 columns (typically 8, but config dependent)
        # Other modes are typically 4 columns, corresponding to the 4 optical paths
        self.max_data_points = 200

        # Init CN0503 model parameters
        # There is a separate class to handle channel-specific items
        self.channels = []
        self.num_ratios = 4
        self.code_mode_columns = 8
        self.current_data_width = 4
        for i in range(1, self.num_ratios + 1):
            self.channels.append(Channel(i))

        self.odr = 1.0
        self.mode_index = 0
        self.log_data = False
        self.log_file = ""
        self.register_changes = []
        self.config_changes = {}
        self.state_index = 0        # 0:Idle, 1:Streaming

        self.thread_pool = QThreadPool()

        # Setup monitors for new data and connection status
        # If not using event loop, these monitors will need to be local
        self.using_event_loop = using_event_loop
        if self.using_event_loop:
            self.monitor = MonitorRx()
            self.monitor.UpdateText.connect(self._dispatcher)
            self.monitor.update_list(self._rx_q)

            self.connection_monitor = MonitorConnection()

    @property
    def mode_index(self):
        """int: Index for modes tuple indicating current device mode"""
        return self.__mode_index

    @mode_index.setter
    def mode_index(self, index):
        if index < 0 or index > (len(self.modes) - 1):
            return

        # Clear data_list and time_list because they're not relevant to new mode
        self.time_list = deque()
        self.data_list = [deque() for _ in range(self._max_columns)]
        # Guess current_data_width based on mode
        if index == 0:
            self.current_data_width = self.code_mode_columns
        else:
            self.current_data_width = self.num_ratios
        self.__mode_index = index

    @property
    def odr(self):
        """Reflects current output data rate (0.01 to 5.0 Hz)"""
        return self.__odr

    @odr.setter
    def odr(self, rate):
        if rate < 0.01 or rate > 5.0:
            return
        self.__odr = rate

    def connect_serial(self, port_number):
        """Connect to COM Port given port_number as text.

        Args:
            port_number: (str) full text of port address. E.g, COM5
        """
        self._vrb.write("COM port is %s" % port_number)

        try:
            if self.uart_server is None:
                self.uart_server = uart_server(self._rx_q, self._tx_q, self._vrb)
                if self.using_event_loop:
                    self.connection_monitor.start_monitoring(self.uart_server.lost_connection)
            self.uart_server.connect(port_number, 115200)
            self._vrb.write("Connected")

        except (serial.serialutil.SerialException, FileNotFoundError) as e:
            self._vrb.err("Error opening the serial device!: {}".format(e))
            return 0

        time.sleep(0.05)
        #self._vrb.write("Reading config from device")
        #self.get_config()

        return 1

    def disconnect_serial(self):
        """Disconnect from serial port."""
        if self.uart_server is None:
            self._vrb.err("Nothing connected")
            self.signaler.ModelUpdated.Emit()
            return

        if self.uart_server.is_connected():
            self.uart_server.quit()
            self._vrb.write("COM Port Disconnected")
        else:
            self._vrb.write("No COM Port to Disconnect")
            self.signaler.ModelUpdated.Emit()

    def serial_connected(self):
        """Check if COM Port is connected.

        Returns:
            bool: True if connected, False otherwise.
        """
        if self.uart_server is None:
            self.uart_server = uart_server(self._rx_q, self._tx_q, self._vrb)
            if self.using_event_loop:
                self.connection_monitor.start_monitoring(self.uart_server.lost_connection)

        return self.uart_server.is_connected()

    def list_serial_ports(self):
        """Return List of Available Serial Ports.

        Returns:
            list: Available serial ports.
        """
        if self.uart_server is None:
            self.uart_server = uart_server(self._rx_q, self._tx_q, self._vrb)
            if self.using_event_loop:
                self.connection_monitor.start_monitoring(self.uart_server.lost_connection)

        return self.uart_server.scan()

    def clear_graph_data(self):
        """Clear all measurement data points currently stored in the model."""
        self.time_list = deque()
        self.data_list = [deque() for _ in range(self._max_columns)]
        self.signaler.DataUpdated.emit()

    def get_config(self, apply_default=0):
        """Read current configuration from device and update model.

        Args:
            apply_default (int, optional): Option to apply default values
                for DEFn ARAT and DEFn SUBE. Defaults to 0. 1 only used
                if there may be some other variation on device currently.
        """
        self.stop_streaming()
        time.sleep(0.1)
        self.stop_streaming()
        cntrlCmd = ["IDLE?", "ODR?", "MODE?", "NUMRAT?"]
        defCmd = ["ARAT", "RFLT", "RATB", "INS1", "INS2", "SUBE"]
        for i in range(0, 4):
            msg = cntrlCmd[i]
            self._send_packet(msg)
            time.sleep(0.02)
        for ratio_index in range(0, self.num_ratios):
            if apply_default:
                # Use Model default ratio expression
                msg = "def" + str(ratio_index) + " ARAT " + self.channels[ratio_index].ratio_expression
                self._send_packet(msg)
                time.sleep(0.1)
                # Use Model default SUBE
                msg = "def" + str(ratio_index) + " SUBE " + str(self.channels[ratio_index].subtract_enabled)
                self._send_packet(msg)
                time.sleep(0.1)
            for i in range(0, 6):
                # msg = "def?:" + str(ratio_index) + " " + defCmd[i]
                msg = "def" + str(ratio_index) + "? " + defCmd[i]
                self._send_packet(msg)
                time.sleep(0.02)
        self._send_packet("RATMASK F")  # F means data output for all paths
        self.signaler.ModelUpdated.emit()

    def load_config(self, file_name):
        """Load a configuration file into the flash software buffer.

        The file can include direct register values as hex address + space +
        hex value, e.g. 10B 03FC or application configuration commands
        DEF, MODE, ODR, and RATMASK. Also handles REG command.

        Convention is that .dcfg file is device config for ADPD4101 registers,
        .lcfg is application values and .cfg is combined registers and
        application values.

        After running load_config, the config is loaded into the SW Flash
        buffer. It is the user's choice whether to use FL_APPLY to apply the
        configuration, FL_PROGRAM 0 to program user flash page, or both.

        Args:
            file_name (str): full file-path and file-name of config file
                as would be returned from a file_dialog. E.g.,
                C:/tmp/CN0503_GUI/app_cn0503/CFG/Colorimeter_ADPD4100_v1p3.cfg
        """
        self.stop_streaming()
        time.sleep(0.1)
        self.send_command_direct("FL_CLEARBUF")
        time.sleep(0.05)
        with open(file_name) as f:
            print("File %s open" % file_name)
            idx = 0
            for line in f:
                # print(line)
                if not line.strip():
                    continue
                if (line[0] == '#' or line[0] == '/'):
                    continue

                stripline = line.rstrip('\n')

                # First take care of application commands (Register map does not go up to 0xDEF)
                if any(substr in stripline.upper() for substr in ("REG", "DEF", "ODR", "MODE", "RATMASK")):
                    if "ARAT" not in stripline:
                        msg = stripline.split("#")[0].strip()
                    else:
                        msg = stripline
                    if "FL_" not in msg:
                        msg = "FL_WRITE " + msg
                    self._vrb.write(msg)
                    self._send_packet(msg)
                # Take care of register writes
                else:
                    splitline = stripline.split(None, 2)
                    try:
                        int(splitline[0], 16)  # Check if first part is hex (reg write)
                        try:
                            # address = int(splitline[0], 16)
                            # value = int(splitline[1], 16)
                            address = splitline[0]
                            value = splitline[1]
                            if (len(splitline) > 2):
                                comment = splitline[2]
                            else:
                                comment = ''
                            # print ("Set [0x%04x] = 0x%04x    Comm: " % (address, value) + comment)
                            msg = "FL_WRITE REG " + address + " " + value
                            self._vrb.write(msg)
                            self._send_packet(msg)
                            time.sleep(0.05)
                        except ValueError:
                            print("    ") + line
                        idx += 1
                    except ValueError:
                        # Not hex or supported command
                        self._vrb.err("Error: Unsupported command " + stripline)
                time.sleep(0.05)
        self.code_mode_columns = 8  # Avoid potential overflow if previous config had more columsn

    def save_config(self, filename):
        """Save the current configuration"""
        #  todo: determine what to save

    def start_streaming(self, samples=0):
        """Begin streaming data.

        Args:
            samples (int, optional): number of samples to stream before
                stopping. Defaults to 0, which means the device streams data
                continuously until a stop_streaming() command is received.
        """
        if samples >= 0:
            self._send_packet("STREAM " + str(samples))

    def stop_streaming(self, idle=1):
        """Stop streaming data.

        Args:
            idle (int, optional): 1 to put the device into idle mode, 0 to
                stay in measurement mode, but just stop the printout of data.
                idle 0 is useful to avoid settling of LEDs when streaming is
                re-enabled. Some configuration actions are disabled when idle
                is 0 (even if printout is suppressed).
        """
        if idle != 1:
            idle = 0
        self._send_packet("IDLE " + str(idle), 1)  # send characters slowly to ensure they are received
        time.sleep(0.05)
        self._send_packet("IDLE " + str(idle), 1)
        time.sleep(0.05)

    def _dispatcher(self, packet):
        """Parse received UART strings and update model values."""
        packet = packet.strip()
        if not packet:  # Empty command
            return
        if packet == ">":
            return
        print("<Received packet>" + packet + "<end packet>")
        if "Unknown" in packet:  # Unknown command. Already printed.
            return
        if "ERROR" in packet:  # Invalid arguments error. Already printed.
            return
        if "STREAM" in packet:  # STREAM echo
            self.state_index = 1
            # clear previous data
            self.clear_graph_data()
            self.signaler.ModelUpdated.emit()
            return

        return_string = packet.split(" ", 1)

        if len(return_string) == 1:
            self._vrb.err("Error: Invalid packet with no spaces")
            return

        # If CODE mode lags and delivers a partial line starting with a number,
        # exit early to avoid going through all of the checks and causing extra
        # delay.
        try:
            int(return_string[0], 16)
        except ValueError:
            pass
        else:
            print("Invalid command " + return_string[0])
            return
        try:
            float(return_string[0])
        except ValueError:
            pass
        else:
            print("Invalid command " + return_string[0])
            return

        if "DATI" in return_string[0]:
            # Hex int data stream in CODE mode

            all_channels_data = return_string[1].split(" ")
            # fixed_length_list = all_channels_data[:16] + [0 for _ in range(16 - len(all_channels_data))]
            column = 0
            if len(self.time_list) == 0:
                self.start_time = time.time()
                self.current_data_width = len(all_channels_data)
                if self.current_data_width > self._max_columns:
                    self._vrb.err("Error: More than " + str(self._max_columns) + " columns not supported")
                    self.current_data_width = self._max_columns
                self.code_mode_columns = self.current_data_width
                self.signaler.ModelUpdated.emit()
            if len(all_channels_data) != self.current_data_width:
                print("Wrong amount of data items. Sample skipped.")
                return
            self.time_list.append(time.time() - self.start_time)
            for data in all_channels_data:
                try:
                    self.data_list[column].append(int(data, 16))
                    column += 1
                except Exception as exc:
                    print("Exception while parsing DATI: ", exc)
                    return

            if len(self.time_list) >= self.max_data_points:
                self.time_list.popleft()
                for i in range(len(self.data_list)):
                    try:
                        self.data_list[i].popleft()
                    except IndexError:
                        pass  # Data wasn't added to all columns

            self.signaler.DataUpdated.emit()
            return

        if "DATF" in return_string[0]:
            # Float data stream in ARAT, RRAT, INS1, and INS2 modes

            all_channels_data = return_string[1].split(" ")
            column = 0
            if len(self.time_list) == 0:
                self.start_time = time.time()
                self.current_data_width = len(all_channels_data)
                self.signaler.ModelUpdated.emit()
            if len(all_channels_data) != self.current_data_width:
                print("Wrong amount of data items. Sample skipped.")
                return
            self.time_list.append(time.time() - self.start_time)
            for data in all_channels_data:
                try:
                    self.data_list[column].append(float(data))
                    column += 1
                except Exception as exc:
                    print("Exception while parsing DATF: ", exc)
                    return
            if len(self.time_list) >= self.max_data_points:
                self.time_list.popleft()
                for i in range(len(self.data_list)):
                    try:
                        self.data_list[i].popleft()
                    except IndexError:
                        pass

            self.signaler.DataUpdated.emit()
            return

        # Remaining responses start with "RESP: " and will
        # have either space or equals sign after command, e.g.,
        # RESP: ODR XX.XX
        # RESP: ODR?=XX.XX
        if "RESP" not in return_string[0]:  # e.g., help command, invalid command, error
            return
        try:
            return_string_1 = return_string[1].replace('=', ' ')
            return_string_1 = return_string_1.split(" ", 1)  # return_string_1[1] is next after RESP:
        except:
            self._vrb.err("Error Return: " + packet[:-2])
            return

        # Flash commands. FL_APPLY means there's a new config.
        # FL_WRITE and FL_READ are echoed
        # FL_CLEARBUF, FL_LOAD, FL_PROGRAM, FL_ERASE don't need to be parsed further
        # FL_DUMP LOADS the config_changes dict and register_changes list, to be printed to a file
        if "FL_APPLY" in return_string_1[0]:  # Need to get new config after apply
            self._vrb.write("Update model with newly applied config.")
            self.get_config()
            return
        elif "FL_READ" in return_string_1[0]:
            self._vrb.write("Value from flash software buffer: " + return_string_1[1])
            return
        elif "FL_WRITE" in return_string_1[0]:
            self._vrb.write(return_string_1[1] + " was written to flash software buffer.")
            return
        elif "FL_DUMP" in return_string_1[0]:
            self.dump_commands.append(return_string_1[1])
            split_str = return_string_1[1].split(" ")
            if any("DEF" in mystr.upper() for mystr in split_str):
                self.config_changes[" ".join(split_str[:2])] = " ".join(split_str[2:])
            elif any(substr in return_string_1[1].upper() for substr in ("ODR", "MODE", "RATMASK")):
                self.config_changes[" ".join(split_str[:1])] = " ".join(split_str[1:])
            elif any("REG" in mystr.upper() for mystr in split_str):
                try:
                    # Check for register 0xf bit 15 set to 1, which is a sw reset and resets registers. If so, clear list.
                    if int(split_str[1], 16) == 0xf and int(split_str[2], 16) & 0x8000 == 0x8000:
                        self._vrb.write("SW Reset encountered. Register changes list cleared.")
                        self.register_changes.clear()
                    self.register_changes.append(" ".join(split_str[1:]))
                except (ValueError, IndexError) as exc:
                    self._vrb.err("In FL_DUMP: " + return_string_1[1] + "Error msg: " + exc)
                    return
            else:
                try:
                    # Check for register 0xf bit 15 set to 1, which is a sw reset and resets registers. If so, clear list.
                    if int(split_str[0], 16) == 0xf and int(split_str[1], 16) & 0x8000 == 0x8000:
                        self._vrb.write("SW Reset encountered. Register changes list cleared.")
                        self.register_changes.clear()
                    self.register_changes.append(return_string_1[1])
                except (ValueError, IndexError) as exc:
                    self._vrb.err("Unrecognized command or error in FL_DUMP: " + return_string_1[1] + "Error msg: " + exc)
                    return
            return
        elif "FL_" in return_string_1[0]:
            return

        # PCB-LEDn command
        if "PCB-LED" in return_string_1[0]:
            try:
                # chan = int(return_string_1[0][len("PCB-LED"):])
                self._vrb.write("LED current registers adjusted to set ADC readback for " + return_string_1[0] + " to "
                                + return_string_1[1] + " saturation.")
                # self._vrb.write("Check registers " + ('', '105 and 106', '125 and 126', '145 and 146','165 and 166')[chan] + " and edit into config file if desired")
                return
            except (ValueError, IndexError):
                return

        # CHANNn command
        if "CHANN" in return_string_1[0]:
            self._vrb.write(return_string_1[0] + " set to " + return_string_1[1]
                            + ". ARAT, SUBE, and LED current registers adjusted. Check jumpers.")
            return

        if "REG" in return_string_1[0]:
            temp_string = return_string_1[1].split(" ")
            temp0 = temp_string[0].zfill(3)
            temp1 = temp_string[1].zfill(4)
            self._vrb.write("Reg 0x" + temp0 + "=0x" + temp1 + "\r\n")
            if "?" not in return_string_1[0]:  # If command, save to reg_changes
                self.register_changes.append(temp0 + " " + temp1)
            return

        elif "MODE" in return_string_1[0]:
            try:
                mode_index = self.modes.index(return_string_1[1])
                if self.mode_index != mode_index:
                    self.mode_index = mode_index
                    self.signaler.ModelUpdated.emit()
                elif mode_index == 0:
                    if self.current_data_width != self.code_mode_columns:
                        self.mode_index = mode_index  # Updates current_data_width
                        self.signaler.ModelUpdated.emit()
                else:
                    if self.current_data_width != self.num_ratios:
                        self.mode_index = mode_index  # Updates current_data_width
                        self.signaler.ModelUpdated.emit()
            except ValueError:
                self._vrb.err("Error: Unrecognized Mode " + return_string_1[1])
            self._vrb.write(return_string_1[1])
            return

        elif "ODR" in return_string_1[0]:
            try:
                if self.odr != float(return_string_1[1]):
                    self.odr = float(return_string_1[1])
                    self.signaler.ModelUpdated.emit()
                self._vrb.write(return_string_1[1])
            except ValueError:
                self._vrb.err("Error: Non-numeric ODR value " + return_string_1[1])
            return

        elif "NUMRAT" in return_string_1[0]:  # Note: NUMRAT not implemented in firmware
            self._vrb.write(return_string_1[1])
            try:
                if int(return_string_1[1]) != self.num_ratios:
                    self._vrb.err("Error: Number of ratios should be 4, but it is " + return_string_1[1])
            except ValueError:
                self._vrb.err("Could not convert %s to int" % return_string_1[1])
            return

        elif "IDLE" in return_string_1[0]:
            if return_string_1[1] == "1":
                if self.state_index != 0:  # idle
                    self.state_index = 0
                    self.signaler.ModelUpdated.emit()
            elif return_string_1[1] == "0":
                if self.state_index != 1:  # streaming
                    self.state_index = 1
                    self.signaler.ModelUpdated.emit()
            self._vrb.write(return_string_1[1])
            return

        elif "DEF" in return_string_1[0]:
            try:
                ratio_index = int(return_string_1[0][3])
            except ValueError:
                self._vrb.err("Ratio index in DEF command not numeric: " + return_string_1[0][3])
                return
            if ratio_index < 0 or ratio_index >= self.num_ratios:
                self._vrb.err("DEFn command for ratio index %d is greater than max %d" % (ratio_index, self.num_ratios))
                return
            return_string_def = return_string_1[1].split(" ")
            def_cmd = return_string_def.pop(0)
            if "ARAT" in def_cmd:
                if self.channels[ratio_index].ratio_expression != return_string_def[0]:
                    self.channels[ratio_index].ratio_expression = return_string_def[0]
                    self.signaler.ModelUpdated.emit()
            elif "RFLT" in def_cmd:
                if self.channels[ratio_index].lpf_cutoff != float(return_string_def[0]):
                    self.channels[ratio_index].lpf_cutoff = float(return_string_def[0])
                    self.signaler.ModelUpdated.emit()
            elif "ALRM" in def_cmd:
                pass
            elif "RATB" in def_cmd:
                if self.channels[ratio_index].baseline_ratio != float(return_string_def[0]):
                    self.channels[ratio_index].baseline_ratio = float(return_string_def[0])
                    self.signaler.ModelUpdated.emit()
            elif "INS1" in def_cmd:
                poly_list = [float(i) for i in return_string_def]
                if self.channels[ratio_index].ins1_polynomial != poly_list:
                    self.channels[ratio_index].ins1_polynomial = poly_list
                    self.signaler.ModelUpdated.emit()
            elif "INS2" in def_cmd:
                poly_list = [float(i) for i in return_string_def]
                if self.channels[ratio_index].ins2_polynomial != poly_list:
                    self.channels[ratio_index].ins2_polynomial = poly_list
                    self.signaler.ModelUpdated.emit()
            elif "SUBE" in def_cmd:
                if self.channels[ratio_index].subtract_enabled != int(return_string_def[0]):
                    self.channels[ratio_index].subtract_enabled = int(return_string_def[0])
                    self.signaler.ModelUpdated.emit()

            self._vrb.write(return_string_1[1])
            return
        elif "INFO" in return_string[0]:
            self._vrb.write(return_string[1])
            return
        elif "RATMASK" in return_string_1[0]:
            self._vrb.write(return_string[1])
        else:
            self._vrb.err("Unrecognized Command " + return_string[1])

    def calculate_polynomials(self, channel_num, order):
        """Least squares fit for calibration points

        This function is WIP. Not tested yet."""
        cal_channel = self.channels[channel_num]
        cal_points = cal_channel.cal_points

        if len(cal_points[0]) < order:
            self._vrb.err("Error: Less points than polynomial order. Please add points.")
            return
        elif order > 5:
            self._vrb.err("Error: Only Polynomials up to 5th order are allowed.")
            return

        ins1_model = numpy.poly1d(numpy.polyfit(cal_points[0], cal_points[1], order))
        ins2_model = numpy.poly1d(numpy.polyfit(cal_points[1], cal_points[2], order))

        ins1_coeff = ins1_model.coefficients.tolist()
        ins1_coeff.reverse()
        ins1_coeff = ins1_coeff[:6] + [0 for _ in range(6 - len(ins1_coeff))]

        ins2_coeff = ins2_model.coefficients.tolist()
        ins2_coeff.reverse()
        ins2_coeff = ins2_coeff[:6] + [0 for _ in range(6 - len(ins2_coeff))]

        self.channels[channel_num].ins1_polynomial = ins1_coeff
        self.channels[channel_num].ins2_polynomial = ins2_coeff

        # Don't write def command yet. Wait for user to apply calibration.

    def measure_baseline(self, channel_num):
        """Measure the baseline ratio/blank value

        This function is WIP. Not tested yet."""
        # todo: change this to measure single point, use it to set ratb and cal points from gui?
        # Set mode to ARAT
        self.stop_streaming()
        self._send_packet("MODE ARAT")
        time.sleep(0.02)
        # Send idle 0 to turn on afe
        self._send_packet("IDLE 0")
        time.sleep(0.5)
        # Send stream 1 command
        self._send_packet("STREAM 1")
        time.sleep(0.2)
        # Check for data
        # Use a results_available = threading.Event()?
        # Avoid divide by zero
        # Send defn command

    def _send_packet(self, msg, slow=0):
        """Send a packet to the server."""
        if self.uart_server is None or not self.uart_server.is_connected():
            self._vrb.err("Not connected to a serial device!")
            return
        # Accept commands with or without end-line char for ease of use.
        msg = msg.rstrip() + "\n"
        if slow or self.state_index == 1:
            # Send command slowly to ensure MCU receives it while streaming
            for char in msg:
                self._tx_q.put(char)
                time.sleep(0.02)
        else:
            self._tx_q.put(msg)

    def send_command(self, cmd, args):
        """Verify and send a command.

        Args:
            cmd (str): value that should match a command in uart_commands
            args (str): additional arguments to send after command
        """
        if self.uart_server is None or not self.uart_server.is_connected():
            self._vrb.err("Not connected to a serial device!")
            return
        try:
            chan = ""
            # Handle commands that have a channel number
            if "DEF" in cmd:
                chan = cmd[len("DEF"):]
                cmd = cmd[:len("DEF")]
            elif "PCB-LED" in cmd:
                chan = cmd[len("PCB-LED"):]
                cmd = cmd[:len("PCB-LED")]
            cmd_index = self.uart_commands.index(cmd.upper())
            msg = self.uart_commands[cmd_index] + chan + " " + str(args)
            self._send_packet(msg)
            time.sleep(0.05)
        except ValueError:
            self._vrb.err("Invalid Command")

    def send_def_command(self, cmd, chan, args):
        """Verify and send a def command.

        Args:
            cmd (str): value that should match a command in def_commands
            chan (str or int): index of channel to send DEF command to.
                E.g., 0 for DEF0 xxx (note index is different than .number)
            args (str): additional arguments to send after command
        """
        if self.uart_server is None or not self.uart_server.is_connected():
            self._vrb.err("Not connected to a serial device!")
            return
        try:
            cmd_index = self.def_commands.index(cmd.upper())
            msg = "DEF" + str(chan) + " " + self.def_commands[cmd_index] + " " + str(args)
            self._send_packet(msg)
            time.sleep(0.05)
        except ValueError:
            self._vrb.err("Invalid DEF Command")

    def send_flash_command(self, cmd, args=""):
        """Verify and send a flash command.

        FL_CLEARBUF and FL_APPLY don't have an argument.
        FL_WRITE and FL_READ do not verify the command after the flash command.

        Args:
            cmd (str): value that should match a command in flash_commands
            args (str, optional): additional arguments to send after command
        """
        if self.uart_server is None or not self.uart_server.is_connected():
            self._vrb.err("Not connected to a serial device!")
            return
        try:
            cmd_index = self.flash_commands.index(cmd.upper())
            msg = self.flash_commands[cmd_index]
            if args:
                msg += " " + str(args)
            self._send_packet(msg)
            time.sleep(0.05)
        except ValueError:
            self._vrb.err("Invalid Flash Command")

    def send_command_direct(self, msg):
        """Send full message directly over uart without verification"""
        if self.uart_server is None or not self.uart_server.is_connected():
            self._vrb.err("Not connected to a serial device!")
            return
        msg = msg.strip()
        if not msg:
            self._vrb.err("Empty command")
            return
        if "help" in msg.lower():
            self._vrb.write("Intercepted help command because of multi-line response")
            return

        self._send_packet(msg)
        time.sleep(0.05)
