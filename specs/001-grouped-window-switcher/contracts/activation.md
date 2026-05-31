# Contract: Window Activation

Defines what "activate" means (FR-010, FR-020) and the cancel/restore behavior (FR-009). Performed by
the live `EffectWindowSource` against KWin (`EffectsHandler::activateWindow`, desktop/activity
switching); modeled in tests by `FakeWindowSource` recording the request.

## activate(windowId)

Given the window the user confirmed:

1. If the window is on a different **virtual desktop**, switch to that desktop (FR-020).
2. If the window is on a different **activity**, switch to that activity (FR-020).
3. **Raise** the window above others and give it keyboard **focus** (frontmost + active).
4. If minimized, **unminimize** it first.
5. The window appears on its own **monitor**; the switcher does not move windows between monitors.

**Result**: exactly one window becomes active and frontmost (SC-008). The overlay then closes
(`Closed`).

**Failure (FR-018)**: if the window was closed between snapshot and confirm, activation returns
false; the controller falls back to the nearest remaining window in the same group, or closes with
no activation if none remain — never errors.

## Group confirm (FR-010)

Confirming an application *group* (in Overview) activates that group's `mostRecentWindow` using the
same `activate()` steps.

## cancel / restore (FR-009)

On Escape: do **not** activate any switcher selection. Request `restoreActive(previouslyActiveWindow)`
so focus returns to whatever was active before the switcher opened. If that window no longer exists,
leave focus untouched (no error).

## Success criteria mapping

- SC-008: intended window focused + frontmost in ≥99% of selections, including cross-monitor and
  cross-desktop targets.
- FR-009/FR-010/FR-020 fully covered.
