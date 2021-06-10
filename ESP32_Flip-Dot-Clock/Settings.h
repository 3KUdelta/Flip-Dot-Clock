
// NTP settings ************************************************************************************************************

#define NTP_SERVER "europe.pool.ntp.org"                     // for NTP Server - use any NPT server nearby
#define NTP_INTERVAL 60000                                   // interval time to read from NTP in ms (default: 60000)

#define TZ 0                                                 // do not change this - grabbing UTC from time server 
#define TZ_SEC ((TZ)*3600)                                   // do not change this

//  REMARK:   It does not make sense to read from NTP servers faster than every 60 seconds. Don't spam these servers.
//*********** The time read from NTP will be loaded into the ESP32 real time clock and this clock is precise enough to 
//*********** keep the precise time for much longer than 60 seconds.

// Timezone settings *******************************************************************************************************
#include <Timezone.h>  // Calculating the right timezone including DST https://github.com/JChristensen/Timezone

// change as you like for your location - see expamples in link above

// Central European Time (Zurich, Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);

// do not change this
TimeChangeRule *tcr;                                        // pointer to the time change rule, use to get the TZ abbrev

// *************************************************************************************************************************
