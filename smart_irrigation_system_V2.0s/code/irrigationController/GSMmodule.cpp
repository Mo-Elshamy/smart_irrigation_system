#include "GSMModule.h"
#include <Arduino.h>


GSMModule::GSMModule() : gsm(12, 11) {}  // Initialize SoftwareSerial in constructor


void GSMModule::begin() {
  gsm.begin(9600);
  delay(1000);

  // Power on the module
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);
  delay(1000); // PWRKEY needs to be low for at least 1 second
  digitalWrite(5, HIGH);
  delay(5000); // Wait for the module to initialize

  // Wait for SIM to be ready
  gsm.println("AT");                // Check module
  if(waitForResponse("OK", 1000) == false){
    pinMode(5, OUTPUT);
    digitalWrite(5, LOW);
    delay(1000); // PWRKEY needs to be low for at least 1 second
    digitalWrite(5, HIGH);
    delay(5000); // Wait for the module to initialize
  }

  gsm.println("AT+CMGF=1");         // Set SMS to text mode
  waitForResponse("OK", 1000);
  gsm.println("AT+CPMS=\"SM\"");    // Use SIM card storage
  waitForResponse("OK", 1000);

  // gsm.println("AT+CMGL=\"ALL\"");
  // waitForResponse("OK", 8000);
  
  while ( gsm.available()) {
      Serial.write(gsm.read());
  }
  // Serial.println("GSM Activated");
}

void GSMModule::sendSMS(const char* number, const char* message) {
  gsm.println("AT+CMGF=1"); // Set text mode
  waitForResponse("OK", 5000);

  gsm.print("AT+CMGS=\"");
  gsm.print(number);
  gsm.println("\"");
  delay(1000);

  // if (!waitForResponse(">", 5000)) {
  //   Serial.println("No prompt for message body");
  //   return;
  // }

  gsm.print(message);
  delay(1000);
  gsm.write(26); // Send Ctrl+Z to finish message
  // if (waitForResponse("OK", 10000)) {
  //    Serial.println("SMS sent successfully");
  // } else {
  //    Serial.println("Failed to send SMS");
  // }
}

String GSMModule::readSMS() {

  gsm.println("AT+CPMS=\"MT\""); // MT = combined ME + SM
  waitForResponse("OK", 2000);
  gsm.println("AT+CMGF=1");
  waitForResponse("OK", 500);
  gsm.println("AT+CMGL=\"REC UNREAD\"");

  String response = "";
  unsigned long lastCharTime = millis();
  const unsigned long timeout = 10000;

  while (millis() - lastCharTime < timeout) {
    while (gsm.available()) {
      response += char(gsm.read());
      lastCharTime = millis();
    }
  }

  // DEBUG
  // Serial.println("Raw GSM response:");
  Serial.println(response);

  // Find first message body
  int cmglIndex = response.indexOf("+CMGL:");
  if (cmglIndex == -1) return "";

  // Find the message content
  int msgStart = response.indexOf("\n", cmglIndex);       // End of metadata
  int msgEnd = response.indexOf("\n", msgStart + 1);       // End of message line
  if (msgStart == -1 || msgEnd == -1) return "";

  String message = response.substring(msgStart + 1, msgEnd);
  message.trim();

  // Delete the message after reading
  int commaIndex = response.indexOf(',', cmglIndex);
  if (commaIndex != -1) {
    String msgIndexStr = response.substring(cmglIndex + 7, commaIndex);
    int msgIndex = msgIndexStr.toInt();
    gsm.print("AT+CMGD=");
    gsm.println(msgIndex);
    waitForResponse("OK", 1000);
  }

  return message;
}



bool GSMModule::waitForResponse(const char* expected, unsigned long timeout) {
  unsigned long start = millis();
  String response = "";
  while (millis() - start < timeout) {
    while (gsm.available()) {
      response += char(gsm.read());
      if (response.indexOf(expected) != -1) {
        return true;
      }
    }
  }
  return false;
}

void GSMModule::flushBuffer() {
  while (gsm.available()) gsm.read();
}

void GSMModule::sendRawCommand(const String& cmd) {
  gsm.println(cmd);
}

String GSMModule::readRawResponse(unsigned long timeout) {
  String response = "";
  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (gsm.available()) {
      response += char(gsm.read());
    }
  }
  return response;
}

