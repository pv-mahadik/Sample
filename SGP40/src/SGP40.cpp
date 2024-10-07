#include <Arduino.h>
#include <HDC2080.h>
#include "Adafruit_SGP40.h"

#define HDC_ADDR 0x40
HDC2080 hdc(HDC_ADDR); // Object of HDC2080
#define SGP_ADDR 0x59
Adafruit_SGP40 sgp;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  hdc.begin();
  hdc.setMeasurementMode(TEMP_AND_HUMID); // Set measurements to temperature and humidity
  hdc.setRate(ONE_HZ);                    // Set measurement frequency to 1 Hz
  hdc.setTempRes(FOURTEEN_BIT);
  hdc.setHumidRes(FOURTEEN_BIT);

  hdc.triggerMeasurement();

  if (!sgp.begin())
  {
    Serial.println("SGP40 sensor not found :(");
    while (1)
      ;
  }
  Serial.print("SGP40 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  bool result = sgp.selfTest();
  if (result == true)
  {
    Serial.println("Self test passed successfully..");
  }
  else
  {
    Serial.println("Self test failed...");
  }
}
int counter = 0;

void loop()
{
  // put your main code here, to run repeatedly:
  uint16_t sraw;
  int32_t voc_index;

  float t = hdc.readTemp();
  Serial.print("Temp = ");
  Serial.print(t);
  Serial.print("\t\t");

  float h = hdc.readHumidity();
  Serial.print("Hum = ");
  Serial.println(h);

  sraw = sgp.measureRaw(t, h);
  Serial.print("Raw measurement: ");
  Serial.println(sraw);

  voc_index = sgp.measureVocIndex(t, h);
  Serial.print("Voc Index: ");
  Serial.println(voc_index);
  Serial.println();

  delay(1000);
}