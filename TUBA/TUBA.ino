#include <PID_v1.h>
#include <DS18B20.h>
#include <OneWire.h>

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
bool runningMeasurements = true;
bool isSensorCooled = false;

unsigned long currentTime;
unsigned long startMeasurementTime;

PID myP(&temperature, &fanSpeed1, &prefferedTemperature, proportionalTerm, 0, 0, REVERSE);
PID myPI(&temperature, &fanSpeed2, &prefferedTemperature, proportionalTerm, integralTerm, 0, REVERSE);
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

  myP.SetMode(AUTOMATIC); 
  myPI.SetMode(AUTOMATIC); 
  myPID.SetMode(AUTOMATIC); 

  sensors.begin();
  sensors.request(address);
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
  while(runningMeasurements and millis() - startMeasurementTime < 121000)
    if(millis() - currentTime > 1000)
    {
      analogWrite(haloPIN, 255);
      currentTime = millis();
      temperature = sensors.readTemperature(address);
      myP.Compute();
      myPI.Compute();
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
  runningMeasurements = false;
}

void goIdle()
{
    analogWrite(haloPIN, 0);
    analogWrite(fanPIN, 0);
}

void loop()
{
if(Serial.available() >= 4)
{
  double incoming[4];
  for(int i = 0; i < 4; i++)
  {
    incoming[i] = Serial.read();
  }
  int command = int(incoming[0]);
  proportionalTerm = double(incoming[1]);
  integralTerm = double(incoming[2]);
  derivativeTerm = double(incoming[3]); 

  Serial.println(command);
  Serial.println(proportionalTerm);
  Serial.println(integralTerm);
  Serial.println(derivativeTerm);
  if(command == '0')
  {
    goIdle();
  }
  if(command == '1')
  {
    coolSensor();
    measureTemperature();
    goIdle();
  }
    //  double rd = Serial.read() - 48;
    //  integralTerm = rd;
    //  derivativeTerm = rd;

    //  myP.SetTunings(proportionalTerm, integralTerm, derivativeTerm);
    //  myPI.SetTunings(proportionalTerm, integralTerm, derivativeTerm);
    //  myPID.SetTunings(proportionalTerm, integralTerm, derivativeTerm);
   }
 
  // coolSensor();
  
  // measureTemperature();

  // goIdle();
}

















  
//  analogWrite(haloPIN, 255);
//
//  previousTime = currentTime;
//  currentTime = millis();
//  elapsedTime = (currentTime - previousTime) / 1000;
//  totalTime += elapsedTime;
//
//  temperature = sensors.readTemperature(address);
//
//  currentError = prefferedTemperature - temperature;
//  calculatedProportionalValue = proportionalTerm * currentError;
//
//  if(-5 < currentError < 5)
//  {
//    calculatedIntegralValue = calculatedIntegralValue + (integralTerm * currentError);
//  }
//
//  calculatedDerivativeValue = derivativeTerm*((currentError - previousError)/elapsedTime);
//  valuePID = calculatedProportionalValue + calculatedIntegralValue + calculatedDerivativeValue;
//
//  if(valuePID < 0)
//  {
//    valuePID = 0;
//  }
//
//  if(valuePID > 255)
//  {
//    valuePID = 255;
//  }
//
//  previousError = currentError;
//  fanSpeed = 255 - valuePID;
//  analogWrite(fanPIN,  fanSpeed);
//  Serial.println(temperature);
//  sensors.request(address);
//  delay(1000);

