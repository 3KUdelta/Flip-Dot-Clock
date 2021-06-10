// **************************************************************************
// flipdotFlipdot-clock-m-v1
// 2020-05-01
//
// c by Rainer Radow, 30171 Hannover, Germany
// radow.org
// SILENT VERSION - NANO
// Pairing file on ESP32: ESP32_Flip-Dot-Clock.ino
//
// June 2021: addons by Marc Stähli
// - precise time fetch via NTP on connected ESP32
// - automatic DST conversion (see Settings.h for ESP32) for any place in the world
// - serial transfer of data to this sketch
// - sending status messages -> 1 = success, 2 = no WiFi, 3 = no NTP
// - dot top left indicates no WiFi, dot top right indicates no NTP
//
// Arduino settings: Arduino Nano, "ATmega328P (Old Bootloader), your port
//
// **************************************************************************

#include "wwFlipGFX.h"                  // Rainer Radow / http://www.radow.org
#include "fonts/wwFont_4x7_fix_v02.h"   // only contains following chars +-0123456789:C°
#include "fonts/dotFont_01_v02.h"       // for scrolling messages
#include <TimeLib.h>                    // For converting NTP time https://github.com/PaulStalloffregen/Time.git
#include <Wire.h>                       // for RTC ds3231
#include "RTClib.h"                     // standard Arduino library handling ds3231 RTC module
#include <AsciiMassageParser.h>         // https://github.com/SofaPirate/AsciiMassage

wwFlipGFX flipdot;                      // Flip Dot initialization
RTC_DS3231 rtc_3231;                    // ds3231 RTC initialization
AsciiMassageParser inbound;             // initializing inbound parser

bool nowifi = true;
bool nontp = true;
int LED = 13;                           // Internal LED on Nano
int lastsecond = 61;
int lastminute = 61;
long watchdog;                          // watchdog for WiFi availability

void setup() {

  Serial.begin(9600);                   // going low speed, simple data transmission
  pinMode(LED, OUTPUT);                 // activate on-board led on Nano
  digitalWrite(LED, HIGH);              // indicating start sequence 1 seconds red led
  delay(1000);
  digitalWrite(LED, LOW);

  flipdot.begin();
  flipdot.oriSet(1);                           // BROSE setup
  flipdot.dotPowerOn();                        // giving power to the dot matrix
  delay(30);                                   // just to be sure
  flipdot.setCoilFlipDuration(1000);           // setting flip time to 1000 microseconds
  flipdot.setCoilSetDuration(1000);
  flipdot.setCoilResetDuration(1000);
  flipdot.setAll(0);                           // flipping all dots to on with 0 delay
  flipdot.resetAll(0);                         // flipping all dots to off with 0 delay

  if (! rtc_3231.begin()) {
    scrolltext("no RTC");
  }
  // *** Inital time setting if needed **************************************************
  // rtc_3231.adjust(DateTime(F(__DATE__), F(__TIME__))); // setting ds3231 to compile time
  // ************************************************************************************
  setTime(rtc_3231.now().unixtime());          // write time stored in ds3231 to Nano RTC
  watchdog = millis();                         // watchdog to see if WiFi is alive
}

void loop() {

  if (inbound.parseStream(&Serial))  {          // any data on serial port available?
    digitalWrite(LED, HIGH);                    // inicate incoming data on Nano red LED

    if (inbound.fullMatch("update")) {          // successful NTP data fetch = updating time
      byte comflag = inbound.nextByte();
      long ntp_time = inbound.nextLong();

      if (comflag == 1) {
        setTime(ntp_time);                      // update time in Nano RTC with received time
        rtc_3231.adjust(DateTime(ntp_time));    // update time in 3231 RTC with received time
        nowifi = false;
        nontp = false;
        watchdog = millis();
      }
      if (comflag == 2) {                       // no WiFi signal
        nowifi = true;
      }
      if (comflag == 3) {                       // no NTP signal
        nontp = true;
      }
    }
    digitalWrite(LED, LOW);                     // switch led off again
    updatedisplay();
  }

  if (minute(now()) != lastminute) {          // did the minute change?
    lastminute = minute(now());
    updatedisplay();
  }

  if (millis() - watchdog > 180000) {           // no Wifi after 3 minutes?
    nowifi = true;
    watchdog = millis();
    updatedisplay();
  }
}

void updatedisplay() {
  // where do you want to place our time xx:xx in the matrix?
  const uint8_t ROW1 = 5;
  const uint8_t COLUMNH10 = 4;  // hours 1x
  const uint8_t COLUMNH01 = 9;  // hours x1
  const uint8_t COLON = 14;     // colon
  const uint8_t COLUMNM10 = 16; // minutes 1x
  const uint8_t COLUMNM01 = 21; // minute x1

  uint8_t hour10, hour01, minute10, minute01;  // for HORIZONTAL only

  flipdot.dotPowerOn();                        // giving power to the dot matrix
  delay(30);

  flipdot.mSetFont(wwFont_4x7_fix_v02);       // set the font

  setTime(rtc_3231.now().unixtime());         // syncing Nano RTC with ds3231
  hour10 = hour(now()) / 10;
  hour01 = hour(now()) % 10;
  minute10 = minute(now()) / 10;
  minute01 = minute(now()) % 10;

  flipdot.mReset();
  flipdot.mDrawDigit(COLUMNM01, ROW1, minute01);
  flipdot.mDrawDigit(COLUMNM10, ROW1, minute10);
  flipdot.mDrawDigit(COLUMNH01, ROW1, hour01);
  flipdot.mDrawDigit(COLUMNH10, ROW1, hour10);


  if (nowifi) flipdot.mSetDot(1, 1);            // set dot in top left corner to indcate "no WiFi"
  else        flipdot.mResetDot(1, 1);          // erase dot in top left corner to indcate WiFi ok

  if (nontp)  flipdot.mSetDot(28, 1);           // set dot in top right corner to indcate "no NTP"
  else        flipdot.mResetDot(28, 1);         // erase dot in top right corner (NTP is online)

  flipdot.mUpdate();
  delay(100);
  flipdot.dotPowerOff();                        // power matrix down until next update
}

void scrolltext(String message) {

  uint32_t startingtime;
  uint16_t switchingtime = 90;                  // minimal refresh time for a smooth scrolling

  flipdot.dotPowerOn();
  delay(30);
  flipdot.setCoilFlipDuration(500);
  flipdot.mSetCharSpace(2);
  flipdot.mSetFont(dotFont_01_v02);
  int length = flipdot.getStringWidth(message);

  for (int x = 14; x > length * -1; x--) {
    startingtime = millis();
    flipdot.mReset();
    flipdot.mDrawString(x, 1, message);
    flipdot.mUpdate();
    while (millis() - startingtime < switchingtime);  // wait for the necessary switching time
  }
  flipdot.setCoilFlipDuration(1000);
  flipdot.dotPowerOff();
}
