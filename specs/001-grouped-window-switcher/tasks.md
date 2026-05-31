---

description: "Task list for Grouped Window Switcher implementation"
---

# Tasks: Grouped Window Switcher

**Input**: Design documents from `/specs/001-grouped-window-switcher/`

**Prerequisites**: [plan.md](./plan.md), [spec.md](./spec.md), [research.md](./research.md),
[data-model.md](./data-model.md), [contracts/](./contracts/)

**Tests**: TDD is **mandatory** — the project constitution makes Test-First non-negotiable
([../../.specify/memory/constitution.md](../../.specify/memory/constitution.md), Principle III).
Logic tests are written **first** and must **fail** before implementation. Live-thumbnail rendering
and theme appearance are verified manually (cannot run under the headless offscreen QPA).

**Organization**: Tasks are grouped by user story so each can be implemented and tested
independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies on incomplete tasks)
- **[Story]**: US1–US4, mapping to the spec's user stories
- All paths are repo-relative and assume the structure in [plan.md](./plan.md)

## Path Conventions

KWin effect project: `src/core` (compositor-free logic), `src/model` (Qt model), `src/effect` (KWin
glue), `src/ui` (QML), `autotests/` (C++ QTest), `tests/qml/` (qmltestrunner).

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and build/test scaffolding

- [ ] T001 Create the source tree per plan (`src/core/`, `src/model/`, `src/effect/`, `src/ui/`, `src/ui/theme/`, `src/config/`, `autotests/fakes/`, `tests/qml/`) with `.gitkeep` placeholders
- [ ] T002 Author root `CMakeLists.txt`: `find_package(ECM)`, Qt6 (Core, Gui, Qml, Quick, DBus, Test), KF6 (Config, CoreAddons, I18n, WindowSystem, Service, GlobalAccel, Kirigami), KWin effects; define a `core` static lib target, the `kwin_grouped_switcher` effect plugin target, and `enable_testing()`; set C++20 floor without overriding `KDECompilerSettings`
- [ ] T003 [P] Create the KWin/Effect plugin descriptor in `src/metadata.json` (KPlugin Id `groupedswitcher`, Name, Description, `KPackageStructure: KWin/Effect`, `X-Plasma-API`)
- [ ] T004 [P] Add `.clang-format` and `.clang-tidy` at repo root matching the KDE/Qt code style (Principle I)
- [ ] T005 [P] Wire test infrastructure in `autotests/CMakeLists.txt` and `tests/qml/CMakeLists.txt`: `include(ECMAddTests)`, register CTest to run with `QT_QPA_PLATFORM=offscreen` and `LANG=C`, and add a `qmltestrunner -input tests/qml` CTest entry

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: The testable core/compositor seam and the effect+QML host skeleton that every user
story builds on

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T006 [P] Define the `WindowDescriptor` value type in `src/core/WindowDescriptor.h` with all fields from [data-model.md](./data-model.md) (id, appKey, appDisplayName, appIconName, caption, desktop, activity, screen, isMinimized, lastUsedAt, opaque windowHandle) — no KWin/Qt-GUI types
- [ ] T007 [P] Define `WindowId` and the abstract `WindowSource` interface in `src/core/WindowSource.h` (`snapshot()`, `activate()`, `restoreActive()`, `previouslyActive()`, `windowAdded/Removed/Changed` signals) per [contracts/window-source.md](./contracts/window-source.md)
- [ ] T008 [P] Implement the `FakeWindowSource` test double in `autotests/fakes/FakeWindowSource.h` / `.cpp` (scripted descriptor lists, on-demand change-signal emission, recorded `activate()`/`restoreActive()` calls)
- [ ] T009 Create the `GroupedSwitcherEffect` skeleton in `src/effect/GroupedSwitcherEffect.h` / `.cpp`: subclass `QuickSceneEffect`, register with `KWIN_EFFECT_FACTORY*`, load `src/ui/main.qml` as the per-screen delegate, expose model + controller as QML context properties (stubbed) — depends on T002, T003
- [ ] T010 Add shortcut + open/close plumbing: a `ShortcutHandler` in `src/ui/main.qml` and `setRunning()`/`grabKeyboard()`/`ungrabKeyboard()` wiring in `GroupedSwitcherEffect` so the bound shortcut shows/hides a blank overlay — depends on T009
- [ ] T011 [P] Implement the `EffectWindowSource` adapter skeleton in `src/effect/EffectWindowSource.h` / `.cpp`: enumerate `stackingOrder()` **across all virtual desktops, activities, and monitors with no per-desktop/per-activity filtering** (FR-020 scope), filter eligibility only by window type (exclude dock/utility/overlay), map `EffectWindow` → `WindowDescriptor` (capturing each window's desktop/activity/screen), and subscribe to KWin add/remove/changed signals (activation/restore deferred to US1/US3) — depends on T006, T007
- [ ] T012 [P] Define the `SwitcherModel` skeleton (`QAbstractItemModel`) in `src/model/SwitcherModel.h` / `.cpp` with the group and entry role enums from [contracts/switcher-model.md](./contracts/switcher-model.md) (data populated in US1/US2)
- [ ] T013 [P] Define the `SessionController` skeleton in `src/core/SessionController.h` / `.cpp` with the `Level` enum (`Closed`/`Overview`/`InGroup`), intent-method stubs, and `stateChanged`/`closed` signals per [contracts/interaction-state-machine.md](./contracts/interaction-state-machine.md)

**Checkpoint**: Shortcut opens a blank overlay; the testable seam and model/controller skeletons
exist — user stories can now proceed.

---

## Phase 3: User Story 1 - Switch between applications from a grouped overview (Priority: P1) 🎯 MVP

**Goal**: Invoke the switcher to see one entry per application (logo + name + window count), navigate
groups, and confirm a group to activate that app's most-recently-used window.

**Independent Test**: Open windows across ≥3 apps, invoke the switcher → exactly one group per app
with correct logo/count; move the highlight; confirm → the chosen app's window is focused.

### Tests for User Story 1 (write first; must fail) ⚠️

- [ ] T014 [P] [US1] `ApplicationResolverTest` in `autotests/ApplicationResolverTest.cpp`: appKey/name/icon resolution via desktop file, `WM_CLASS`/`app_id` fallback, and the `__unidentified__` fallback key (FR-001/002/017)
- [ ] T015 [P] [US1] `GroupingEngineTest` in `autotests/GroupingEngineTest.cpp`: one group per app, MRU group ordering, default highlight = previous app, single-window group, fallback group (FR-001/002/003/017)
- [ ] T016 [P] [US1] `SessionControllerTest` (overview subset) in `autotests/SessionControllerTest.cpp`: open with N apps → Overview + group count; advance/retreat/move highlight (FR-004); confirm group → `FakeWindowSource.activate(MRU window)` (FR-010); open with 0 windows → empty then Closed (FR-019)
- [ ] T017 [P] [US1] `SwitcherModelTest` (group roles) in `autotests/SwitcherModelTest.cpp`: group roles (appDisplayName/appIconName/windowCount/isFallback), MRU order, reset on source change
- [ ] T050 [P] [US1] Window-scope test in `autotests/GroupingEngineTest.cpp`: given `FakeWindowSource` descriptors spanning multiple virtual desktops, activities, and monitors, **all** windows appear in the grouped result with no per-desktop/activity filtering (FR-020 scope) — *added by analysis remediation (C1)*

### Implementation for User Story 1

- [ ] T018 [P] [US1] Implement `ApplicationResolver` in `src/core/ApplicationResolver.h` / `.cpp` (KService `serviceByStorageId`, name/icon, class/app_id fallback, placeholder icon) — makes T014 pass
- [ ] T019 [P] [US1] Implement `GroupingEngine` in `src/core/GroupingEngine.h` / `.cpp` (descriptors → ordered `ApplicationGroup` list, MRU, fallback group, mostRecentWindow) — makes T015 pass
- [ ] T020 [US1] Implement `SessionController` Overview transitions + confirm in `src/core/SessionController.cpp` (open/advance/retreat/move/confirm, empty handling) — depends on T013, T019; makes T016 pass
- [ ] T021 [US1] Implement `SwitcherModel` group level in `src/model/SwitcherModel.cpp` (populate groups from the engine, expose group roles, handle source changes) — depends on T012, T019; makes T017 pass
- [ ] T022 [US1] Implement window activation in `EffectWindowSource::activate()` in `src/effect/EffectWindowSource.cpp` (`activateWindow`, switch desktop/activity, unminimize) per [contracts/activation.md](./contracts/activation.md) — depends on T011
- [ ] T023 [P] [US1] Build `src/ui/GroupOverview.qml`: row/list of groups showing app logo + name + count, highlight, keyboard + mouse selection bound to the controller (FR-001/002/004), with placeholder icon for missing logos (FR-002 edge)
- [ ] T024 [US1] Wire `src/ui/main.qml` to show `GroupOverview` when `level==Overview`, route shortcut/keys/click to `advanceHighlight`/`moveHighlight`/`enterHighlightedGroup`/`confirm`, and show an empty state (FR-019) — depends on T010, T020, T023
- [ ] T025 [US1] Connect `EffectWindowSource` → `GroupingEngine` → `SessionController`/`SwitcherModel` in `GroupedSwitcherEffect` (build groups on open) — depends on T009, T020, T021, T022

**Checkpoint**: MVP — invoke → grouped overview with logos/counts → confirm → switch to the app's
MRU window. Independently demoable.

---

## Phase 4: User Story 2 - Drill into a group to pick a specific window (Priority: P2)

**Goal**: Enter an application group to see a uniform, scrollable grid of live window thumbnails and
select a specific window; thumbnails are warmed eagerly on activation.

**Independent Test**: Open one app with several windows, invoke → enter its group → all windows
appear as aligned-grid thumbnails (scroll if many); select one → it is activated.

### Tests for User Story 2 (write first; must fail) ⚠️

- [ ] T026 [P] [US2] `SessionController` in-group tests in `autotests/SessionControllerTest.cpp`: `enterHighlightedGroup` → InGroup with the group's windows; move within grid; `back` → Overview with origin group still highlighted; confirm window → `activate(window)` (FR-005/006/010)
- [ ] T027 [P] [US2] `SwitcherModel` entry-role tests in `autotests/SwitcherModelTest.cpp`: entry roles (caption/windowHandle/thumbnailState/appIconName), MRU window order, entries reflect the open group
- [ ] T028 [P] [US2] `tests/qml/tst_groupgrid.qml` (qmltestrunner): grid cells are uniform (equal cellWidth/cellHeight, no overlap) and scrolling makes every entry reachable (FR-006/007)

### Implementation for User Story 2

- [ ] T029 [US2] Extend `SessionController` with InGroup transitions (enter/back/move-in-grid/confirm-window/scrollOffset) in `src/core/SessionController.cpp` — depends on T020; makes T026 pass
- [ ] T030 [US2] Extend `SwitcherModel` with per-group window entries + entry roles in `src/model/SwitcherModel.cpp` — depends on T021; makes T027 pass
- [ ] T031 [P] [US2] Implement `src/ui/WindowTile.qml`: `WindowThumbnail { client: windowHandle }` with the title/selection ring placed *beside* the thumbnail (not overlaid — research §4), placeholder while `Warming`, app icon on `IconFallback` (FR-011/012)
- [ ] T032 [US2] Implement `src/ui/ApplicationGroupGrid.qml`: `GridView` with uniform cells + `ScrollBar`, keyboard + mouse selection, scroll-to-keep-selection-visible (FR-006/007) — depends on T031; makes T028 pass
- [ ] T033 [US2] Wire `src/ui/main.qml` to show the grid when `level==InGroup`, route enter/back/confirm, preserve the overview highlight (FR-005) — depends on T024, T029, T032
- [ ] T034 [US2] Implement eager thumbnail warming on open in `GroupedSwitcherEffect`/`EffectWindowSource`: prewarm thumbnails for all mapped windows at activation (not at group-open), minimized → icon fallback (FR-011, SC-003, research §4) — depends on T025, T031

**Checkpoint**: US1 + US2 — drill into a group, scan an aligned grid of live thumbnails (scrolling
when many), pick a window.

---

## Phase 5: User Story 3 - Keep the switcher open for deliberate browsing (Priority: P3)

**Goal**: The overlay stays open after the trigger keys are released; the shortcut re-press advances
the highlight (wraps) without closing; Escape cancels and restores the prior window; the window set
updates live.

**Independent Test**: Invoke, release all keys → overlay stays open; re-press → highlight advances;
Escape → closes with no switch and prior focus restored.

### Tests for User Story 3 (write first; must fail) ⚠️

- [ ] T035 [P] [US3] `SessionController` lifecycle tests in `autotests/SessionControllerTest.cpp`: re-press `advanceHighlight` wraps and never closes (FR-008/009); `cancel` restores previously-active and activates nothing (FR-009); `windowRemoved` clamps selection and remove-last → empty/Closed; `windowAdded` inserts a group without losing the highlight target (FR-018/019)
- [ ] T051 [P] [US3] Single-instance / re-invocation test in `autotests/SessionControllerTest.cpp`: calling `open()` while already in `Overview`/`InGroup` reuses the one session (advances highlight) instead of creating a second; rapid repeated `open()` leaves exactly one consistent session (spec Edge Cases / Assumptions) — *added by analysis remediation (C2)*

### Implementation for User Story 3

- [ ] T036 [US3] Implement the persistent keyboard-grab lifecycle in `src/effect/GroupedSwitcherEffect.cpp`: grab on open, **no** auto-close on modifier release or timeout, ungrab + close only on confirm/cancel (FR-008); enforce a **single-instance guard** so re-invoking the shortcut while open reuses the existing session (advances the highlight) and rapid repeated invocation never stacks overlapping overlays (spec Edge Cases / Assumptions) — depends on T010, T025
- [ ] T037 [US3] Map shortcut re-press → `advanceHighlight` (wrap) and Escape → `cancel` in `src/ui/main.qml` + controller (FR-009) — depends on T020, T024, T036
- [ ] T038 [US3] Implement cancel/restore in `src/core/SessionController.cpp` + `EffectWindowSource::restoreActive()` in `src/effect/EffectWindowSource.cpp`: capture `previouslyActive` on open, restore it on cancel (FR-009) — depends on T011, T020; part of T035
- [ ] T039 [US3] Implement live-update handling in `SwitcherModel`/`SessionController`: react to `windowAdded/Removed/Changed`, clamp selection, close-on-empty (FR-018/019) — depends on T021, T029, T030; part of T035

**Checkpoint**: Overlay persists after key release; re-press cycles; Escape restores; live
open/close of windows updates the view.

---

## Phase 6: User Story 4 - Modern appearance following the system light/dark theme (Priority: P3)

**Goal**: A clean, modern overlay that follows the system color scheme (light/dark) by default and
updates when the system theme changes, with legible contrast.

**Independent Test**: System dark → invoke → dark overlay; system light → invoke → light overlay;
logos/labels/thumbnails legible in both.

### Tests for User Story 4 (write first; must fail) ⚠️

- [ ] T040 [P] [US4] `tests/qml/tst_theme.qml` (qmltestrunner): overlay components bind colors to `Kirigami.Theme` roles and respond to `colorSet` changes (FR-014/015)

### Implementation for User Story 4

- [ ] T041 [P] [US4] Create theme helpers in `src/ui/theme/` built on `Kirigami.Theme` (color roles, `colorSet`), follow-system by default with an optional override (FR-014) — makes T040 pass
- [ ] T042 [US4] Apply modern styling across `src/ui/GroupOverview.qml`, `src/ui/ApplicationGroupGrid.qml`, `src/ui/WindowTile.qml`, `src/ui/main.qml` (spacing, rounded selection, contrast in light/dark) (FR-015) — depends on T041, T023, T032
- [ ] T043 [P] [US4] Add KConfigXT config in `src/config/main.xml` (+ config UI) for the invocation shortcut and theme override, loaded by `GroupedSwitcherEffect` (FR-014/016)

**Checkpoint**: Overlay matches the system light/dark scheme with a modern look; shortcut/theme are
configurable.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Performance verification, packaging, accessibility, and end-to-end validation

- [ ] T044 [P] Add a performance measurement harness and record results for open latency (<100 ms p95, SC-001) and 60 fps cycling/scroll with ≥50 windows / ≥10 groups (SC-002, Principle II) — in `autotests/` (benchmark) + manual notes
- [ ] T045 [P] Instrument/measure that ≥90% of thumbnails are `Ready` by group-open (SC-003)
- [ ] T046 [P] Accessibility pass: keyboard-only operation, screen-reader labels on tiles/groups, reduced-motion respect across `src/ui/` (Principle IV, spec accessibility)
- [ ] T047 [P] Documentation: add repo `README.md` (build/install/enable) and reconcile any deltas in [quickstart.md](./quickstart.md); note the KWin-version ABI coupling (research §9)
- [ ] T048 Finalize CMake install rules (effect plugin → `${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins/`, QML/config packaged) in `CMakeLists.txt`
- [ ] T049 Run [quickstart.md](./quickstart.md) manual validation end-to-end on **both** Wayland and X11 sessions (FR-008/010/011/014, all SCs; Principle I dual-session requirement)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — start immediately
- **Foundational (Phase 2)**: Depends on Setup — **BLOCKS all user stories**
- **User Stories (Phase 3–6)**: All depend on Foundational; then proceed in priority order or in
  parallel where staffing allows
- **Polish (Phase 7)**: Depends on the targeted user stories being complete

### User Story Dependencies

- **US1 (P1)**: Foundational only — the MVP, no dependency on other stories
- **US2 (P2)**: Foundational; extends US1's `SessionController`/`SwitcherModel`/`main.qml` but is
  independently testable (in-group navigation + grid)
- **US3 (P3)**: Foundational; layers persistence/cancel/live-update onto the effect shell +
  controller — independently testable (open/persist/cancel)
- **US4 (P3)**: Foundational; purely presentational, independently testable (theme switch)

### Within Each User Story

- Tests are written first and must fail (Principle III)
- Core logic (`ApplicationResolver`, `GroupingEngine`, `SessionController`) before model before QML
- Story complete before moving to the next priority

---

## Parallel Opportunities

- **Setup**: T003, T004, T005 in parallel (after T002 scaffolds CMake)
- **Foundational**: T006, T007 in parallel; then T008, T011, T012, T013 in parallel (T009→T010 are
  sequential on the effect shell)
- **US1 tests**: T014, T015, T016, T017 together; **US1 impl**: T018, T019, T023 together (different
  files), then T020/T021 once T019 lands
- **US2 tests**: T026, T027, T028 together; T031 parallel with T029/T030
- **Polish**: T044, T045, T046, T047 in parallel

### Parallel Example: User Story 1 tests

```bash
Task: "ApplicationResolverTest in autotests/ApplicationResolverTest.cpp"
Task: "GroupingEngineTest in autotests/GroupingEngineTest.cpp"
Task: "SessionControllerTest (overview subset) in autotests/SessionControllerTest.cpp"
Task: "SwitcherModelTest (group roles) in autotests/SwitcherModelTest.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 only)

1. Phase 1: Setup
2. Phase 2: Foundational (CRITICAL — blocks all stories)
3. Phase 3: User Story 1
4. **STOP and VALIDATE**: invoke → grouped overview → confirm → switch works on Wayland and X11
5. Demo the MVP

### Incremental Delivery

1. Setup + Foundational → blank overlay opens on the shortcut
2. + US1 → grouped switching (MVP)
3. + US2 → drill-in grid with live thumbnails + scrolling
4. + US3 → persistent open, re-press cycling, cancel/restore, live updates
5. + US4 → system light/dark theming and modern look
6. Polish → performance budgets verified, packaged, dual-session validated

---

## Notes

- `[P]` = different files, no dependency on incomplete tasks
- TDD is mandatory (constitution Principle III): write the failing test, then implement
- Live-thumbnail rendering and theme appearance can't run headless — verify manually (quickstart §6)
- Keep all behavioral logic in `src/core`/`src/model` (compositor-free); confine KWin coupling to
  `src/effect` (Principle II, and minimizes KWin-ABI blast radius)
- Auto-commit is enabled — each Spec Kit step commits; commit after each task or logical group too
- Stop at any checkpoint to validate a story independently
- **Analysis remediation**: T050 (US1, FR-020 window-scope test) and T051 (US3, single-instance/
  re-invocation test) were added after `/speckit-analyze` to close coverage gaps C1 and C2; T011 and
  T036 were extended accordingly. These two IDs are out of numeric sequence by design (appended so
  existing T001–T049 and their issues keep their numbers). Total tasks: **51**.
