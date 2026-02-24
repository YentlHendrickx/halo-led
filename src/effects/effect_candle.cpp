#include "effect_context.h"
#include "effect_common.h"
#include "led_effects.h"

void effectCandle(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  uint32_t seed = (uint32_t)(t * 100.0f) * 7919;

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::Candle, cr, cg, cb);

  for (uint16_t i = 0; i < n; i++) {
    float dist = (float)i / (float)(n > 1 ? n : 1);
    uint8_t flicker = effect_common::noise8(seed + i * 31) % 40;
    uint8_t base = (uint8_t)(255 * (1.0f - dist * 0.85f));
    if (base > flicker) base -= flicker; else base = 0;
    ctx.strip->setPixelColor(i,
      effect_common::scale8(cr, base), effect_common::scale8(cg, base), effect_common::scale8(cb, base));
  }
  ctx.strip->show();
}
