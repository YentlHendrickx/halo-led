#include <Arduino.h>
#include "ota.h"
#include "led_strip.h"
#include "led_effects.h"
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

AsyncWebServer server(80);
int brightness = 200;

char* decodeMessage(uint8_t *data, size_t len) {
  static char buffer[256];
  size_t copyLen = len < sizeof(buffer) - 1 ? len : sizeof(buffer) - 1;
  memcpy(buffer, data, copyLen);
  buffer[copyLen] = '\0';
  return buffer;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  otaConnectWiFi();
  otaBegin();

  WebSerial.begin(&server);
  server.begin();

  ledStrip.begin();
  ledEffectsBegin();

  WebSerial.onMessage([&](uint8_t *data, size_t len) {
    char* message = decodeMessage(data, len);
    if (strcmp(message, "next") == 0) {
      String effect = ledEffectsNext(brightness);
      WebSerial.println("Effect: " + effect);
      delay(50);
      return;
    }

    if (strcmp(message, "prev") == 0) {
      ledEffectsPrev(brightness);
      return;
    }

    if (strcmp(message, "clear") == 0) {
      ledStrip.clear();
      ledStrip.show();
      return;
    }

    if (strncmp(message, "fps-", 4) == 0) {
      int b = atoi(message + 4);
      if (b >= 0 && b <= 100) {
        ledEffectsSetTargetFps((uint8_t)b);
        WebSerial.println("FPS set to " + String(b));
        return;
      }
    }

    if (strncmp(message, "brightness-", 11) == 0) {
      int b = atoi(message + 11);
      if (b >= 0 && b <= 255) {
        brightness = b;
        // TODO: Maybe this should be in 'led_effects'?
        ledStrip.setBrightness(brightness);
        WebSerial.println("Brightness set to " + String(b));
        return;
      }
    }

    WebSerial.println("Unknown command.");
  });

  WebSerial.println("LED CONTROL READY!");
}

void loop() {
  otaHandle();
  ledEffectsUpdate();
}
