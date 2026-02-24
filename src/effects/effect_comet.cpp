#include "effect_context.h"
#include "effect_common.h"
#include "led_effects.h"

void effectComet(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  float speed = (float)n * 0.25f * ctx.intensity;
  float head = effect_common::fmodf_positive(t * speed, (float)(n + 40));
  float tailLen = 28.0f * ctx.intensity;

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::Comet, cr, cg, cb);

  ctx.strip->clear();
  for (float i = 0; i <= tailLen; i += 1.0f) {
    int16_t idx = (int16_t)(head - i);
    if (idx < 0 || (uint16_t)idx >= n) continue;
    uint8_t bright = (uint8_t)(255.0f * (1.0f - i / tailLen));
    ctx.strip->setPixelColor((uint16_t)idx,
      effect_common::scale8(cr, bright), effect_common::scale8(cg, bright), effect_common::scale8(cb, bright));
  }
  ctx.strip->show();
}
