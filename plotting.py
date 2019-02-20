import matplotlib.pyplot as plt
import numpy
import serial
import sys
import time
import os
from PyQt5 import QtWidgets, uic
from PyQt5.QtCore import QThread, pyqtSignal, pyqtSlot
import pyqtgraph as pg

uiFile = 'mainwindow.ui'

Ui_MainWindow, QtBaseClass = uic.loadUiType(uiFile)

arduinoData = serial.Serial('COM3', 9600)
data = []
tempList = []


def map_value(value, leftMin, leftMax, rightMin, rightMax):
    leftSpan = leftMax - leftMin
    rightSpan = rightMax - rightMin

    valueScaled = float(value - leftMin) / float(leftSpan)

    return rightMin + (valueScaled * rightSpan)


class DataThread(QThread):
    plotting_signal = pyqtSignal('QString')
    temperature_signal = pyqtSignal('QString')

    def run(self):
        global arduinoData
        global data
        previous_millis = int(round(time.time() * 1000))
        while True:
            try:
                data = arduinoData.readline().decode('utf-8').split(":")
                tempList.append(float(data[0]))
                current_millis = int(round(time.time() * 1000))
                previous_millis = current_millis
                self.plotting_signal.emit('PLOT')
                self.temperature_signal.emit(data[0])
            except:
                pass


class MyApp(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self):
        pg.setConfigOption('background', 'w')
        pg.setConfigOption('foreground', 'k')
        QtWidgets.QMainWindow.__init__(self)
        Ui_MainWindow.__init__(self)
        self.setupUi(self)
        self.measurement_button.clicked.connect(self.start_measurement)
        self.cool_button.clicked.connect(self.cool_sensor)
        self.simple_button.clicked.connect(self.start_simple_measurement)
        self.fanControlButton.clicked.connect(self.setFanControl)
        self.lampControlButton.clicked.connect(self.setLampControl)
        self.measurement_running = False
        self.cooling_sensor = False
        self.controlCommand = 0

        self.data_thread = DataThread()
        self.data_thread.plotting_signal.connect(self.plot_temperature)
        self.data_thread.temperature_signal.connect(self.update_status_bar)
        self.data_thread.start()


    def cool_sensor(self):
        if(self.measurement_running == False and self.cooling_sensor == False):
            self.cool_button.setText('Stop cooling')
            self.measurement_button.setEnabled(False)
            self.simple_button.setEnabled(False)
            command = str(1)
            arduinoData.write(
                (f'<{command},{0},{0},{0},{0},{0},{0},{0}>').encode())
            self.cooling_sensor = True
        else:
            self.cool_button.setText('Cool sensor')
            self.cooling_sensor = False
            self.measurement_button.setEnabled(True)
            self.simple_button.setEnabled(True)
            command = str(0)
            arduinoData.write(
                (f'<{command},{0},{0},{0},{45},{self.controlCommand},{0},{0}>').encode())

    def start_measurement(self):
        global tempList
        if(self.measurement_running == False):
            self.measurement_button.setText('Stop measurement')
            self.measurement_running = True
            self.simple_button.setEnabled(False)
            self.cool_button.setEnabled(False)
            command = str(2)
            p = str(self.p_box.value())
            i = str(self.i_box.value())
            d = str(self.d_box.value())
            PWM_cut = str(self.regulator.value())
            target_temperature = str(self.temperature_box.value())
            arduinoData.write(
                (f'<{command},{p},{i},{d},{target_temperature},{self.controlCommand},{PWM_cut},{0}>').encode())
        else:
            self.measurement_button.setText('Start measurement')
            self.measurement_running = False
            self.simple_button.setEnabled(True)
            self.cool_button.setEnabled(True)
            command = str(0)
            arduinoData.write(
                (f'<{command},{0},{0},{0},{45},{self.controlCommand},{50},{0}>').encode())

    def start_simple_measurement(self):
        if(self.measurement_running == False):
            self.simple_button.setText('Stop measurement')
            self.measurement_running = True
            self.measurement_button.setEnabled(False)
            self.cool_button.setEnabled(False)
            command = str(3)
            target_temperature = str(self.temperature_box.value())
            delta_h = str(self.delta_h_box.value())
            arduinoData.write(
                (f'<{command},{0},{0},{0},{target_temperature},{self.controlCommand},{0},{delta_h}>').encode())
        else:
            self.simple_button.setText('Start measurement in  OFF/On mode')
            self.measurement_running = False
            self.measurement_button.setEnabled(True)
            self.cool_button.setEnabled(True)
            command = str(0)
            arduinoData.write(
                (f'<{command},{0},{0},{0},{45},{self.controlCommand},{0},{0}>').encode())

    def setFanControl(self):
        self.controlCommand = 0

    def setLampControl(self):
        self.controlCommand = 1

    @pyqtSlot('QString')
    def plot_temperature(self, value):
        self.plotWidget.clear()
        time = numpy.arange(len(tempList))
        time = [x/10 for x in time]
        try:
            self.plotWidget.plot(time, tempList, pen=pg.mkPen('r', width=3))
            self.plotWidget.plot([self.temperature_box.value()]*int(time[-1]+2), pen=pg.mkPen('b', width=3))
        except:
            pass

    @pyqtSlot('QString')
    def update_status_bar(self, value):
        self.progressBar.setValue(
            map_value(self.regulator.value(), 0, 255, 0, 100))
        self.statusBar.showMessage(
            f'Current temperature: {data[0]}, Current Fan PWM: {data[1]}, Current Control Command: {data[2]}')


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    window = MyApp()
    window.show()
    sys.exit(app.exec_())
