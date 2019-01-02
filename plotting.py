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
plt.ion()
tempList = []
cnt = 0

class DataThread(QThread):
    plotting_signal = pyqtSignal('QString')
    temperature_signal = pyqtSignal('QString')
    
    def run(self):
        global arduinoData
        global data
        previous_millis = int(round(time.time() * 1000))  
        while True:
            data = arduinoData.readline().decode('utf8').split(":")
            tempList.append(float(data[0]))
            current_millis = int(round(time.time() * 1000))
            self.temperature_signal.emit(data[0])
            if current_millis - previous_millis > 1000:
                previous_millis = current_millis
                self.plotting_signal.emit('PLOT')
                


class MyApp(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)
        Ui_MainWindow.__init__(self)
        self.setupUi(self)
        self.progressBar.setValue(0)
        self.measurement_button.clicked.connect(self.start_measurement)
        self.cool_button.clicked.connect(self.cool_sensor)
        self.simple_button.clicked.connect(self.start_simple_measurement)
        self.measurement_running = False
        self.cooling_sensor = False

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
            arduinoData.write((f'<{command},{0},{0},{0},{45}>').encode())
            self.cooling_sensor = True
        else:
            self.cool_button.setText('Cool sensor')
            self.cooling_sensor = False
            self.measurement_button.setEnabled(True)
            self.simple_button.setEnabled(True)
            command = str(0)
            arduinoData.write((f'<{command},{0},{0},{0},{45}>').encode()) 


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
            target_temperature = str(self.temperature_box.value())
            arduinoData.write((f'<{command},{p},{i},{d},{target_temperature}>').encode())
        else:
            self.measurement_button.setText('Start measurement')
            self.measurement_running = False
            self.simple_button.setEnabled(True)
            self.cool_button.setEnabled(True)
            command = str(0)
            arduinoData.write((f'<{command},{0},{0},{0},{45}>').encode())   

    def start_simple_measurement(self):
        if(self.measurement_running == False):
            self.simple_button.setText('Stop measurement')
            self.measurement_running = True
            self.measurement_button.setEnabled(False)
            self.cool_button.setEnabled(False)
            command = str(3)
            target_temperature = str(self.temperature_box.value())
            arduinoData.write((f'<{command},{0},{0},{0},{target_temperature}>').encode())
        else:
            self.simple_button.setText('Start measurement in  OFF/On mode')
            self.measurement_running = False
            self.measurement_button.setEnabled(True)
            self.cool_button.setEnabled(True)
            command = str(0)
            arduinoData.write((f'<{command},{0},{0},{0},{45}>').encode())   

    @pyqtSlot('QString')
    def plot_temperature(self, value):
        if(self.measurement_running == True):
            target_temperature_line = []
            tt = self.temperature_box.value()
            target_temperature_line = target_temperature_line + [tt]*(len(tempList)+10)
            self.plotWidget.clear()
            self.plotWidget.plot(tempList, pen = 'r')
            self.plotWidget.plot(target_temperature_line, pen = 'y')
            
    @pyqtSlot('QString')
    def update_status_bar(self, value):
        self.progressBar.setValue((float(data[1])/255) * 100)
        self.statusBar.showMessage(f'Current temperature: {data[0]}, Current Fan PWM: {data[1]}')
        

if __name__=='__main__':
    app = QtWidgets.QApplication(sys.argv)
    window = MyApp()
    window.show()
    sys.exit(app.exec_())