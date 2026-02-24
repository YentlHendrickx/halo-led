#include "effect_context.h"
#include "effect_common.h"
#include "led_effects.h"
#include "config.h"
#include <Arduino.h>

#define FIRE_HEAT_SIZE ((LED_COUNT) + 2)
static uint8_t s_heat[FIRE_HEAT_SIZE];

static void heatToFireColorRed(uint8_t h, uint8_t& r, uint8_t& g, uint8_t& b) {
  if (h < 30) {
    r = (h * 200) / 30;
    g = 0;
    b = 0;
  } else if (h < 100) {
    r = 200 + ((h - 30) * 55) / 70;
    g = 0;
    b = 0;
  } else if (h < 160) {
    r = 255;
    g = ((h - 100) * 180) / 60;
    b = 0;
  } else if (h < 200) {
    r = 255;
    g = 180 + ((h - 160) * 75) / 40;
    b = 0;
  } else {
    uint8_t fade = 255 - ((h - 200) * 4);
    if (fade > 255) fade = 0;
    r = effect_common::scale8(255, fade);
    g = effect_common::scale8(255, fade);
    b = 0;
  }
}

static void heatToFireColorBlue(uint8_t h, uint8_t& r, uint8_t& g, uint8_t& b) {
  if (h < 40) {
    r = 0;
    g = 0;
    b = (h * 255) / 40;
  } else if (h < 120) {
    r = 0;
    g = 0;
    b = 255;
    uint8_t bright = ((h - 40) * 255) / 80;
    if (bright > 255) bright = 255;
    b = effect_common::scale8(b, bright);
  } else if (h < 180) {
    r = 0;
    g = ((h - 120) * 180) / 60;
    b = 255;
  } else {
    uint8_t fade = 255 - ((h - 180) * 3);
    if (fade > 255) fade = 0;
    r = 0;
    g = effect_common::scale8(150, fade);
    b = effect_common::scale8(255, fade);
  }
}

static void heatToFireColor(EffectContext& ctx, uint8_t h, uint8_t& r, uint8_t& g, uint8_t& b) {
  uint8_t variationMax = (h < 80) ? 8 : 15;
  uint8_t variation = effect_common::noise8((uint32_t)h * 7919 + ctx.timeMs) % variationMax;
  uint8_t adjustedH = h;
  if (h > 5) {
    if (variation < variationMax / 2) {
      adjustedH = h + variation;
      if (adjustedH > 255) adjustedH = 255;
    } else {
      adjustedH = h - (variation - variationMax / 2);
      if (adjustedH > h) adjustedH = h;
    }
  }
  if (ctx.fireVariant == FireVariant::Red) {
    heatToFireColorRed(adjustedH, r, g, b);
  } else {
    heatToFireColorBlue(adjustedH, r, g, b);
  }
}

void effectFire(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  const uint16_t len = n + 2;
  if (len > FIRE_HEAT_SIZE) return;

  float heightPercent;
  if (ctx.intensity <= 1.0f) {
    heightPercent = 0.4f * ctx.intensity;
  } else if (ctx.intensity <= 2.0f) {
    float t = (ctx.intensity - 1.0f) / 1.0f;
    heightPercent = 0.4f + t * 0.35f;
  } else {
    float t = (ctx.intensity - 2.0f) / 1.0f;
    heightPercent = 0.75f + t * 0.25f;
    if (heightPercent > 1.0f) heightPercent = 1.0f;
  }
  uint16_t maxFlameHeight = (uint16_t)((float)n * heightPercent);
  if (maxFlameHeight > n) maxFlameHeight = n;
  if (maxFlameHeight < 1) maxFlameHeight = 1;

  for (uint16_t i = 0; i < len; i++) {
    if (s_heat[i] > 0) {
      uint8_t coolRate = (i < len / 4) ? 1 : 2;
      if (s_heat[i] > coolRate) {
        s_heat[i] -= coolRate;
      } else {
        s_heat[i] = 0;
      }
    }
  }

  for (uint16_t i = len - 1; i >= 1; i--) {
    uint8_t below = s_heat[i - 1];
    uint8_t current = s_heat[i];
    uint8_t blend = (below * 2 + current) / 3;
    uint8_t flickerMax = (i < 5) ? 2 : 4;
    uint8_t flicker = effect_common::noise8((uint32_t)i * 7919 + ctx.timeMs) % flickerMax;
    if (flicker < flickerMax / 2 && blend > 0) {
      blend += flicker;
      if (blend > 255) blend = 255;
    }
    s_heat[i] = blend;
  }

  if (random(0, 100) < 60) {
    uint8_t spark = 180 + (effect_common::noise8(ctx.timeMs * 7919) % 75);
    if (spark > 255) spark = 255;
    s_heat[0] = (s_heat[0] + spark) > 255 ? 255 : s_heat[0] + spark;
  }

  for (uint16_t i = 0; i < n; i++) {
    if (i >= maxFlameHeight) {
      ctx.strip->setPixelColor(i, 0, 0, 0);
      continue;
    }
    float positionRatio = (float)i / (float)maxFlameHeight;
    uint8_t normalizedHeat;
    if (positionRatio < 0.3f) {
      float zoneRatio = positionRatio / 0.3f;
      normalizedHeat = 255 - (uint8_t)(zoneRatio * 55.0f);
    } else if (positionRatio < 0.7f) {
      float zoneRatio = (positionRatio - 0.3f) / 0.4f;
      normalizedHeat = 200 - (uint8_t)(zoneRatio * 100.0f);
    } else {
      float zoneRatio = (positionRatio - 0.7f) / 0.3f;
      normalizedHeat = 100 - (uint8_t)(zoneRatio * 100.0f);
    }
    uint16_t heatIdx = i + 1;
    if (heatIdx >= len) {
      ctx.strip->setPixelColor(i, 0, 0, 0);
      continue;
    }
    uint8_t actualHeat = s_heat[heatIdx];
    uint8_t h = (normalizedHeat * 3 + actualHeat) / 4;
    uint8_t r, g, b;
    heatToFireColor(ctx, h, r, g, b);
    ctx.strip->setPixelColor(i, r, g, b);
  }
  ctx.strip->show();
}

void effectFireTop(EffectContext& ctx) {
  const uint16_t n = ctx.strip->numPixels();
  const uint16_t len = n + 2;
  if (len > FIRE_HEAT_SIZE) return;

  uint16_t maxFlameHeight = (uint16_t)((float)n * 0.4f * ctx.intensity);
  if (maxFlameHeight > n) maxFlameHeight = n;
  if (maxFlameHeight < 1) maxFlameHeight = 1;

  for (uint16_t i = 0; i < len; i++) {
    if (s_heat[i] > 0) {
      uint8_t coolRate = (i > len * 3 / 4) ? 1 : 2;
      if (s_heat[i] > coolRate) {
        s_heat[i] -= coolRate;
      } else {
        s_heat[i] = 0;
      }
    }
  }

  for (uint16_t i = 0; i < len - 1; i++) {
    uint8_t above = s_heat[i + 1];
    uint8_t current = s_heat[i];
    uint8_t blend = (above * 2 + current) / 3;
    uint16_t distFromTop = len - 1 - i;
    uint8_t flickerMax = (distFromTop < 5) ? 2 : 4;
    uint8_t flicker = effect_common::noise8((uint32_t)i * 7919 + ctx.timeMs) % flickerMax;
    if (flicker < flickerMax / 2 && blend > 0) {
      blend += flicker;
      if (blend > 255) blend = 255;
    }
    s_heat[i] = blend;
  }

  if (random(0, 100) < 55) {
    uint8_t sparkHeat = 180 + (effect_common::noise8(ctx.timeMs * 7919) % 75);
    if (sparkHeat > 255) sparkHeat = 255;
    uint16_t topIdx = len - 1;
    s_heat[topIdx] = (s_heat[topIdx] + sparkHeat) > 255 ? 255 : s_heat[topIdx] + sparkHeat;
  }

  for (uint16_t i = 0; i < n; i++) {
    if (i < (n - maxFlameHeight)) {
      ctx.strip->setPixelColor(i, 0, 0, 0);
      continue;
    }
    uint16_t positionInFlame = n - 1 - i;
    float positionRatio = (float)positionInFlame / (float)maxFlameHeight;
    uint8_t normalizedHeat;
    if (positionRatio < 0.3f) {
      float zoneRatio = positionRatio / 0.3f;
      normalizedHeat = 255 - (uint8_t)(zoneRatio * 55.0f);
    } else if (positionRatio < 0.7f) {
      float zoneRatio = (positionRatio - 0.3f) / 0.4f;
      normalizedHeat = 200 - (uint8_t)(zoneRatio * 100.0f);
    } else {
      float zoneRatio = (positionRatio - 0.7f) / 0.3f;
      normalizedHeat = 100 - (uint8_t)(zoneRatio * 100.0f);
    }
    uint16_t offsetFromTop = n - 1 - i;
    uint16_t heatIdx = len - 1 - offsetFromTop;
    if (heatIdx >= len || heatIdx < 1) {
      ctx.strip->setPixelColor(i, 0, 0, 0);
      continue;
    }
    uint8_t actualHeat = s_heat[heatIdx];
    uint8_t h = (normalizedHeat * 3 + actualHeat) / 4;
    uint8_t r, g, b;
    heatToFireColor(ctx, h, r, g, b);
    ctx.strip->setPixelColor(i, r, g, b);
  }
  ctx.strip->show();
}
