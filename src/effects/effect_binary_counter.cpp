#include "effect_common.h"
#include "effect_context.h"
#include "led_effects.h"
#include <math.h>

// Binary counter: each segment is one bit. Count advances at (speed) per second
// (1.0 = 1 count/s). segmentSize = LEDs per bit (from context; globally
// controllable).

void effectBinaryCounter(EffectContext &ctx) {
  LedStrip *strip = ctx.strip;
  if (!strip)
    return;
  const uint16_t n = strip->numPixels();
  if (n == 0)
    return;

  float seg = ctx.segmentSize;
  if (seg < 1.0f)
    seg = 1.0f;
  uint16_t segPixels = (uint16_t)(seg + 0.5f);
  if (segPixels < 1)
    segPixels = 1;
  uint16_t numSegments = n / segPixels;
  if (numSegments == 0)
    numSegments = 1;

  // Count = seconds elapsed (scaled by speed), so speed 1.0 = 1 count per
  // second
  uint32_t count = (uint32_t)ctx.timeSec;
  uint32_t maxVal = 1u;
  if (numSegments < 32)
    maxVal = 1u << numSegments;
  else
    maxVal = 0xFFFFFFFFu;
  uint32_t value = count % maxVal;

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::BinaryCounter, cr, cg, cb);

  for (uint16_t i = 0; i < n; i++) {
    uint16_t segIndex = i / segPixels;
    if (segIndex >= numSegments)
      segIndex = numSegments - 1;
    uint32_t bit = (value >> segIndex) & 1u;
    uint8_t v = (uint8_t)(bit * 255);
    strip->setPixelColor(i, effect_common::scale8(cr, v),
                         effect_common::scale8(cg, v),
                         effect_common::scale8(cb, v));
  }
  strip->show();
}
