
#include <Arduino.h>
// #include <ESP8266WiFi.h>
#include <CMMC_Sensor.h>
#include <CMMC_Interval.h>
#include <Ticker.h>
#include "CMMC_ESPNow.h"
// #include "utils.hpp"
CMMC_ESPNow espNow;

Ticker refreshLCDTimer;
Ticker readSensorTimer;
CMMC_Interval updateTimeInterval;
CMMC_SENSOR_DATA_T sensorData;
String devicename = "ESP8266-DEVICE-004";
uint8_t master_mac[6] = { 0x30, 0xAE, 0xA4, 0x99, 0x0C, 0xEC } ;

#define LDR A0

void setup() {
  Serial.begin(115200);
  espNow.init(NOW_MODE_SLAVE);
  espNow.enable_retries(true);
  espNow.on_message_sent([](uint8_t *macaddr, u8 status) {
    Serial.printf("sent status=%lu\r\n", status);
  });

  pinMode(2, OUTPUT);
  espNow.on_message_recv([](uint8_t * macaddr, uint8_t * data, uint8_t len) {
    Serial.printf("RECV: len = %u byte, sleepTime = %lu at(%lu ms)\r\n", len, data[0], millis());
    digitalWrite(2, data[0]);
  });
}

void loop() {
  // put your main code here, to run repeatedly:
    updateTimeInterval.every_ms(2* 1000, [&]() {
      sensorData.field1 = analogRead(LDR) * 100;
      // sensorData.field2 = 2 * 100;
      // sensorData.field3 = 2 * 100;
      // sensorData.field4 = 2 * 100;
      strcpy(sensorData.sensorName, devicename.c_str());
      // msgId++;
      espNow.send(master_mac, (u8*) &sensorData, sizeof(sensorData), [&]() {
        Serial.println("espnow send timeout");
      }, 2 * 1000);
  });

}
