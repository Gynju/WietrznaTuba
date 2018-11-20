import serial
import numpy
import matplotlib.pyplot as plt
from drawnow import *
from matplotlib.widgets import Slider

timeList=  []
PWMList1=  []
PWMList2=  []
PWMList3=  []
arduinoData = serial.Serial('COM3', 9600)

plt.ion()

cnt = 0

def makeFig():
    plt.title('Plot of PMW\'s')
    plt.grid(True)
    plt.plot(PWMList1, 'ro-', label='PWM_1')
    plt.plot(PWMList2, 'go-', label='PWM_2')
    plt.plot(PWMList3, 'bo-', label='PWM_3')
    plt.legend(loc='upper left')
    plt.ylim([0, 255])

previousTime = 0
while True:
    while(arduinoData.inWaiting()==0):
        pass
    
    arduinoString = arduinoData.readline()
    data = arduinoString.decode('utf8').split(": ")

    PWMList1.append(float(data[1]))
    PWMList2.append(float(data[2]))
    PWMList3.append(float(data[3]))
    #timeList.append(float(data[1])+ previousTime)
    #previousTime = timeList[-1]

    drawnow(makeFig)
    drawnow(makeFig)
    drawnow(makeFig)

    cnt = cnt + 1
    if(cnt > 200):
        tempList.pop(0)
        timeList.pop(0)
            
    
