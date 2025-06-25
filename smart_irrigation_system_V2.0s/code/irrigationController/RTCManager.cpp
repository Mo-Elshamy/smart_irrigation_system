#include "RTCManager.h"
#include <SoftwareSerial.h>

RTCManager::RTCManager() {}

void RTCManager::begin() {
  rtc.begin();
  while(! rtc.begin()){
    Serial.println("Couldn't find RTC");
    Serial.flush();
    delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, trying to sync from GSM");
    if (!syncFromGSM()) {
      Serial.println("Failed to sync from GSM, using compile time");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }
}

DateTime RTCManager::now() {
  return rtc.now();
}

bool RTCManager::syncFromGSM() {
  SoftwareSerial gsmSerial(12, 11); // RX, TX - adjust pins as needed
  gsmSerial.begin(9600);

  gsmSerial.println("AT+CCLK?");
  delay(1000);
  String response = "";

  unsigned long start = millis();
  while (millis() - start < 2000) {
    if (gsmSerial.available()) {
      response += gsmSerial.readString();
    }
  }

  int idx = response.indexOf("+CCLK: \"");
  if (idx == -1) return false;
  response = response.substring(idx + 8);
  int endIdx = response.indexOf('"');
  if (endIdx == -1) return false;
  response = response.substring(0, endIdx);

  int yy = response.substring(0, 2).toInt() + 2000;
  int MM = response.substring(3, 5).toInt();
  int dd = response.substring(6, 8).toInt();
  int hh = response.substring(9, 11).toInt();
  int mm = response.substring(12, 14).toInt();
  int ss = response.substring(15, 17).toInt();

  rtc.adjust(DateTime(yy, MM, dd, hh, mm, ss));
  return true;
}
