#include <WiFi.h>
#include <esp_wifi.h>

// Set your new MAC Address
uint8_t newMACAddress[] = {0xF4, 0xCF, 0xA2, 0x91, 0xED, 0x64};

// provide your own WiFi SSID and password
const char *ssid = "Viliso_2.4G";
const char *password = "shekhar19";

void setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_STA);

  Serial.print("[OLD] ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  // ESP32 Board add-on before version < 1.0.5
  // esp_wifi_set_mac(ESP_IF_WIFI_STA, &newMACAddress[0]);

  // ESP32 Board add-on after version > 1.0.5
  esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);

  Serial.print("[NEW] ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  // Print your WiFi's SSID (might be insecure)
  Serial.println(ssid);
}

void loop()
{
}