#ifndef OTA_H
#define OTA_H

#include <Arduino.h>

// Connect to WiFi (blocks until connected or restart after failure).
// Call once from setup().
void otaConnectWiFi();

// Configure and start Arduino OTA. Call after otaConnectWiFi().
void otaBegin();

// Process OTA updates. Call every loop() iteration.
void otaHandle();

#endif
