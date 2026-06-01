# Grouped Window Switcher

A custom KDE task switcher (KWin effect) that replaces Alt+Tab with a persistent,
full-screen overlay: windows are **grouped by application** (one tile per app, with
the app logo and a window count); drilling into a group shows a **uniform, scrollable
grid of live window thumbnails**. The overlay stays open after you release the keys and
closes only on confirm or cancel. Appearance follows the system light/dark theme.

> **Status:** in development. The compositor-free core is implemented and unit-tested;
> the KWin effect + QML overlay build and load, and have been verified to open/close
> without crashing in a headless compositor. Live, on-screen verification (with real
> thumbnails) and remaining polish are tracked in
> [`specs/001-grouped-window-switcher/tasks.md`](specs/001-grouped-window-switcher/tasks.md).

## Target platform

**KDE Plasma 5.27 LTS / KWin 5.27** (Qt 5, KDE Frameworks 5), Wayland and X11. Built as a
`QuickSceneEffect` plugin — *not* a TabBox "Visualization" theme, because TabBox cannot
stay open past modifier release or do two-level grouped navigation (see
[`research.md`](specs/001-grouped-window-switcher/research.md) §"Retarget to Plasma 5.27").
The KWin effects ABI is not stable across releases — build against the KWin you run.

## Architecture

```
src/core/    Compositor-free logic — NO KWin (unit-tested):
             WindowDescriptor, WindowSource (seam), ApplicationResolver,
             GroupingEngine, SessionController (the open→overview→in-group state machine)
src/model/   Qt models for QML: SwitcherModel (groups), WindowEntryModel (in-group windows)
src/effect/  KWin glue (the only compositor-coupled code):
             GroupedSwitcherEffect (QuickSceneEffect), EffectWindowSource (live WindowSource)
src/ui/      QML overlay: main, GroupOverview, ApplicationGroupGrid, WindowTile
autotests/   C++ QTest suites (headless, offscreen QPA)
tools/       dev-nested.sh — nested-compositor dev harness
```

All behavioral logic lives behind the `WindowSource` interface so it is testable with a
fake and no running compositor (project constitution, Principle III).

## Build

Dependencies (Ubuntu 24.04 / Plasma 5.27): see
[`specs/001-grouped-window-switcher/quickstart.md`](specs/001-grouped-window-switcher/quickstart.md) §1.

```bash
cmake -B build -S .
cmake --build build -j"$(nproc)"
```

The core library + tests build even without the KWin headers; the effect plugin builds
only when `KWinEffects` is found.

## Test

```bash
ctest --test-dir build --output-on-failure   # headless C++ unit tests
```

## Install, enable, run

See [quickstart.md](specs/001-grouped-window-switcher/quickstart.md) §4–6. In short:

```bash
sudo cmake --install build
# enable: System Settings → Desktop Effects → "Grouped Window Switcher"
# bind a shortcut: System Settings → Shortcuts → "Toggle Grouped Window Switcher"
```

It can also be toggled over D-Bus (useful for testing/automation):

```bash
qdbus org.kde.KWin /GroupedWindowSwitcher toggle
```

## Iterating

A crash-isolated dev loop and a nested-compositor harness are described in
[quickstart.md](specs/001-grouped-window-switcher/quickstart.md) and `tools/dev-nested.sh`.
On X11 the fast loop is: edit → `cmake --build build` → `sudo cmake --install build` →
`kwin_x11 --replace` (~1 s) → trigger.

## Known limitations

- **Live thumbnails require OpenGL compositing.** Under software (QPainter) compositing the
  effect still runs, but window previews fall back to the application icon.
- Minimized windows have no live texture and show the app icon (KWin behavior).

## License

GPL-2.0-or-later.
