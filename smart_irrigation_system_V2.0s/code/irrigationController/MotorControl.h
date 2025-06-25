#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

class MotorControl {
    private:
        int _pin;
    public:
    MotorControl(int pin);
    void MotorOn();
    void MotorOff();
};
#endif
