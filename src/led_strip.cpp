#include "led_strip.h"
#include "config.h"
#include <Adafruit_NeoPixel.h>

#define NEO_PIXEL_TYPE (NEO_RGB + NEO_KHZ800)

struct LedStrip::Impl {
  Adafruit_NeoPixel strip;
  Impl() : strip(LED_COUNT, LED_PIN, NEO_PIXEL_TYPE) {}
};

LedStrip::LedStrip() : _impl(new Impl()), _numPixels(LED_COUNT) {}

LedStrip::~LedStrip() {
  delete _impl;
  _impl = nullptr;
}

void LedStrip::begin() {
  _impl->strip.begin();
  _impl->strip.show();
  _impl->strip.setBrightness(80);
}

void LedStrip::setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
  if (_impl && index < _numPixels)
    _impl->strip.setPixelColor(index, r, g, b);
}

void LedStrip::setPixelColor(uint16_t index, uint32_t color) {
  if (_impl && index < _numPixels)
    _impl->strip.setPixelColor(index, color);
}

void LedStrip::show() {
  if (_impl) _impl->strip.show();
}

void LedStrip::clear() {
  if (_impl) _impl->strip.clear();
}

void LedStrip::fill(uint8_t r, uint8_t g, uint8_t b) {
  if (!_impl) return;
  for (uint16_t i = 0; i < _numPixels; i++)
    _impl->strip.setPixelColor(i, r, g, b);
}

void LedStrip::setBrightness(uint8_t b) {
  if (_impl) _impl->strip.setBrightness(b);
}

LedStrip ledStrip;
