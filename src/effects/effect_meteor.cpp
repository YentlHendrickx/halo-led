#include "effect_context.h"
#include "effect_common.h"
#include "led_effects.h"
#include "config.h"

#define METEOR_COUNT 5

static uint8_t s_meteorHue[METEOR_COUNT];
static bool s_meteorInited = false;

void effectMeteorShower(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  float t = ctx.timeSec;
  float tailLen = (float)n * 0.15f;
  if (tailLen < 8) tailLen = 8;
  float cycle = (float)n + tailLen;
  float speed = cycle * 0.4f;

  if (!s_meteorInited) {
    for (int i = 0; i < METEOR_COUNT; i++)
      s_meteorHue[i] = (i * 51) % 256;
    s_meteorInited = true;
  }

  ctx.strip->clear();
  for (int m = 0; m < METEOR_COUNT; m++) {
    float phase = (float)m / (float)METEOR_COUNT;
    float head = effect_common::fmodf_positive(t * speed + phase * cycle, cycle);
    uint8_t hr, hg, hb;
    effect_common::hueToRgb(s_meteorHue[m], hr, hg, hb);
    for (float ti = 0; ti <= tailLen; ti += 1.0f) {
      int16_t idx = (int16_t)(head - ti);
      if (idx < 0 || (uint16_t)idx >= n) continue;
      float f = 1.0f - ti / tailLen;
      uint8_t bright = (uint8_t)(255.0f * f * f);
      ctx.strip->setPixelColor((uint16_t)idx,
        effect_common::scale8(hr, bright), effect_common::scale8(hg, bright), effect_common::scale8(hb, bright));
    }
  }
  ctx.strip->show();
}
