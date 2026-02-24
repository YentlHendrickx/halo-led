#ifndef LED_EFFECTS_H
#define LED_EFFECTS_H

#include <Arduino.h>

// All animations are time-based (millis()). No delays. Speed scales animation rate.
// LED 0 = bottom (vertical mount).

enum class LedEffect {
  Aurora,
  Fire,
  FireTop,
  MeteorShower,
  Breathing,
  Comet,
  Plasma,
  Confetti,
  Twinkle,
  Pride,
  Scanner,
  ColorWipe,
  TheaterChase,
  StaticColor,
  Rain,
  Candle,
  RainbowCycle,
  Rule30,
  Starfield,
  Sinelon,
  Noise,
  Count
};

const char* ledEffectsGetCurrentName();
const char* const* ledEffectsGetNames();
int ledEffectsGetNameCount();
void ledEffectsSetStaticColor(uint8_t r, uint8_t g, uint8_t b);
void ledEffectsGetStaticColor(uint8_t& r, uint8_t& g, uint8_t& b);

// Accent color for effects that support it (Comet, Scanner, ColorWipe, TheaterChase, Rain, Candle, Twinkle, etc.)
void ledEffectsSetAccentColor(uint8_t r, uint8_t g, uint8_t b);
void ledEffectsGetAccentColor(uint8_t& r, uint8_t& g, uint8_t& b);

// Per-effect color management
void ledEffectsSetEffectColor(LedEffect e, uint8_t r, uint8_t g, uint8_t b);
void ledEffectsGetEffectColor(LedEffect e, uint8_t& r, uint8_t& g, uint8_t& b);
void ledEffectsResetEffectColor(LedEffect e);  // Revert to default
bool ledEffectsHasEffectColor(LedEffect e);  // Check if effect has custom color

// Global color (used by effects without custom colors)
void ledEffectsSetGlobalColor(uint8_t r, uint8_t g, uint8_t b);
void ledEffectsGetGlobalColor(uint8_t& r, uint8_t& g, uint8_t& b);

// Intensity parameter (scales effect sizes, e.g., fire height)
void ledEffectsSetIntensity(float intensity);
float ledEffectsGetIntensity();

void ledEffectsBegin();
void ledEffectsLoadDefaults();  // Reset all user settings to defaults (for config clear / no saved config)
void ledEffectsUpdate();

void ledEffectsSet(LedEffect e);
bool ledEffectsSetByName(const char* name);
const char* ledEffectsNext();
const char* ledEffectsPrev();
LedEffect ledEffectsCurrent();

void ledEffectsSetBrightness(uint8_t b);
uint8_t ledEffectsGetBrightness();
void ledEffectsSetSpeed(float speed);
float ledEffectsGetSpeed();
void ledEffectsSetAutoCycleMs(unsigned long ms);
void ledEffectsSetTargetFps(uint8_t fps);
uint8_t ledEffectsGetTargetFps();

// Fire variant: Red or Blue
enum class FireVariant {
  Red,
  Blue
};
void ledEffectsSetFireVariant(FireVariant variant);
FireVariant ledEffectsGetFireVariant();

struct EffectContext;
void ledEffectsFillContext(EffectContext* ctx);
void ledEffectsDispatchUpdate(EffectContext* ctx);

#endif
