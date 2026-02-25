#include "effect_common.h"
#include "effect_context.h"

void effectRainbowCycle(EffectContext &ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  t = effect_common::fmodf_positive(t, 10.0f);
  float hueFloat = effect_common::fmodf_positive(t * 80.0f, 256.0f);
  uint8_t hue = (uint8_t)hueFloat;
  uint32_t c = effect_common::hueToColor(hue);
  for (uint16_t i = 0; i < n; i++)
    ctx.strip->setPixelColor(i, c);
  ctx.strip->show();
}
