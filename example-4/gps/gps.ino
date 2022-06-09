#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
 
TinyGPSPlus gps;
SoftwareSerial gpsSerial(D4, D3); // RX, TX
char buffer[100];
 
void printData() 
{
    if (gps.location.isUpdated()) {
    //  if (gps.location.isValid()){
        double lat = gps.location.lat();
        double lng = gps.location.lng();
 
        double altitude = gps.altitude.meters();
 
        int year = gps.date.year();
        int month = gps.date.month();
        int day = gps.date.day();
 
        int hour = gps.time.hour();
        int minute = gps.time.minute();
        int second = gps.time.second();
        
 
        snprintf(buffer, sizeof(buffer),
                 "Latitude: %.8f, Longitude: %.8f, Altitude: %.2f m, "
                 "Date/Time: %d-%02d-%02d %02d:%02d:%02d",
                 lat, lng, altitude, year, month, day, hour, minute, second);
 
        Serial.println(buffer);
    }
}
 
void setup() 
{
    Serial.begin(9600);
    gpsSerial.begin(57600);
    delay(2000);
    Serial.println("init ok");
}
 
void loop() 
{
    while (gpsSerial.available() > 0) {
      //Serial.println(gpsSerial.read());
        if (gps.encode(gpsSerial.read())) {
            printData();
        }
    }
}
