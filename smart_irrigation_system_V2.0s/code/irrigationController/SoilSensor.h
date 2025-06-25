#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

class SoilSensor {
  public:
    SoilSensor(int pin);
    int read();
  private:
    int _pin;
};
#endif