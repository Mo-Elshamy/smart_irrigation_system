#ifndef CONFIG_H
#define CONFIG_H

#define EEPROM_MAGIC 0x42
#include <Arduino.h>

struct SystemConfig  {
  uint8_t magic = EEPROM_MAGIC;
  int periodsPerDay;
  int periodLengthMin;
  int scheduledHours[10];
  bool isPaused;
  bool manualIrrigation;
  bool notificationsEnabled;
  bool soilMoistureSensor;
  char phone[14];
  String password;
};

// Global config instance
extern SystemConfig systemConfig;

// Save/load declarations
void saveConfig(SystemConfig& config);
void loadConfig(SystemConfig& config);
void resetConfig(SystemConfig& config);
#endif
