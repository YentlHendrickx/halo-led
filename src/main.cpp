#include "api.h"
#include "config.h"
#include "config_store.h"
#include "led_effects.h"
#include "led_strip.h"
#include "ota.h"
#include "web_serial_ui.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  otaConnectWiFi();
  otaBegin();

  ledStrip.begin();
  ledEffectsBegin();
  if (!configStoreLoad()) {
    ledEffectsLoadDefaults();
  }

  apiRegister(&server);
#if ENABLE_WEB_SERIAL
  webSerialUiBegin(&server);
#endif
  server.begin();
}

void loop() {
  otaHandle();
  ledEffectsUpdate();
}
