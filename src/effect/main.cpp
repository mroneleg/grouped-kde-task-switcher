#include "effect/GroupedSwitcherEffect.h"

#include <kwineffects.h>
#include <kwinquickeffect.h>

KWIN_EFFECT_FACTORY_SUPPORTED(gks::GroupedSwitcherEffect,
                              "metadata.json",
                              return KWin::QuickSceneEffect::supported();)

#include "main.moc"
