#include "SoilSensor.h"
#include <Arduino.h>

SoilSensor::SoilSensor(int pin) : _pin(pin) {
  pinMode(_pin, INPUT);
}

int SoilSensor::read() {
  return analogRead(_pin);
}