#include "SMSCommandHandler.h"
#include "Config.h"
#include "SoilSensor.h"
#include <EEPROM.h>

// Load system configuration struct
extern SystemConfig systemConfig;
extern SoilSensor soil;


SMSCommandHandler::SMSCommandHandler(){}
/**
 * Initialize password from saved config.
 * If missing or invalid, fall back to default.
 */
void SMSCommandHandler::begin() {}

/**
 * Save a new 4-digit password to the config and persist it.
 */
void SMSCommandHandler::savePassword(String newPass) {
  if (newPass.length() == 4) {
    systemConfig.password = newPass;
    saveConfig(systemConfig);
  }
}

/**
 * Entry point to handle an SMS message and route to the appropriate handler.
 */
void SMSCommandHandler::handle(String msg, GSMModule& gsm) {
  msg.trim();
  msg.replace(" ", "");
  if (msg.length() == 0) return;

  // Extract mode (s/i/f)
  int comma = msg.indexOf(',');
  if (comma == -1) return;
  String mode = msg.substring(0, comma);
  msg = msg.substring(comma + 1);

  // Extract and verify password
  comma = msg.indexOf(',');
  String providedPass;
  if (comma == -1) {
    providedPass = msg;
    msg = "";
  } else {
    providedPass = msg.substring(0, comma);
    msg = msg.substring(comma + 1);
  }

  if (providedPass != systemConfig.password) {
    gsm.sendSMS(systemConfig.phone, "Wrong password");
    return;
  }

  // Dispatch based on mode
  if (mode == "s") {
    handleSetting(msg, gsm);
  } else if (mode == "i") {
    handleIrrigation(msg, gsm);
  } else if (mode == "f") {
    handleFeedback(gsm);
  }
}


/**
 * Handles settings updates: password, phone number, notifications toggle.
 * Format: s,<pass>,<chg_pass>,<new_pass>,<chg_num>,<new_num>,<notif>,<reset>
 */
void SMSCommandHandler::handleSetting(String msg, GSMModule& gsm) {
  // Serial.println(msg);
  String parts[7];
  for (int i = 0; i < 7; i++) {
    int sep = msg.indexOf(',');
    if (sep == -1) {
      parts[i] = msg;
      msg = "";
    } else {
      parts[i] = msg.substring(0, sep);
      msg = msg.substring(sep + 1);
    }
  }

  // Serial.print("Part 1:");
  // Serial.println(parts[1]);

  if (parts[0] == "1" && parts[1].length() == 4) {
    savePassword(parts[1]);
  }
  // Serial.print("New Pass:");
  // Serial.println(systemConfig.password);

  if (parts[2] == "1" && parts[3].length() > 5) {
    strncpy(systemConfig.phone, parts[3].c_str(), sizeof(systemConfig.phone) - 1);
    systemConfig.phone[sizeof(systemConfig.phone) - 1] = '\0';
    Serial.println(systemConfig.phone);
  }


  if (parts[4] == "1") {
    systemConfig.notificationsEnabled = parts[4].toInt();
  }
  if (parts[4] == "0") {
    systemConfig.notificationsEnabled = parts[4].toInt();
  }
  if (parts[5] == "1") {
    gsm.sendSMS(systemConfig.phone, "System reset to default");
    resetConfig(systemConfig);
    return; 
  }

  saveConfig(systemConfig);
  gsm.sendSMS(systemConfig.phone, "Settings updated");
}

/**
 * Handles irrigation config and commands.
 * Format: i,<pass>,<chg_periods>,<periods>,<chg_length>,<length>,<chg_times>,<t1,t2,...>,<now>,<pause/resume>,<SoilMoisture_sensor(on/off)>
 */
void SMSCommandHandler::handleIrrigation(String msg, GSMModule& gsm) {
  // Serial.println(msg);

  String parts[11];

  for (int i = 0; i < 11; i++) {
    int sep = msg.indexOf(',');
    if (sep == -1) {
      parts[i] = msg;
      msg = "";
    } else {
      parts[i] = msg.substring(0, sep);
      msg = msg.substring(sep + 1);
    }
  }

  if (parts[0] == "1") systemConfig.periodsPerDay = parts[1].toInt();
  if (parts[2] == "1") systemConfig.periodLengthMin = parts[3].toInt();

  if (parts[4] == "1") {
    String times = parts[5];  // now in format: "6-12-18"
    int index = 0;
    while (times.length() > 0 && index < 10) {
      int sep = times.indexOf('-');
      if (sep == -1) sep = times.length();
      systemConfig.scheduledHours[index++] = times.substring(0, sep).toInt();
      times = (sep < times.length()) ? times.substring(sep + 1) : "";
    }
  }

  if (parts[6] == "1") {
    systemConfig.manualIrrigation = true;
    delay(1000);
    gsm.sendSMS(systemConfig.phone, "Manual irrigation requested");
  }

  if (parts[7] == "1") {
    systemConfig.isPaused = true;
    delay(1000);
    gsm.sendSMS(systemConfig.phone, "Irrigation Paused");
  } else if (parts[7] == "0") {
    systemConfig.isPaused = false;
    delay(1000);
    gsm.sendSMS(systemConfig.phone, "Irrigation Resumed");
  }

  if(parts[8] == "1"){
    systemConfig.soilMoistureSensor = true;
    delay(1000);
    gsm.sendSMS(systemConfig.phone, "Soil Moisture Sensor Enabled");
  }
  if(parts[8] == "0"){
    systemConfig.soilMoistureSensor = false;
    delay(1000);
    gsm.sendSMS(systemConfig.phone, "Soil Moisture Sensor Disabled");
  }

  saveConfig(systemConfig);
  // gsm.sendSMS(systemConfig.phone, "Settings updated");
}

/**
 * Responds with a configuration status summary.
 */
void SMSCommandHandler::handleFeedback(GSMModule& gsm) {
  char buffer[160];  // SMS max size is 160 characters
  char hoursBuffer[64] = "";
  char temp[6];

  for (int i = 0; i < systemConfig.periodsPerDay; i++) {
    itoa(systemConfig.scheduledHours[i], temp, 10);
    strcat(hoursBuffer, temp);
    if (i < systemConfig.periodsPerDay - 1) strcat(hoursBuffer, ",");
  }

  const char* notifStatus = systemConfig.notificationsEnabled ? "ON" : "OFF";
  const char* sysStatus   = systemConfig.isPaused ? "Paused" : "Running";
  const char* soilStatus;

  if (systemConfig.soilMoistureSensor) {
    int moisture = soil.read();
    soilStatus = (moisture > 400) ? "Dry" : "Wet";
  } else {
    soilStatus = "Disabled";
  }

  snprintf(
    buffer,
    sizeof(buffer),
    "Conf: %d periods, %d min, Notif: %s, Hours: %s, Sys_Status: %s, Soil Sensor: %s",
    systemConfig.periodsPerDay,
    systemConfig.periodLengthMin,
    notifStatus,
    hoursBuffer,
    sysStatus,
    soilStatus
  );
  
  gsm.sendSMS(systemConfig.phone, buffer);
}

