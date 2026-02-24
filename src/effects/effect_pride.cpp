#include "effect_context.h"
#include "effect_common.h"

void effectPride(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  t = effect_common::fmodf_positive(t, 100.0f);
  float phase = t * 80.0f;
  for (uint16_t i = 0; i < n; i++) {
    float pos = (float)i / (float)(n > 1 ? n : 1);
    float hueFloat = effect_common::fmodf_positive(phase + pos * 255.0f, 256.0f);
    uint8_t hue = (uint8_t)hueFloat;
    ctx.strip->setPixelColor(i, effect_common::hueToColor(hue));
  }
  ctx.strip->show();
}
