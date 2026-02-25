#include "effect_common.h"
#include "effect_context.h"
#include "led_effects.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void effectTwinkle(EffectContext &ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  t = effect_common::fmodf_positive(t, 2.0f * (float)M_PI / 3.0f);
  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::Twinkle, cr, cg, cb);
  for (uint16_t i = 0; i < n; i++) {
    float phase = (float)i * 0.17f + t * 3.0f;
    float v = (float)sin((double)phase) * 0.5f + 0.5f;
    v = v * v;
    uint8_t bright = (uint8_t)(v * 255);
    ctx.strip->setPixelColor(i, effect_common::scale8(cr, bright),
                             effect_common::scale8(cg, bright),
                             effect_common::scale8(cb, bright));
  }
  ctx.strip->show();
}
