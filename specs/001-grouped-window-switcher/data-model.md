# Phase 1 Data Model: Grouped Window Switcher

Entities are derived from the spec's Key Entities and Functional Requirements. The model is
transient (per switcher session) — no persistence beyond config. Types below are conceptual; C++
declarations live in `src/core` and `src/model`.

---

## WindowDescriptor (value type, `src/core`)

An immutable snapshot of one eligible window at the moment the switcher opened. Produced by
`WindowSource`; consumed by `GroupingEngine`/`SwitcherModel`. Carries **no KWin types** so it is
testable.

| Field | Type | Notes / Source |
|-------|------|----------------|
| `id` | opaque stable id (e.g. `QUuid`/`WId`) | Identifies the window for activation; FR-010 |
| `appKey` | string | Grouping key — resolved desktop-file id, else `WM_CLASS`/`app_id` fallback; FR-001/017 |
| `appDisplayName` | string | From `KService::name()`; group label; FR-002 |
| `appIconName` | string | From `KService::icon()`; resolved via theme; placeholder if absent; FR-002/edge |
| `caption` | string | Window title; shown on the tile; FR-005 |
| `desktop` | virtual-desktop ref | For scope + activation desktop switch; FR-020/010 |
| `activity` | activity id | For scope + activation; FR-020/010 |
| `screen` | screen ref | Monitor the window is on; FR-020 |
| `isMinimized` | bool | Drives thumbnail-vs-icon fallback; FR-012, research §4 |
| `lastUsedAt` | monotonic timestamp/rank | MRU ordering of groups and entries; FR-003 |
| `windowHandle` | opaque pointer (runtime only) | The live KWin window for `WindowThumbnail.client`; **null in tests** |

**Validation / rules**:
- `appKey` is never empty: if no desktop file and no class/app_id, assign the shared fallback key
  (`__unidentified__`) → fallback group (FR-017).
- Descriptors with non-user-facing window types (utility, dock/panel, the overlay itself) are
  excluded by `WindowSource` before reaching the core (Assumptions: window eligibility).

---

## ApplicationGroup (`src/core`, exposed via model)

All eligible windows of one application. Built by `GroupingEngine` from descriptors sharing an
`appKey`.

| Field | Type | Notes |
|-------|------|-------|
| `appKey` | string | Identity; unique within a session |
| `displayName` | string | App name (FR-002) |
| `iconName` | string | App logo; placeholder when unknown (FR-002/edge) |
| `windows` | ordered list<WindowDescriptor> | Members, MRU-ordered (FR-003/006) |
| `count` | int | `windows.size()` (FR-002) |
| `isFallback` | bool | True for the unidentified-apps group (FR-017) |
| `mostRecentWindow` | WindowDescriptor ref | Target when the group (not a window) is confirmed (FR-010) |

**Rules**:
- Group ordering: by the recency of each group's `mostRecentWindow`, most-recent first (FR-003).
  The group of the currently-active window sorts such that the *previous* app is the fast default
  highlight.
- A single-window group is valid; entering it shows one tile (edge case), and confirming the group
  may activate that window directly.
- Within-group window ordering: MRU (FR-006 stable ordering).

---

## SwitcherSession (`src/core`: `SessionController` state)

Transient state of one open switcher. Embodies the interaction contract (see
[contracts/interaction-state-machine.md](./contracts/interaction-state-machine.md)).

| Field | Type | Notes |
|-------|------|-------|
| `level` | enum {`Closed`, `Overview`, `InGroup`} | Current view level; FR-005/008 |
| `groups` | ordered list<ApplicationGroup> | The built model for this session |
| `highlightedGroupIndex` | int | Selection in Overview; FR-004 |
| `openGroupIndex` | int (valid only when `InGroup`) | Which group is drilled into |
| `highlightedWindowIndex` | int (valid only when `InGroup`) | Selection in the grid; FR-006 |
| `scrollOffset` | per-level scroll position | Preserved while navigating; FR-007 |
| `previouslyActiveWindowId` | window id | Restored on cancel; FR-009 (cancel) |

**State transitions** (full contract in the interaction-state-machine doc):
- `Closed → Overview`: shortcut invoked; build groups; default highlight = previous app (FR-003);
  begin eager thumbnail warming (FR-011).
- `Overview → Overview`: shortcut re-press advances `highlightedGroupIndex` (wraps); arrows move
  highlight (FR-009/004).
- `Overview → InGroup`: enter the highlighted group (FR-005).
- `InGroup → Overview`: back; `highlightedGroupIndex` stays on the originating group (FR-005).
- `* → Closed (activate)`: confirm → activate selected window/group's MRU window (FR-010).
- `* → Closed (cancel)`: Escape → restore focus to `previouslyActiveWindow`, activate nothing
  (FR-009).
- Live update: windows opening/closing mutate `groups` and clamp indices so no stale window is
  selectable (FR-018); empty result → empty state then close (FR-019).

---

## ThemeState (`src/ui`, via `Kirigami.Theme`)

Not a stored entity — a binding to the system color scheme. Active mode (light/dark) and color roles
come from `Kirigami.Theme`; the overlay binds to it and updates live on system change (FR-014). An
optional config override (`follow-system` | `light` | `dark`) may pin the mode.

---

## ThumbnailState (per WindowTile, `src/ui`)

UI-only state for a tile's preview lifecycle (FR-011/012):

| State | Meaning |
|-------|---------|
| `Warming` | Thumbnail requested (eagerly, on activation); placeholder shown |
| `Ready` | Live `WindowThumbnail` rendering |
| `IconFallback` | No live texture (e.g. minimized) or compositing unavailable → app icon shown |

Transitions are driven by window mapped/minimized state and thumbnail availability; never block the
UI thread (Principle II).

---

## Relationships (summary)

```
WindowSource ──snapshot──▶ [WindowDescriptor] ──GroupingEngine──▶ [ApplicationGroup] ──▶ SwitcherModel ──▶ QML
                                   │                                      │
                            ApplicationResolver                    SessionController (level, selection, scroll)
                            (appKey, name, icon)                          │
                                                                   activation ──▶ WindowSource.activate(id)
```
