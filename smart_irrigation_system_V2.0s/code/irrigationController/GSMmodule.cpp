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
  gsm.println("AT");
  delay(200);
  gsm.println("AT+CPIN?");
  delay(500);

  // Set SMS mode to text
  gsm.println("AT+CMGF=1");
  delay(200);

  // Test AT communication
  gsm.println("AT");
  delay(1000);
  while ( gsm.available()) {
      Serial.write(gsm.read());
  }
}

void GSMModule::sendSMS(const char* number, const char* message) {
  gsm.println("AT+CMGF=1");         // Text mode
  delay(200);

  gsm.print("AT+CMGS=\"");
  gsm.print(number);
  gsm.println("\"");
  delay(200);

  gsm.print(message);
  delay(200);

  gsm.write(26); // Ctrl+Z to send
  delay(5000);   // Wait for message to send
}


String GSMModule::readSMS() {
  gsm.println("AT+CMGF=1");
  delay(100);
  gsm.println("AT+CMGL=\"REC UNREAD\"");
  delay(1000);

  String message = "";
  while (gsm.available()) {
    message += char(gsm.read());
  }

  int start = message.indexOf("\n", message.indexOf("+CMGL"));
  int end = message.indexOf("\n", start + 1);
  if (start > 0 && end > start) {
    return message.substring(start + 1, end);
  }

  return "";
}