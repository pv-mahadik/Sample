#include <Arduino.h>
#include <SensirionI2CSht4x.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

#define SSID "Viliso_2.4G"
#define PASS "shekhar19"
/* Default mqtt server */
#define MQTT_SERVER "192.168.1.234"
#define MQTT_PORT 1883
#define MQTT_USER "viliso"
#define MQTT_PASS "viliso@123"

StaticJsonDocument<200> doc;
SensirionI2CSht4x sht4x;
WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
char jsonString[256];
String nchipId;
int sendInterval = 1000;
unsigned long lastMsg = 0;
bool heaterFlag = false;
float temperature;
float humidity;
float Htemp, Hhumi;
uint16_t error;
char errorMessage[256];
int HumiCounter = 0;

char Ttemp[32] = "temp/";
char Thumi[32] = "humi/";
char Tsensor[32] = "sensor/"; // send required values in json
char Theater[32] = "SHT_heater/";
char THtemp[32] = "Htemp/";
char THhum[32] = "Hhumi/";

void setup_wifi();
void reconnect();
void callback(char *, byte *, unsigned int);

void initSHT41()
{
    uint16_t error;
    char errorMessage[256];

    sht4x.begin(Wire);

    uint32_t serialNumber;
    error = sht4x.serialNumber(serialNumber);
    if (error)
    {
        Serial.print("Error trying to execute serialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
    else
    {
        Serial.print("Serial Number: ");
        Serial.println(serialNumber);
    }
}

String getDeviceID()
{
    uint64_t chipid;
    char ChipID[32];
    chipid = ESP.getEfuseMac();
    Serial.printf("Viliso Chip model = %s Rev %d with %d cores\n", ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
    sprintf(ChipID, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
    Serial.printf("Viliso Chip ID = %s\n", ChipID);
    return String(ChipID);
}

void setup()
{

    Serial.begin(115200);
    while (!Serial)
    {
        delay(100);
    }

    Wire.begin();
    nchipId = getDeviceID();
    Serial.printf("Chip id: %s\n", nchipId.c_str());
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback);
    initSHT41();
    if (sendInterval == 0)
    {
        sendInterval = 1000;
    }
    setup_wifi();

    strcat(Ttemp, nchipId.c_str());
    strcat(Thumi, nchipId.c_str());
    strcat(Tsensor, nchipId.c_str());
    // strcat(Theater, nchipId.c_str());

    Serial.println(Ttemp);
    Serial.println(Thumi);
    Serial.println(Tsensor);
    // Serial.println(Theater);

    timeClient.begin();
    timeClient.setTimeOffset(19800); // +5.30
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    timeClient.update();

    /* use this way to avoid the delay() later for complicated tasks we can use
    nodelay library. */
    long now = millis();
    if (now - lastMsg > sendInterval)
    {
        lastMsg = now;
        // unsigned long ts = timeClient.getEpochTime();

        delay(1000);

        error = sht4x.measureHighPrecision(temperature, humidity);
        char tempString[8];
        dtostrf(temperature, 1, 1, tempString);
        client.publish(Ttemp, tempString);

        char humiString[8];
        dtostrf(humidity, 1, 1, humiString);
        client.publish(Thumi, humiString);

        if (error)
        {
            Serial.print("Error trying to execute measureHighPrecision(): ");
            errorToString(error, errorMessage, 256);
            Serial.println(errorMessage);
        }
        else
        {
            Serial.print("Temperature:");
            Serial.print(temperature);
            Serial.print("\t");
            Serial.print("Humidity:");
            Serial.println(humidity);
        }

        // if (humidity >= 90)
        // {
        //     HumiCounter++;
        //     if (HumiCounter == 1800)
        //     {
        //         HumiCounter = 0;
        //         sht4x.activateMediumHeaterPowerShort(Htemp, Hhumi);
        //         // heaterFlag = true;
        //         // char HtempString[8];
        //         // dtostrf(Htemp, 1, 1, HtempString);
        //         // client.publish(THtemp, HtempString);

        //         // char HhumString[8];
        //         // dtostrf(Hhumi, 1, 1, HhumString);
        //         // client.publish(THhum, HhumString);
        //     }
        //     else
        //     {
        //         Serial.printf("counter value is:%d\n", HumiCounter);
        //     }
        // }
        // else
        // {
        //     Serial.println("Humidity is under the 90% RH...");
        // }

        // if (heaterFlag == true)
        // {
        //     doc["device"] = nchipId;
        //     doc["temp"] = Htemp;
        //     doc["humi"] = Hhumi;
        // }
        // else
        // {
            doc["device"] = nchipId;
            doc["temp"] = temperature;
            doc["humi"] = humidity;
        // }

        serializeJson(doc, jsonString);

        Serial.print("value of json is : ");
        Serial.println(jsonString);

        client.publish(Tsensor, jsonString);
    }
}

void setup_wifi()
{
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);

    WiFi.begin(SSID, PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

// This function gets called when we receive anything from mqtt
void callback(char *topic, byte *message, unsigned int length)
{
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.println(". Message: ");
    String messageRcvd;

    for (int i = 0; i < length; i++)
    {
        Serial.print((char)message[i]);
        messageRcvd += (char)message[i];
    }
    if (String(topic) == Theater)
    {
        if (messageRcvd == "on")
        {
            heaterFlag = true;
            sht4x.activateMediumHeaterPowerShort(temperature, humidity);

            Serial.println("SHT41 Heater ON for 1.0 sec ");

        }
        else if (messageRcvd == "off")
        {
            heaterFlag = false;
            Serial.println("SHT41 heater not activated....");
        }
    }
}
void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(nchipId.c_str()))
        {
            Serial.println("connected");
            client.subscribe(Theater);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

/***************************************************
  This is an example for the SHT4x Humidity & Temp Sensor

  Designed specifically to work with the SHT4x sensor from Adafruit
  ----> https://www.adafruit.com/products/4885

  These sensors use I2C to communicate, 2 pins are required to
  interface
 ****************************************************/

// #include "Adafruit_SHT4x.h"

// Adafruit_SHT4x sht4 = Adafruit_SHT4x();

// void setup()
// {
//     Serial.begin(115200);

//     while (!Serial)
//         delay(10); // will pause Zero, Leonardo, etc until serial console opens

//     Serial.println("Adafruit SHT4x test");
//     if (!sht4.begin())
//     {
//         Serial.println("Couldn't find SHT4x");
//         while (1)
//             delay(1);
//     }
//     Serial.println("Found SHT4x sensor");
//     Serial.print("Serial number 0x");
//     Serial.println(sht4.readSerial(), HEX);

//     // You can have 3 different precisions, higher precision takes longer
//     sht4.setPrecision(SHT4X_HIGH_PRECISION);
//     switch (sht4.getPrecision())
//     {
//     case SHT4X_HIGH_PRECISION:
//         Serial.println("High precision");
//         break;
//     case SHT4X_MED_PRECISION:
//         Serial.println("Med precision");
//         break;
//     case SHT4X_LOW_PRECISION:
//         Serial.println("Low precision");
//         break;
//     }

//     // You can have 6 different heater settings
//     // higher heat and longer times uses more power
//     // and reads will take longer too!
//     sht4.setHeater(SHT4X_NO_HEATER);
//     switch (sht4.getHeater())
//     {
//     case SHT4X_NO_HEATER:
//         Serial.println("No heater");
//         break;
//     case SHT4X_HIGH_HEATER_1S:
//         Serial.println("High heat for 1 second");
//         break;
//     case SHT4X_HIGH_HEATER_100MS:
//         Serial.println("High heat for 0.1 second");
//         break;
//     case SHT4X_MED_HEATER_1S:
//         Serial.println("Medium heat for 1 second");
//         break;
//     case SHT4X_MED_HEATER_100MS:
//         Serial.println("Medium heat for 0.1 second");
//         break;
//     case SHT4X_LOW_HEATER_1S:
//         Serial.println("Low heat for 1 second");
//         break;
//     case SHT4X_LOW_HEATER_100MS:
//         Serial.println("Low heat for 0.1 second");
//         break;
//     }
// }

// void loop()
// {
//     sensors_event_t humidity, temp;

//     uint32_t timestamp = millis();
//     sht4.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
//     timestamp = millis() - timestamp;

//     Serial.print("Temperature: ");
//     Serial.print(temp.temperature);
//     Serial.println(" degrees C");
//     Serial.print("Humidity: ");
//     Serial.print(humidity.relative_humidity);
//     Serial.println("% rH");

//     Serial.print("Read duration (ms): ");
//     Serial.println(timestamp);

//     delay(1000);
// }