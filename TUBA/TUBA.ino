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
    receivedData = false;
  }

  switch(command)
  {
    case 0:
      sendTemperature();
      goIdle();
      break;

    case 1:
      coolSensor();
      break;

    case 2:
      measureTemperature();
      break;
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
      analogWrite(haloPIN, 255);
      currentTime = millis();
      temperature = sensors.readTemperature(address);
      sensors.request(address);
      myPID.Compute();
      if(fanSpeed < 50)
        fanSpeed = 0;
      analogWrite(fanPIN, fanSpeed);
      sendTemperature();
    }
  }
  command = 0;
}

void goIdle()
{
  fanSpeed = 0;
  analogWrite(haloPIN, 0);
  analogWrite(fanPIN, fanSpeed);
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
}