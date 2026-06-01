#include "effect/GroupedSwitcherEffect.h"

#include <kwineffects.h>
#include <kwinquickeffect.h>

// Supported under any active compositing (not OpenGL-only): the QML overlay
// renders via the QtQuick software backend too; only live thumbnails need GL
// (they fall back to the app icon otherwise). This also lets the effect run in
// a headless/software KWin for testing.
KWIN_EFFECT_FACTORY_SUPPORTED(gks::GroupedSwitcherEffect,
                              "metadata.json",
                              return KWin::effects->compositingType() != KWin::NoCompositing;)

#include "main.moc"
