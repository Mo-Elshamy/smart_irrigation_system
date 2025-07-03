#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H
#include <RTClib.h>
#include "GSMModule.h" // Required for gsm.gsm


class RTCManager {
  public:
    RTCManager();
    void begin(GSMModule& gsm);
    DateTime now();
    bool syncFromGSM(GSMModule& gsm);
  private:
    RTC_DS3231 rtc;
};
#endif
