// **************************************************************************
// flipdotFlipdot-clock-m-v1
// 2020-05-01
//
// c by Rainer Radow, 30171 Hannover, Germany
// radow.org
//
// Pairing file on ESP32: ESP32_wwFlipGFX-scrolling_clock_ng.ino
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

bool showblinkingcolon = false;         // define if you want blinking dots
bool showsecondsline = true;            // define if you want to see the seconds progress line

bool colon = true;                      // switching varialbe for colon
bool nowifi = true;
bool nontp = true;
int LED = 13;                           // Internal LED on Nano
int lastsecond = 61;
int lastminute = 61;
long t1, t2;                            // timing variables
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
  vertical(2);                                 // vertical scroll though the dots to avoid "hanging" dots, 2 runs
  invert(1);                                   // inverting scroll through, 1 run
  t1 = millis();                               // t1 is used to do timed display movement
  watchdog = millis();                         // watchdog to see if WiFi is alive
}

void loop() {

  bool flipsomedots = false;                   // update flag switch back
  t2 = millis();

  if ( t2 >= t1 + 120000)                      // all 2 mintes move all dots on the matrix
  {                                            // to prevent "hanging" dots
    t1 = t2;
    invert(2);
  }

  if (inbound.parseStream(&Serial))  {          // any data on serial port available?
    digitalWrite(LED, HIGH);                    // inicate incoming data on Nano red LED

    if (inbound.fullMatch("update")) {          // successful NTP data fetch = updating time
      byte comflag = inbound.nextByte();
      long ntp_time = inbound.nextLong();

      if (comflag == 1) {
        setTime(ntp_time);                      // update time in Nano RTC with received time
        rtc_3231.adjust(DateTime(ntp_time));    // update time in 3231 RTC with received time
        vertical(2);
        nowifi = false;
        nontp = false;
        watchdog = millis();
      }
      if (comflag == 2) {                       // no WiFi signal
        vertical(1);
        scrolltext("no WiFi");
        nowifi = true;
      }
      if (comflag == 3) {                       // no NTP signal
        vertical(1);
        scrolltext("no NTP");
        nontp = true;
      }
    }
    flipsomedots = true;                        // set update flag
    digitalWrite(LED, LOW);                     // switch led off again
  }

  if (second(now()) != lastsecond) {            // change of second?
    lastsecond = second(now());

    if (showblinkingcolon)
      flipsomedots = true;                      // set update flag if colon should blink
      
    if (showsecondsline)
      flipsomedots = true;                      // set update flag if line should be displayed

    if (minute(now()) != lastminute) {          // did the minute change?
      lastminute = minute(now());
      flipsomedots = true;
    }
  }

  if (millis() - watchdog > 180000) {           // no Wifi after 3 minutes?
    vertical(1);
    scrolltext("no WiFi");
    nowifi = true;
    watchdog = millis();
    flipsomedots = true;
  }

  // *********** funny messages during the day **************************************
  if (hour(now()) == 8 && (minute(now()) == 15) && (second(now()) == 0)) {
    vertical(2);
    scrolltext("tired?");
  }
  if (hour(now()) == 10 && (minute(now()) == 14) && (second(now()) == 0)) {
    vertical(2);
    scrolltext("happy?");
  }
  if (hour(now()) == 11 && (minute(now()) == 54) && (second(now()) == 0)) {
    vertical(2);
    scrolltext("hungry?");
  }
  if (hour(now()) == 14 && (minute(now()) == 10) && (second(now()) == 0)) {
    vertical(2);
    scrolltext("bored?");
  }
  if (hour(now()) == 16 && (minute(now()) == 50) && (second(now()) == 0)) {
    vertical(2);
    scrolltext("Beer?");
  }
  if (hour(now()) == 22 && (minute(now()) == 40) && (second(now()) == 0)) {
    vertical(2);
    scrolltext("sleepy?");
  }
  if (hour(now()) == 23 && (minute(now()) == 55) && (second(now()) == 0)) {
    vertical(2);
    scrolltext("Bed?");
  }
  //***********************************************************************************

  if (flipsomedots)                              // any changes? then update display
    updatedisplay();
}

void updatedisplay() {
  // where do you want to place our time xx:xx in the matrix?
  const uint8_t ROW1 = 5;
  const uint8_t COLUMNH10 = 4;  // hours 1x
  const uint8_t COLUMNH01 = 9;  // hours x1
  const uint8_t COLON = 14;     // colon
  const uint8_t COLUMNM10 = 16; // minutes 1x
  const uint8_t COLUMNM01 = 21; // minute x1

  static uint8_t hour10old = 10, hour01old = 10, minute10old = 10, minute01old = 10;  // for HORIZONTAL only
  uint8_t hour10, hour01, minute10, minute01;  // for HORIZONTAL only

  flipdot.dotPowerOn();                        // giving power to the dot matrix
  delay(30);

  flipdot.mSetFont(wwFont_4x7_fix_v02);       // set the font
  flipdot.mScrollDirection(flipdot.UP);       // set number's scroll direction
  flipdot.mScrollDelay(60);                   // scroll delay in ms
  flipdot.mScrollSpace(2);                    // set the dots space between scrolling points
  setTime(rtc_3231.now().unixtime());         // syncing Nano RTC with ds3231
  hour10 = hour(now()) / 10;
  hour01 = hour(now()) % 10;
  minute10 = minute(now()) / 10;
  minute01 = minute(now()) % 10;

  // ---- Numbers ------------------------
  if (minute01 != minute01old) {
    flipdot.mScrollDigit(COLUMNM01, ROW1, minute01old, minute01);
    minute01old = minute01;
    if (minute10 != minute10old) {
      flipdot.mScrollDigit(COLUMNM10, ROW1, minute10old, minute10);
      minute10old = minute10;
      if (hour01 != hour01old) {
        flipdot.mScrollDigit(COLUMNH01, ROW1, hour01old, hour01);
        hour01old = hour01;
        if (hour10 != hour10old) {
          flipdot.mScrollDigit(COLUMNH10, ROW1, hour10old, hour10);
          hour10old = hour10;
        }
      }
    }
  }

  if (showblinkingcolon) {                       // ---- blinking colon ----
    if (colon)
      flipdot.mDrawString(COLON, ROW1, ":");
    else
      flipdot.mEraseVerticalLine(COLON, ROW1, 7);
    colon = !colon;
  }
  else flipdot.mDrawString(COLON, ROW1, ":");

  if (showsecondsline) {
    uint8_t s = 0;
    while (s < second(now())){
      flipdot.mSetDot(s++/2, 16);
    }
  }
  if (second(now()) == 59) {
    uint8_t s = 1;
    while (s < 29){
      flipdot.mResetDot(s++, 16);
    }
  }
  
  flipdot.mDrawDigit(COLUMNH10, ROW1, hour10);   // re-fill numbers into matrix in case of error messages
  flipdot.mDrawDigit(COLUMNH01, ROW1, hour01);
  flipdot.mDrawDigit(COLUMNM10, ROW1, minute10);
  flipdot.mDrawDigit(COLUMNM01, ROW1, minute01);

  if (nowifi) flipdot.mSetDot(1, 1);            // set dot in top left corner to indcate "no WiFi"
  else        flipdot.mResetDot(1, 1);          // erase dot in top left corner to indcate WiFi ok

  if (nontp)  flipdot.mSetDot(28, 1);           // set dot in top right corner to indcate "no NTP"
  else        flipdot.mResetDot(28, 1);         // erase dot in top right corner (NTP is online)

  flipdot.mUpdate();
  delay(100);
  flipdot.dotPowerOff();                         // power matrix down until next update
}

void invert(unsigned int loops) {
  flipdot.dotPowerOn();                         // once a while keep the dots flying to avoid "hanging" dots
  delay(30);
  for (uint8_t l = 0; l < loops; l++) {
    flipdot.mInvert();
    flipdot.mUpdate();
    flipdot.mInvert();
    flipdot.mUpdate();
  }
  flipdot.dotPowerOff();
}

void vertical(unsigned int loops) {
  flipdot.dotPowerOn();                         // once a while keep the dots flying to avoid "hanging" dots
  delay(30);
  for (uint8_t l = 0; l < loops; l++) {
    for (uint8_t y = 1; y <= YCOUNTPERMODULE; y++) {
      for (uint8_t x = 1; x <= XCOUNTPERMODULE ; x++) {
        flipdot.mSetDot(x, y);
        flipdot.mUpdate();
        delay(0);
      }
    }
    for (uint8_t y = 1; y <= YCOUNTPERMODULE; y++) {
      for (uint8_t x = 1; x <= XCOUNTPERMODULE ; x++) {
        flipdot.mResetDot(x, y);
        flipdot.mUpdate();
        delay(0);
      }
    }
  }
  flipdot.dotPowerOff();
}

void scrolltext(String message) {

  uint32_t startingtime;
  uint16_t switchingtime = 90;                       // minimal refresh time for a smooth scrolling

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
