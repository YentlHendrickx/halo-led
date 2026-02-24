#ifndef OTA_TEST_CONFIG_H
#define OTA_TEST_CONFIG_H

// WiFi (define in wifi_credentials.h; copy wifi_credentials.h.example to get started)
#include "wifi_credentials.h"

// LED strip: pin and length
#define LED_PIN   2
#define LED_COUNT 110   // strip length; LED 0 = bottom (vertical mount)

// WebSerial console (browser-based serial). Set to 0 to disable, 1 to enable.
#define ENABLE_WEB_SERIAL 1

#endif /* OTA_TEST_CONFIG_H */
