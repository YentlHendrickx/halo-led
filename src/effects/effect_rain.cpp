#include "effect_common.h"
#include "effect_context.h"
#include "led_effects.h"

#define RAIN_DROPS 12

void effectRain(EffectContext &ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  if (n == 0)
    return;

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::Rain, cr, cg, cb);

  float fallSpeed = (float)n * 0.5f;
  ctx.strip->clear();
  for (int d = 0; d < RAIN_DROPS; d++) {
    float phase = (float)d / (float)RAIN_DROPS;
    float y = effect_common::fmodf_positive(
        phase * (float)(n + 5) - t * fallSpeed, (float)(n + 5));
    int16_t idx = (int16_t)y;
    if (idx >= 0 && (uint16_t)idx < n)
      ctx.strip->setPixelColor((uint16_t)idx, cr, cg, cb);
  }
  ctx.strip->show();
}
