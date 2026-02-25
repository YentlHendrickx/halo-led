#include "effect_common.h"
#include "effect_context.h"
#include "led_effects.h"

void effectTheaterChase(EffectContext &ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  const uint8_t spacing = 3;
  uint16_t offset =
      (uint16_t)effect_common::fmodf_positive(t * 25.0f, (float)spacing);

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::TheaterChase, cr, cg, cb);

  for (uint16_t i = 0; i < n; i++) {
    if ((i + offset) % spacing == 0)
      ctx.strip->setPixelColor(i, cr, cg, cb);
    else
      ctx.strip->setPixelColor(i, 0, 0, 0);
  }
  ctx.strip->show();
}
