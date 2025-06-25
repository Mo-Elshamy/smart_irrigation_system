#include "MotorControl.h"
#include <Arduino.h>

MotorControl::MotorControl(int pin) : _pin(pin){
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin,0);
}

void MotorControl::MotorOn(){
    digitalWrite(_pin,1);
}

void MotorControl::MotorOff(){
    digitalWrite(_pin,0);
}