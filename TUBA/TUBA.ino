#include <DS18B20.h>
#include <OneWire.h>

#define sensorPIN 2
#define fanPIN 9
#define haloPIN 3

int preffered_temp = 38;
byte address[8] = {0x28, 0x6E, 0x7D, 0x24, 0x6, 0x0, 0x0, 0xE0};

double osc = 20;
double kp = 2*osc; double ki = 1.5*osc; double kd = ki/5;

double PID_p = 0;
double PID_i = 0;
double PID_d = 0;
int fanSpeed = 0;
float PID_error = 0;
float previous_error = 0;
float elapsedTime, Time, timePrev;
int PID_value;

OneWire onewire(sensorPIN);
DS18B20 sensors(&onewire);

void setup()
{
  pinMode(fanPIN, OUTPUT);
  pinMode(haloPIN, OUTPUT);

  Serial.begin(9600);

  sensors.begin();
  sensors.request(address);
}

void loop() {
  analogWrite(haloPIN, 255);

  float temperature = sensors.readTemperature(address);

  PID_error = preffered_temp - temperature;
  PID_p = kp * PID_error;
  if(-3 < PID_error < 3)
  {
    PID_i = PID_i + (ki * PID_error);
  }
  timePrev = Time;
  Time = millis();
  elapsedTime = (Time - timePrev) / 1000;
  PID_d = kd*((PID_error - previous_error)/elapsedTime);
  PID_value = PID_p + PID_i + PID_d;
  if(PID_value < 0)
  {
    PID_value = 0;
  }
  if(PID_value > 255)
  {
    PID_value = 255;
  }
  previous_error = PID_error;

  analogWrite(fanPIN,  255 - PID_value);
  Serial.println(temperature);
  sensors.request(address);
  delay(200);

}
