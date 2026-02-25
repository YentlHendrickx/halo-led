#include "led_effects.h"
#include "effect_context.h"
#include "led_strip.h"
#include <climits>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Effect names (must match enum order).
static const char *const EFFECT_NAMES[] = {
    "Aurora",  "Fire",         "FireTop",      "MeteorShower", "Breathing",
    "Comet",   "Plasma",       "Confetti",     "Twinkle",      "Pride",
    "Scanner", "ColorWipe",    "TheaterChase", "StaticColor",  "Rain",
    "Candle",  "RainbowCycle", "Rule30",       "Starfield",    "Sinelon",
    "Noise"};

// --- Engine state ---
static LedEffect s_effect = LedEffect::Aurora;
static float s_speed = 1.0f;
static unsigned long s_autoCycleMs = 15000;
static unsigned long s_lastCycleAt = 0;
static unsigned long s_lastFrameMs = 0;
static uint8_t s_targetFps = 50;
static unsigned long s_effectStartMs = 0;
static uint8_t s_brightness = 200;
static uint8_t s_staticR = 255, s_staticG = 100, s_staticB = 50;
static uint8_t s_accentR = 255, s_accentG = 120,
               s_accentB = 40; // warm orange default
// Global color (used by effects without custom colors)
static uint8_t s_globalR = 255, s_globalG = 120,
               s_globalB = 40; // warm orange default

// Per-effect colors (nullptr = use global/default)
struct EffectColor {
  uint8_t r, g, b;
  bool custom; // true if custom color set, false if using default
};
static EffectColor s_effectColors[static_cast<int>(LedEffect::Count)];

// Intensity parameter (default 1.0, scales effect sizes)
static float s_intensity = 1.0f;

// Fire variant
static FireVariant s_fireVariant = FireVariant::Red;

// Default colors for each effect
static void initDefaultColors() {
  // Initialize all effects to use defaults (no custom colors)
  for (int i = 0; i < static_cast<int>(LedEffect::Count); i++) {
    s_effectColors[i].custom = false;
    s_effectColors[i].r = 0;
    s_effectColors[i].g = 0;
    s_effectColors[i].b = 0;
  }
}

void ledEffectsBegin() {
  s_lastCycleAt = millis();
  s_lastFrameMs = millis();
  s_effectStartMs = millis();
  s_brightness = 200;
  s_intensity = 1.0f;
  s_fireVariant = FireVariant::Red;
  initDefaultColors();
  ledStrip.setBrightness(s_brightness);
}

void ledEffectsLoadDefaults() {
  ledEffectsSet(LedEffect::Aurora);
  ledEffectsSetSpeed(1.0f);
  ledEffectsSetTargetFps(50);
  ledEffectsSetBrightness(200);
  ledEffectsSetStaticColor(255, 100, 50);
  ledEffectsSetAccentColor(255, 120, 40);
  ledEffectsSetGlobalColor(255, 120, 40);
  ledEffectsSetIntensity(1.0f);
  ledEffectsSetFireVariant(FireVariant::Red);
  ledEffectsSetAutoCycleMs(15000);
  for (int i = 0; i < static_cast<int>(LedEffect::Count); i++) {
    ledEffectsResetEffectColor(static_cast<LedEffect>(i));
  }
}

void ledEffectsSetBrightness(uint8_t b) {
  s_brightness = b;
  ledStrip.setBrightness(b);
}

uint8_t ledEffectsGetBrightness() { return s_brightness; }

const char *ledEffectsGetCurrentName() {
  uint8_t idx = static_cast<uint8_t>(s_effect);
  if (idx >= sizeof(EFFECT_NAMES) / sizeof(EFFECT_NAMES[0]))
    return "?";
  return EFFECT_NAMES[idx];
}

const char *const *ledEffectsGetNames() { return EFFECT_NAMES; }

int ledEffectsGetNameCount() { return static_cast<int>(LedEffect::Count); }

void ledEffectsSetStaticColor(uint8_t r, uint8_t g, uint8_t b) {
  s_staticR = r;
  s_staticG = g;
  s_staticB = b;
}

void ledEffectsGetStaticColor(uint8_t &r, uint8_t &g, uint8_t &b) {
  r = s_staticR;
  g = s_staticG;
  b = s_staticB;
}

void ledEffectsSetSpeed(float speed) {
  if (speed < 0.1f)
    speed = 0.1f;
  if (speed > 4.0f)
    speed = 4.0f;
  s_speed = speed;
}

float ledEffectsGetSpeed() { return s_speed; }

void ledEffectsSetAutoCycleMs(unsigned long ms) { s_autoCycleMs = ms; }

void ledEffectsSetTargetFps(uint8_t fps) {
  if (fps < 10)
    fps = 10;
  if (fps > 100)
    fps = 100;
  s_targetFps = fps;
}

uint8_t ledEffectsGetTargetFps() { return s_targetFps; }

void ledEffectsSet(LedEffect e) {
  if (e >= LedEffect::Count)
    e = LedEffect::Aurora;
  s_effect = e;
  s_effectStartMs = millis();
  ledStrip.setBrightness(s_brightness);
  if (e == LedEffect::Rule30)
    effectRule30Reset();
}

void ledEffectsSetAccentColor(uint8_t r, uint8_t g, uint8_t b) {
  s_accentR = r;
  s_accentG = g;
  s_accentB = b;
}

void ledEffectsGetAccentColor(uint8_t &r, uint8_t &g, uint8_t &b) {
  r = s_accentR;
  g = s_accentG;
  b = s_accentB;
}

// Get color for an effect (custom -> global -> default accent)
static void getEffectColor(LedEffect e, uint8_t &r, uint8_t &g, uint8_t &b) {
  int idx = static_cast<int>(e);
  if (idx >= 0 && idx < static_cast<int>(LedEffect::Count)) {
    if (s_effectColors[idx].custom) {
      r = s_effectColors[idx].r;
      g = s_effectColors[idx].g;
      b = s_effectColors[idx].b;
      return;
    }
  }
  // Fall back to global color
  r = s_globalR;
  g = s_globalG;
  b = s_globalB;
}

static void contextGetEffectColor(LedEffect e, uint8_t &r, uint8_t &g,
                                  uint8_t &b) {
  getEffectColor(e, r, g, b);
}

// --- Time (scaled by speed). Used to fill context. ---
static inline float effectTimeSec() {
  unsigned long now = millis();
  unsigned long elapsed;
  if (now >= s_effectStartMs) {
    elapsed = now - s_effectStartMs;
  } else {
    elapsed = (ULONG_MAX - s_effectStartMs) + now + 1;
  }
  elapsed = elapsed % 86400000UL;
  return (float)elapsed * 0.001f * s_speed;
}

static inline uint32_t effectTimeMs() {
  unsigned long now = millis();
  unsigned long elapsed;
  if (now >= s_effectStartMs) {
    elapsed = now - s_effectStartMs;
  } else {
    elapsed = (ULONG_MAX - s_effectStartMs) + now + 1;
  }
  elapsed = elapsed % 86400000UL;
  float scaled = (float)elapsed * s_speed;
  if (scaled > 4294967295.0f)
    scaled = fmodf(scaled, 4294967295.0f);
  return (uint32_t)scaled;
}

static bool shouldDrawFrame() {
  unsigned long now = millis();
  unsigned long interval = 1000 / (unsigned long)s_targetFps;
  unsigned long elapsed;
  if (now >= s_lastFrameMs) {
    elapsed = now - s_lastFrameMs;
  } else {
    elapsed = (ULONG_MAX - s_lastFrameMs) + now + 1;
  }
  if (elapsed >= interval) {
    s_lastFrameMs = now;
    return true;
  }
  return false;
}

void ledEffectsFillContext(EffectContext *ctx) {
  if (!ctx)
    return;
  ctx->strip = &ledStrip;
  ctx->timeSec = effectTimeSec();
  ctx->timeMs = effectTimeMs();
  ctx->intensity = s_intensity;
  ctx->speed = s_speed;
  ctx->fireVariant = s_fireVariant;
  ctx->staticR = s_staticR;
  ctx->staticG = s_staticG;
  ctx->staticB = s_staticB;
  ctx->brightness = s_brightness;
  ctx->getEffectColor = contextGetEffectColor;
}

void ledEffectsSetEffectColor(LedEffect e, uint8_t r, uint8_t g, uint8_t b) {
  int idx = static_cast<int>(e);
  if (idx >= 0 && idx < static_cast<int>(LedEffect::Count)) {
    s_effectColors[idx].r = r;
    s_effectColors[idx].g = g;
    s_effectColors[idx].b = b;
    s_effectColors[idx].custom = true;
  }
}

void ledEffectsGetEffectColor(LedEffect e, uint8_t &r, uint8_t &g, uint8_t &b) {
  getEffectColor(e, r, g, b);
}

void ledEffectsResetEffectColor(LedEffect e) {
  int idx = static_cast<int>(e);
  if (idx >= 0 && idx < static_cast<int>(LedEffect::Count)) {
    s_effectColors[idx].custom = false;
  }
}

bool ledEffectsHasEffectColor(LedEffect e) {
  int idx = static_cast<int>(e);
  if (idx >= 0 && idx < static_cast<int>(LedEffect::Count)) {
    return s_effectColors[idx].custom;
  }
  return false;
}

void ledEffectsSetGlobalColor(uint8_t r, uint8_t g, uint8_t b) {
  s_globalR = r;
  s_globalG = g;
  s_globalB = b;
}

void ledEffectsGetGlobalColor(uint8_t &r, uint8_t &g, uint8_t &b) {
  r = s_globalR;
  g = s_globalG;
  b = s_globalB;
}

void ledEffectsSetIntensity(float intensity) {
  if (intensity < 0.1f)
    intensity = 0.1f;
  if (intensity > 3.0f)
    intensity = 3.0f;
  s_intensity = intensity;
}

float ledEffectsGetIntensity() { return s_intensity; }

void ledEffectsSetFireVariant(FireVariant variant) { s_fireVariant = variant; }

FireVariant ledEffectsGetFireVariant() { return s_fireVariant; }

bool ledEffectsSetByName(const char *name) {
  if (!name)
    return false;
  const size_t n = sizeof(EFFECT_NAMES) / sizeof(EFFECT_NAMES[0]);
  for (size_t i = 0; i < n; i++) {
    if (strcmp(EFFECT_NAMES[i], name) == 0) {
      ledEffectsSet(static_cast<LedEffect>(i));
      return true;
    }
  }
  return false;
}

const char *ledEffectsNext() {
  auto n = static_cast<uint8_t>(s_effect) + 1;
  if (n >= static_cast<uint8_t>(LedEffect::Count))
    n = 0;
  ledEffectsSet(static_cast<LedEffect>(n));
  return ledEffectsGetCurrentName();
}

const char *ledEffectsPrev() {
  auto n = static_cast<uint8_t>(s_effect);
  if (n == 0)
    n = static_cast<uint8_t>(LedEffect::Count) - 1;
  else
    n -= 1;
  ledEffectsSet(static_cast<LedEffect>(n));
  return ledEffectsGetCurrentName();
}

LedEffect ledEffectsCurrent() { return s_effect; }

void ledEffectsDispatchUpdate(EffectContext *ctx) {
  if (!ctx)
    return;
  switch (s_effect) {
  case LedEffect::RainbowCycle:
    effectRainbowCycle(*ctx);
    break;
  case LedEffect::Fire:
    effectFire(*ctx);
    break;
  case LedEffect::FireTop:
    effectFireTop(*ctx);
    break;
  case LedEffect::MeteorShower:
    effectMeteorShower(*ctx);
    break;
  case LedEffect::Breathing:
    effectBreathing(*ctx);
    break;
  case LedEffect::Comet:
    effectComet(*ctx);
    break;
  case LedEffect::Plasma:
    effectPlasma(*ctx);
    break;
  case LedEffect::Confetti:
    effectConfetti(*ctx);
    break;
  case LedEffect::Twinkle:
    effectTwinkle(*ctx);
    break;
  case LedEffect::Pride:
    effectPride(*ctx);
    break;
  case LedEffect::Scanner:
    effectScanner(*ctx);
    break;
  case LedEffect::ColorWipe:
    effectColorWipe(*ctx);
    break;
  case LedEffect::TheaterChase:
    effectTheaterChase(*ctx);
    break;
  case LedEffect::StaticColor:
    effectStaticColor(*ctx);
    break;
  case LedEffect::Rain:
    effectRain(*ctx);
    break;
  case LedEffect::Candle:
    effectCandle(*ctx);
    break;
  case LedEffect::Aurora:
    effectAurora(*ctx);
    break;
  case LedEffect::Rule30:
    effectRule30(*ctx);
    break;
  case LedEffect::Starfield:
    effectStarfield(*ctx);
    break;
  case LedEffect::Sinelon:
    effectSinelon(*ctx);
    break;
  case LedEffect::Noise:
    effectNoise(*ctx);
    break;
  default:
    effectAurora(*ctx);
    break;
  }
}

void ledEffectsUpdate() {
  if (!shouldDrawFrame())
    return;
  EffectContext ctx;
  ledEffectsFillContext(&ctx);
  ledEffectsDispatchUpdate(&ctx);
}
