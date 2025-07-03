#include "RTCManager.h"
#include "GSMModule.h" // Required for gsm.gsm


RTCManager::RTCManager() {}

void RTCManager::begin(GSMModule& gsm) {
  rtc.begin();
  while(! rtc.begin()){
    Serial.println("Couldn't find RTC");
    delay(500);
  }
  delay(5000);
  syncFromGSM(gsm);
  while(!syncFromGSM(gsm)){
    Serial.println("Cant sync from gsm! trying again");
    delay(5000);
  }
  // if (rtc.lostPower()) {
  //   Serial.println("RTC lost power, trying to sync from GSM");
  //   if (!syncFromGSM(gsm)) {
  //     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //   }
  // }
  // else {
  //   Serial.println("✅ RTC synced from GSM successfully.");
  // }
  // Serial.println("RTC Activated");

}
DateTime RTCManager::now() {
  return rtc.now();
}


bool RTCManager::syncFromGSM(GSMModule& gsmModule) {
  gsmModule.sendRawCommand("AT+CCLK?");
  delay(1000);

  String response = gsmModule.readRawResponse(3000);
  // Serial.println("GSM response: " + response);

  int idx = response.indexOf("+CCLK: \"");
  if (idx >= 0) {
    response = response.substring(idx + 8);
    int endIdx = response.indexOf('"');
    if (endIdx == -1) return false;
    response = response.substring(0, endIdx);
    // Serial.println("Extracted +CCLK: " + response);

    int yy = response.substring(0, 2).toInt() + 2000;
    int MM = response.substring(3, 5).toInt();
    int dd = response.substring(6, 8).toInt();
    int hh = response.substring(9, 11).toInt();
    int mm = response.substring(12, 14).toInt();
    int ss = response.substring(15, 17).toInt();

    rtc.adjust(DateTime(yy, MM, dd, hh, mm, ss));
    return true;
  }

  // Fallback to *PSUTTZ:
  idx = response.indexOf("*PSUTTZ:");
  if (idx >= 0) {
    response = response.substring(idx + 9);
    response.trim();

    int yy = response.substring(0, 4).toInt();
    int MM = response.substring(5, 7).toInt();
    int dd = response.substring(8, 10).toInt();
    int hh = response.substring(11, 13).toInt();
    int mm = response.substring(14, 16).toInt();
    int ss = response.substring(17, 19).toInt();

    rtc.adjust(DateTime(yy, MM, dd, hh, mm, ss));
    // Serial.println("RTC updated from *PSUTTZ.");
    return true;
  }

  // Serial.println("Failed to sync from GSM.");
  return false;
}
