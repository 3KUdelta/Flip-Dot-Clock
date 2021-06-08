
// NTP settings ************************************************************************************************************

#define NTP_SERVER "ch.pool.ntp.org"                         // for NTP Server - use any NPT server nearby
#define TZ 0                                                 // grabbing UTC from time server
#define TZ_SEC ((TZ)*3600)                                   // do not change this

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
