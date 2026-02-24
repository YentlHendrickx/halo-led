#include "effect_context.h"
#include "effect_common.h"
#include "led_effects.h"

void effectStarfield(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  t = effect_common::fmodf_positive(t, 1000.0f);
  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::Starfield, cr, cg, cb);
  ctx.strip->clear();
  for (uint16_t i = 0; i < n; i++) {
    float timeComponent = effect_common::fmodf_positive(t * 100.0f, 100000.0f);
    uint32_t seed = (uint32_t)(i * 31 + timeComponent) * 7919;
    float v = (float)(effect_common::noise8(seed) % 256) / 255.0f;
    if (v > 0.92f) {
      uint8_t bright = (uint8_t)((v - 0.92f) / 0.08f * 255.0f);
      ctx.strip->setPixelColor(i, effect_common::scale8(cr, bright), effect_common::scale8(cg, bright), effect_common::scale8(cb, bright));
    }
  }
  ctx.strip->show();
}
