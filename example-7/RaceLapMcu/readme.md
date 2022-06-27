options

RaceLap is GPS Tracker and LapTimer
This is a small R&D project intendet to familiarize myself with programming of Mircoprocessors in the Arduino environment. The goal is to create a GPS tracker and LapTimer that records laps performed at a race track and shows the delta time to the fastest lap. The code is currently very messy and might get some cleanup at a later point.

Features:
(Somewhat) Implemented
define start/finish line
detect start/finish crossing to start/end a lap
show laptime, lap number and various other information
show delta time to reference lap (with number and a red/green bar)
save completed laps to SD card in a structured manner
connect to WiFi and offer WebServer to retrieve recorded laps from the tracker
analyse recorded laps in APP
OTA update over wifi

