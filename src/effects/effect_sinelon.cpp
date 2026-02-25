#include "effect_common.h"
#include "effect_context.h"
#include "led_effects.h"

void effectSinelon(EffectContext &ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  float period = (float)(n * 2 - 2) * 0.04f;
  float phase = effect_common::fmodf_positive(t, period);
  float pos;
  if (phase < (float)(n - 1) * 0.04f)
    pos = phase / 0.04f;
  else
    pos = (float)(n - 1) - (phase - (float)(n - 1) * 0.04f) / 0.04f;

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::Sinelon, cr, cg, cb);

  ctx.strip->clear();
  float trailLen = 15.0f * ctx.intensity;
  for (float i = 0; i <= trailLen; i += 1.0f) {
    int16_t idx = (int16_t)(pos - i);
    if (idx < 0)
      continue;
    if ((uint16_t)idx >= n)
      continue;
    uint8_t bright = (uint8_t)(255.0f * (1.0f - i / trailLen));
    ctx.strip->setPixelColor((uint16_t)idx, effect_common::scale8(cr, bright),
                             effect_common::scale8(cg, bright),
                             effect_common::scale8(cb, bright));
  }
  ctx.strip->show();
}
