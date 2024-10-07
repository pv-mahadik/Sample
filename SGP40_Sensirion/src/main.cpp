#include <Arduino.h>
#include <SensirionI2CSgp40.h>
#include <Wire.h>

#define SGP_ADDR 0x59
SensirionI2CSgp40 sgp40;

void setup()
{

  Serial.begin(115200);
  while (!Serial)
  {
    delay(100);
  }

  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  sgp40.begin(Wire);

  uint16_t serialNumber[3];
  uint8_t serialNumberSize = 3;

  error = sgp40.getSerialNumber(serialNumber, serialNumberSize);

  if (error)
  {
    Serial.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else
  {
    Serial.print("SerialNumber:");
    Serial.print("0x");
    for (size_t i = 0; i < serialNumberSize; i++)
    {
      uint16_t value = serialNumber[i];
      Serial.print(value < 4096 ? "0" : "");
      Serial.print(value < 256 ? "0" : "");
      Serial.print(value < 16 ? "0" : "");
      Serial.print(value, HEX);
    }
    Serial.println();
  }

  uint16_t testResult;
  error = sgp40.executeSelfTest(testResult);
  if (error)
  {
    Serial.print("Error trying to execute executeSelfTest(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else if (testResult != 0xD400)
  {
    Serial.print("executeSelfTest failed with error: ");
    Serial.println(testResult);
  }
}

void loop()
{
  uint16_t error;
  char errorMessage[256];
  uint16_t defaultRh = 0x8000;
  uint16_t defaultT = 0x6666;
  uint16_t srawVoc = 0;

  delay(1000);

  error = sgp40.measureRawSignal(defaultRh, defaultT, srawVoc);
  if (error)
  {
    Serial.print("Error trying to execute measureRawSignal(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else
  {
    Serial.print("SRAW_VOC:");
    Serial.println(srawVoc);
  }
}
