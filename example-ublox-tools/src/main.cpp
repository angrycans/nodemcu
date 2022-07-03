#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

TinyGPSPlus gps;
// SoftwareSerial gpsSerial(D4, D3); // RX, TX

SoftwareSerial gpsSerial;
char buffer[100];

const uint8_t Rate_10hz[] PROGMEM = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12};                                                                                                                                         // 10hz
const uint8_t Prt_576[] PROGMEM = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xE1, 0x00, 0x00, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xC3};                                                       // 57600hz
const uint8_t Prt_384[] PROGMEM = {0xb5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xd0, 0x08, 0x00, 0x00, 0x00, 0x96, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0xa8, 0xb5, 0x62, 0x06, 0x00, 0x01, 0x00, 0x01, 0x08, 0x22}; // 38400hz

const uint8_t Prt_3842[] PROGMEM = {0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xd0, 0x08, 0x00, 0x00, 0x00, 0x96, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0xa8, 0xb5, 0x62, 0x06, 0x00, 0x01, 0x00, 0x01}; // 38400hz

const unsigned char ubxRate5Hz[] PROGMEM =
    {0x06, 0x08, 0x06, 0x00, 200, 0x00, 0x01, 0x00, 0x01, 0x00};

const unsigned char ubxRate10Hz[] PROGMEM =
    {0x06, 0x08, 0x06, 0x00, 100, 0x00, 0x01, 0x00, 0x01, 0x00};
unsigned long last = 0;

void printData()
{
  if (gps.location.isUpdated())
  {
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

void sendUBX(const unsigned char *progmemBytes, size_t len)
{
  gpsSerial.write(0xB5); // SYNC1
  gpsSerial.write(0x62); // SYNC2

  uint8_t a = 0, b = 0;
  while (len-- > 0)
  {
    uint8_t c = pgm_read_byte(progmemBytes++);
    a += c;
    b += a;
    gpsSerial.write(c);
  }

  gpsSerial.write(a); // CHECKSUM A
  gpsSerial.write(b); // CHECKSUM B

} // sendUBX

void setup()
{
  Serial.begin(9600);

  Serial.println("init ok0");
  delay(2000);

  // gpsSerial.begin(9600);
  gpsSerial.begin(9600, SWSERIAL_8N1, D4, D3, false);
  // gpsSerial.begin(9600, SWSERIAL_8N1, D4, D3, false, 256);

  // for (uint8_t i = 0; i < sizeof(Prt_384); i++)
  // {
  //   gpsSerial.write(pgm_read_byte(Prt_384 + i));
  // }
  while (1)
  {
    if (gpsSerial.available() > 0)
    {
      Serial.write("gpsSerial.available");
      gpsSerial.print("\r\n");
      sendUBX(Prt_3842, sizeof(Prt_3842));
      gpsSerial.print("\r\n");
      //   sendUBX(ubxRate10Hz, sizeof(ubxRate10Hz));

      break;
    }
    delay(100);
    Serial.write("gpsSerial connet .");
  }
  Serial.println("init ok1");

  //  for (uint8_t i = 0; i < sizeof(Prt_576); i++)
  //  {
  //    gpsSerial.write(pgm_read_byte(Prt_576 + i));
  //  };
  //  delay(250);
  //    for (uint8_t i = 0; i < sizeof(Prt_576); i++)
  //  {
  //    gpsSerial.write(pgm_read_byte(Prt_576 + i));
  //  };
  // gpsSerial.println("helloworld");
  delay(250);

  Serial.println("init ok2");
  // gpsSerial.begin(38400);

  // delay(10000);
  // gpsSerial.begin(9600);

  // gpsSerial.println("helloworld");
  //  gpsSerial.begin(57600);
  delay(2000);
  Serial.println("init ok3");
}

int first = 0;
void loop()
{
  //    while (gpsSerial.available() > 0) {
  //      //Serial.println(gpsSerial.read());
  //        if (gps.encode(gpsSerial.read())) {
  //            printData();
  //        }
  //    }

  while (gpsSerial.available() > 0)
  {

    // if (first == 0)
    // {
    //   // sendUBX( ubxRate10Hz, sizeof(ubxRate10Hz) );
    //   gpsSerial.print("\r\n");
    //   for (uint8_t i = 0; i < sizeof(Prt_576); i++)
    //   {
    //     gpsSerial.write(pgm_read_byte(Prt_576 + i));
    //   };
    //   gpsSerial.print("\r\n");
    //   gpsSerial.flush();
    //   Serial.println("set ok");
    //   first = 1;
    // }
    char inByte = gpsSerial.read();
    Serial.write(inByte);
    // Serial.print(inByte,HEX);
  }
}