import serial
import numpy
import matplotlib.pyplot as plt
from drawnow import *
from matplotlib.widgets import Slider

tempList = []
PWMList1 = []
PWMList2 = []
PWMList3 = []

arduinoData = serial.Serial('COM3', 9600)

plt.ion()

cnt = 0

def makeFig():
    plt.title('Plot of temperature')
    plt.grid(True)
    plt.plot(tempList, 'ro-', label='Degrees')
    plt.plot(PWMList1, 'go-', label='PWM_1')
    plt.plot(PWMList2, 'bo-', label='PWM_2')
    plt.plot(PWMList3, 'yo-', label='PWM_3')
    plt.legend(loc='upper left')
    plt.ylim([0, 255])

previousTime = 0
while True:
    while(arduinoData.inWaiting()==0):
        pass
    
    arduinoString = arduinoData.readline()
    data = arduinoString.decode('utf8').split(":")

    tempList.append(float(data[0]))
    PWMList1.append(float(data[1]))
    PWMList2.append(float(data[2]))
    PWMList3.append(float(data[3]))

    drawnow(makeFig)

    cnt = cnt + 1
    if(cnt > 50):
        tempList.pop(0)
            
    
