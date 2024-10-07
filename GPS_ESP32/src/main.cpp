#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <BlynkSimpleEsp32.h>

float latitude, longitude;
String latitude_string, longitiude_string;

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64

// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char *ssid = "Viliso_2.4G";
const char *pass = "shekhar19";
char auth[] = "4c53aW0tMlnXjaYrAhecN1oEnCC9msY1";

WidgetMap myMap(V0);
WiFiClient client;
TinyGPSPlus gps;
HardwareSerial SerialGPS(2);

void setup()
{
  Serial.begin(115200);
  // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  // {
  //   Serial.println(F("SSD1306 allocation failed"));
  // for (;;)
  //   ;

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");

  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
  // SerialGPS.begin(115200, SERIAL_8N1, 16, 17);
  Serial.println("GPS is initialized....");
  // Blynk.begin("4c53aW0tMlnXjaYrAhecN1oEnCC9msY1", "Viliso_2.4G", "shekhar19");
  // Serial.println("Blynk is initialized...");
  // Blynk.virtualWrite(V0, "clr");
}
void loop()
{
  // while (SerialGPS.available() > 0)
  // {
  //   Serial.print("SerialGPS.available is: ");
  //   Serial.println(SerialGPS.available());

  //   if (gps.encode(SerialGPS.read()))
  //   {
  //     Serial.print("SerialGPS.read is: ");
  //     Serial.println(SerialGPS.read());
  //     Serial.print("gps.encode is: ");
  //     Serial.println(gps.encode(SerialGPS.read()));

  //     // if (gps.location.isValid())
  //     // {
  //     if (gps.location.isUpdated())
  //     {
  latitude = gps.location.lat();
  latitude_string = String(latitude, 6);
  longitude = gps.location.lng();
  longitiude_string = String(longitude, 6);
  Serial.print("Latitude = ");
  Serial.println(latitude_string);
  Serial.println(latitude);
  Serial.print("Longitude = ");
  Serial.println(longitiude_string);
  Serial.println(longitude);

  // display.clearDisplay();
  // display.setTextSize(1);
  // display.setTextColor(WHITE);
  // display.setCursor(0, 20);
  // display.println("Latitude: ");
  // display.setCursor(45, 20);
  // display.print(latitude_string);
  // display.setCursor(0, 40);
  // display.print("Longitude: ");
  // display.setCursor(45, 40);
  // display.print(longitiude_string);

  Blynk.virtualWrite(V0, 1, latitude, longitude, "Location");
  // display.display();
  // }
  // }
  delay(100);
  Serial.println();
  // }
  // }
  Blynk.run();
}


