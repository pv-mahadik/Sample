#include <Arduino.h>
#include <Wire.h>
#include <HDC2080.h>
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;
#define ADDR 0x40
HDC2080 hdc(ADDR);

uint32_t getAbsoluteHumidity(float HDC_Temp, float HDC_Humi)
{
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((HDC_Humi / 100.0f) * 6.112f * exp((17.62f * HDC_Temp) / (243.12f + HDC_Temp)) / (273.15f + HDC_Temp)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                // [mg/m^3]
  return absoluteHumidityScaled;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  hdc.begin();

  hdc.setMeasurementMode(TEMP_AND_HUMID);
  hdc.setRate(ONE_HZ);
  hdc.setTempRes(FOURTEEN_BIT);
  hdc.setHumidRes(FOURTEEN_BIT);

  hdc.triggerMeasurement();

  while (!Serial)
  {
    delay(10);
  } // Wait for serial console to open!

  Serial.println("SGP30 test");

  if (!sgp.begin())
  {
    Serial.println("Sensor not found :(");
    while (1)
      ;
  }
  Serial.println("SGP30 Found.");
  Serial.print("SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
}
int counter = 0;

void loop()
{
  // put your main code here, to run repeatedly:
  // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
  // float temperature = 22.1; // [°C]
  // float humidity = 45.2; // [%RH]
  
  float HDC_Temp =  hdc.readTemp();
  float HDC_Humi = hdc.readHumidity();
  sgp.setHumidity(getAbsoluteHumidity(HDC_Temp, HDC_Humi));

  if (!sgp.IAQmeasure())
  {
    Serial.println("Measurement failed");
    return;
  }
  Serial.print("TVOC ");
  Serial.print(sgp.TVOC);
  Serial.println(" ppb");
  Serial.print("eCO2 ");
  Serial.print(sgp.eCO2);
  Serial.println(" ppm");

  if (!sgp.IAQmeasureRaw())
  {
    Serial.println("Raw Measurement failed");
    return;
  }
  Serial.print("Raw H2 ");
  Serial.println(sgp.rawH2);
  Serial.print("Raw Ethanol ");
  Serial.print(sgp.rawEthanol);
  Serial.println("");
  Serial.println();

  delay(1000);

  counter++;
  if (counter == 30)
  {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
    {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x");
    Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x");
    Serial.println(TVOC_base, HEX);
  }
}