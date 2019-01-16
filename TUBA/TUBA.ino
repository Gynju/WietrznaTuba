#include <PID_v1.h>
#include <DS18B20.h>
#include <OneWire.h>

char inputBuffer[64];
int command = 0;
int bytesRecived = 0;
const char startMarker = '<';
const char endMarker = '>';

bool receivedData = false;
bool readInProgress = false;

double proportionalTerm = 0;
double integralTerm = 0;
double derivativeTerm = 0;

#define sensorPIN 2
#define fanPIN 9
#define haloPIN 3

int controlCommand = 0;
int staticPIN = haloPIN;
int controlPIN = fanPIN;

double prefferedTemperature = 45;
double temperature;
double fanSpeed = 0;
byte address[8] = {0x28, 0x6E, 0x7D, 0x24, 0x6, 0x0, 0x0, 0xE0};

unsigned long currentTime;
unsigned long startMeasurementTime;

PID myPID(&temperature, &fanSpeed, &prefferedTemperature, proportionalTerm, integralTerm, derivativeTerm, REVERSE);

OneWire onewire(sensorPIN);
DS18B20 sensors(&onewire);

void setup()
{
  pinMode(fanPIN, OUTPUT);
  pinMode(haloPIN, OUTPUT);

  analogWrite(haloPIN, 0);
  analogWrite(fanPIN, 0);

  Serial.begin(9600);

  myPID.SetMode(AUTOMATIC); 

  sensors.begin();
  sensors.request(address);
}

void loop()
{
  readDataFromSerial();

  if(receivedData)
  {
    myPID.SetTunings(proportionalTerm, integralTerm, derivativeTerm);
    checkControlledElement();
    receivedData = false;
  }

  switch(command)
  {
    case 0:
      sendTemperature();
      standBy();
      break;

    case 1:
      coolSensor();
      break;

    case 2:
      measureTemperature();
      break;
      
    case 3:
      simpleMeasureTemperature();
      break;
  }
}

void checkControlledElement()
{
  if(!controlCommand)
    {
      controlPIN = fanPIN;
      staticPIN = haloPIN;
    }
  else
    {
      controlPIN = haloPIN;
      staticPIN = fanPIN;
    }
}

void coolSensor()
{
  unsigned long coolingStartTime = millis();
  
  fanSpeed = 255;
  while(true)
  {
    readDataFromSerial();
    if(command == 0)
    {
      receivedData = false;
      break;
    }
    analogWrite(haloPIN, 0);
    analogWrite(fanPIN, fanSpeed);
    sendTemperature();
  }
  command = 0;
  fanSpeed = 0;
}

void measureTemperature()
{
  currentTime = millis();
  while(true)
  {
    readDataFromSerial();
    if(command == 0)
    {
      receivedData = false;
      break;
    }
    if(millis() - currentTime > 100)
    {
      analogWrite(staticPIN, 255);
      currentTime = millis();
      temperature = sensors.readTemperature(address);
      sensors.request(address);
      myPID.Compute();
      if(fanSpeed < 50)
        fanSpeed = 0;
      if(controlCommand == 1)
        fanSpeed = map(fanSpeed, 0, 255, 255, 0);
      analogWrite(controlPIN, fanSpeed);
      sendTemperature();
    }
  }
  command = 0;
}

void simpleMeasureTemperature()
{
  currentTime = millis();
  while(true)
  {
    readDataFromSerial();
    if(command == 0)
    {
      receivedData = false;
      break;
    }
    if(millis() - currentTime > 100)
    {
      analogWrite(staticPIN, 255);
      currentTime = millis();
      temperature = sensors.readTemperature(address);
      sensors.request(address);
      if(temperature > prefferedTemperature)
        fanSpeed = 255;
      else
        fanSpeed = 0;
       if(controlCommand == 1)
        fanSpeed = map(fanSpeed, 0, 255, 255, 0);
      analogWrite(controlPIN, fanSpeed);
      sendTemperature();
    }
  }
  command = 0;
}

void standBy()
{
  analogWrite(haloPIN, 0);
  analogWrite(fanPIN, 0);
}

void sendTemperature()
{
  float temp = sensors.readTemperature(address);
  Serial.print(temp);
  Serial.print(":");
  Serial.println(fanSpeed);
  sensors.request(address);
}

void readDataFromSerial()
{
  if(Serial.available() > 0)
  {
    char receivedByte = Serial.read();

    if (receivedByte == endMarker)
    {
      readInProgress = false;
      receivedData = true;
      inputBuffer[bytesRecived] = 0;
      parseData();
    }
    
    if(readInProgress)
    {
      inputBuffer[bytesRecived] = receivedByte;
      bytesRecived ++;
      if (bytesRecived == 64) {
        bytesRecived = 64 - 1;
      }
    }

    if (receivedByte == startMarker)
    { 
      bytesRecived = 0; 
      readInProgress = true;
    }
  }
}

void parseData()
{   
  char * strtokIndx;
  
  strtokIndx = strtok(inputBuffer, ",");
  command = atoi(strtokIndx);
  
  strtokIndx = strtok(NULL, ",");
  proportionalTerm = atof(strtokIndx);
  
  strtokIndx = strtok(NULL, ","); 
  integralTerm = atof(strtokIndx);

  strtokIndx = strtok(NULL, ","); 
  derivativeTerm = atof(strtokIndx);

  strtokIndx = strtok(NULL, ","); 
  prefferedTemperature = atof(strtokIndx);

  strtokIndx = strtok(NULL, ","); 
  controlCommand = atof(strtokIndx);
}