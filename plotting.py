import matplotlib.pyplot as plt
import numpy
import serial
import sys
import time
from PyQt5 import QtWidgets, uic
from PyQt5.QtCore import QThread, pyqtSignal, pyqtSlot
import pyqtgraph as pg

#from drawnow import *

uiFile = 'mainwindow.ui'

Ui_MainWindow, QtBaseClass = uic.loadUiType(uiFile)



arduinoData = serial.Serial('COM3', 9600)
plt.ion()
tempList = []
cnt = 0


class DataThread(QThread):
    sygnal = pyqtSignal('QString')
	
    def foo(self):
        sygnal.emit("Elo")

class MyApp(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)
        Ui_MainWindow.__init__(self)
        self.setupUi(self)
        self.measurement_button.clicked.connect(self.start_measurement)
        self.measurement_running = False
        self.dataThread = DataThread()
        self.dataThread.sygnal.connect(self.bar)
        self.dataThread.start()
	
    @pyqtSlot('QString')
    def bar(self, value):
        print(value)
    
    def start_measurement(self):
        if(self.measurement_running == False):
            self.measurement_button.setText('Stop measurement')
            self.measurement_running = True
            command = str(2)
            p = str(self.p_box.value())
            i = str(self.i_box.value())
            d = str(self.d_box.value())
            arduinoData.write((f'<{command},{p},{i},{d}>').encode())
            # self.plot_temperature()
        else:
            self.measurement_button.setText('Start measurement')
            self.measurement_running = False
            command = str(0)
            arduinoData.write((f'<{command},{0},{0},{0}>').encode())
    
    def makeFig(self):
        # plt.title('Plot of temperature')
        # plt.grid(True)
        self.plotWidget.plot(tempList)
        # plt.plot(tempList, 'ro-', label='Degrees')
        # plt.legend(loc='upper left')
        # plt.ylim([20, 50])

    def plot_temperature(self):
        global arduinoData
        arduinoString = arduinoData.readline()
        data = arduinoString.decode('utf8').split(":")
        tempList.append(float(data[0]))
        self.makeFig()
            

if __name__=='__main__':
    app = QtWidgets.QApplication(sys.argv)
    window = MyApp()
    window.show()
    sys.exit(app.exec_())