#ifndef WEB_SERIAL_UI_H
#define WEB_SERIAL_UI_H

#include <ESPAsyncWebServer.h>

// Register WebSerial on the given server. Call before server.begin().
// No-op when ENABLE_WEB_SERIAL is 0 (see config.h).
void webSerialUiBegin(AsyncWebServer* server);

// Log a line to the WebSerial console. No-op when ENABLE_WEB_SERIAL is 0.
void webSerialLog(const char* msg);

#endif
