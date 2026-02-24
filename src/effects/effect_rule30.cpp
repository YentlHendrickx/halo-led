#include "effect_context.h"
#include "effect_common.h"
#include "led_effects.h"
#include "config.h"
#include <Arduino.h>
#include <limits.h>

#define RULE30_SIZE LED_COUNT
static uint8_t s_rule30[RULE30_SIZE];
static bool s_rule30Inited = false;
static unsigned long s_rule30LastStep = 0;

void effectRule30Reset() {
  s_rule30Inited = false;
}

void effectRule30(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  if (n == 0) return;
  if (n > RULE30_SIZE) return;

  if (!s_rule30Inited) {
    for (uint16_t i = 0; i < n; i++) s_rule30[i] = 0;
    s_rule30[n / 2] = 1;
    s_rule30Inited = true;
    s_rule30LastStep = millis();
  }

  unsigned long now = millis();
  unsigned long elapsed;
  if (now >= s_rule30LastStep) {
    elapsed = now - s_rule30LastStep;
  } else {
    elapsed = (ULONG_MAX - s_rule30LastStep) + now + 1;
  }
  if (elapsed >= 100) {
    s_rule30LastStep = now;
    uint8_t prev = 0;
    for (uint16_t i = 0; i < n; i++) {
      uint8_t left = prev;
      uint8_t self = s_rule30[i];
      uint8_t right = (i + 1 < n) ? s_rule30[i + 1] : 0;
      prev = self;
      s_rule30[i] = (left ^ (self | right)) & 1;
    }
  }

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::Rule30, cr, cg, cb);

  for (uint16_t i = 0; i < n; i++) {
    uint8_t v = s_rule30[i];
    ctx.strip->setPixelColor(i, effect_common::scale8(cr, v * 255), effect_common::scale8(cg, v * 255), effect_common::scale8(cb, v * 255));
  }
  ctx.strip->show();
}
