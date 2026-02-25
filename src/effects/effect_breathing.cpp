#include "effect_common.h"
#include "effect_context.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void effectBreathing(EffectContext &ctx) {
  float t = ctx.timeSec;
  t = effect_common::fmodf_positive(t, 2.0f);
  float breath = 0.5f + 0.5f * (float)sin((double)(t * 0.5 * 2.0 * M_PI));
  uint8_t b = (uint8_t)((30 + breath * 225) * (uint32_t)ctx.brightness / 255);
  if (b > 255)
    b = 255;
  ctx.strip->setBrightness(b);

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::ColorWipe, cr, cg, cb);
  ctx.strip->fill(cr, cg, cb);
  ctx.strip->show();
}
