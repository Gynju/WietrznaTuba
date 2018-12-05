#include <PID_v1.h>
#include <DS18B20.h>
#include <OneWire.h>

char inputBuffer[64];
int command = 0;
int bytesRecvd = 0;
const char startMarker = '<';
const char endMarker = '>';

bool receivedData = false;
bool readInProgress = false;
bool isRunningMeasurements = true;
bool isSensorCooled = false;

double proportionalTerm = 0;
double integralTerm = 0;
double derivativeTerm = 0;

#define sensorPIN 2
#define fanPIN 9
#define haloPIN 3

double prefferedTemperature = 40;
double temperature;
double fanSpeed1 = 0;
double fanSpeed2 = 0;
double fanSpeed3 = 0;
byte address[8] = {0x28, 0x6E, 0x7D, 0x24, 0x6, 0x0, 0x0, 0xE0};


unsigned long currentTime;
unsigned long startMeasurementTime;

PID myPID(&temperature, &fanSpeed3, &prefferedTemperature, proportionalTerm, integralTerm, derivativeTerm, REVERSE);

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
  if(command == 1 and !readInProgress)
  {
    coolSensor();
    measureTemperature();
    goIdle();
  }
  // if(Serial.available() > 0)
  // {
  //   char incoming data = Serial.read();
  //   Serial.read();
  //   Serial.println(Serial.read());
    // int command = int(incoming[0]);
    // proportionalTerm = double(incoming[1]);
    // integralTerm = double(incoming[2]);
    // derivativeTerm = double(incoming[3]); 

    // if(command == '0')
    // {
    //   goIdle();
    // }
    // if(command == '1')
    // {
    //   coolSensor();
    //   measureTemperature();
    //   goIdle();
    // }
      //  double rd = Serial.read() - 48;
      //  integralTerm = rd;
      //  derivativeTerm = rd;

      //  myP.SetTunings(proportionalTerm, integralTerm, derivativeTerm);
      //  myPI.SetTunings(proportionalTerm, integralTerm, derivativeTerm);
      //  myPID.SetTunings(proportionalTerm, integralTerm, derivativeTerm);
}

void coolSensor()
{
  unsigned long coolingStartTime = millis();
  
  while(!isSensorCooled and millis() - coolingStartTime < 30000)
  {
    analogWrite(haloPIN, 0);
    analogWrite(fanPIN, 255);
  }
  isSensorCooled = true;
}

void measureTemperature()
{
  startMeasurementTime = millis();
  currentTime = millis();
  while(isRunningMeasurements and millis() - startMeasurementTime < 121000)
    if(millis() - currentTime > 1000)
    {
      analogWrite(haloPIN, 255);
      currentTime = millis();
      temperature = sensors.readTemperature(address);
      myPID.Compute();
      analogWrite(fanPIN, fanSpeed3);
      sensors.request(address);
    
      Serial.print(temperature);
      Serial.print(":");
      Serial.print(fanSpeed1);
      Serial.print(":");
      Serial.print(fanSpeed2);
      Serial.print(":");
      Serial.println(fanSpeed3);
    }
  isRunningMeasurements = false;
}

void goIdle()
{
    command = 0;
    analogWrite(haloPIN, 0);
    analogWrite(fanPIN, 0);
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
      inputBuffer[bytesRecvd] = 0;
      parseData();
    }
    
    if(readInProgress)
    {
      inputBuffer[bytesRecvd] = receivedByte;
      bytesRecvd ++;
      if (bytesRecvd == 64) {
        bytesRecvd = 64 - 1;
      }
    }

    if (receivedByte == startMarker)
    { 
      bytesRecvd = 0; 
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