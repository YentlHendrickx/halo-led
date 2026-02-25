#include "config.h"
#include "effect_common.h"
#include "effect_context.h"
#include "led_effects.h"

#define CONFETTI_PARTICLES 24

struct ConfettiParticle {
  float phase;
  uint8_t hue;
  float vel;
};
static ConfettiParticle s_confetti[CONFETTI_PARTICLES];
static bool s_confettiInited = false;

void effectConfetti(EffectContext &ctx) {
  const uint16_t n = ctx.strip->numPixels();
  if (n == 0)
    return;
  float tSec = ctx.timeSec;

  if (!s_confettiInited) {
    for (int i = 0; i < CONFETTI_PARTICLES; i++) {
      s_confetti[i].phase = (float)i / (float)CONFETTI_PARTICLES;
      s_confetti[i].hue = (uint8_t)((i * 37) % 256);
      s_confetti[i].vel =
          18.0f + (effect_common::noise8((uint32_t)i * 123) % 20);
    }
    s_confettiInited = true;
  }

  ctx.strip->clear();
  float nf = (float)n;
  for (int i = 0; i < CONFETTI_PARTICLES; i++) {
    float pos = effect_common::fmodf_positive(
        tSec * s_confetti[i].vel + s_confetti[i].phase * nf, nf);
    uint16_t idx = (uint16_t)pos;
    if (idx < n) {
      uint8_t r, g, b;
      effect_common::hueToRgb(s_confetti[i].hue, r, g, b);
      ctx.strip->setPixelColor(idx, r, g, b);
    }
  }
  ctx.strip->show();
}
