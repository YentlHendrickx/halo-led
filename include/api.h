#ifndef API_H
#define API_H

#include <ESPAsyncWebServer.h>

// Register REST API routes on the given server. Call before server.begin().
void apiRegister(AsyncWebServer* server);

#endif
