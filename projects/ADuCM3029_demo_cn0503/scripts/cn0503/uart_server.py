"""Uart Server

Copyright (c) 2020 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
"""
import serial, threading


# An abstract uart server base class.
class _uart_server_base():
    def __init__(self, rx_q, tx_q, vrb):
        tx_ack_evt = threading.Event()
        self.vrb = vrb
        self.serial_rx_thread = threading.Thread(target=self.rx_run, args=[rx_q,tx_ack_evt])
        self.serial_rx_thread.setDaemon(True)
        self.serial_tx_thread = threading.Thread(target=self.tx_run, args=[tx_q,tx_ack_evt])
        self.serial_tx_thread.setDaemon(True)
        self.verbose = False
        self.server_setup()
        self.vrb.write("uart server up!", 4)

    def server_setup():
        '''
        Defines any internal setup that a server has to do.
        DO NOT put anything here that touches a real interface; that should go in connect()
        '''
        raise Exception.NotImplementedError()

    def connect():
        '''
        Defines how a connection should be opened to the interface.
        Returns True on success, and False on failure.
        This function must start the receive threads:
            self._start()
        '''
        raise Exception.NotImplementedError()

    def is_connected():
        '''
        Returns True if the interface is currently connected, False otherwise.
        '''
        raise Exception.NotImplementedError()

    def rx_run():
        '''
        The thread which receives packets from the interface
        '''
        raise Exception.NotImplementedError()

    def tx_run():
        '''
        The thread which sends packets to the interface
        '''
        raise Exception.NotImplementedError()

    def scan():
        '''
        Fetches a list of available instances of the interface.
        Examples include:
            - COM ports available on the PC
            - BLE devices available for connection
        '''
        raise Exception.NotImplementedError()

    def quit():
        '''
        Closes the interface. Closes ports, cleans up in-progress transactions, etc.
        '''
        raise Exception.NotImplementedError()

    def _start(self):
        self.serial_tx_thread.start()
        self.serial_rx_thread.start()


class uart_server(_uart_server_base):

    def __init__(self, rx_q, tx_q, vrb):
        self.serial_name = None
        self.tx_ack_evt = threading.Event()
        self.lost_connection = threading.Event()
        self.vrb = vrb
        self.rx_q = rx_q
        self.tx_q = tx_q
        self.serial_rx_thread = threading.Thread(target=self.rx_run, args=[self.rx_q, self.tx_ack_evt])
        self.serial_rx_thread.setDaemon(True)
        self.serial_tx_thread = threading.Thread(target=self.tx_run, args=[self.tx_q, self.tx_ack_evt])
        self.serial_tx_thread.setDaemon(True)
        # self._stop = threading.Event()
        self._running = False
        self.verbose = False
        self._connected_once = False
        self.server_setup()
        self.vrb.write("uart server up!", 4)

    def server_setup(self):
        self.serial_device = None

    def connect(self, serial_address, serial_baud):
        if self._connected_once:
            self.serial_device.setPort(serial_address)
            self._reopen()
            return True
        try:
            self.serial_device = serial.Serial(serial_address, serial_baud, timeout=5)
        except Exception as e:
            self.vrb.err("Server error!: {}".format(e))
            return False
        self.verbose = False
        self._start()
        self._connected_once = True
        self.vrb.write("Serial server up!", 4)
        return True

    def _reopen(self):
        self._running = True
        self.serial_device.open()
        if not self.serial_tx_thread.is_alive():
            # Create new tx thread if it was killed
            self.serial_tx_thread = None
            self.serial_tx_thread = threading.Thread(target=self.tx_run, args=[self.tx_q, self.tx_ack_evt])
            self.serial_tx_thread.setDaemon(True)
            self.serial_tx_thread.start()
        if not self.serial_rx_thread.is_alive():
            # Create new rx thread if it was killed
            self.serial_rx_thread = None
            self.serial_rx_thread = threading.Thread(target=self.rx_run, args=[self.rx_q, self.tx_ack_evt])
            self.serial_rx_thread.setDaemon(True)
            self.serial_rx_thread.start()

    def is_connected(self):
        if self.serial_device is None:
            return False
        elif not self.serial_device.is_open:
            return False
        else:
            return True

    def rx_run(self, rx_q, tx_ack_evt):
        #self.rx_q = rx_q
        try:
            while self._running:
                self.data = self._read_packet(tx_ack_evt)
                rx_q.put(self.data)
        except (serial.SerialException, TypeError, AttributeError):
            self.quit()

    def _read_packet(self, tx_ack_evt):
        line = bytearray()

        try:
            for n in range(255):
                c = self.serial_device.read(1)
                line += c
                if c == b'\n':
                    break
        except (serial.SerialException, TypeError, AttributeError):
            print("Serial Port Closed")
            self.quit()
        
        return bytes(line)
            
    def tx_run(self, tx_q, tx_ack_evt):
        #self.tx_q = tx_q
        while self._running:
            pkt = tx_q.get() #self.tx_q.get()
            # print("tx msg is "+ pkt)
            try:
                self.serial_device.write(pkt.encode())
            except Exception as e:
                self.vrb.write("Failed to do a serial write, did you forget to connect?: {}".format(e), 4)

    def scan(self):
        import sys, glob
        result = []
        if sys.platform.startswith('win'):
            ports = ['COM%s' % (i + 1) for i in range(256)]
        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
            # this excludes your current terminal "/dev/tty"
            ports = glob.glob('/dev/tty[A-Za-z]*')
        elif sys.platform.startswith('darwin'):
            ports = glob.glob('/dev/tty.*')
        else:
            raise EnvironmentError('Unsupported platform')
        for port in ports:
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
            except (OSError, serial.SerialException):
                pass
        return result

    def quit(self):
        self._running = False
        self.serial_device.close()
        self.lost_connection.set()
        self.lost_connection.clear()

    def _start(self):
        self._running = True
        self.serial_tx_thread.start()
        self.serial_rx_thread.start()
