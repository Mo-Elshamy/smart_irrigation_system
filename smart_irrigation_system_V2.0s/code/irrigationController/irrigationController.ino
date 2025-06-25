#include <Adafruit_SSD1306.h>
#include <splash.h>

// Include required libraries and modules
#include <EEPROM.h>
#include "RTCManager.h"
#include "SoilSensor.h"
#include "WaterFlowSensor.h"
#include "GSMModule.h"
#include "MotorControl.h"
#include "Config.h"
#include "SMSCommandHandler.h"

// Instantiate hardware interfaces
RTCManager rtc;
SoilSensor soil(A0);
WaterFlowSensor flow(2);
GSMModule gsm;
MotorControl pump(3);
SMSCommandHandler smsHandler;
// System configuration struct instance
SystemConfig systemConfig;

// Define pin
const int valvePin = 5;



// Function declarations
bool soilMoisture_check();
bool flowRate_Check(WaterFlowSensor& flow, MotorControl& pump);
void Watering(MotorControl& pump, WaterFlowSensor& flow, RTCManager& rtc, int durationMin);
void readMsg(GSMModule& gsm, SystemConfig& systemConfig, SMSCommandHandler& smsHandler);

void setup() {
  Serial.begin(9600);
  pinMode(valvePin, OUTPUT);
  rtc.begin();
  flow.begin();
  gsm.begin();
  smsHandler.begin();
  loadConfig(systemConfig);
  
  gsm.sendSMS(systemConfig.phone, "Irrigation system initialized.");
}

void loop() {
  readMsg(gsm, systemConfig, smsHandler);
  DateTime now = rtc.now();
  for (int i = 0; i < systemConfig.periodsPerDay; i++) {
    if (now.hour() == systemConfig.scheduledHours[i] && now.minute() == 0 && (systemConfig.soilMoistureSensor ? soilMoisture_check() : true) && systemConfig.isPaused == false) {
      if (flowRate_Check(flow, pump)) {
        Watering(pump, flow, rtc, systemConfig.periodLengthMin);
      } 
      else {
        int retryCount = 0;
        while (retryCount < 3) {
          delay(10 * 60 * 1000);         // 10 min
          if (flowRate_Check(flow, pump)) {
            Watering(pump, flow, rtc, systemConfig.periodLengthMin);
            break;
          }
          retryCount++;
        }
        if (retryCount == 3 && systemConfig.notificationsEnabled) {
          gsm.sendSMS(systemConfig.phone, sprintf("Irrigation failed: No water flow after retries.,%d:%d,%d/%d/%d",now.hour(),now.minute(),now.day(),now.month(),now.year()));
        }
      }
    }
    readMsg(gsm, systemConfig, smsHandler);
    if(systemConfig.isPaused == false && systemConfig.notificationsEnabled){
      gsm.sendSMS(systemConfig.phone, "Irrigation failed: System Paused");
    }
  }
  delay(60000); //60 sec
}

bool soilMoisture_check() {
  int moisture = soil.read();
  return (moisture > 400);
}

bool flowRate_Check(WaterFlowSensor& flow, MotorControl& pump) {
  pump.MotorOn();
  digitalWrite(valvePin, HIGH);
  delay(5000);
  float flowRate = flow.getFlowRate();
  pump.MotorOff();
  digitalWrite(valvePin, LOW);
  return flowRate > 0.5;
}

void Watering(MotorControl& pump, WaterFlowSensor& flow, RTCManager& rtc, int durationMin) {
  DateTime now = rtc.now();
  float flowRate=0;
  pump.MotorOn();
  digitalWrite(valvePin, HIGH);
  unsigned long start = millis();
  flowRate= flow.getFlowRate();
  while (millis() - start < durationMin * 60000 && flow.getFlowRate() > 0.5) {
    delay(10000);
  }
  pump.MotorOff();
  digitalWrite(valvePin, LOW);

  if (systemConfig.notificationsEnabled) {
    if (millis() - start >= durationMin * 60000) {
      gsm.sendSMS(systemConfig.phone, sprintf("Irrigation successful,%d:%d,%d/%d/%d,%f,",now.hour(),now.minute(),now.day(),now.month(),now.year(),flowRate));
    } else {
      gsm.sendSMS(systemConfig.phone, sprintf("Irrigation stopped early: no flow,%d:%d,%d/%d/%d",now.hour(),now.minute(),now.day(),now.month(),now.year()));
    }
  }
}

void readMsg(GSMModule& gsm, SystemConfig& systemConfig, SMSCommandHandler& smsHandler){
  String msg = gsm.readSMS();
  if (msg.length() > 0) {
    smsHandler.handle( msg, gsm );
    saveConfig(systemConfig);
  }

  if (systemConfig.manualIrrigation) {
    Watering(pump, flow, rtc, systemConfig.periodLengthMin);
    systemConfig.manualIrrigation = false;
    saveConfig(systemConfig);
  }
}