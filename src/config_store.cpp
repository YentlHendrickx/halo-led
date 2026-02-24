#include "config_store.h"
#include "led_effects.h"
#include <EEPROM.h>
#include <string.h>

namespace {

const uint32_t CONFIG_MAGIC = 0x486C4366u;  // "HlCf" in little-endian
const uint16_t CONFIG_VERSION = 1;
const size_t EEPROM_SIZE = 128;

#pragma pack(push, 1)
struct EffectColorSlot {
  uint8_t r, g, b;
  uint8_t custom;  // 0 = use default, 1 = use r,g,b
};

struct StoredConfig {
  uint32_t magic;
  uint16_t version;
  uint8_t effect;       // LedEffect as byte
  float speed;
  uint8_t targetFps;
  uint8_t brightness;
  uint8_t staticR, staticG, staticB;
  uint8_t accentR, accentG, accentB;
  uint8_t globalR, globalG, globalB;
  float intensity;
  uint8_t fireVariant;  // 0 = Red, 1 = Blue
  uint8_t reserved;
  EffectColorSlot effectColors[21];  // LedEffect::Count
};
#pragma pack(pop)

static_assert(sizeof(StoredConfig) <= EEPROM_SIZE, "config struct too large");

const int EFFECT_COUNT = static_cast<int>(LedEffect::Count);

void applyConfig(const StoredConfig& c) {
  if (c.effect < EFFECT_COUNT) {
    ledEffectsSet(static_cast<LedEffect>(c.effect));
  }
  ledEffectsSetSpeed(c.speed);
  ledEffectsSetTargetFps(c.targetFps);
  ledEffectsSetBrightness(c.brightness);
  ledEffectsSetStaticColor(c.staticR, c.staticG, c.staticB);
  ledEffectsSetAccentColor(c.accentR, c.accentG, c.accentB);
  ledEffectsSetGlobalColor(c.globalR, c.globalG, c.globalB);
  ledEffectsSetIntensity(c.intensity);
  ledEffectsSetFireVariant(c.fireVariant == 1 ? FireVariant::Blue : FireVariant::Red);

  for (int i = 0; i < EFFECT_COUNT && i < 21; i++) {
    if (c.effectColors[i].custom) {
      ledEffectsSetEffectColor(
          static_cast<LedEffect>(i),
          c.effectColors[i].r,
          c.effectColors[i].g,
          c.effectColors[i].b);
    } else {
      ledEffectsResetEffectColor(static_cast<LedEffect>(i));
    }
  }
}

void captureConfig(StoredConfig& c) {
  c.magic = CONFIG_MAGIC;
  c.version = CONFIG_VERSION;
  c.effect = static_cast<uint8_t>(ledEffectsCurrent());
  c.speed = ledEffectsGetSpeed();
  c.targetFps = ledEffectsGetTargetFps();
  c.brightness = ledEffectsGetBrightness();

  ledEffectsGetStaticColor(c.staticR, c.staticG, c.staticB);
  ledEffectsGetAccentColor(c.accentR, c.accentG, c.accentB);
  ledEffectsGetGlobalColor(c.globalR, c.globalG, c.globalB);

  c.intensity = ledEffectsGetIntensity();
  c.fireVariant = (ledEffectsGetFireVariant() == FireVariant::Blue) ? 1 : 0;
  c.reserved = 0;

  for (int i = 0; i < EFFECT_COUNT && i < 21; i++) {
    LedEffect e = static_cast<LedEffect>(i);
    c.effectColors[i].custom = ledEffectsHasEffectColor(e) ? 1 : 0;
    if (c.effectColors[i].custom) {
      ledEffectsGetEffectColor(e, c.effectColors[i].r, c.effectColors[i].g, c.effectColors[i].b);
    } else {
      c.effectColors[i].r = c.effectColors[i].g = c.effectColors[i].b = 0;
    }
  }
}

}  // namespace

bool configStoreLoad() {
  EEPROM.begin(EEPROM_SIZE);
  StoredConfig c;
  EEPROM.get(0, c);
  EEPROM.end();

  if (c.magic != CONFIG_MAGIC || c.version != CONFIG_VERSION) {
    return false;
  }
  if (c.effect >= EFFECT_COUNT) {
    return false;
  }

  applyConfig(c);
  return true;
}

bool configStoreSave() {
  StoredConfig c;
  captureConfig(c);

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(0, c);
  bool ok = EEPROM.commit();
  EEPROM.end();

  return ok;
}

bool configStoreClear() {
  EEPROM.begin(EEPROM_SIZE);
  StoredConfig c;
  memset(&c, 0, sizeof(c));
  EEPROM.put(0, c);
  bool ok = EEPROM.commit();
  EEPROM.end();

  return ok;
}
