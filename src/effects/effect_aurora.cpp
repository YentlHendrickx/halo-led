#include "effect_common.h"
#include "effect_context.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void effectAurora(EffectContext &ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  t = effect_common::fmodf_positive(t, 100.0f);
  for (uint16_t i = 0; i < n; i++) {
    float y = (float)i / (float)(n > 1 ? n : 1);
    float wave = (float)sin((double)(y * 3.0 * M_PI + t * 1.5)) * 0.5f + 0.5f;
    wave += (float)sin((double)(y * 6.0 * M_PI - t * 0.8)) * 0.3f;
    if (wave < 0)
      wave = 0;
    if (wave > 1)
      wave = 1;
    float hueFloat =
        effect_common::fmodf_positive(y * 80.0f + t * 30.0f, 256.0f);
    uint8_t hue = (uint8_t)hueFloat;
    uint8_t r, g, b;
    effect_common::hueToRgb(hue, r, g, b);
    uint8_t bright = (uint8_t)(wave * 200.0f);
    ctx.strip->setPixelColor(i, effect_common::scale8(r, bright),
                             effect_common::scale8(g, bright),
                             effect_common::scale8(b, bright));
  }
  ctx.strip->show();
}
