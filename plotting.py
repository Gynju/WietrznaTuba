import serial
import numpy
import matplotlib.pyplot as plt
from drawnow import *
from matplotlib.widgets import Slider

timeList=  []
tempList=  []
arduinoData = serial.Serial('COM3', 9600)

plt.ion()

cnt = 0

def makeFig():
    plt.title('Plot of temperature')
    plt.grid(True)
    plt.plot(timeList, tempList, 'ro-', label='Degrees')
    plt.xticks(numpy.arange(timeList[0], timeList[-1], 2.0))
    plt.legend(loc='upper left')
    plt.ylim([32, 37])

previousTime = 0
while previousTime < 61:
    while(arduinoData.inWaiting()==0):
        pass
    
    arduinoString = arduinoData.readline()
    data = arduinoString.decode('utf8').split(":")

    tempList.append(float(data[0]))
    timeList.append(float(data[1])+ previousTime)
    previousTime = timeList[-1]

    drawnow(makeFig)

    cnt = cnt + 1
    if(cnt > 61):
        tempList.pop(0)
        timeList.pop(0)
            
    
