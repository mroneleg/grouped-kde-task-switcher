# Phase 0 Research: Grouped Window Switcher

This document records the technology decisions that resolve the unknowns in the plan's Technical
Context. Each entry is **Decision / Rationale / Alternatives considered**. Sources are KDE primary
references (invent.kde.org, develop.kde.org, api.kde.org, plasma-workspace) gathered during
research.

---

## Retarget to Plasma 5.27 (2026-05-31)

The development machine runs **Plasma 5.27 LTS / KWin 5.27.11 (Qt 5, KF5)** on Ubuntu 24.04, not
Plasma 6. The original research below was written for Plasma 6; the **architecture is unchanged** —
`QuickSceneEffect` exists in KWin 5.27 — but the following API/version specifics supersede the
Plasma-6 details in the sections that follow. Verified against the `Plasma/5.27` branch of
invent.kde.org/plasma/kwin.

| Concern | Plasma 6 (original) | **Plasma 5.27 (actual target)** |
|---|---|---|
| Effect base class | `QuickSceneEffect` (`src/effect/quickeffect.h`) | `KWin::QuickSceneEffect` (`libkwineffects/kwinquickeffect.h`); link `kwineffects` |
| Reference effect | Overview / Window View | **Overview** (`src/effects/overview`, added Plasma 5.24) |
| Global shortcut | QML `ShortcutHandler` | **`KGlobalAccel::self()->setShortcut(QAction*)`** — no `registerGlobalShortcut` on `EffectsHandler`; link `KF5::GlobalAccel` |
| Keyboard grab | `grabKeyboard()` | `effects->grabKeyboard(this)` / `ungrabKeyboard()` + override `Effect::grabbedKeyboardEvent()` (`hasKeyboardGrab()` is impl-only, not callable out-of-tree) |
| Thumbnail QML | `WindowThumbnail { client }` (`org.kde.kwin`) | **`WindowThumbnailItem { wId: <internalId QUuid> }`** via `import org.kde.kwin 3.0`; window model = `WindowModel` + `ClientFilterModel` |
| App grouping key | `desktopFileName` → KService | **`EffectWindow::windowClass()` (WM_CLASS)** — `desktopFileName()` does NOT exist on 5.27 `EffectWindow`; resolve display name via KService where possible, icon via `EffectWindow::icon()` |
| Window id (Wayland-safe) | `QUuid` internalId | `EffectWindow::internalId()` (QUuid); `windowId()` is X11-only XID |
| `EffectWindow` desktops | `desktops()` | `desktops()` → `QVector<uint>` (`int desktop()` is deprecated); `activities()`, `screen()` → `EffectScreen*`, `isMinimized()`, `caption()`, `icon()` |
| Theming | `Kirigami.Theme` | **`PlasmaCore.ColorScope`** (`org.kde.plasma.core 2.0`) + `PlasmaComponents 3.0` + `Kirigami 2.20`; follows system scheme live (as Overview does) |
| Toolchain | Qt6 / KF6 / C++23 | **Qt5 / KF5 / C++20** (`CMAKE_CXX_STANDARD 20`); `find_package(Qt5 COMPONENTS Core Gui Quick DBus)`, `find_package(KF5 COMPONENTS Config CoreAddons WindowSystem GlobalAccel I18n Service)`, `find_package(KWin)` → `kwineffects` |
| Plugin install | `${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins/` | same, via `kcoreaddons_add_plugin(... INSTALL_NAMESPACE "kwin/effects/plugins")`; QML → `${KDE_INSTALL_DATADIR}/kwin/effects/<id>/qml/` |
| Plugin factory macro | `KWIN_EFFECT_FACTORY*` | `KWIN_EFFECT_FACTORY_SUPPORTED(...)` in `main.cpp` + `#include "main.moc"` |

**apt packages (Ubuntu 24.04)**: `cmake extra-cmake-modules kwin-dev qtbase5-dev qtdeclarative5-dev
libkf5config-dev libkf5coreaddons-dev libkf5windowsystem-dev libkf5globalaccel-dev libkf5i18n-dev
libkf5service-dev kirigami2-dev libkf5configwidgets-dev`.

**Caveat — Plasma 5 is end-of-life** (5.27 is the last Plasma 5 series). The design avoids private
APIs (no `WindowHeap` dependency) so a future port to Plasma 6 is mostly QML-import + KF5→KF6 +
`KGlobalAccel`↔`ShortcutHandler` changes; the compositor-free core/model/state-machine port
unchanged.

The decisions in §§1–9 below remain valid as written **except** where the table above overrides the
specific class/import/package names.

---

## 1. Implementation vehicle — how to build the switcher

**Decision**: Build a **C++ `QuickSceneEffect` KWin 6 plugin** (the architecture used by KWin's
built-in Overview, Window View, and Desktop Grid effects). It hosts a full-screen QML scene per
screen, grabs the keyboard via `EffectsHandler::grabKeyboard()`, and owns its own open/close
lifecycle.

**Rationale**:
- **Persistence (FR-008)**: Inside an effect we own the keyboard grab, so the overlay stays open
  regardless of modifier state and closes only when *we* decide (confirm/cancel). This is exactly
  how Overview/Window View behave.
- **Live thumbnails on Wayland + X11 (FR-011/013, Principle I)**: The QML `WindowThumbnail`
  (import `org.kde.kwin`) is composited by KWin from the window's existing GPU texture, so one code
  path works on both session types without screen-scraping.
- **Two-level grouped navigation + grid (FR-001/005/006)**: We fully own the QML scene and its
  input handling, so grouped overview → in-group grid drill-down is straightforward.

**Alternatives considered**:
- **KWin TabBox switcher theme (QML)** — *Rejected.* KWin core owns the TabBox lifecycle, grabs the
  keyboard itself, drives selection, and **closes the switcher when the held modifier is released**.
  A theme is a visual layout over a fixed `model`/`currentIndex` contract and cannot implement
  persistent-open or a second navigation level. Directly blocks FR-008.
- **Scripted (JavaScript) KWin effect** — *Rejected.* Insufficient UI capability for a custom
  full-screen grouped grid; the QML/C++ effect path is the supported route for rich overlays.
- **Standalone Qt application using Wayland/X11 protocols** — *Rejected.* Cannot obtain live window
  thumbnails on Wayland without compositor cooperation; would require fragile per-session capture
  paths and would not be "native-first." Fails Principle I and II.

---

## 2. Language choice — C++ vs. Rust/FFI

**Decision**: **C++ only** (Qt 6 / KF6) for the entire plugin.

**Rationale**:
- The effect shell *must* be C++: subclassing `QuickSceneEffect` requires Qt's moc meta-object
  system and the `KWIN_EFFECT_FACTORY`/`KPluginFactory` plugin entry point — not expressible from
  Rust.
- Everything the would-be Rust core needs to touch is Qt/KDE-typed anyway (`KService` for app
  resolution, `QAbstractItemModel` + roles for QML, `QIcon`, `KConfig`), so a Rust core would add a
  second toolchain and per-snapshot FFI marshalling for no hot-path benefit — the real hot paths are
  the compositor's thumbnail rendering and Qt model updates, neither of which Rust accelerates.
- Keeping one language serves Principle IV (simplicity) and Principle I (KDE ecosystem conventions).

**Alternatives considered**:
- **C++ effect shell + Rust core over a C ABI (cxx/cbindgen)** — *Rejected.* Marginal benefit,
  added FFI boundary and build complexity. (Decision explicitly confirmed with the user.)
- **Rust QAbstractItemModel via KDAB cxx-qt** — *Rejected.* Highest complexity/risk; cxx-qt inside a
  KWin effect plugin is largely untrodden ground.

---

## 3. Grouping windows by application

**Decision**: Resolve each window to a **desktop file id** and group on that, mirroring Plasma's own
`libtaskmanager`. Resolution order:
1. Read the window's `desktopFileName` (KWin exposes it; X11 apps advertise it via
   `_KDE_NET_WM_DESKTOP_FILE`/`_GTK_APPLICATION_ID`, Wayland via `app_id`).
2. Resolve via `KService::serviceByStorageId()`; use the resulting `applications:<menuId>` as the
   stable group key; display name = `service->name()`, icon = `service->icon()` →
   `QIcon::fromTheme()`.
3. **Fallback** when no service resolves: use the `WM_CLASS` *Class* string (X11) / `app_id`
   (Wayland) as a synthetic group key (FR-017's labeled fallback group, with a placeholder icon).

**Rationale**: Matches the canonical Plasma Task Manager grouping (`TasksModel::GroupApplications`,
`AbstractTasksModel::AppId`), so grouping behaves consistently with the rest of the desktop and
handles the same edge cases (multiple windows of one app collapse to one group). The desktop file is
the most reliable cross-session identity when present; the class/app_id fallback covers Wine/Electron
/unpackaged apps.

**Alternatives considered**:
- **Group on raw `WM_CLASS` only** — *Rejected as primary.* Less reliable than the desktop file
  (e.g. apps whose class differs from their launcher); kept only as the fallback.
- **Group on PID** — *Rejected.* Breaks multi-process apps (e.g. browsers) and single-instance
  launchers; does not match user expectation of "all Chrome windows together."

---

## 4. Live thumbnails + eager warming + minimized windows

**Decision**: Use QML `WindowThumbnail { client: <window> }` (import `org.kde.kwin`). On switcher
activation, **eagerly instantiate/warm thumbnails for all mapped windows** (not just the open
group). For **minimized windows** (which may have no live texture), fall back to the resolved
**application icon**; optionally cache a last-known snapshot captured before minimize if a real image
is required later (treated as an enhancement, not v1-blocking).

**Rationale**: Thumbnails are composited by KWin from the already-resident GPU texture — cheap, and
correct on both sessions. Eager warming satisfies FR-011/SC-003 (previews ready by the time a group
is opened). The minimized-window limitation is a documented KWin behavior; falling back to the icon
satisfies FR-012/edge cases without blocking.

**Caveats** (from research, carry into implementation):
- You **cannot** draw other QML elements *on top of* a `WindowThumbnail` (it is composited outside
  the QML scene graph) — design tiles so overlays (title, selection ring) sit beside/around the
  thumbnail, not over it.
- No published benchmark for ~50 simultaneous thumbnails; architecturally cheap but **must be
  profiled** against SC-002 (Principle II requires measurement).

---

## 5. Dark/light theming following the system

**Decision**: Style the overlay with **`Kirigami.Theme`** (`import org.kde.kirigami as Kirigami`),
binding colors to its roles (`backgroundColor`, `textColor`, `highlightColor`, …) and selecting
`colorSet` per container. Default to following the system color scheme; expose an optional override
in config.

**Rationale**: Kirigami.Theme follows the system color scheme and updates **in real time** when the
user switches light/dark (documented behavior), satisfying FR-014 with no manual reload. It is the
Plasma-6-recommended path for QML overlays; `PlasmaCore.ColorScope` is deprecated.

**Alternatives considered**:
- **Raw QPalette / hardcoded colors** — *Rejected.* Violates Principle I (theme respect) and would
  not auto-switch.
- **PlasmaComponents styling only** — *Acceptable for controls* (buttons), but color primitives come
  from Kirigami.Theme; we use PlasmaComponents where a styled control is needed.

---

## 6. Invocation shortcut + persistent grab

**Decision**: Register a configurable global shortcut via the QML **`ShortcutHandler`**
(`org.kde.kwin`) (KGlobalAccel under the hood) to toggle the effect; on activation call
`setRunning(true)` and `grabKeyboard()`. The overlay then receives all key events directly and
closes only on confirm (activate) or Escape (cancel); pressing the shortcut again advances the
highlight (FR-009).

**Rationale**: This is the verified, current mechanism (the older C++
`EffectsHandler::registerGlobalShortcut` is not present in current KWin master). The keyboard grab is
what makes "stays open after key release" possible (FR-008).

**Open item**: Whether to *also* let a configured trailing modifier release confirm the selection is
a UX nicety, but the spec's confirmed contract is advance-on-repress + Enter/click to confirm, so the
default binding does **not** confirm on release.

---

## 7. Build & test toolchain

**Decision**:
- **Build**: CMake + **extra-cmake-modules (ECM)**; KDE install dirs/compiler settings; plugin
  registered with `KWIN_EFFECT_FACTORY_*` macros + `metadata.json` (`KPackageStructure:
  KWin/Effect`). Install to `${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins/`.
- **C++ tests**: QTest in **`autotests/`** via `ecm_add_test(... LINK_LIBRARIES Qt6::Test)`,
  `QTEST_GUILESS_MAIN` for logic; run headless with `QT_QPA_PLATFORM=offscreen`, `LANG=C`.
- **QML tests**: QtQuick Test (`TestCase`) run with **`qmltestrunner`**.
- **C++ standard**: C++20 floor for our code; match KWin's standard when linking its headers.

**Rationale**: These are the documented KDE conventions; the offscreen QPA lets the (compositor-free)
core/model/state-machine tests run in CI. Anything requiring real GL (live thumbnail rendering) is
out of headless scope and verified manually / under `xvfb-run`.

**Alternatives considered**:
- **GoogleTest / Catch2** — *Rejected.* QTest integrates with Qt signals/`QSignalSpy` and ECM, and
  is the KDE-standard choice.

---

## 8. Keeping behavior testable without a compositor (Principle III)

**Decision**: Define a `WindowSource` interface returning immutable `WindowDescriptor` snapshots and
emitting change signals. The live implementation (`EffectWindowSource`) adapts KWin's
`EffectWindow`/`EffectsHandler`; tests use a `FakeWindowSource`. `GroupingEngine`, `SwitcherModel`,
and `SessionController` depend only on `WindowSource` + descriptors, never on KWin types.

**Rationale**: This is the seam that makes the Red-Green-Refactor cycle possible for all grouping,
ordering, selection, and state-transition logic — the parts most likely to regress — without a
running Plasma session.

**Caveat**: `WindowThumbnail` needs the *live* KWin window object, so the QML-facing model carries an
opaque `windowHandle` role that is the real window at runtime and null/fake in tests. Thumbnail
rendering itself is verified manually, not in unit tests.

---

## 9. Risks & version coupling

- **KWin effects ABI is not stable across releases.** C++ effects generally must be rebuilt per KWin
  minor version. *Mitigation*: pin and record the target KWin version at build time; confine KWin
  coupling to `src/effect/`; provide a clear rebuild path in quickstart.
- **`EffectWindow` accessors vary by version.** `windowClass()`/`icon()`/`caption()`/`desktops()`/
  `activities()`/`screen()`/`isMinimized()` are confirmed; a direct `desktopFileName()` accessor was
  *not* verified on `EffectWindow` in master. *Mitigation*: resolve the desktop file from available
  metadata (class/app_id + KWindowSystem/KService) in `ApplicationResolver`, verified against the
  target version during implementation; do not hard-depend on an unverified accessor.
- **Real-GL tests can't run under the offscreen QPA.** *Mitigation*: keep thumbnail rendering out of
  unit scope; verify manually and optionally under `xvfb-run`.

All NEEDS CLARIFICATION items from Technical Context are resolved; no blockers remain for Phase 1.
