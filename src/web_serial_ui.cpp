#include "web_serial_ui.h"
#include "config.h"
#include "config_store.h"
#include "led_strip.h"
#include "led_effects.h"
#include <ESPAsyncWebServer.h>
#include <string.h>
#include <stdio.h>

#if ENABLE_WEB_SERIAL
#include <WebSerial.h>
#endif

static char s_cmdBuf[64];

#if ENABLE_WEB_SERIAL

static const char* trimCommand(uint8_t* data, size_t len) {
  size_t copyLen = len < sizeof(s_cmdBuf) - 1 ? len : sizeof(s_cmdBuf) - 1;
  memcpy(s_cmdBuf, data, copyLen);
  s_cmdBuf[copyLen] = '\0';
  for (size_t i = 0; i < copyLen; i++) {
    if (s_cmdBuf[i] == '\r' || s_cmdBuf[i] == '\n') {
      s_cmdBuf[i] = '\0';
      break;
    }
  }
  char* p = s_cmdBuf;
  while (*p == ' ' || *p == '\r' || *p == '\n') p++;
  if (*p == '\0') return nullptr;
  return p;
}

static void reply(const char* msg) {
  WebSerial.println(msg);
}

static void replyEffect(const char* name) {
  WebSerial.print("effect: ");
  WebSerial.println(name);
}

static void onMessage(uint8_t* data, size_t len) {
  const char* cmd = trimCommand(data, len);
  if (!cmd) return;

  if (strcmp(cmd, "next") == 0) {
    replyEffect(ledEffectsNext());
    return;
  }

  if (strcmp(cmd, "prev") == 0) {
    replyEffect(ledEffectsPrev());
    return;
  }

  if (strcmp(cmd, "status") == 0 || strcmp(cmd, "effect") == 0) {
    replyEffect(ledEffectsGetCurrentName());
    WebSerial.print("brightness: ");
    WebSerial.println(ledEffectsGetBrightness());
    return;
  }

  if (strcmp(cmd, "clear") == 0) {
    ledStrip.clear();
    ledStrip.show();
    reply("ok");
    return;
  }

  if (strcmp(cmd, "config-store") == 0) {
    if (configStoreSave()) {
      reply("config stored");
    } else {
      reply("config store failed");
    }
    return;
  }

  if (strcmp(cmd, "config-clear") == 0) {
    if (configStoreClear()) {
      ledEffectsLoadDefaults();
      reply("config cleared; defaults applied");
    } else {
      reply("config clear failed");
    }
    return;
  }

  if (strncmp(cmd, "speed-", 6) == 0) {
    float v = atof(cmd + 6);
    if (v >= 0.01f && v <= 20.0f) {
      ledEffectsSetSpeed(v);
      WebSerial.print("speed: ");
      WebSerial.println(v);
    } else {
      reply("speed 0.01-20.0");
    }
    return;
  }

  if (strncmp(cmd, "fps-", 4) == 0) {
    int v = atoi(cmd + 4);
    if (v >= 10 && v <= 100) {
      ledEffectsSetTargetFps((uint8_t)v);
      WebSerial.print("fps: ");
      WebSerial.println(v);
    } else {
      reply("fps 10-100");
    }
    return;
  }

  if (strncmp(cmd, "brightness-", 11) == 0) {
    int v = atoi(cmd + 11);
    if (v >= 0 && v <= 255) {
      ledEffectsSetBrightness((uint8_t)v);
      WebSerial.print("brightness: ");
      WebSerial.println(v);
    } else {
      reply("brightness 0-255");
    }
    return;
  }

  if (strncmp(cmd, "color-", 6) == 0) {
    int r = -1, g = -1, b = -1;
    if (sscanf(cmd + 6, "%d,%d,%d", &r, &g, &b) == 3 &&
        r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
      ledEffectsSetStaticColor((uint8_t)r, (uint8_t)g, (uint8_t)b);
      ledEffectsSet(LedEffect::StaticColor);
      replyEffect("StaticColor");
      WebSerial.print("color: ");
      WebSerial.print(r);
      WebSerial.print(",");
      WebSerial.print(g);
      WebSerial.print(",");
      WebSerial.println(b);
    } else {
      reply("color-R,G,B (0-255 each)");
    }
    return;
  }

  if (strncmp(cmd, "accent-", 7) == 0) {
    int r = -1, g = -1, b = -1;
    if (sscanf(cmd + 7, "%d,%d,%d", &r, &g, &b) == 3 &&
        r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
      ledEffectsSetAccentColor((uint8_t)r, (uint8_t)g, (uint8_t)b);
      WebSerial.print("accent: ");
      WebSerial.print(r);
      WebSerial.print(",");
      WebSerial.print(g);
      WebSerial.print(",");
      WebSerial.println(b);
    } else {
      reply("accent-R,G,B (0-255 each)");
    }
    return;
  }

  // Global color: global-color-R,G,B
  if (strncmp(cmd, "global-color-", 13) == 0) {
    int r = -1, g = -1, b = -1;
    if (sscanf(cmd + 13, "%d,%d,%d", &r, &g, &b) == 3 &&
        r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
      ledEffectsSetGlobalColor((uint8_t)r, (uint8_t)g, (uint8_t)b);
      WebSerial.print("global-color: ");
      WebSerial.print(r);
      WebSerial.print(",");
      WebSerial.print(g);
      WebSerial.print(",");
      WebSerial.println(b);
    } else {
      reply("global-color-R,G,B (0-255 each)");
    }
    return;
  }

  // Per-effect color: effectname-color-R,G,B or effectname-color-default
  const char* const* effectNames = ledEffectsGetNames();
  int nameCount = ledEffectsGetNameCount();
  for (int i = 0; i < nameCount; i++) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "%s-color-", effectNames[i]);
    if (strncmp(cmd, pattern, strlen(pattern)) == 0) {
      const char* colorPart = cmd + strlen(pattern);
      if (strcmp(colorPart, "default") == 0) {
        ledEffectsResetEffectColor(static_cast<LedEffect>(i));
        WebSerial.print(effectNames[i]);
        WebSerial.println("-color: default");
      } else {
        int r = -1, g = -1, b = -1;
        if (sscanf(colorPart, "%d,%d,%d", &r, &g, &b) == 3 &&
            r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
          ledEffectsSetEffectColor(static_cast<LedEffect>(i), (uint8_t)r, (uint8_t)g, (uint8_t)b);
          WebSerial.print(effectNames[i]);
          WebSerial.print("-color: ");
          WebSerial.print(r);
          WebSerial.print(",");
          WebSerial.print(g);
          WebSerial.print(",");
          WebSerial.println(b);
        } else {
          reply("effectname-color-R,G,B or effectname-color-default");
        }
      }
      return;
    }
  }

  // Intensity: intensity-VALUE (0.1-3.0)
  if (strncmp(cmd, "intensity-", 10) == 0) {
    float v = atof(cmd + 10);
    if (v >= 0.1f && v <= 3.0f) {
      ledEffectsSetIntensity(v);
      WebSerial.print("intensity: ");
      WebSerial.println(v);
    } else {
      reply("intensity 0.1-3.0");
    }
    return;
  }

  // Fire variant: fire-variant-red or fire-variant-blue
  if (strncmp(cmd, "fire-variant-", 13) == 0) {
    const char* variant = cmd + 13;
    if (strcmp(variant, "red") == 0) {
      ledEffectsSetFireVariant(FireVariant::Red);
      reply("fire-variant: red");
    } else if (strcmp(variant, "blue") == 0) {
      ledEffectsSetFireVariant(FireVariant::Blue);
      reply("fire-variant: blue");
    } else {
      reply("fire-variant-red or fire-variant-blue");
    }
    return;
  }

  reply("unknown");
}

#endif /* ENABLE_WEB_SERIAL */

void webSerialUiBegin(AsyncWebServer* server) {
#if ENABLE_WEB_SERIAL
  if (!server) return;
  WebSerial.begin(server);
  WebSerial.onMessage(onMessage);
  reply("ready");
  replyEffect(ledEffectsGetCurrentName());
#else
  (void)server;
#endif
}

void webSerialLog(const char* msg) {
#if ENABLE_WEB_SERIAL
  if (msg) WebSerial.println(msg);
#else
  (void)msg;
#endif
}
