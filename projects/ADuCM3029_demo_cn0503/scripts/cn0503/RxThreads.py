"""Serial Communication Threads

Copyright (c) 2020 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
"""

import queue
from PyQt5 import QtCore
from PyQt5.QtCore import QThread
from PyQt5.QtCore import QRunnable
from PyQt5.QtCore import pyqtSlot
#from PyQt5.QtCore import QThreadPool

"""Monitor for Rx Queue. Notify when data is in the queue.

Can be used with or without using the pyqt event-loop
"""
class MonitorRx(QtCore.QObject):

    UpdateText = QtCore.pyqtSignal(str)
    Rx_q = None

    """update_list initializes and starts the monitor
    args:
        rxQ: the queue to be monitored
        (optional) cn0503: if not using a pyqt event-loop, pass in the cn0503 object
    """
    def update_list(self, rxQ, cn0503_obj=None):
        self.Rx_q = rxQ
        self.cn0503 = cn0503_obj
        self.rx_monitor = ThreadRx(self.monitor_Rx, args=0, kwargs=0, parent=self)
        self.rx_monitor.daemon = True
        #rx_monitor.setName('monitorRx')
        self.rx_monitor.start()

    def monitor_Rx(self):
        packet = self.Rx_q.get().decode()
        if self.cn0503 is None:
            self.UpdateText.emit(packet)
        else:
            self.cn0503._dispatcher(packet)


class MonitorConnection(QtCore.QObject):

    LostConnection = QtCore.pyqtSignal()

    def start_monitoring(self, disconnect_event):
        self.disconnect_event = disconnect_event
        connection_monitor = ThreadRx(self.monitor_connection, args=0, kwargs=0, parent=self)
        connection_monitor.daemon = True
        connection_monitor.start()

    def monitor_connection(self):
        while not self.disconnect_event.wait(10):
            pass
        self.LostConnection.emit()


class ThreadRx(QtCore.QThread):

    def __init__(self, fn, args, kwargs, parent=None):
        super(ThreadRx, self).__init__(parent)
        self._fn = fn 
        self._args = args 
        self._kwargs = kwargs 

    def run(self):
        while(1):            
            self._fn()


class ThreadWorker(QRunnable):
    def __init__(self, fn, args, *kwargs):
        super(ThreadWorker, self).__init__()
        # Store constructor arguments (re-used for processing)
        self.fn = fn
        self.args = args
        self.kwargs = kwargs

    @pyqtSlot()
    def run(self):
        '''
        Initialise the runner function with passed args, kwargs.
        '''
        self.fn(self.args)

'''
class PlotRxData(QtCore.QObject):

    UpdateText = QtCore.pyqtSignal(str)
    Rx_q = None

    def update_list(self, rxQ):
        self.Rx_q = rxQ
        rx_monitor = ThreadPlot(self.monitor_Rx, args=0, kwargs=0, parent=self)
        rx_monitor.daemon = True
        #rx_monitor.setName('monitorRx')
        rx_monitor.start()

    def monitor_Rx(self):
        packet = self.Rx_q.get().decode()
        self.UpdateText.emit(packet)


class ThreadPlot(QtCore.QThread):

    def __init__(self, fn, args, kwargs, parent=None):
        super(ThreadPlot, self).__init__(parent)
        self._fn = fn 
        self._args = args 
        self._kwargs = kwargs 

    def run(self):
        #self._fn(*self._args, **self._kwargs)
        while(1):            
            self._fn()
'''