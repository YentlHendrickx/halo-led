#ifndef LED_EFFECTS_H
#define LED_EFFECTS_H

#include <Arduino.h>

// All animations are time-based (millis()). No delays. Speed scales animation rate.
// Call ledEffectsUpdate() every loop; it will throttle strip updates internally.

enum class LedEffect {
  RainbowWave,
  Fire,
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
  Count
};

// Initialize. Call once after ledStrip.begin().
void ledEffectsBegin();

// Run current effect. Call every loop(); uses millis() only, no blocking.
void ledEffectsUpdate();

void ledEffectsSet(LedEffect e);
String ledEffectsNext(int brightness);
void ledEffectsPrev(int brightness);
LedEffect ledEffectsCurrent();

// Speed multiplier: 0.5 = half speed, 2.0 = double speed. Default 1.0.
void ledEffectsSetSpeed(float speed);
float ledEffectsGetSpeed();

// Auto-cycle to next effect after this many ms. 0 = disabled. Default 15000.
void ledEffectsSetAutoCycleMs(unsigned long ms);

// Target FPS for strip updates (throttle). Default 50. No delay() used.
void ledEffectsSetTargetFps(uint8_t fps);

#endif
