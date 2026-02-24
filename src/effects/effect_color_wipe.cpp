#include "effect_context.h"
#include "effect_common.h"
#include "led_effects.h"

void effectColorWipe(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  float period = (float)n * 0.03f;
  float phase = effect_common::fmodf_positive(t, period * 2.0f);
  uint16_t edge = phase < period ? (uint16_t)(phase / 0.03f) : (uint16_t)((period * 2.0f - phase) / 0.03f);
  if (edge > n) edge = n;

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::ColorWipe, cr, cg, cb);

  for (uint16_t i = 0; i < n; i++) {
    if (i < edge)
      ctx.strip->setPixelColor(i, cr, cg, cb);
    else
      ctx.strip->setPixelColor(i, 0, 0, 0);
  }
  ctx.strip->show();
}
