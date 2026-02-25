#ifndef EFFECT_CONTEXT_H
#define EFFECT_CONTEXT_H

#include "led_effects.h"
#include "led_strip.h"

// Filled by led_effects and passed to effect_xxx.cpp. No engine state in effect
// files.
struct EffectContext {
  LedStrip *strip;

  float timeSec;
  uint32_t timeMs;

  float intensity;
  float segmentSize; // e.g. LEDs per bit (BinaryCounter); general segment scale
  float speed;
  FireVariant fireVariant;

  uint8_t staticR, staticG, staticB;
  uint8_t brightness;

  void (*getEffectColor)(LedEffect e, uint8_t &r, uint8_t &g, uint8_t &b);
};

#include "effects_decl.h"

#endif
