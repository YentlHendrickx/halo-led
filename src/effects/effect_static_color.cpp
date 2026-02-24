#include "effect_context.h"
#include "led_effects.h"

void effectStaticColor(EffectContext& ctx) {
  if (!ctx.strip) return;
  ctx.strip->fill(ctx.staticR, ctx.staticG, ctx.staticB);
  ctx.strip->show();
}
