#include <Arduino.h>
#include <SensirionI2CSgp41.h>
#include <HDC2080.h>
#include <Wire.h>
#include <VOCGasIndexAlgorithm.h>
#include <NOxGasIndexAlgorithm.h>

SensirionI2CSgp41 sgp41;
VOCGasIndexAlgorithm voc_algorithm;
NOxGasIndexAlgorithm nox_algorithm;

// time in seconds needed for NOx conditioning
uint16_t conditioning_s = 10;
float temperature, humidity;

#define ADDR 0x40
HDC2080 sensor(ADDR);

void setup()
{

  Serial.begin(115200);
  // Initialize I2C communication
  sensor.begin();

  // Begin with a device reset
  sensor.reset();
  // Configure Measurements
  sensor.setMeasurementMode(TEMP_AND_HUMID); // Set measurements to temperature and humidity
  sensor.setRate(ONE_HZ);                    // Set measurement frequency to 1 Hz
  sensor.setTempRes(FOURTEEN_BIT);
  sensor.setHumidRes(FOURTEEN_BIT);

  // begin measuring
  sensor.triggerMeasurement();
  while (!Serial)
  {
    delay(100);
  }

  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  sgp41.begin(Wire);

  uint16_t serialNumber[3];
  uint8_t serialNumberSize = 3;

  error = sgp41.getSerialNumber(serialNumber, serialNumberSize);

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
  error = sgp41.executeSelfTest(testResult);
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
  uint16_t srawVoc = 0;
  uint16_t srawNox = 0;

  temperature = sensor.readTemp();
  humidity = sensor.readHumidity();
  delay(1000);

  if (conditioning_s > 0)
  {
    // During NOx conditioning (10s) SRAW NOx will remain 0
    error = sgp41.executeConditioning(humidity, temperature, srawVoc);
    conditioning_s--;
  }
  else
  {
    // Read Measurement
    error = sgp41.measureRawSignals(humidity, temperature, srawVoc, srawNox);
  }

  if (error)
  {
    Serial.print("Error trying to execute measureRawSignals(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else
  {
    Serial.print("SRAW_VOC:");
    Serial.println(srawVoc);
    int32_t voc_index = voc_algorithm.process(srawVoc);
    Serial.print("VOC Index: ");
    Serial.println(voc_index);
    Serial.print("SRAW_NOx:");
    Serial.println(srawNox);
    int32_t nox_index = nox_algorithm.process(srawNox);
    Serial.print("NOx Index: ");
    Serial.println(nox_index);
  }
}