# Contract: Interaction State Machine

This is the "contract to implement these" behaviors the spec calls out (persistent open, re-press,
confirm/cancel). It is the authoritative definition of FR-008, FR-009, and FR-010 and is implemented
by `SessionController` — fully unit-testable with `FakeWindowSource`.

## States

| State | Meaning |
|-------|---------|
| `Closed` | No overlay; keyboard not grabbed |
| `Overview` | Full-screen overlay showing application groups (FR-001) |
| `InGroup` | Showing the grid of one group's window thumbnails (FR-005/006) |

## Transitions

| From | Event | Guard | To | Effects |
|------|-------|-------|----|---------|
| `Closed` | `open()` (shortcut) | ≥1 eligible window | `Overview` | grab keyboard; build groups; highlight = previous app (FR-003); begin eager thumbnail warming (FR-011) |
| `Closed` | `open()` | 0 eligible windows | `Closed` | show transient empty state, then close (FR-019) |
| `Overview` | `advanceHighlight()` (shortcut re-press / forward) | — | `Overview` | move highlight to next group, **wrap** after last; overlay stays open (FR-009/008) |
| `Overview` | `retreatHighlight()` / `moveHighlight()` | — | `Overview` | move highlight (FR-004) |
| `Overview` | `enterHighlightedGroup()` | group has ≥1 window | `InGroup` | open group; highlight its MRU window; preserve overview highlight for back (FR-005) |
| `Overview` | `confirm()` | — | `Closed` | activate highlighted group's MRU window; ungrab; close (FR-010) |
| `InGroup` | `moveHighlight()` / `advanceHighlight()` | — | `InGroup` | move within grid; scroll to keep selection visible (FR-006/007) |
| `InGroup` | `back()` | — | `Overview` | return; originating group stays highlighted (FR-005) |
| `InGroup` | `confirm()` | — | `Closed` | activate highlighted window; ungrab; close (FR-010) |
| `Overview`/`InGroup` | `cancel()` (Escape) | — | `Closed` | restore focus to previously-active window; activate nothing; ungrab; close (FR-009) |
| `Overview`/`InGroup` | `windowAdded/Removed/Changed` | — | same | rebuild/patch groups; clamp indices so no stale window is selectable; if now empty → `Closed` (FR-018/019) |

## Invariants

- **Persistence (FR-008)**: No transition is triggered by modifier-key release or by any timeout.
  Only `confirm`, `cancel`, or `windowRemoved`-to-empty leave `Overview`/`InGroup`.
- **Re-press (FR-009)**: the bound shortcut maps to `advanceHighlight()` while open — it never
  closes the overlay.
- **Confirm vs. cancel (FR-010/009)**: confirm always activates exactly one window and closes;
  cancel always closes without changing the active window.
- Indices are always within bounds of the current `groups`/group after any live update (FR-018).

## Test scenarios (must be written first — Principle III)

1. open with 0 windows → stays/returns Closed, empty state flagged.
2. open with 3 apps → Overview, 3 groups, highlight defaults to previous app.
3. repeated `advanceHighlight()` cycles groups and **wraps**; never closes.
4. `enterHighlightedGroup()` → InGroup with that group's windows; `back()` → Overview, same group
   highlighted.
5. `confirm()` in Overview activates the highlighted group's MRU window; in InGroup activates the
   highlighted window.
6. `cancel()` activates nothing and requests restore of previously-active window.
7. `windowRemoved` of the highlighted window clamps selection; removing the last window closes with
   empty state.
8. `windowAdded` for a new app inserts a group without losing the current highlight target.
