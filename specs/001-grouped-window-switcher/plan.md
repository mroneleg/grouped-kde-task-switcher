# Implementation Plan: Grouped Window Switcher

**Branch**: `001-grouped-window-switcher` | **Date**: 2026-05-31 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/001-grouped-window-switcher/spec.md`

## Summary

A custom KDE Plasma task switcher that replaces the stock Alt+Tab with a persistent, full-screen
overlay. Windows are grouped by their owning application (one entry per app, with the app's logo
and window count); drilling into a group reveals a uniform, scrollable grid of live window
thumbnails. The overlay stays open after the trigger keys are released and closes only on confirm
or cancel.

**Technical approach** (from research): implement it as a **C++ `QuickSceneEffect` KWin 6 plugin**
(the same vehicle as KWin's built-in Overview / Window View effects), with a **QML overlay** for
the declarative UI and theming. This is the only supported vehicle that (a) keeps the switcher open
independent of modifier state — via `EffectsHandler::grabKeyboard()` — and (b) renders **live,
compositor-backed thumbnails** that work identically on Wayland and X11. A stock TabBox switcher
theme cannot do either (KWin core owns its lifecycle and closes it on modifier release). All
behavioral logic — grouping, ordering, selection/cycling, and the open→overview→in-group state
machine — lives in a **pure C++ core** behind a `WindowSource` abstraction, so it is unit-testable
with fake windows and no running compositor (Constitution Principle III).

The implementation language is **C++ only** (Qt 6 / KF6): the effect shell *must* be C++ (moc-based
`QuickSceneEffect` subclass + `KPluginFactory`), and keeping the whole stack C++ avoids an FFI
boundary for logic that is not on a hot path. A Rust core via FFI was considered and rejected — see
[research.md](./research.md) §"Language choice".

## Technical Context

**Language/Version**: C++20 (minimum; match the installed KWin's standard when linking its effect
headers — KWin `master` uses C++23, stable Plasma 6.x branches are lower). QML (Qt Quick) for the
overlay UI only.

**Primary Dependencies**: KWin 6 effects API (`QuickSceneEffect`, `EffectsHandler`,
`EffectWindow`); Qt 6 (Core, Quick/QML, DBus); KDE Frameworks 6 — KConfig, KCoreAddons, KI18n,
KWindowSystem, KService (app/desktop-file resolution), KGlobalAccel (shortcut), KF6 Kirigami
(theming). QML imports `org.kde.kwin` (`WindowThumbnail`, `WindowModel`, `ShortcutHandler`) and
`org.kde.kirigami` (`Kirigami.Theme`).

**Storage**: Local config only (KConfig/KConfigXT) for the invocation shortcut and optional theme
override. No persistent data store; window/thumbnail state is transient per session.

**Testing**: QTest (C++ logic, in `autotests/`, `QTEST_GUILESS_MAIN`, run under
`QT_QPA_PLATFORM=offscreen`); QtQuick Test / `qmltestrunner` (`TestCase`) for QML view logic;
optional `xvfb-run`/software-GL job for anything exercising real GL. Manual verification on a live
Plasma session for thumbnail rendering and shortcut behavior.

**Target Platform**: KDE Plasma 6 / KWin 6 on Linux, Wayland **and** X11 sessions. Developed against
the user's installed stable Plasma 6.x; the exact KWin build version is recorded at build time
because the KWin effects ABI is not stable across releases.

**Project Type**: Desktop system component — an out-of-tree KWin compositor effect plugin (C++
shared library) plus a QML UI package.

**Performance Goals**: Overlay visible within 100 ms of the shortcut (p95); 60 fps (≤16 ms frame)
while cycling groups and scrolling thumbnails; window thumbnails fully rendered for ≥90% of windows
by the time their group is opened (eager warming on activation).

**Constraints**: In-process with the compositor — must never block the compositor's main thread;
thumbnail memory bounded (warm only mapped/visible windows, fall back to icon for minimized);
must degrade to the application icon when a live thumbnail is unavailable; both session types
supported with documented exceptions only where a platform cannot comply.

**Scale/Scope**: Smooth with ≥50 windows across ≥10 application groups (SC-002); remains navigable
(scrolling, no overlap) up to several hundred windows. Single-user desktop; one switcher instance
at a time.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Assessment | Status |
|-----------|------------|--------|
| **I. KDE & Plasma Conventions** | Uses the supported KWin effects API (`QuickSceneEffect`), KWindowSystem/KService for window→app resolution, KGlobalAccel for the shortcut, Kirigami.Theme for theming, and standard ECM/kpackage install paths. Live thumbnails are compositor-rendered, so Wayland and X11 are both supported through one code path. No scraping or unsupported internals (we implement our own grid rather than depend on KWin's private `WindowHeap`). | ✅ PASS |
| **II. Performance & Native-First** | The effect runs in-process with the compositor in C++; window enumeration, grouping, ordering, and selection are native. Thumbnails are GPU/compositor-backed (cheap), warmed eagerly on activation for mapped windows. QML is used only for declarative layout/theming, never for hot paths. Budgets (100 ms open, 60 fps) are explicit and will be measured. | ✅ PASS |
| **III. Test-First (NON-NEGOTIABLE)** | All behavioral logic sits in a pure C++ core (`GroupingEngine`, `SessionController`, `SwitcherModel`) behind a `WindowSource` interface, unit-testable with fake windows and no compositor, headless via the offscreen QPA. Tests are written before implementation (Red-Green-Refactor), enforced in tasks. | ✅ PASS |
| **IV. Simplicity & UX Clarity** | One purpose (find + activate a window, grouped by app). Zero-config defaults: follows system theme, most-recently-used ordering. Only configuration is the (re-bindable) shortcut and an optional theme override. Single plugin; the interaction model is a small, explicit state machine. C++-only avoids a second toolchain/FFI. | ✅ PASS |

**Initial gate**: PASS — no violations. No entries required in Complexity Tracking.

**Post-Design re-check** (after Phase 1): PASS — the design keeps KWin coupling confined to a thin
`EffectWindowSource` adapter + the QML thumbnail binding; the core, model, and state machine remain
compositor-free and fully testable. No new violations introduced. See [data-model.md](./data-model.md)
and [contracts/](./contracts/).

## Project Structure

### Documentation (this feature)

```text
specs/001-grouped-window-switcher/
├── plan.md              # This file
├── spec.md              # Feature specification
├── research.md          # Phase 0 output — technology decisions
├── data-model.md        # Phase 1 output — entities & state
├── quickstart.md        # Phase 1 output — build/install/test/run
├── contracts/           # Phase 1 output — interface contracts
│   ├── window-source.md          # core ⇄ compositor boundary (testable seam)
│   ├── switcher-model.md         # QML-facing model & controller API
│   ├── interaction-state-machine.md  # open/overview/in-group contract (FR-008/009/010)
│   └── activation.md             # window activation behavior
└── checklists/
    └── requirements.md  # spec quality checklist
```

### Source Code (repository root)

```text
src/
├── core/                         # Pure logic — NO KWin/Qt-GUI deps (unit-testable)
│   ├── WindowDescriptor.h        # value type: id, appKey, caption, icon name, desktop, activity, screen, recency, minimized
│   ├── WindowSource.h            # abstract interface: snapshot() -> list<WindowDescriptor>, change signals
│   ├── ApplicationResolver.*     # window metadata -> (appKey, display name, icon) via KService, with WM_CLASS/app_id fallback
│   ├── GroupingEngine.*          # descriptors -> ordered ApplicationGroups (MRU), fallback group
│   └── SessionController.*       # interaction state machine: Closed/Overview/InGroup, selection, cycling
├── model/                        # Qt model layer (QObject/QAbstractItemModel; no compositor calls)
│   └── SwitcherModel.*           # exposes groups/entries + roles (incl. opaque live-window handle) to QML
├── effect/                       # KWin glue — the only compositor-coupled code
│   ├── GroupedSwitcherEffect.*   # QuickSceneEffect subclass: shortcut, keyboard grab, lifecycle, QML host
│   └── EffectWindowSource.*      # adapts EffectWindow -> WindowDescriptor; activate/raise/switch-desktop
├── ui/                           # QML overlay (declarative + theming only)
│   ├── main.qml                  # per-screen root; wires model/controller; ShortcutHandler
│   ├── GroupOverview.qml         # grouped app list with logos + counts
│   ├── ApplicationGroupGrid.qml  # uniform, scrollable grid of WindowThumbnail
│   ├── WindowTile.qml            # one thumbnail tile (live thumbnail + title + placeholder)
│   └── theme/                    # Kirigami.Theme-based styling helpers
└── metadata.json                 # KWin/Effect plugin metadata (KPlugin block)

autotests/                        # C++ QTest (KDE convention), headless via offscreen QPA
├── ApplicationResolverTest.cpp
├── GroupingEngineTest.cpp
├── SessionControllerTest.cpp
├── SwitcherModelTest.cpp
└── fakes/FakeWindowSource.*      # deterministic test double for WindowSource

tests/qml/                        # qmltestrunner TestCase specs for view logic
├── tst_groupgrid.qml             # grid alignment, scrolling reachability
└── tst_navigation.qml            # overview<->group navigation, highlight movement

CMakeLists.txt                    # ECM + KF6 + KWin; targets: effect plugin, core lib, tests
```

**Structure Decision**: A single out-of-tree KWin effect project. The decisive split is **core vs.
effect**: everything that defines behavior lives in `src/core` + `src/model` with no compositor
dependency (testable per Principle III), while `src/effect` is a thin adapter to the live KWin
`EffectWindow`/`EffectsHandler` API and `src/ui` is declarative QML. This keeps native code on the
hot paths (Principle II) and confines the unstable KWin ABI coupling to one directory so version
churn has the smallest possible blast radius.

## Complexity Tracking

> No constitution violations — no justifications required.
