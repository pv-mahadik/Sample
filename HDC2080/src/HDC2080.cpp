#include <Arduino.h>
#include <HDC2080.h>

#define ADDR 0x40
HDC2080 hdc(ADDR);

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  hdc.begin();

  hdc.setMeasurementMode(TEMP_AND_HUMID);
  hdc.setRate(ONE_HZ);
  hdc.setTempRes(FOURTEEN_BIT);
  hdc.setHumidRes(FOURTEEN_BIT);

  hdc.triggerMeasurement();
}

void loop()
{
  // put your main code here, to run repeatedly:
  Serial.print("Temperature is: ");
  Serial.println(hdc.readTemp());
  Serial.println();
  Serial.print("Humidity is: ");
  Serial.println(hdc.readHumidity());
  delay(1000);
}
