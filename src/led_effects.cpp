#include "led_effects.h"
#include "led_strip.h"
#include "config.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_LEDS 256
#define FIRE_HEAT_SIZE ((MAX_LEDS) + 2)
#define CONFETTI_PARTICLES 24
#define METEOR_COUNT 5
#define SCANNER_WIDTH 12

namespace {

// Hue to RGB (0-255). Full brightness.
void hueToRgb(uint8_t hue, uint8_t& r, uint8_t& g, uint8_t& b) {
  hue = 255 - hue;
  if (hue < 85) {
    r = 255 - hue * 3;
    g = hue * 3;
    b = 0;
  } else if (hue < 170) {
    hue -= 85;
    r = hue * 3;
    g = 255;
    b = 0;
  } else {
    hue -= 170;
    r = 0;
    g = 255 - hue * 3;
    b = hue * 3;
  }
}

uint32_t hueToColor(uint8_t hue) {
  uint8_t r, g, b;
  hueToRgb(hue, r, g, b);
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

uint8_t scale8(uint8_t x, uint8_t scale) {
  return (uint16_t)x * scale >> 8;
}

// Simple pseudo-random 0-255 from seed (for deterministic effects if needed)
uint8_t noise8(uint32_t x) {
  x = (x >> 13) ^ x;
  x = (x * (x * x * 15731 + 789221) + 1376312589) & 0x7FFFFFFF;
  return (uint8_t)(x >> 23);
}

float fmodf_positive(float x, float m) {
  if (m <= 0) return 0;
  while (x >= m) x -= m;
  while (x < 0) x += m;
  return x;
}

} // namespace

// --- Engine state ---
static LedEffect s_effect = LedEffect::RainbowWave;
static float s_speed = 1.0f;
static unsigned long s_autoCycleMs = 15000;
static unsigned long s_lastCycleAt = 0;
static unsigned long s_lastFrameMs = 0;
static uint8_t s_targetFps = 50;
static unsigned long s_effectStartMs = 0;

// Fire: heat buffer (index 0 = first LED, heat propagates "up" the strip)
static uint8_t s_heat[FIRE_HEAT_SIZE];

// Confetti: phase and vel per particle; position = fmod(t*vel + phase*n, n)
struct ConfettiParticle {
  float phase;  // 0..1 start offset
  uint8_t hue;
  float vel;
};
static ConfettiParticle s_confetti[CONFETTI_PARTICLES];
static bool s_confettiInited = false;

// Meteor hues (positions derived from time)
static uint8_t s_meteorHue[METEOR_COUNT];
static bool s_meteorInited = false;

void ledEffectsBegin() {
  s_lastCycleAt = millis();
  s_lastFrameMs = millis();
  s_effectStartMs = millis();
  s_confettiInited = false;
  s_meteorInited = false;
  for (unsigned i = 0; i < FIRE_HEAT_SIZE; i++) s_heat[i] = 0;
}

void ledEffectsSetSpeed(float speed) {
  if (speed < 0.1f) speed = 0.1f;
  if (speed > 4.0f) speed = 4.0f;
  s_speed = speed;
}

float ledEffectsGetSpeed() {
  return s_speed;
}

void ledEffectsSetAutoCycleMs(unsigned long ms) {
  s_autoCycleMs = ms;
}

void ledEffectsSetTargetFps(uint8_t fps) {
  if (fps < 10) fps = 10;
  if (fps > 100) fps = 100;
  s_targetFps = fps;
}

void ledEffectsSet(LedEffect e, int brightness) {
  if (e >= LedEffect::Count) e = LedEffect::RainbowWave;
  s_effect = e;
  s_effectStartMs = millis();
  ledStrip.setBrightness(brightness);  // reset so Breathing doesn't leave strip dim
}

String ledEffectsNext(int brightness) {
  auto n = static_cast<uint8_t>(s_effect) + 1;
  if (n >= static_cast<uint8_t>(LedEffect::Count)) n = 0;
  ledEffectsSet(static_cast<LedEffect>(n), brightness);

  // return name of new effect for display
  return String(static_cast<int>(s_effect));
}

void ledEffectsPrev(int brightness) {
  auto n = static_cast<uint8_t>(s_effect);
  if (n == 0) n = static_cast<uint8_t>(LedEffect::Count) - 1;
  else n -= 1;
  ledEffectsSet(static_cast<LedEffect>(n), brightness);
}

LedEffect ledEffectsCurrent() {
  return s_effect;
}

// --- Time in seconds (scaled by speed). No delays. ---
static inline float effectTimeSec() {
  return (float)(millis() - s_effectStartMs) * 0.001f * s_speed;
}

static inline uint32_t effectTimeMs() {
  return (uint32_t)((float)(millis() - s_effectStartMs) * s_speed);
}

// Throttle: only update strip at target FPS.
static bool shouldDrawFrame() {
  unsigned long now = millis();
  unsigned long interval = 1000 / (unsigned long)s_targetFps;
  if ((now - s_lastFrameMs) >= interval) {
    s_lastFrameMs = now;
    return true;
  }
  return false;
}

// ----- Effects (all time-based, no delay) -----

static void effectRainbowWave() {
  const uint16_t n = ledStrip.numPixels();
  float t = effectTimeSec();
  float phase = t * 60.0f;  // hue travel speed
  for (uint16_t i = 0; i < n; i++) {
    float pos = (float)i / (float)(n > 1 ? n : 1);
    uint8_t hue = (uint8_t)((uint32_t)(phase * 255.0f / 6.28318f + pos * 255.0f) % 256);
    ledStrip.setPixelColor(i, hueToColor(hue));
  }
  ledStrip.show();
}

static void effectFire() {
  const uint16_t n = ledStrip.numPixels();
  const uint16_t len = n + 2;
  if (len > FIRE_HEAT_SIZE) return;

  // Cool and propagate heat (classic fire algorithm)
  for (uint16_t i = 0; i < len; i++) {
    uint8_t v = (s_heat[i] + s_heat[i + 1] + s_heat[i + 2]) / 3;
    if (v > 2) v -= 2;
    else v = 0;
    s_heat[i] = v;
  }

  // Spark at bottom (index len-1 is "below" first LED)
  uint32_t seed = effectTimeMs() * 7919;
  if (random(0, 100) < 45) {
    uint8_t spark = 180 + (noise8(seed) % 76);
    s_heat[len - 1] = (s_heat[len - 1] + spark) > 255 ? 255 : s_heat[len - 1] + spark;
  }
  s_heat[len - 2] = s_heat[len - 1];
  s_heat[len - 1] = 0;

  // Heat to color (black -> red -> orange -> yellow -> white)
  for (uint16_t i = 0; i < n; i++) {
    uint8_t h = s_heat[i + 2];
    uint8_t r = h;
    uint8_t g = scale8(h, 80);
    uint8_t b = h > 160 ? (h - 160) * 2 : 0;
    ledStrip.setPixelColor(i, r, g, b);
  }
  ledStrip.show();
}

static void effectMeteorShower() {
  const uint16_t n = ledStrip.numPixels();
  float t = effectTimeSec();
  float tailLen = (float)n * 0.15f;
  if (tailLen < 8) tailLen = 8;
  float cycle = (float)n + tailLen;
  float speed = cycle * 0.4f;  // one full cycle per ~2.5 sec

  if (!s_meteorInited) {
    for (int i = 0; i < METEOR_COUNT; i++)
      s_meteorHue[i] = (i * 51) % 256;
    s_meteorInited = true;
  }

  ledStrip.clear();
  for (int m = 0; m < METEOR_COUNT; m++) {
    float phase = (float)m / (float)METEOR_COUNT;
    float head = fmodf_positive(t * speed + phase * cycle, cycle);
    uint8_t hr, hg, hb;
    hueToRgb(s_meteorHue[m], hr, hg, hb);
    for (float ti = 0; ti <= tailLen; ti += 1.0f) {
      int16_t idx = (int16_t)(head - ti);
      if (idx < 0 || (uint16_t)idx >= n) continue;
      float f = 1.0f - ti / tailLen;
      uint8_t bright = (uint8_t)(255.0f * f * f);
      ledStrip.setPixelColor((uint16_t)idx,
        scale8(hr, bright), scale8(hg, bright), scale8(hb, bright));
    }
  }
  ledStrip.show();
}

static void effectBreathing() {
  float t = effectTimeSec();
  float breath = 0.5f + 0.5f * (float)sin((double)(t * 0.5 * 2.0 * M_PI));
  uint8_t b = (uint8_t)(30 + breath * 225);
  ledStrip.setBrightness(b);
  ledStrip.fill(255, 60, 20);
  ledStrip.show();
}

static void effectComet() {
  const uint16_t n = ledStrip.numPixels();
  float t = effectTimeSec();
  float speed = (float)n * 0.25f;
  float head = fmodf_positive(t * speed, (float)(n + 40));
  float tailLen = 28.0f;

  ledStrip.clear();
  for (float i = 0; i <= tailLen; i += 1.0f) {
    int16_t idx = (int16_t)(head - i);
    if (idx < 0 || (uint16_t)idx >= n) continue;
    uint8_t bright = (uint8_t)(255.0f * (1.0f - i / tailLen));
    ledStrip.setPixelColor((uint16_t)idx, bright, bright / 2, 0);
  }
  ledStrip.show();
}

static void effectPlasma() {
  const uint16_t n = ledStrip.numPixels();
  float t = effectTimeSec();
  for (uint16_t i = 0; i < n; i++) {
    float x = (float)i / (float)(n > 1 ? n : 1);
    float v = (float)sin((double)(x * 4.0 * M_PI + t * 2.0)) * 0.5f + 0.5f;
    v += (float)sin((double)(x * 8.0 * M_PI - t * 1.5)) * 0.3f;
    v += (float)sin((double)((x + t * 0.3) * 12.0 * M_PI)) * 0.2f;
    if (v < 0) v = 0;
    if (v > 1) v = 1;
    uint8_t hue = (uint8_t)(v * 255.0f);
    ledStrip.setPixelColor(i, hueToColor(hue));
  }
  ledStrip.show();
}

static void effectConfetti() {
  const uint16_t n = ledStrip.numPixels();
  if (n == 0) return;
  float tSec = effectTimeSec();

  if (!s_confettiInited) {
    for (int i = 0; i < CONFETTI_PARTICLES; i++) {
      s_confetti[i].phase = (float)i / (float)CONFETTI_PARTICLES;
      s_confetti[i].hue = (uint8_t)((i * 37) % 256);
      s_confetti[i].vel = 18.0f + (noise8((uint32_t)i * 123) % 20);
    }
    s_confettiInited = true;
  }

  ledStrip.clear();
  float nf = (float)n;
  for (int i = 0; i < CONFETTI_PARTICLES; i++) {
    float pos = fmodf_positive(tSec * s_confetti[i].vel + s_confetti[i].phase * nf, nf);
    uint16_t idx = (uint16_t)pos;
    if (idx < n) {
      uint8_t r, g, b;
      hueToRgb(s_confetti[i].hue, r, g, b);
      ledStrip.setPixelColor(idx, r, g, b);
    }
  }
  ledStrip.show();
}

static void effectTwinkle() {
  const uint16_t n = ledStrip.numPixels();
  float t = effectTimeSec();
  for (uint16_t i = 0; i < n; i++) {
    float phase = (float)i * 0.17f + t * 3.0f;
    float v = (float)sin((double)phase) * 0.5f + 0.5f;
    v = v * v;  // sharper falloff
    uint8_t hue = (uint8_t)((uint32_t)(t * 40.0f + (float)i * 2.0f) % 256);
    uint8_t r, g, b;
    hueToRgb(hue, r, g, b);
    ledStrip.setPixelColor(i, scale8(r, (uint8_t)(v * 255)), scale8(g, (uint8_t)(v * 255)), scale8(b, (uint8_t)(v * 255)));
  }
  ledStrip.show();
}

static void effectPride() {
  const uint16_t n = ledStrip.numPixels();
  float t = effectTimeSec();
  float phase = t * 80.0f;
  for (uint16_t i = 0; i < n; i++) {
    float pos = (float)i / (float)(n > 1 ? n : 1);
    uint8_t hue = (uint8_t)((uint32_t)((uint32_t)(phase + pos * 255.0f)) % 256);
    ledStrip.setPixelColor(i, hueToColor(hue));
  }
  ledStrip.show();
}

static void effectScanner() {
  const uint16_t n = ledStrip.numPixels();
  float t = effectTimeSec();
  float period = (float)(n + SCANNER_WIDTH * 2) * 0.03f;
  float phase = fmodf_positive(t, period * 2.0f);
  float pos;
  if (phase < period)
    pos = phase / 0.03f;
  else
    pos = (period * 2.0f - phase) / 0.03f;

  ledStrip.clear();
  for (int w = -SCANNER_WIDTH; w <= SCANNER_WIDTH; w++) {
    int16_t idx = (int16_t)(pos + w);
    if (idx < 0 || (uint16_t)idx >= n) continue;
    uint8_t bright = 255 - (uint8_t)((abs(w) * 255) / (SCANNER_WIDTH + 1));
    ledStrip.setPixelColor((uint16_t)idx, 0, bright, 0);
  }
  ledStrip.show();
}

static void effectColorWipe() {
  const uint16_t n = ledStrip.numPixels();
  float t = effectTimeSec();
  float period = (float)n * 0.03f;
  float phase = fmodf_positive(t, period * 2.0f);
  uint16_t edge = phase < period ? (uint16_t)(phase / 0.03f) : (uint16_t)((period * 2.0f - phase) / 0.03f);
  if (edge > n) edge = n;

  for (uint16_t i = 0; i < n; i++) {
    if (i < edge)
      ledStrip.setPixelColor(i, 0, 0, 150);
    else
      ledStrip.setPixelColor(i, 0, 0, 0);
  }
  ledStrip.show();
}

static void effectTheaterChase() {
  const uint16_t n = ledStrip.numPixels();
  float t = effectTimeSec();
  const uint8_t spacing = 3;
  float pos = fmodf_positive(t * 25.0f, (float)spacing);
  uint16_t offset = (uint16_t)pos;

  for (uint16_t i = 0; i < n; i++) {
    if ((i + offset) % spacing == 0)
      ledStrip.setPixelColor(i, 255, 255, 255);
    else
      ledStrip.setPixelColor(i, 0, 0, 0);
  }
  ledStrip.show();
}

void ledEffectsUpdate() {
  if (!shouldDrawFrame())
    return;

  switch (s_effect) {
    case LedEffect::RainbowWave:  effectRainbowWave();  break;
    case LedEffect::Fire:         effectFire();         break;
    case LedEffect::MeteorShower: effectMeteorShower(); break;
    case LedEffect::Breathing:    effectBreathing();    break;
    case LedEffect::Comet:        effectComet();        break;
    case LedEffect::Plasma:       effectPlasma();       break;
    case LedEffect::Confetti:     effectConfetti();     break;
    case LedEffect::Twinkle:      effectTwinkle();      break;
    case LedEffect::Pride:        effectPride();        break;
    case LedEffect::Scanner:      effectScanner();      break;
    case LedEffect::ColorWipe:    effectColorWipe();    break;
    case LedEffect::TheaterChase: effectTheaterChase(); break;
    default:                      effectRainbowWave(); break;
  }
}
