#include "config.h"
#include "effect_common.h"
#include "effect_context.h"
#include "led_effects.h"
#include <Arduino.h>
#include <limits.h>

#define AUTOMATON_SIZE LED_COUNT
#define HASH_HISTORY_LEN 16

static uint8_t s_cells[AUTOMATON_SIZE];
static bool s_inited = false;
static unsigned long s_lastStep = 0;

// Loop detection: hashes of recent states (circular buffer)
static uint32_t s_hashHistory[HASH_HISTORY_LEN];
static uint8_t s_hashCount = 0;
static uint8_t s_hashWrite = 0;

static uint32_t stateHash(const uint8_t *cells, uint16_t n) {
  uint32_t h = 2166136261u;
  for (uint16_t i = 0; i < n; i++)
    h = (h ^ (uint32_t)cells[i]) * 16777619u;
  return h;
}

static bool hashSeenInHistory(uint32_t hash) {
  for (uint8_t i = 0; i < s_hashCount; i++) {
    if (s_hashHistory[i] == hash)
      return true;
  }
  return false;
}

static void pushHash(uint32_t hash) {
  s_hashHistory[s_hashWrite] = hash;
  s_hashWrite = (s_hashWrite + 1) % HASH_HISTORY_LEN;
  if (s_hashCount < HASH_HISTORY_LEN)
    s_hashCount++;
}

static void clearHashHistory() {
  s_hashCount = 0;
  s_hashWrite = 0;
}

// Re-seed the automaton so it doesn't stay stuck
static void regenerate(uint16_t n) {
  randomSeed(millis());
  for (uint16_t i = 0; i < n; i++)
    s_cells[i] = 0;
  // 1–3 random cells on, so we get varied evolution
  uint8_t seeds = (uint8_t)(1 + (random() % 3));
  for (uint8_t k = 0; k < seeds; k++)
    s_cells[(uint16_t)(random() % n)] = 1;
  clearHashHistory();
}

void effectRule30Reset() { s_inited = false; }

void effectRule30(EffectContext &ctx) {
  const uint16_t n = ctx.strip->numPixels();
  if (n == 0)
    return;
  if (n > AUTOMATON_SIZE)
    return;

  if (!s_inited) {
    for (uint16_t i = 0; i < n; i++)
      s_cells[i] = 0;
    s_cells[n / 2] = 1;
    s_inited = true;
    s_lastStep = millis();
    clearHashHistory();
  }

  unsigned long now = millis();
  unsigned long elapsed;
  if (now >= s_lastStep) {
    elapsed = now - s_lastStep;
  } else {
    elapsed = (ULONG_MAX - s_lastStep) + now + 1;
  }
  if (elapsed >= 100) {
    s_lastStep = now;

    // Rule 30: new = left XOR (self OR right), boundaries = 0
    uint8_t prev = 0;
    for (uint16_t i = 0; i < n; i++) {
      uint8_t left = prev;
      uint8_t self = s_cells[i];
      uint8_t right = (i + 1 < n) ? s_cells[i + 1] : 0;
      prev = self;
      s_cells[i] = (left ^ (self | right)) & 1;
    }

    uint32_t hash = stateHash(s_cells, n);

    // Stuck or loop: same state seen before in recent history
    if (hashSeenInHistory(hash)) {
      regenerate(n);
    } else {
      pushHash(hash);
    }

    // All zeros or all ones = dead or fixed point → regenerate
    uint16_t pop = 0;
    for (uint16_t i = 0; i < n; i++)
      pop += s_cells[i];
    if (pop == 0 || pop == n)
      regenerate(n);
  }

  uint8_t cr, cg, cb;
  ctx.getEffectColor(LedEffect::Rule30, cr, cg, cb);

  for (uint16_t i = 0; i < n; i++) {
    uint8_t v = s_cells[i];
    ctx.strip->setPixelColor(i, effect_common::scale8(cr, v * 255),
                             effect_common::scale8(cg, v * 255),
                             effect_common::scale8(cb, v * 255));
  }
  ctx.strip->show();
}
