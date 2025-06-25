#ifndef GSM_MODULE_H
#define GSM_MODULE_H

#include <SoftwareSerial.h>


class GSMModule {
  public:
    GSMModule();
    void begin();
    void sendSMS(const char* number, const char* message);
    String readSMS();
  private:
    SoftwareSerial gsm;
};
#endif
