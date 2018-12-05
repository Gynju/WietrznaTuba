import matplotlib.pyplot as plt
import numpy
import serial
import sys
import time
from matplotlib.widgets import Slider
from PyQt5 import QtWidgets, uic

# from drawnow import *

uiFile = 'mainwindow.ui'

Ui_MainWindow, QtBaseClass = uic.loadUiType(uiFile)

# tempList = []

arduinoData = serial.Serial('COM3', 9600)

# plt.ion()

# cnt = 0

# def makeFig():
#     plt.title('Plot of temperature')
#     plt.grid(True)
#     plt.plot(tempList, 'ro-', label='Degrees')
#     plt.legend(loc='upper left')
#     plt.ylim([20, 50])

# previousTime = 0
# while True:
#     while(arduinoData.inWaiting()==0):
#         pass
    
#     arduinoString = arduinoData.readline()
#     data = arduinoString.decode('utf8').split(":")

#     tempList.append(float(data[0]))

#     drawnow(makeFig)

#     cnt = cnt + 1
#     if(cnt > 120):
#         break
# arduinoData = None

class MyApp(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)
        Ui_MainWindow.__init__(self)
        self.setupUi(self)
        self.start_measurement_button.clicked.connect(self.start_measurement)
        self.pressed = False
    
    def start_measurement(self):
        if(self.pressed == False):
            command = str(1)
            p = str(self.p_box.value())
            i = str(self.i_box.value())
            d = str(self.d_box.value())
            arduinoData.write((f'<{command},{p},{i},{d}>').encode())
        else:
            arduinoData.write(b'0')
        self.pressed = not self.pressed

if __name__=='__main__':
    app = QtWidgets.QApplication(sys.argv)
    window = MyApp()
    window.show()
    sys.exit(app.exec_())