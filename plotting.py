import serial
import numpy
import matplotlib.pyplot as plt
from drawnow import *
from matplotlib.widgets import Slider

tempList = []

arduinoData = serial.Serial('COM3', 9600)

plt.ion()

cnt = 0

def makeFig():
    plt.title('Plot of temperature')
    plt.grid(True)
    plt.plot(tempList, 'ro-', label='Degrees')
    plt.legend(loc='upper left')
    plt.ylim([20, 50])

previousTime = 0
while True:
    while(arduinoData.inWaiting()==0):
        pass
    
    arduinoString = arduinoData.readline()
    data = arduinoString.decode('utf8').split(":")

    tempList.append(float(data[0]))

    drawnow(makeFig)

    cnt = cnt + 1
    if(cnt > 120):
        break
arduinoData = None
            
    
