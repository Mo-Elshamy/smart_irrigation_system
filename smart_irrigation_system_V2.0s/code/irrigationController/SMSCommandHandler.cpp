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
void SMSCommandHandler::begin() {
  // Load password from config
  if (systemConfig.password.length() != 4) {
    // Default password if not set
    systemConfig.password = "0000";
    saveConfig(systemConfig);
  }
}

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
  if (comma == -1) return;
  String providedPass = msg.substring(0, comma);
  msg = msg.substring(comma + 1);

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
 * Format: s,<chg_pass>,<new_pass>,<chg_num>,<new_num>,<notif>
 */
void SMSCommandHandler::handleSetting(String msg, GSMModule& gsm) {
  String parts[6];
  for (int i = 0; i < 6; i++) {
    int sep = msg.indexOf(',');
    if (sep == -1) {
      parts[i] = msg;
      msg = "";
    } else {
      parts[i] = msg.substring(0, sep);
      msg = msg.substring(sep + 1);
    }
  }

  if (parts[0] == "1" && parts[1].length() == 4) {
    savePassword(parts[1]);
  }

  if (parts[2] == "1" && parts[3].length() > 5) {
    strncpy(systemConfig.phone, parts[3].c_str(), sizeof(systemConfig.phone) - 1);
    systemConfig.phone[sizeof(systemConfig.phone) - 1] = '\0';
  }

  if (parts[4].length() == 1) {
    systemConfig.notificationsEnabled = parts[4].toInt();
  }

  saveConfig(systemConfig);
  gsm.sendSMS(systemConfig.phone, "Settings updated");
}

/**
 * Handles irrigation config and commands.
 * Format: i,<chg_periods>,<periods>,<chg_length>,<length>,<chg_times>,<t1,t2,...>,<now>,<pause/resume>,<SoilMoisture_sensor(on/off)>
 */
void SMSCommandHandler::handleIrrigation(String msg, GSMModule& gsm) {
  String parts[10];
  for (int i = 0; i < 10; i++) {
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
    String times = parts[5];
    int index = 0;
    while (times.length() > 0 && index < 10) {
      int sep = times.indexOf(',');
      if (sep == -1) sep = times.length();
      systemConfig.scheduledHours[index++] = times.substring(0, sep).toInt();
      times = times.substring(sep + 1);
    }
  }

  if (parts[6] == "1") {
    systemConfig.manualIrrigation = true;
    gsm.sendSMS(systemConfig.phone, "Manual irrigation requested");
  }

  if (parts[7] == "1") {
    systemConfig.isPaused = true;
    gsm.sendSMS(systemConfig.phone, "Irrigation Paused");
  } else if (parts[7] == "0") {
    systemConfig.isPaused = false;
    gsm.sendSMS(systemConfig.phone, "Irrigation Resumed");
  }

  if(parts[8] == "1"){
    systemConfig.soilMoistureSensor = true;
    gsm.sendSMS(systemConfig.phone, "Soil Moisture Sensor Enabled");
  }
  if(parts[8] == "0"){
    systemConfig.soilMoistureSensor = false;
    gsm.sendSMS(systemConfig.phone, "Soil Moisture Sensor Disabled");
  }

  saveConfig(systemConfig);
}

/**
 * Responds with a configuration status summary.
 */
void SMSCommandHandler::handleFeedback(GSMModule& gsm) {
  char buffer[160];
  String hours = "";
  for (int i = 0; i < systemConfig.periodsPerDay; i++) {
    hours += String(systemConfig.scheduledHours[i]);
    if (i < systemConfig.periodsPerDay - 1) hours += ",";
  }

  sprintf(
    buffer,
    "Conf: %d periods, %d min, Notif: %s, Hours: %s, Sys_Status: %s, Soil Sensor status: %s",
    systemConfig.periodsPerDay,
    systemConfig.periodLengthMin,
    systemConfig.notificationsEnabled ? "ON" : "OFF",
    hours.c_str(),
    systemConfig.isPaused ? "Paused" : "Running" ,
    systemConfig.soilMoistureSensor ? (soil.read() > 400 ? "Dry" : "Wet"):"Disabled"
  );
  gsm.sendSMS(systemConfig.phone, buffer);
}
