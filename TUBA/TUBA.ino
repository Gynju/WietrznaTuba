#include <DS18B20.h>
#include <OneWire.h>

#define sensorPIN 2
#define fanPIN 9
#define haloPIN 3

int prefferedTemperature = 35;
byte address[8] = {0x28, 0x6E, 0x7D, 0x24, 0x6, 0x0, 0x0, 0xE0};

double osc = 0;
double proportionalTerm = 2^128000; double integralTerm = 0; double derivativeTerm = 0;

double calculatedProportionalValue = 0;
double calculatedIntegralValue = 0;
double calculatedDerivativeValue = 0;

double currentError = 0;
double previousError = 0;

int fanSpeed = 0;

double elapsedTime, currentTime, previousTime, totalTime;
double valuePID;

OneWire onewire(sensorPIN);
DS18B20 sensors(&onewire);

void setup()
{
  pinMode(fanPIN, OUTPUT);
  pinMode(haloPIN, OUTPUT);

  analogWrite(haloPIN, 0);
  analogWrite(fanPIN, 0);

  Serial.begin(9600);

  sensors.begin();
  sensors.request(address);
}

void loop()
{
  analogWrite(haloPIN, 255);

  previousTime = currentTime;
  currentTime = millis();
  elapsedTime = (currentTime - previousTime) / 1000;
  totalTime += elapsedTime;

  float temperature = sensors.readTemperature(address);

  currentError = prefferedTemperature - temperature;
  calculatedProportionalValue = proportionalTerm * currentError;

  if(-5 < currentError < 5)
  {
    calculatedIntegralValue = calculatedIntegralValue + (integralTerm * currentError);
  }

  calculatedDerivativeValue = derivativeTerm*((currentError - previousError)/elapsedTime);
  valuePID = calculatedProportionalValue + calculatedIntegralValue + calculatedDerivativeValue;

  if(valuePID < 0)
  {
    valuePID = 0;
  }

  if(valuePID > 255)
  {
    valuePID = 255;
  }

  previousError = currentError;

  analogWrite(fanPIN,  255 - valuePID);
  Serial.print(temperature);
  Serial.print(":");
  Serial.println(elapsedTime);
  sensors.request(address);
  delay(1000);
}
