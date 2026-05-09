#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "PMS.h"
#include <WiFi.h>
#include "secret.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>

Adafruit_BMP280 bmp;
PMS pms(Serial2); 
PMS::DATA data;

WiFiClient espClient;
PubSubClient client(espClient);

void WiFi_connection() {
  Serial.println("Proba polaczenia do WiFi z SSID ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  Serial.println("Polaczono z siecia o adresie IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Proba polaczenia MQTT...");
    if (client.connect("ESP32_Station_01")) {
      Serial.println("polaczono z brokerem!");
    } else {
      Serial.print("blad, stan=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  WiFi_connection();

  client.setServer(MQTT_SERVER, 1883);

  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  Wire.begin(21, 22);

  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 nie znaleziony!");
  } else {
    Serial.println("BMP280 znaleziony!");
  }

  pms.activeMode();
  pms.wakeUp();
}

void loop() {
if (!client.connected()) {
    reconnect();
  }
  client.loop();

  pms.read(data); 

  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 30000) {
    lastMsg = now;

    JsonDocument doc;
    doc["temp"] = bmp.readTemperature();
    doc["press"] = bmp.readPressure() / 100.0;
    doc["altidute"] = bmp.readAltitude(1013.25);
    
    doc["pm10"] = data.PM_AE_UG_1_0;
    doc["pm25"] = data.PM_AE_UG_2_5;
    doc["pm100"] = data.PM_AE_UG_10_0;
    doc["client"] = "station_01"; 

    char buffer[256];
    serializeJson(doc, buffer);
    client.publish("sensors/air_quality", buffer);
    
    Serial.print("Wyslano dane: ");
    Serial.println(buffer);
  }
}   