#ifndef LED_STRIP_H
#define LED_STRIP_H

#include <Arduino.h>

// WS2815-compatible strip (same protocol as WS2812B).
// Manages the physical strip; effects live in led_effects.
class LedStrip {
public:
  LedStrip();
  ~LedStrip();

  // Call once from setup(). Uses LED_PIN and LED_COUNT from config.h.
  void begin();

  // Low-level access for effects
  void setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
  void setPixelColor(uint16_t index, uint32_t color);
  void show();
  void clear();
  void fill(uint8_t r, uint8_t g, uint8_t b);
  void setBrightness(uint8_t b);

  uint16_t numPixels() const { return _numPixels; }

private:
  class Impl;
  Impl* _impl;
  uint16_t _numPixels;
};

// Single global strip instance (defined in led_strip.cpp)
extern LedStrip ledStrip;

#endif
