#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H
#include <RTClib.h>

class RTCManager {
  public:
    RTCManager();
    void begin();
    DateTime now();
    bool syncFromGSM();
  private:
    RTC_DS3231 rtc;
};
#endif
