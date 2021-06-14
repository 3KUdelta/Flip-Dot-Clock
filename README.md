# Flip-Dot-Clock
Ultra precise and always up-to-date. Fun clock with Flip-Dots.

Author: Marc Staehli, initial upload June 2021

[![ESP8266-World-Clock](https://github.com/3KUdelta/Flip-Dot-Clock/blob/main/Pics/IMG_20210614_084116.jpg)](https://github.com/3KUdelta/Flip-Dot-Clock)

Features:
- cool time display using unique Flip-Dot technology
- Arduino Nano to control the display, ESP32 to do the online (WiFi, NTP) part
- precise time fetch every minute via NTP on connected ESP32
- RTC 3231 module in case of no WiFi available
- automatic DST conversion (see Settings.h for ESP32) for any place in the world
- serial transfer of data from ESP32 to Nano
- sending status messages -> 1 = success, 2 = no WiFi, 3 = no NTP
- Error messages: dot top left indicates no WiFi, dot top right indicates no NTP
- Switchable seconds blinking on the colon
- Switchable time progress per minute

Build:
- ESP32 Dev Kit 1.0 - using WiFi to get NTP Time
- Arduino Nano - controlling the Flip-Dot matrix
- Flip-Dot controller board by Rainer Radow (radow.org)
- 16x28 Brose Flip-Dot Matrix (library also works with Krüger Flip-Dot-Matrix)
- RTC 3231 module 

Settings.h includes all individal settings. 
Including <Timezone.h> for calculating the right timezone including global DST settings (https://github.com/JChristensen/Timezone).

Print the stand yourself: (https://)

[![Flip-Dot-Clock](https://github.com/3KUdelta/Flip-Dot-Clock/blob/main/Pics/IMG_20210614_084328.jpg)](https://github.com/3KUdelta/Flip-Dot-Clock)

[![Flip-Dot-Clock](https://github.com/3KUdelta/Flip-Dot-Clock/blob/main/Pics/IMG_20210614_084238.jpg)](https://github.com/3KUdelta/Flip-Dot-Clock)

[![Flip-Dot-Clock](https://github.com/3KUdelta/Flip-Dot-Clock/blob/main/Pics/IMG_20210614_084258.jpg)](https://github.com/3KUdelta/Flip-Dot-Clock)

[![Flip-Dot-Clock](https://github.com/3KUdelta/Flip-Dot-Clock/blob/main/Pics/VID_20210608_193220%7E2.mp4)](https://github.com/3KUdelta/Flip-Dot-Clock)


