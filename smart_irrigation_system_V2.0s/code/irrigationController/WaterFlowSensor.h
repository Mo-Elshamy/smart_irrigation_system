#ifndef WATER_FLOW_SENSOR_H
#define WATER_FLOW_SENSOR_H

class WaterFlowSensor {
  public:
    WaterFlowSensor(int pin);
    void begin();
    void pulseCounter();
    float getFlowRate();
  private:
    int _pin;
    volatile unsigned int pulseCount;
    unsigned long lastMeasurement;
};
#endif
