#include "effect_context.h"
#include "effect_common.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void effectPlasma(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  t = effect_common::fmodf_positive(t, 100.0f);
  for (uint16_t i = 0; i < n; i++) {
    float x = (float)i / (float)(n > 1 ? n : 1);
    float v = (float)sin((double)(x * 4.0 * M_PI + t * 2.0)) * 0.5f + 0.5f;
    v += (float)sin((double)(x * 8.0 * M_PI - t * 1.5)) * 0.3f;
    v += (float)sin((double)((x + t * 0.3) * 12.0 * M_PI)) * 0.2f;
    if (v < 0) v = 0;
    if (v > 1) v = 1;
    uint8_t hue = (uint8_t)(v * 255.0f);
    ctx.strip->setPixelColor(i, effect_common::hueToColor(hue));
  }
  ctx.strip->show();
}
