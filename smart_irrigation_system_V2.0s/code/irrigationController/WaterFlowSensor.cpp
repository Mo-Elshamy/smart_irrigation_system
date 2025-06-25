#include "WaterFlowSensor.h"
#include <Arduino.h>

volatile unsigned int globalPulseCount = 0;

void flowISR() {
  globalPulseCount++;
}

WaterFlowSensor::WaterFlowSensor(int pin) : _pin(pin), pulseCount(0), lastMeasurement(0) {}

void WaterFlowSensor::begin() {
  pinMode(_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_pin), flowISR, RISING);
}

float WaterFlowSensor::getFlowRate() {
  noInterrupts();
  unsigned int count = globalPulseCount;
  globalPulseCount = 0;
  interrupts();
  return count / 5.5; // L/min (flow >= 0.5)
}
