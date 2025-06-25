#ifndef SMS_COMMAND_HANDLER_H
#define SMS_COMMAND_HANDLER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "GSMmodule.h"

class SMSCommandHandler {
  public:
    SMSCommandHandler();
    void begin();
    void handle(String msg, GSMModule& gsm);

  private:
    String currentPass;
    bool notificationEnabled;


    void handleSetting(String msg, GSMModule& gsm);
    void handleIrrigation(String msg, GSMModule& gsm);
    void handleFeedback(GSMModule& gsm);
    void savePassword(String newPass);

};

#endif