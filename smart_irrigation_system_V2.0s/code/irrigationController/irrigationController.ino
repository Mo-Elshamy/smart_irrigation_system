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
const int valvePin = 4;
bool success = false ;



// Function declarations
bool soilMoisture_check();
bool flowRate_Check(WaterFlowSensor& flow, MotorControl& pump);
void Watering(MotorControl& pump, WaterFlowSensor& flow, RTCManager& rtc, SystemConfig& systemConfig ,GSMModule& gsm);
void readMsg(GSMModule& gsm, SystemConfig& systemConfig, SMSCommandHandler& smsHandler);

void setup() {
  Serial.begin(9600);
  pinMode(valvePin, OUTPUT);
  gsm.begin();
  rtc.begin(gsm);
  flow.begin();
  smsHandler.begin();
  loadConfig(systemConfig);
  DateTime now = rtc.now();
  String msg = "Irrigation system initialized." + String(now.hour())+":" + String(now.minute()) + "," + String(now.day()) + "/" + String(now.month())  +"/" + String(now.year());
  gsm.sendSMS(systemConfig.phone, msg.c_str() );
}

void loop() {
  readMsg(gsm, systemConfig, smsHandler);
  DateTime now = rtc.now(); 
  // Serial.println("Looping");
  // while(1){
  //   //   pump.MotorOn();
  //   // float rate = flow.getFlowRate();
  //   // if (rate != -1) {
  //   //   Serial.print("Flow rate: ");
  //   //   Serial.println(rate);
  //   // }  
  // //   delay(5000);
  // //   pump.MotorOff();
  // //   delay(5000);
  //  }
  for (int i = 0; i < systemConfig.periodsPerDay; i++) {
    if (now.hour() == systemConfig.scheduledHours[i] && now.minute() == 0  && (systemConfig.soilMoistureSensor ? soilMoisture_check() : true) && systemConfig.isPaused == false) { 
      if (flowRate_Check(flow, pump)) {
        Watering(pump, flow, rtc, systemConfig , gsm );
      } 
      else {
        int retryCount = 0;
        while (retryCount < 3) {

          int delayCount = 0;
          while(delayCount < 10){   // 10 min  
            delay(60000);   
            delayCount++;       
          }
          if (flowRate_Check(flow, pump)) {
            Watering(pump, flow, rtc, systemConfig , gsm);
            break;
          }
          retryCount++;
        }

        if (retryCount == 3 && systemConfig.notificationsEnabled) {
          Serial.println("No water after retries");
          delay(30000);
          // gsm.flushBuffer();  // Safely flush buffer
          char buffer[160];
          sprintf(buffer, "Irrigation failed: No water flow after retries.,%d:%d,%d/%d/%d",
          now.hour(), now.minute(), now.day(), now.month(), now.year());
          gsm.sendSMS(systemConfig.phone, buffer);
        }
      }
    }
    // readMsg(gsm, systemConfig, smsHandler);
    // if(systemConfig.isPaused  && systemConfig.notificationsEnabled){
    //   gsm.sendSMS(systemConfig.phone, "Irrigation failed: System Paused");
    // }
    // Serial.println("Waiting");

  }
  delay(6000); //6 sec
}

bool soilMoisture_check() {
  int moisture = soil.read();
  return (moisture > 400);
}

bool flowRate_Check(WaterFlowSensor& flow, MotorControl& pump) {
  // Serial.println("Checking flow rate");
  pump.MotorOn();
  digitalWrite(valvePin, HIGH);
  delay(20000);
  float flowRate = flow.getFlowRate();
  pump.MotorOff();
  digitalWrite(valvePin, LOW);
  return flowRate > 0.5;
}

void Watering(MotorControl& pump, WaterFlowSensor& flow, RTCManager& rtc, SystemConfig& systemConfig , GSMModule& gsm ) {
  DateTime now = rtc.now();
  // float flowRate=0;
  pump.MotorOn();
  digitalWrite(valvePin, HIGH);
  delay(20000);
  unsigned long start = millis();
  // flowRate= flow.getFlowRate();
  while (millis() - start < systemConfig.periodLengthMin * 60000) {
    float flowRate = flow.getFlowRate();  // Call only once
    // Serial.print("Flow rate: ");
    // Serial.println(flowRate);

    if (flowRate <= 0.1) {
      // Serial.println("Flow stopped or too low.");
      success = false;
      break;
    }
    delay(10000);
    success = true;
  }
  pump.MotorOff();
  digitalWrite(valvePin, LOW);

  if (systemConfig.notificationsEnabled) {
    // Serial.println("Sending msg:");
    if (success) { 
      delay(60000);
      String msg = "Irrigation successful:" + String(now.hour())+":" + String(now.minute()) + "," + String(now.day()) + "/" + String(now.month())  +"/" + String(now.year());
      gsm.sendSMS(systemConfig.phone, msg.c_str());
    } else {
      delay(60000);
      String msg = "Irrigation stopped early: no flow," + String(now.hour())+":" + String(now.minute()) + "," + String(now.day()) + "/" + String(now.month())  +"/" + String(now.year());
      gsm.sendSMS(systemConfig.phone, msg.c_str());
    }
  }
}

void readMsg(GSMModule& gsm, SystemConfig& systemConfig, SMSCommandHandler& smsHandler){
  String msg = gsm.readSMS();
  // Serial.println("Reading msg");
  // Serial.println(msg);
  // Serial.println(msg.length());

  if (msg.length() > 0) {
    smsHandler.handle( msg, gsm );
    saveConfig(systemConfig);
  }

  if (systemConfig.manualIrrigation) {
    Watering(pump, flow, rtc, systemConfig , gsm);
    systemConfig.manualIrrigation = false;
    delay(1000);
    saveConfig(systemConfig);
  }
}