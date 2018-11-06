import serial
import numpy
import matplotlib.pyplot as plt
from drawnow import *
from matplotlib.widgets import Slider

tempF=  []
arduinoData = serial.Serial('COM3', 9600)

plt.ion()

cnt = 0

def makeFig():
    plt.title('Plot of temperature')
    plt.grid(True)
    plt.plot(tempF, 'ro-', label='Degrees')
    plt.legend(loc='upper left')
    plt.ylim([30, 50])

while True:
    while(arduinoData.inWaiting()==0):
        pass
    arduinoString = arduinoData.readline()

    temp = float(arduinoString)
    tempF.append(temp)

    drawnow(makeFig)

    cnt = cnt + 1
    if(cnt > 50):
        tempF.pop(0)
            
    
