#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <FS.h>       // File System for Web Server Files
#include <LittleFS.h> // This file system is used.
#include <Wire.h>
#include <SPI.h>
#include <SD.h>


TinyGPSPlus gps;
SoftwareSerial gpsSerial(D4, D0); // RX, TX
char buffer[100];
const int chipSelect = 15;

File gpsFile;


const uint8_t UBLOX_INIT[] PROGMEM = {
  // Rate (pick one)
  0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12, //(10Hz)
  //0xB5,0x62,0x06,0x08,0x06,0x00,0xC8,0x00,0x01,0x00,0x01,0x00,0xDE,0x6A, //(5Hz)
  // 0xB5,0x62,0x06,0x08,0x06,0x00,0xE8,0x03,0x01,0x00,0x01,0x00,0x01,0x39, //(1Hz)
  // Disable specific NMEA sentences
  //0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x24, // GxGGA off
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x2B, // GxGLL off
  0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x32, // GxGSA off
  //0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x39, // GxGSV off
  //0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x04,0x00,0x00,0x00,0x00,0x00,0x01,0x04,0x40, // GxRMC off
  0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x47, // GxVTG off
  //57600
  0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xE1,
  0x00, 0x00, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xC3,
  0xb5, 0x62, 0x06, 0x00, 0x01, 0x00, 0x01, 0x08, 0x22
};

const uint8_t Rate_10hz[] PROGMEM = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12};                                                                                   // 10hz
const uint8_t Prt_576[] PROGMEM = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xE1, 0x00, 0x00, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xC3}; // 57600hz
const uint8_t Prt_384[] PROGMEM = {0xb5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xd0, 0x08, 0x00, 0x00, 0x00, 0x96, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0xa8, 0xb5, 0x62, 0x06, 0x00, 0x01, 0x00, 0x01, 0x08, 0x22
                                  }; // 38400hz


const unsigned char ubxRate5Hz[] PROGMEM =
{ 0x06, 0x08, 0x06, 0x00, 200, 0x00, 0x01, 0x00, 0x01, 0x00 };

const unsigned char ubxRate10Hz[] PROGMEM =
{ 0x06, 0x08, 0x06, 0x00, 100, 0x00, 0x01, 0x00, 0x01, 0x00 };
unsigned long last = 0;


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

void sendUBX( const unsigned char *progmemBytes, size_t len )
{
  gpsSerial.write( 0xB5 ); // SYNC1
  gpsSerial.write( 0x62 ); // SYNC2

  uint8_t a = 0, b = 0;
  while (len-- > 0) {
    uint8_t c = pgm_read_byte( progmemBytes++ );
    a += c;
    b += a;
    gpsSerial.write( c );
  }

  gpsSerial.write( a ); // CHECKSUM A
  gpsSerial.write( b ); // CHECKSUM B

} // sendUBX

void setup()
{

  delay(5000);
  
  Serial.begin(9600);

  
  while (!SD.begin(chipSelect))
    {
      Serial.println("init SD Card Failed");
    
    }

    gpsFile = SD.open("/RLDATA/gps.txt", FILE_WRITE);

   if (gpsFile)
   {
    gpsFile.println("\n----------------------------");
   }

   
  //   Serial.begin(57600);
  //
  //       for (uint8_t i = 0; i < sizeof(UBLOX_INIT); i++)
  //        {
  //          Serial.write(pgm_read_byte(UBLOX_INIT + i));
  //        }

  Serial.println("init ok0");
    gpsSerial.begin(9600);
  delay(2000);
  while (1) {
    if (gpsSerial.available() > 0) {
      Serial.println("gpsSerial.available");


      for (uint8_t i = 0; i < sizeof(UBLOX_INIT); i++)
      {
        gpsSerial.write(pgm_read_byte(UBLOX_INIT + i));
      }

      delay(2000);
      break;
    }
    delay(100);
    Serial.println("gpsSerial connet .");
  }
  delay(100);
  gpsSerial.begin(57600);
  Serial.println("init ok1");
  while (!gpsSerial.available())
  {
    Serial.println("GPS module invaild");

  }
  //
  //  for (uint8_t i = 0; i < sizeof(Prt_384); i++)
  //        {
  //          gpsSerial.write(pgm_read_byte(Prt_384 + i));
  //        }
  //    while (1) {
  //   if (gpsSerial.available() > 0){
  //        Serial.write("gpsSerial.available");
  ////          for (uint8_t i = 0; i < sizeof(Prt_384); i++)
  ////        {
  ////          gpsSerial.write(pgm_read_byte(Prt_384 + i));
  ////        }
  //
  //sendUBX( ubxRate10Hz, sizeof(ubxRate10Hz) );
  //
  //        break;
  //   }
  //   delay(100);
  //   Serial.write("gpsSerial connet .");
  //      }
  // Serial.println("init ok1");

  //  for (uint8_t i = 0; i < sizeof(Prt_576); i++)
  //  {
  //    gpsSerial.write(pgm_read_byte(Prt_576 + i));
  //  };
  //  delay(250);
  //    for (uint8_t i = 0; i < sizeof(Prt_576); i++)
  //  {
  //    gpsSerial.write(pgm_read_byte(Prt_576 + i));
  //  };
  //gpsSerial.println("helloworld");
  //delay(250);

  //Serial.println("init ok2");
  //gpsSerial.begin(38400);

  // delay(10000);
  //gpsSerial.begin(9600);

  // gpsSerial.println("helloworld");
  //  gpsSerial.begin(57600);
  //delay(2000);
  Serial.println("init ok3");
}

int first = 0;
void loop()
{
  //    while (Serial.available() > 0) {
  //      //Serial.println(gpsSerial.read());
  //        if (gps.encode(Serial.read())) {
  //            printData();
  //        }
  //    }

  while (gpsSerial.available() > 0) {
    char inByte = gpsSerial.read();
    //Serial.write(inByte);
    Serial.print(inByte);
    gpsFile.print(inByte);
    gpsFile.flush();
  }

  //
  //while (Serial.available() > 0) {
  ////
  ////  if (first==0){
  ////   //sendUBX( ubxRate10Hz, sizeof(ubxRate10Hz) );
  ////   gpsSerial.print("\r\n");
  ////       for (uint8_t i = 0; i < sizeof(Prt_576); i++)
  ////  {
  ////    gpsSerial.write(pgm_read_byte(Prt_576 + i));
  ////  };
  ////  gpsSerial.print("\r\n");
  ////  gpsSerial.flush();
  ////Serial.println("set ok");
  ////      first=1;
  ////  }
  //    char inByte = Serial.read();
  //    //Serial.write(inByte);
  //    Serial.print(inByte);
  //  }
  //


}
