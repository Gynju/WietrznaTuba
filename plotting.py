import matplotlib.pyplot as plt
import numpy
import serial
import sys
import time
import os
from PyQt5 import QtWidgets, uic
from PyQt5.QtCore import QThread, pyqtSignal, pyqtSlot
import pyqtgraph as pg
from multiprocessing import Pool

uiFile = 'mainwindow.ui'

Ui_MainWindow, QtBaseClass = uic.loadUiType(uiFile)

arduinoData = serial.Serial('COM3', 9600)
plt.ion()
tempList = []
cnt = 0

class DataThread(QThread):
    signal = pyqtSignal('QString')
    
    def run(self):
        global arduinoData
        previous_millis = int(round(time.time() * 1000))
        while True:
            current_millis = int(round(time.time() * 1000))
            
            global arduinoData
            arduinoString = arduinoData.readline()
            data = arduinoString.decode('utf8').split(":")
            tempList.append(float(data[0]))
            tempList.append(float(data[0]))
            previous_millis = current_millis
            
            if current_millis - previous_millis > 1000:
                self.signal.emit('PLOT THIS MADAFAKA')


class MyApp(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)
        Ui_MainWindow.__init__(self)
        self.setupUi(self)
        self.measurement_button.clicked.connect(self.start_measurement)
        self.measurement_running = False

        self.data_thread = DataThread()
        self.data_thread.signal.connect(self.plot_temperature)
        self.data_thread.start()
    
    def start_measurement(self):
        if(self.measurement_running == False):
            self.measurement_button.setText('Stop measurement')
            self.measurement_running = True
            command = str(2)
            p = str(self.p_box.value())
            i = str(self.i_box.value())
            d = str(self.d_box.value())
            arduinoData.write((f'<{command},{p},{i},{d}>').encode())
        else:
            self.measurement_button.setText('Start measurement')
            self.measurement_running = False
            command = str(0)
            arduinoData.write((f'<{command},{0},{0},{0}>').encode())   

    @pyqtSlot('QString')
    def plot_temperature(self, value):
        self.plotWidget.plot(tempList)
            

if __name__=='__main__':
    app = QtWidgets.QApplication(sys.argv)
    window = MyApp()
    window.show()
    sys.exit(app.exec_())