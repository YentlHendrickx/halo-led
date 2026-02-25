#include "effect_common.h"
#include "effect_context.h"
#include "led_effects.h"

#define SCANNER_WIDTH 12

void effectScanner(EffectContext &ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  float period = (float)(n + SCANNER_WIDTH * 2) * 0.03f;
  float phase = effect_common::fmodf_positive(t, period * 2.0f);
  float pos = phase < period ? phase / 0.03f : (period * 2.0f - phase) / 0.03f;

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::Scanner, cr, cg, cb);

  ctx.strip->clear();
  for (int w = -SCANNER_WIDTH; w <= SCANNER_WIDTH; w++) {
    int16_t idx = (int16_t)(pos + w);
    if (idx < 0 || (uint16_t)idx >= n)
      continue;
    uint8_t bright = 255 - (uint8_t)((abs(w) * 255) / (SCANNER_WIDTH + 1));
    ctx.strip->setPixelColor((uint16_t)idx, effect_common::scale8(cr, bright),
                             effect_common::scale8(cg, bright),
                             effect_common::scale8(cb, bright));
  }
  ctx.strip->show();
}
