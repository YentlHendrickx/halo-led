#ifndef EFFECT_COMMON_H
#define EFFECT_COMMON_H

#include <Arduino.h>

// Stateless helpers for LED effects. No dependency on led_effects state.
// Include this from effect_xxx.cpp files.

namespace effect_common {

// Hue to RGB (0-255). Full brightness.
inline void hueToRgb(uint8_t hue, uint8_t& r, uint8_t& g, uint8_t& b) {
  hue = 255 - hue;
  if (hue < 85) {
    r = 255 - hue * 3;
    g = hue * 3;
    b = 0;
  } else if (hue < 170) {
    hue -= 85;
    r = hue * 3;
    g = 255;
    b = 0;
  } else {
    hue -= 170;
    r = 0;
    g = 255 - hue * 3;
    b = hue * 3;
  }
}

inline uint32_t hueToColor(uint8_t hue) {
  uint8_t r, g, b;
  hueToRgb(hue, r, g, b);
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

inline uint8_t scale8(uint8_t x, uint8_t scale) {
  return (uint16_t)x * scale >> 8;
}

// Deterministic pseudo-random 0-255 from seed.
inline uint8_t noise8(uint32_t x) {
  x = (x >> 13) ^ x;
  x = (x * (x * x * 15731 + 789221) + 1376312589) & 0x7FFFFFFF;
  return (uint8_t)(x >> 23);
}

inline float fmodf_positive(float x, float m) {
  if (m <= 0) return 0;
  while (x >= m) x -= m;
  while (x < 0) x += m;
  return x;
}

}  // namespace effect_common
#endif
