
#include <Arduino.h>
#include <CMMC_Sensor.h>
#include <CMMC_Interval.h>
#include <Ticker.h>
#include "CMMC_ESPNow.h"
#include <Adafruit_NeoPixel.h>
#include <AceButton.h>

uint32_t Wheel(byte WheelPos);
void rainbow(uint8_t wait);

#define PIN            13
#define NUMPIXELS      5

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
uint8_t buttonId = 2;
uint8_t recvId = 0;

// #include "utils.hpp"
CMMC_ESPNow espNow;

Ticker refreshLCDTimer;
Ticker readSensorTimer;
CMMC_Interval updateTimeInterval;
// CMMC_SENSOR_DATA_T buttonId;

CMMC_PIXELS pixelsData;
String devicename = "ESP8266-DEVICE-004";
uint8_t master_mac[6] = { 0xD8, 0xA0, 0x1D, 0x63, 0x49, 0xE0 } ;
bool dirty = false;

#define LDR A0
const int BUTTON_PIN = 0;
const int LED_PIN = 2;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

using namespace ace_button;

AceButton button(BUTTON_PIN);
void handleEvent(AceButton*, uint8_t, uint8_t);

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  button.setEventHandler(handleEvent);
  Serial.println(".....");
  espNow.init(NOW_MODE_SLAVE);
  espNow.enable_retries(true);
  espNow.on_message_sent([](uint8_t *macaddr, u8 status) {
    Serial.printf("sent status=%lu\r\n", status);
  });

  strip.begin(); // This initializes the NeoPixel library.

  pinMode(LED_PIN, OUTPUT);
  espNow.on_message_recv([](uint8_t * macaddr, uint8_t * data, uint8_t len) {
    dirty = true;
    // memcpy()
    // memcpy(&pixelsData, data, len);
    memcpy(&recvId, data, 1);
    Serial.printf("RECV: len = %u byte, Id=%lu at(%lu ms)\r\n", len, data[0], millis());
    // for (size_t i = 0; i < 5; i++) {
    //   Serial.printf("RGB[%d] = %d,%d,%d\n", i, pixelsData.pixels[i].r, pixelsData.pixels[i].g,pixelsData.pixels[i].b);
    // }
    // digitalWrite(LED_PIN, data[0]);
  });  // level events.
  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
}

int flag = 1;

void loop() {
  button.check();
  // put your main code here, to run repeatedly:
  //   updateTimeInterval.every_ms(200, [&]() {
  // button.check();
  // });

  if ( (recvId > 0) && (recvId == buttonId)) {
  //    // for(int i=0;i<NUMPIXELS;i++){
  //    //  // strip.Color takes RGB values, from 0,0,0 up to 255,255,255
  //    //  uint8_t r = pixelsData.pixels[i].r;
  //    //  uint8_t g = pixelsData.pixels[i].g;
  //    //  uint8_t b = pixelsData.pixels[i].b;
  //     // strip.setPixelColor(i, strip.Color(r,g,b)); // Moderately bright green color.
  //     // strip.show(); // This sends the updated pixel color to the hardware.
  //     // delay(delayval); // Delay for a period of time (in milliseconds).
  //   // }
      rainbow(5);
  }
  else {
     for(int i=0;i<NUMPIXELS;i++){
      // strip.Color takes RGB values, from 0,0,0 up to 255,255,255
      uint8_t r = 0;
      uint8_t g = 0;
      uint8_t b = 0;
      strip.setPixelColor(i, strip.Color(r,g,b)); // Moderately bright green color.
    }
    strip.show(); // This sends the updated pixel color to the hardware.
    strip.clear();
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256 && !dirty; j++) {
    for(i=0; i<strip.numPixels() && !dirty; i++) {
      strip.setPixelColor(i, Wheel((i*1+j) & 255));
      button.check();
    }
    strip.show();
    delay(wait);
  }
  dirty = false;
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void handleEvent(AceButton* /* button */, uint8_t eventType, uint8_t /* buttonState */) {
  Serial.println("handle event.");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("Pressed.");
      digitalWrite(LED_PIN, LED_ON);
      break;
    case AceButton::kEventReleased:
      Serial.println("Released.");
      digitalWrite(LED_PIN, LED_OFF);
      espNow.send(master_mac, (u8*) &buttonId, sizeof(buttonId), []() {

      }, 100);
      break;
  }
}
