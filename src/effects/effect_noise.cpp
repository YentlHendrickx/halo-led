#include "effect_context.h"
#include "effect_common.h"
#include "led_effects.h"

void effectNoise(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  t = effect_common::fmodf_positive(t, 1000.0f);
  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::Noise, cr, cg, cb);
  float scroll = t * 20.0f;
  scroll = effect_common::fmodf_positive(scroll, 100000.0f);
  for (uint16_t i = 0; i < n; i++) {
    float x = (float)i * 0.1f + scroll;
    x = effect_common::fmodf_positive(x, 100000.0f);
    float v = (float)(effect_common::noise8((uint32_t)(x * 100.0f) & 0x7FFFFFFF) % 256) / 255.0f;
    v += (float)(effect_common::noise8((uint32_t)(x * 47.0f + 1000.0f) & 0x7FFFFFFF) % 256) / 255.0f * 0.5f;
    if (v > 1.0f) v = 1.0f;
    uint8_t bright = (uint8_t)(v * 220.0f);
    ctx.strip->setPixelColor(i, effect_common::scale8(cr, bright), effect_common::scale8(cg, bright), effect_common::scale8(cb, bright));
  }
  ctx.strip->show();
}
