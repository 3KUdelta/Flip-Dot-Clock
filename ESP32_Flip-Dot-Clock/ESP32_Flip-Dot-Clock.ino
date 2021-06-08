// **************************************************************************
// flipdotFlipdot-clock-m-v1
// 2020-05-01
//
// c by Rainer Radow, 30171 Hannover, Germany
// radow.org
// This file is for your ESP32
// Pairing file on Nano: Nano_Flip-Dot-Clock.ino
//
// June 2021: addons by Marc Stähli
// - precise time fetch via NTP on connected ESP32
// - automatic DST conversion (see Settings.h) for any place in the world
// - serial transfer of data from this sketch
// - sending status messages -> 1 = success, 2 = no WiFi, 3 = no NTP
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
#include <EasyNTPClient.h>                  // For NTP Signal read https://github.com/aharshac/EasyNTPClient
#include <TimeLib.h>                        // For converting NTP time https://github.com/PaulStalloffregen/Time.git
#include <WiFiManager.h>                    // https://github.com/tzapu/WiFiManager
#include <AsciiMassagePacker.h>             // https://github.com/SofaPirate/AsciiMassage

WiFiManager wm;                                    // initializing WiFi Manager
WiFiUDP udp;                                       // WiFi UDP initialization for NTP
EasyNTPClient ntpClient(udp, NTP_SERVER);          // NTP initialization
AsciiMassagePacker outbound;                       // https://i2.wp.com/randomnerdtutorials.com/wp-content/uploads/2018/08/ESP32-DOIT-DEVKIT-V1-Board-Pinout-36-GPIOs-updated.jpg?quality=100&strip=all&ssl=1Serial transmission initialization

long t1, t2;

void setup() {

  Serial.begin(9600); while (!Serial); delay(200);
  Serial2.begin(9600);                             // going low speed, simple data transmission

  Serial.println();
  Serial.println("Starting Time Fetcher Flip-Dot Clock...");

  pinMode(2, OUTPUT);                              // initialize onboard LED

  //************************************************************************************************
  // wm.resetSettings();                     // for debugging puposes: erase wifi credentials
  //************************************************************************************************

  wm.setConfigPortalBlocking(true);         // setiing WiFi manager to blocking mode
  wm.setCaptivePortalEnable(true);          // setting WiFi manager to captive portal

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

  t1 = millis();                            // t1 is used to do timed NTP reading
}

void loop() {

  t2 = millis();
  if ( t2 >= t1 + 60000) {                                    // get every minute NTP time
                                                              // to avoid overloading NTP servers
    t1 = t2;
    digitalWrite(2, HIGH);                                    // indicate transmission with LED
    if (get_NTP_time()) {
      outbound.beginPacket("update");                         // package identifier
      outbound.addByte(1);                                    // successful NTP time fetch
      outbound.addLong(CE.toLocal(now(), &tcr));              // time conversion to DST rules (see Settings.h)
      outbound.streamPacket(&Serial2);                        // send package via Serial2
    }
    else {
      outbound.beginPacket("update");                         // package identifier
      outbound.addByte(3);                                    // no NTP message
      outbound.addLong(0);                                    // no time update - send 0
      outbound.streamPacket(&Serial2);                        // send package via Serial2
    }
    digitalWrite(2, LOW);                                     // End of transmission
  }
}

bool get_NTP_time() {

  static long epochtime_old;
  bool wificonnection = 1;

  if (WiFi.status() != WL_CONNECTED) {
    wificonnection = wifi_reconnect();
  }

  if (wificonnection)
  {
    Serial.println("---> Now reading time from NTP Server");
    long epochtime = ntpClient.getUnixTime();
    
    if (epochtime <= epochtime_old) {
      Serial.println("NTP read not successul!");
      return 0;
    }
    setTime(epochtime);                                       // set systemtime in ESP32 to UTC fetched from NTP
    Serial.println("NTP read success");
    Serial.print("Epochtime raw: ");
    Serial.print(hour(now()));
    Serial.print(":");
    Serial.println(minute(now()));
    Serial.print("Local time: ");
    Serial.print(hour(CE.toLocal(now(), &tcr)));
    Serial.print(":");
    Serial.println(minute(CE.toLocal(now(), &tcr)));
    epochtime_old = epochtime;
    return 1;
  }
  else {                                                     // something went wrong in fetching NTP time
    Serial.println("NTP read not successul!");
    return 0;
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
      digitalWrite(2, HIGH);                                 // indicate transmitting data
      outbound.beginPacket("update");                        // package identifier
      outbound.addByte(2);                                   // no WiFi message
      outbound.addLong(0);                                   // no time update - send 0
      outbound.streamPacket(&Serial2);                       // send package via Serial2
      digitalWrite(2, LOW);                                  // LED off because we are online
      return 0;
    }
    Serial.print(".");
  }
  Serial.println("Wifi connected ok.");
  return 1;
}
