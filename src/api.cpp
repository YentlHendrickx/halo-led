#include "api.h"
#include "config_store.h"
#include "led_strip.h"
#include "led_effects.h"
#include <Arduino.h>
#include <string.h>

static void sendStatus(AsyncWebServerRequest* request) {
  uint8_t ar, ag, ab;
  ledEffectsGetAccentColor(ar, ag, ab);
  uint8_t gr, gg, gb;
  ledEffectsGetGlobalColor(gr, gg, gb);
  String body;
  body += "effect: ";
  body += ledEffectsGetCurrentName();
  body += "\nbrightness: ";
  body += String(ledEffectsGetBrightness());
  body += "\nfps: ";
  body += String(ledEffectsGetTargetFps());
  body += "\nintensity: ";
  body += String(ledEffectsGetIntensity());
  body += "\naccent: ";
  body += String(ar) + "," + String(ag) + "," + String(ab);
  body += "\nglobal-color: ";
  body += String(gr) + "," + String(gg) + "," + String(gb);
  body += "\nfire-variant: ";
  body += (ledEffectsGetFireVariant() == FireVariant::Red) ? "red" : "blue";
  request->send(200, "text/plain", body);
}

static void handleEffectNext(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  const char* name = ledEffectsNext();
  request->send(200, "text/plain", String("effect: ") + name);
}

static void handleEffectPrev(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  const char* name = ledEffectsPrev();
  request->send(200, "text/plain", String("effect: ") + name);
}

static void handleEffectSet(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  bool post = (request->method() == HTTP_POST);
  if (!request->hasParam("name", post)) {
    request->send(400, "text/plain", "missing name");
    return;
  }
  String name = request->getParam("name", post)->value();
  if (!ledEffectsSetByName(name.c_str())) {
    request->send(400, "text/plain", "unknown effect");
    return;
  }
  request->send(200, "text/plain", String("effect: ") + ledEffectsGetCurrentName());
}

static void handleBrightness(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  bool post = (request->method() == HTTP_POST);
  if (!request->hasParam("value", post)) {
    request->send(400, "text/plain", "missing value");
    return;
  }
  int v = request->getParam("value", post)->value().toInt();
  if (v < 0 || v > 255) {
    request->send(400, "text/plain", "brightness 0-255");
    return;
  }
  ledEffectsSetBrightness((uint8_t)v);
  request->send(200, "text/plain", String("brightness: ") + String(v));
}

static void handleColor(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  bool post = (request->method() == HTTP_POST);
  if (!request->hasParam("r", post) || !request->hasParam("g", post) || !request->hasParam("b", post)) {
    request->send(400, "text/plain", "missing r,g,b");
    return;
  }
  int r = request->getParam("r", post)->value().toInt();
  int g = request->getParam("g", post)->value().toInt();
  int b = request->getParam("b", post)->value().toInt();
  if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
    request->send(400, "text/plain", "r,g,b 0-255");
    return;
  }
  ledEffectsSetStaticColor((uint8_t)r, (uint8_t)g, (uint8_t)b);
  ledEffectsSet(LedEffect::StaticColor);
  request->send(200, "text/plain", "effect: StaticColor");
}

static void handleClear(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  ledStrip.clear();
  ledStrip.show();
  request->send(200, "text/plain", "ok");
}

static void handleSpeed(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  bool post = (request->method() == HTTP_POST);
  if (!request->hasParam("value", post)) {
    request->send(400, "text/plain", "missing value");
    return;
  }
  float v = request->getParam("value", post)->value().toFloat();
  if (v < 0.01f || v > 20.0f) {
    request->send(400, "text/plain", "speed 0.01-20.0");
    return;
  }
  ledEffectsSetSpeed(v);
  request->send(200, "text/plain", String("speed: ") + String(v));
}

static void handleFps(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  bool post = (request->method() == HTTP_POST);
  if (!request->hasParam("value", post)) {
    request->send(400, "text/plain", "missing value");
    return;
  }
  int v = request->getParam("value", post)->value().toInt();
  if (v < 10 || v > 100) {
    request->send(400, "text/plain", "fps 10-100");
    return;
  }
  ledEffectsSetTargetFps((uint8_t)v);
  request->send(200, "text/plain", String("fps: ") + String(v));
}

static void handleAccent(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  bool post = (request->method() == HTTP_POST);
  if (!request->hasParam("r", post) || !request->hasParam("g", post) || !request->hasParam("b", post)) {
    request->send(400, "text/plain", "missing r,g,b");
    return;
  }
  int r = request->getParam("r", post)->value().toInt();
  int g = request->getParam("g", post)->value().toInt();
  int b = request->getParam("b", post)->value().toInt();
  if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
    request->send(400, "text/plain", "r,g,b 0-255");
    return;
  }
  ledEffectsSetAccentColor((uint8_t)r, (uint8_t)g, (uint8_t)b);
  request->send(200, "text/plain", String("accent: ") + String(r) + "," + String(g) + "," + String(b));
}

static void handleGlobalColor(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  bool post = (request->method() == HTTP_POST);
  if (!request->hasParam("r", post) || !request->hasParam("g", post) || !request->hasParam("b", post)) {
    request->send(400, "text/plain", "missing r,g,b");
    return;
  }
  int r = request->getParam("r", post)->value().toInt();
  int g = request->getParam("g", post)->value().toInt();
  int b = request->getParam("b", post)->value().toInt();
  if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
    request->send(400, "text/plain", "r,g,b 0-255");
    return;
  }
  ledEffectsSetGlobalColor((uint8_t)r, (uint8_t)g, (uint8_t)b);
  request->send(200, "text/plain", String("global-color: ") + String(r) + "," + String(g) + "," + String(b));
}

// Helper to get LedEffect from name without setting it
static LedEffect getEffectFromName(const char* name) {
  const char* const* names = ledEffectsGetNames();
  int n = ledEffectsGetNameCount();
  for (int i = 0; i < n; i++) {
    if (strcmp(names[i], name) == 0) {
      return static_cast<LedEffect>(i);
    }
  }
  return LedEffect::Count;  // Invalid
}

static void handleEffectColor(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  bool post = (request->method() == HTTP_POST);
  if (!request->hasParam("effect", post)) {
    request->send(400, "text/plain", "missing effect");
    return;
  }
  String effectName = request->getParam("effect", post)->value();
  
  // Get effect enum from name
  LedEffect e = getEffectFromName(effectName.c_str());
  if (e == LedEffect::Count) {
    request->send(400, "text/plain", "unknown effect");
    return;
  }
  
  // Check if it's a reset request
  if (request->hasParam("reset", post) && request->getParam("reset", post)->value() == "true") {
    ledEffectsResetEffectColor(e);
    request->send(200, "text/plain", String("effect-color: ") + effectName + " reset to default");
    return;
  }
  
  // Set color for effect
  if (!request->hasParam("r", post) || !request->hasParam("g", post) || !request->hasParam("b", post)) {
    request->send(400, "text/plain", "missing r,g,b or reset=true");
    return;
  }
  
  int r = request->getParam("r", post)->value().toInt();
  int g = request->getParam("g", post)->value().toInt();
  int b = request->getParam("b", post)->value().toInt();
  if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
    request->send(400, "text/plain", "r,g,b 0-255");
    return;
  }
  ledEffectsSetEffectColor(e, (uint8_t)r, (uint8_t)g, (uint8_t)b);
  request->send(200, "text/plain", String("effect-color: ") + effectName + " " + String(r) + "," + String(g) + "," + String(b));
}

static void handleIntensity(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  bool post = (request->method() == HTTP_POST);
  if (!request->hasParam("value", post)) {
    request->send(400, "text/plain", "missing value");
    return;
  }
  float v = request->getParam("value", post)->value().toFloat();
  if (v < 0.1f || v > 3.0f) {
    request->send(400, "text/plain", "intensity 0.1-3.0");
    return;
  }
  ledEffectsSetIntensity(v);
  request->send(200, "text/plain", String("intensity: ") + String(v));
}

static void handleFireVariant(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  bool post = (request->method() == HTTP_POST);
  if (!request->hasParam("variant", post)) {
    request->send(400, "text/plain", "missing variant (red or blue)");
    return;
  }
  String variant = request->getParam("variant", post)->value();
  // Case-insensitive comparison
  for (size_t i = 0; i < variant.length(); i++) {
    if (variant[i] >= 'A' && variant[i] <= 'Z') {
      variant[i] = variant[i] - 'A' + 'a';
    }
  }
  if (variant == "red") {
    ledEffectsSetFireVariant(FireVariant::Red);
    request->send(200, "text/plain", "fire-variant: red");
  } else if (variant == "blue") {
    ledEffectsSetFireVariant(FireVariant::Blue);
    request->send(200, "text/plain", "fire-variant: blue");
  } else {
    request->send(400, "text/plain", "variant must be 'red' or 'blue'");
  }
}

static void handleConfigStore(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  if (configStoreSave()) {
    request->send(200, "text/plain", "ok");
  } else {
    request->send(500, "text/plain", "store failed");
  }
}

static void handleConfigClear(AsyncWebServerRequest* request) {
  if (request->method() != HTTP_GET && request->method() != HTTP_POST) {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }
  if (configStoreClear()) {
    ledEffectsLoadDefaults();
    request->send(200, "text/plain", "ok");
  } else {
    request->send(500, "text/plain", "clear failed");
  }
}

void apiRegister(AsyncWebServer* server) {
  server->on("/api/status", HTTP_GET, sendStatus);

  server->on("/api/effect/next", HTTP_GET, handleEffectNext);
  server->on("/api/effect/next", HTTP_POST, handleEffectNext);
  server->on("/api/effect/prev", HTTP_GET, handleEffectPrev);
  server->on("/api/effect/prev", HTTP_POST, handleEffectPrev);

  server->on("/api/effect", HTTP_GET, handleEffectSet);
  server->on("/api/effect", HTTP_POST, handleEffectSet);

  server->on("/api/brightness", HTTP_GET, handleBrightness);
  server->on("/api/brightness", HTTP_POST, handleBrightness);

  server->on("/api/color", HTTP_GET, handleColor);
  server->on("/api/color", HTTP_POST, handleColor);

  server->on("/api/clear", HTTP_GET, handleClear);
  server->on("/api/clear", HTTP_POST, handleClear);

  server->on("/api/fps", HTTP_GET, handleFps);
  server->on("/api/fps", HTTP_POST, handleFps);

  server->on("/api/accent", HTTP_GET, handleAccent);
  server->on("/api/accent", HTTP_POST, handleAccent);

  server->on("/api/global-color", HTTP_GET, handleGlobalColor);
  server->on("/api/global-color", HTTP_POST, handleGlobalColor);

  server->on("/api/effect-color", HTTP_GET, handleEffectColor);
  server->on("/api/effect-color", HTTP_POST, handleEffectColor);

  server->on("/api/intensity", HTTP_GET, handleIntensity);
  server->on("/api/intensity", HTTP_POST, handleIntensity);

  server->on("/api/fire-variant", HTTP_GET, handleFireVariant);
  server->on("/api/fire-variant", HTTP_POST, handleFireVariant);

  server->on("/api/config/store", HTTP_GET, handleConfigStore);
  server->on("/api/config/store", HTTP_POST, handleConfigStore);
  server->on("/api/config/clear", HTTP_GET, handleConfigClear);
  server->on("/api/config/clear", HTTP_POST, handleConfigClear);
}
