#ifndef GSM_MODULE_H
#define GSM_MODULE_H

#include <SoftwareSerial.h>


class GSMModule {
  public:
    GSMModule();
    void begin();
    void sendSMS(const char* number, const char* message);
    String readSMS();
    bool waitForResponse(const char* expected, unsigned long timeout);
    bool isGSMResponsive();
    void powerOnGSMIfNeeded();
    void flushBuffer();
    void sendRawCommand(const String& cmd);
    String readRawResponse(unsigned long timeout = 3000);
  private:
    SoftwareSerial gsm;
};
#endif
