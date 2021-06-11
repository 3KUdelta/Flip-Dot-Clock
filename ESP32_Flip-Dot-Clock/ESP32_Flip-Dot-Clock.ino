// **************************************************************************
// flipdotFlipdot-clock-m-v1
// 2020-05-01
//
// c by Rainer Radow, 30171 Hannover, Germany
// radow.org
// This file is for your ESP32
// Pairing file on Nano: Nano_Flip-Dot-Clock.ino
// or for the silent version use: Nano_Flip-Dot-Clock_silent.ino
//
// June 2021: addons by Marc Stähli
// - precise time fetch via NTP on connected ESP32
// - automatic DST conversion (see Settings.h) for any place in the world
// - serial transfer of data from this sketch
// - sending status messages -> 1 = success, 2 = no WiFi, 3 = no NTP
// - indicated on matrix: top left dot: no Wifi, top right dot: no NTP
//
// At startup the ESP32 keeps the (blue) LED on as long as there is no WiFi connection.
//
// To set your WiFi credentials for the first time or if WiFi changes:
//   Open WLAN and connect to "FlipDot_Clock_AP"
//   captive portal should open automatically to set your credentials
//   if not, enter browser on 192.168.4.1
//
// Arduino settings: WEMOS LOLIN32 (best experience with these settings)
//
// **************************************************************************

#include "Settings.h"
#include <WiFi.h>                           // WiFI
#include <WiFiUdp.h>                        // For NTP Signal fetch
#include <NTPClient.h>                      // For NTP time signal fetch https://github.com/arduino-libraries/NTPClient
#include <TimeLib.h>                        // For converting NTP time https://github.com/PaulStalloffregen/Time.git
#include <WiFiManager.h>                    // https://github.com/tzapu/WiFiManager
#include <AsciiMassagePacker.h>             // https://github.com/SofaPirate/AsciiMassage

WiFiManager wm;                                                  // initializing WiFi Manager
WiFiUDP udp;                                                     // WiFi UDP initialization for NTP
NTPClient ntpClient(udp, NTP_SERVER, TZ_SEC, NTP_INTERVAL);      // NTP initialization
AsciiMassagePacker outbound;                                     // transmission initialization

unsigned long interval;

void setup() {

  Serial.begin(9600); while (!Serial); delay(200);
  Serial2.begin(9600);                                           // going low speed, simple data transmission

  Serial.println();
  Serial.println("Starting Time Fetcher Flip-Dot Clock...");

  pinMode(2, OUTPUT);                              // initialize onboard LED

  //************************************************************************************************
  // wm.resetSettings();                     // for debugging puposes: erase wifi credentials
  //************************************************************************************************

  wm.setConfigPortalBlocking(true);         // setiing WiFi manager to blocking mode
  wm.setCaptivePortalEnable(true);          // setting WiFi manager to activate captive portal

  WiFi.mode(WIFI_STA);                      // going online using Wifi Manager
  WiFi.hostname("Flip_Dot_Timefetcher");
  Serial.print("---> Connecting to WiFi ");
  digitalWrite(2, HIGH);                    // indicate WiFi access with LED
  outbound.beginPacket("update");           // package identifier
  outbound.addByte(2);                      // no WiFi message
  outbound.addLong(0);                      // no time update - send 0
  outbound.streamPacket(&Serial2);          // send package via Serial2

  wm.autoConnect("FlipDot_Clock_AP");       // doing some magic: using WiFi manager (blocking mode)
  digitalWrite(2, LOW);                     // LED off because we are online

  ntpClient.begin();
  interval = millis();
}

void loop() {

  if (millis() - interval > NTP_INTERVAL) {
    if (ntpClient.update()) {
      Serial.println("---> Now reading time from NTP Server");
      setTime(ntpClient.getEpochTime());                        // set systemtime in ESP32 to UTC fetched from NTP
      Serial.println("NTP read success");

      /*Serial.print("UTC: ");
        Serial.print(hour(now()));
        Serial.print(":");
        Serial.println(minute(now()));
        Serial.print("Local time: ");
        Serial.print(hour(CE.toLocal(now(), &tcr)));
        Serial.print(":");
        Serial.println(minute(CE.toLocal(now(), &tcr))); */

      digitalWrite(2, HIGH);                                  // indicate transmission with LED
      outbound.beginPacket("update");                         // package identifier
      outbound.addByte(1);                                    // successful NTP time fetch
      outbound.addLong(CE.toLocal(now(), &tcr));              // time conversion to DST rules (see Settings.h)
      outbound.streamPacket(&Serial2);                        // send package via Serial2
      digitalWrite(2, LOW);                                   // End of transmission
    }
    else {
      if (WiFi.status() != WL_CONNECTED) {
        if (!wifi_reconnect()) {
          digitalWrite(2, HIGH);                               // indicate transmission with LED
          outbound.beginPacket("update");                      // package identifier
          outbound.addByte(2);                                 // no WiFi connection
          outbound.addLong(0);                                 // no time update - send 0
          outbound.streamPacket(&Serial2);                     // send package via Serial2
          digitalWrite(2, LOW);                                // End of transmission
        }
      }
      else {
        digitalWrite(2, HIGH);                                   // indicate transmission with LED
        outbound.beginPacket("update");                          // package identifier
        outbound.addByte(3);                                     // no NTP connection
        outbound.addLong(0);                                     // no time update - send 0
        outbound.streamPacket(&Serial2);                         // send package via Serial2
        digitalWrite(2, LOW);                                    // End of transmission
      }
    }
    interval = millis();
  }
}

bool wifi_reconnect() {
  int ssidlength = wm.getWiFiSSID(true).length() + 1;   // reading stored strings and convert into char
  int passlength = wm.getWiFiPass(true).length() + 1;
  char ssid[ssidlength];
  char pass[passlength];
  wm.getWiFiSSID(true).toCharArray(ssid, ssidlength);
  wm.getWiFiPass(true).toCharArray(pass, passlength);

  Serial.print("---> Re-connecting to WiFi ");
  WiFi.begin(ssid, pass);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    i++;
    if (i > 20) {
      Serial.println("Could not connect to WiFi!");
      return 0;
    }
    Serial.print(".");
  }
  Serial.println("Wifi connected ok.");
  return 1;
}
