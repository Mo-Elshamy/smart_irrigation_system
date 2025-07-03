#include <EEPROM.h>
#include "Config.h"
#include <string.h>



void loadConfig(SystemConfig& config) {
  int offset = 0;
  EEPROM.get(offset, config.periodsPerDay);
  EEPROM.get(offset + 2, config.periodLengthMin);
  EEPROM.get(offset + 4, config.isPaused);
  EEPROM.get(offset + 5, config.manualIrrigation);
  EEPROM.get(offset + 6, config.notificationsEnabled);
  EEPROM.get(offset + 46, config.soilMoistureSensor);
  for (int i = 0; i < 10; i++) {
    EEPROM.get(offset + 7 + i * 2, config.scheduledHours[i]);
  }
  
  // Load phone number (15 chars)
  char phoneBuf[14];
  for (int i = 0; i < 13; i++) {
    phoneBuf[i] = EEPROM.read(offset + 27 + i);
  }
  phoneBuf[14] = '\0';
  strncpy(config.phone, phoneBuf, sizeof(config.phone) - 1);
  config.phone[sizeof(config.phone) - 1] = '\0';  // Ensure null-termination

  if (phoneBuf[0] < '0' || phoneBuf[0] > '9') {
    strncpy(config.phone, "+201119760999", sizeof(config.phone));
    config.phone[sizeof(config.phone) - 1] = '\0';
  }

  // Load password (4 chars)
  char passBuf[5];
  for (int i = 0; i < 4; i++) {
    passBuf[i] = EEPROM.read(offset + 42 + i);
  }
  passBuf[4] = '\0';
  config.password = String(passBuf);



  // Set defaults if uninitialized or invalid
  if (config.isPaused != true && config.isPaused != false ){
    config.isPaused = false;
  }
  if (config.manualIrrigation != true && config.manualIrrigation != false ){
    config.manualIrrigation = false;
  }
  if (config.notificationsEnabled != true && config.notificationsEnabled != false ){
    config.notificationsEnabled = true;
  }
  if (config.periodsPerDay < 1 || config.periodsPerDay > 10) {
    config.periodsPerDay = 2;
    config.scheduledHours[0] = 6;
    config.scheduledHours[1] = 18;
  }
  if (config.periodLengthMin < 1 || config.periodLengthMin > 60) {
    config.periodLengthMin = 10;
  }
  if (config.password.length() != 4) {
    config.password = "0000";
  }
  if (config.soilMoistureSensor != true && config.soilMoistureSensor != false) {
    config.soilMoistureSensor = false;
  }


  // Serial.println("Config:");
  // Serial.print("periodsPerDay = ");
  // Serial.println(config.periodsPerDay);
  // Serial.print("periodLengthMin =");
  // Serial.println(config.periodLengthMin);
  // Serial.print("isPaused = ");
  // Serial.println(config.isPaused);
  // Serial.print("manualIrrigation = ");
  // Serial.println(config.manualIrrigation);
  // Serial.print("notificationsEnabled = ");
  // Serial.println(config.notificationsEnabled);
  // Serial.print("soilMoistureSensor = ");
  // Serial.println(config.soilMoistureSensor);
  // Serial.print("scheduledHours = ");
  // for(auto i : config.scheduledHours){
  //   Serial.print(i);
  //   Serial.print(",");
  // }
  // Serial.println("");

  // Serial.print("Phone = ");
  // Serial.println(config.phone);
  // Serial.print("Pass = ");
  Serial.println(config.password);

  // Serial.println("Loaded Sucess");
}

void saveConfig(SystemConfig& config) {
  int offset = 0;
  EEPROM.put(offset, config.periodsPerDay);
  EEPROM.put(offset + 2, config.periodLengthMin);
  EEPROM.put(offset + 4, config.isPaused);
  EEPROM.put(offset + 5, config.manualIrrigation);
  EEPROM.put(offset + 6, config.notificationsEnabled);
  EEPROM.put(offset + 46, config.soilMoistureSensor);

  for (int i = 0; i < 10; i++) {
    EEPROM.put(offset + 7 + i * 2, config.scheduledHours[i]);
  }

  // Save phone
  for (int i = 0; i < 15 && i < strlen(config.phone); i++) {
    EEPROM.write(offset + 27 + i, config.phone[i]);
  }

  // Save password
  for (int i = 0; i < 4 && i < config.password.length(); i++) {
    EEPROM.write(offset + 42 + i, config.password[i]);
    // Serial.println("Pass:");
  }
  // Serial.println("Pass:");
  // Serial.println(config.password);

}

void resetConfig(SystemConfig& config) {
  config.periodsPerDay = 2;
  config.periodLengthMin = 10;
  config.isPaused = false;
  config.manualIrrigation = false;
  config.notificationsEnabled = true;
  config.soilMoistureSensor = false;
  config.scheduledHours[0] = 6;
  config.scheduledHours[1] = 18;
  for (int i = 2; i < 10; i++) config.scheduledHours[i] = 0;
  config.phone[0] = '\0';  
  config.password = "0000";

  saveConfig(config);
  // Serial.println("Reset Sucess");

}
