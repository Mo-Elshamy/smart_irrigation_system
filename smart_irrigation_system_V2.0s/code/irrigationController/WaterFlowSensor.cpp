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
  static unsigned long lastCheck = 0;
  static unsigned int lastCount = 0;

  unsigned long now = millis();
  if (now - lastCheck < 2000) {
    return -1; // not enough time passed
  }

  noInterrupts();
  unsigned int count = globalPulseCount;
  globalPulseCount = 0;
  interrupts();

  lastCheck = now;
  float flow = count / 5.5 / 2.0; // divide by 2 seconds
  return flow; // in L/min
}
