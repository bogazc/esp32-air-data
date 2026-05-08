#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "PMS.h"

Adafruit_BMP280 bmp;
PMS pms(Serial2); 
PMS::DATA data;

void setup() {
  Serial.begin(115200);
  
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  Wire.begin(21, 22);

  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 nie znaleziony!");
  } else {
    Serial.println("BMP280 znaleziony!");
  }

  pms.activeMode();
  pms.wakeUp();
  
  Serial.println("Uruchamiam pomiary");
  Serial.println("----------------------------------------------------------------------------------------------------------------");
}

void loop() {
  // Dane z PMS
  if (pms.read(data)) {
    Serial.print("PM 1.0: "); Serial.print(data.PM_AE_UG_1_0); Serial.print(" ug/m3 | ");
    Serial.print("PM 2.5: "); Serial.print(data.PM_AE_UG_2_5); Serial.print(" ug/m3 | ");
    Serial.print("PM 10: ");  Serial.print(data.PM_AE_UG_10_0); Serial.println(" ug/m3 | ");
    
    // Dane z BMP280
    Serial.print("Temp: ");    Serial.print(bmp.readTemperature(), 1); Serial.print(" *C | ");
    Serial.print("Cisnienie: "); Serial.print(bmp.readPressure() / 100.0, 1); Serial.print(" hPa | ");
    Serial.print("Wysokosc: "); Serial.print(bmp.readAltitude(1013.25), 0); Serial.println(" m n.p.m.");
    
    Serial.println("----------------------------------------------------------------------------------------------------------------");
    
    delay(2000);
  }
}