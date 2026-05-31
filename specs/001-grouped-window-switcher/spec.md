# Feature Specification: Grouped Window Switcher

**Feature Branch**: `001-grouped-window-switcher`

**Created**: 2026-05-31

**Status**: Draft

**Input**: User description: "Build a custom kde task switcher that elegantly groups windows by application type (chrome/vscode/etc) and include the application's logo. Activating the task switcher should not close it unless alt+tab (or whatever it was bound to) is hit again, if possible with the contract to implement these. Once the grouped apps are shown, going into a group should show a thumbnail of all the windows. They previews shouldn't be scattered randomly, and should be aligned in a grid, for easy scanning of them. If there are too many, scrolling should be available. If thumbnail operations are expensive, those should start as soon as things are activated (not waiting until the group is opened). It should have a modern look and feel and support dark and light modes (defaulting to the system)."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Switch between applications from a grouped overview (Priority: P1)

A user with many windows open across several applications invokes the switcher with their
keyboard shortcut. Instead of a flat list of every window, they see one entry per application
(e.g. one entry for all Chrome windows, one for all VS Code windows), each labeled with that
application's logo and a count of how many windows it has. They move the highlight to the
application they want and confirm, and the most relevant window of that application is brought
to the front and focused.

**Why this priority**: This is the core value of the feature — turning a cluttered window list
into a compact, scannable, application-grouped overview. It is a usable product on its own even
before per-window drill-down exists, because confirming an application group switches to that
application's active window.

**Independent Test**: Open several windows across at least three applications, invoke the
switcher, verify exactly one entry per application appears with the correct logo and window
count, move the highlight, confirm a selection, and verify the chosen application's window
receives focus.

**Acceptance Scenarios**:

1. **Given** five windows are open across three applications, **When** the user invokes the
   switcher, **Then** exactly three application groups are shown, each with the application's
   logo, name, and number of windows.
2. **Given** the switcher is open with a group highlighted, **When** the user advances the
   highlight with the switcher shortcut or arrow keys, **Then** the highlight moves to the next
   group in a consistent, predictable order.
3. **Given** a group is highlighted, **When** the user confirms the selection, **Then** the most
   recently used window of that application is activated and the switcher closes.
4. **Given** only one application has open windows, **When** the user invokes the switcher,
   **Then** a single group is shown and can be selected.

---

### User Story 2 - Drill into an application group to pick a specific window (Priority: P2)

After the grouped overview appears, the user enters a specific application's group and sees a
thumbnail preview of every window belonging to that application. The previews are arranged in a
neat, aligned grid (not scattered or overlapping) so the user can scan them quickly. If the
group has more windows than fit on screen, the user can scroll to reach the rest. The user picks
one window and it is activated.

**Why this priority**: This is what makes grouping useful when an application has many windows
(e.g. dozens of browser windows). It builds directly on the P1 overview and delivers precise
window selection. It is independently testable once the overview exists.

**Independent Test**: Open one application with several windows, invoke the switcher, enter that
application's group, and verify all of its windows appear as thumbnails arranged in an aligned
grid; select one and verify it is activated.

**Acceptance Scenarios**:

1. **Given** the grouped overview is shown, **When** the user enters a highlighted group, **Then**
   a thumbnail of every window in that application is displayed.
2. **Given** a group's window previews are shown, **When** the user views them, **Then** the
   previews are aligned in a uniform grid with consistent sizing and spacing, in a stable order.
3. **Given** a group has more windows than fit in the visible area, **When** the user navigates
   past the visible previews, **Then** the view scrolls to reveal the remaining previews and all
   windows remain reachable.
4. **Given** window previews are shown, **When** the user selects one preview, **Then** that
   specific window is activated and the switcher closes.
5. **Given** the user has entered a group, **When** the user requests to go back, **Then** the
   grouped overview is shown again with the originating group still highlighted.

---

### User Story 3 - Keep the switcher open for deliberate browsing (Priority: P3)

A user wants to browse their windows without the switcher vanishing the instant they let go of
the shortcut keys. Once invoked, the switcher stays open and usable for as long as the user
needs. The switcher is dismissed only by an explicit action: confirming a selection (which
activates it) or cancelling. This lets the user release the keyboard, use the mouse, read window
titles, and make an unhurried choice. Pressing the bound shortcut again does not close the
switcher — it advances the highlight to the next group.

**Why this priority**: This persistence is the distinguishing interaction model the user asked
for, removing the time pressure of hold-to-preview switchers. It layers on top of P1/P2 without
changing what those flows show.

**Independent Test**: Invoke the switcher, release all keys, and confirm the switcher remains
open and interactive; then trigger the dismissal action and confirm it closes as specified.

**Acceptance Scenarios**:

1. **Given** the user invokes the switcher and then releases all keys, **When** no further input
   occurs, **Then** the switcher remains open and interactive indefinitely.
2. **Given** the switcher is open, **When** the user presses the switcher shortcut again, **Then**
   the highlight advances to the next group and the switcher stays open (it closes only on confirm
   or cancel).
3. **Given** the switcher is open, **When** the user presses the cancel key (Escape), **Then** the
   switcher closes and no window is activated, leaving focus on the previously active window.

---

### User Story 4 - Modern appearance that follows the system light/dark theme (Priority: P3)

The switcher presents a clean, modern overlay. By default its appearance matches the system's
light or dark mode, so it feels native and is comfortable in any lighting. If the system theme
changes, the switcher reflects the active mode the next time it is shown.

**Why this priority**: Visual fit and theme correctness strongly affect perceived quality and
comfort, but the switcher is functionally usable before this is polished, so it ranks below the
core flows.

**Independent Test**: Set the system to dark mode and invoke the switcher to confirm a dark
appearance; switch the system to light mode and invoke again to confirm a light appearance.

**Acceptance Scenarios**:

1. **Given** the system is in dark mode, **When** the switcher is shown, **Then** it appears in a
   dark style consistent with the system.
2. **Given** the system is in light mode, **When** the switcher is shown, **Then** it appears in a
   light style consistent with the system.
3. **Given** the switcher is shown, **When** the user views it, **Then** application logos,
   labels, and thumbnails are legible with adequate contrast in the active mode.

---

### Edge Cases

- **No windows open**: Invoking the switcher when there are no eligible windows shows a clear
  empty state and dismisses without error; nothing is activated.
- **Single window in a group**: Entering a group that has only one window shows that single
  thumbnail in the grid; confirming the group directly may activate that window without requiring
  drill-down.
- **Unidentifiable application**: A window that cannot be mapped to a known application is shown
  under a generic fallback group with a placeholder logo, never dropped silently.
- **Missing or low-quality logo**: When an application's logo cannot be found, a neutral
  placeholder icon is shown so the entry is still selectable and labeled.
- **Thumbnail not ready yet**: If a window's preview has not finished generating when its group is
  opened, a placeholder is shown that is replaced by the real preview as soon as it is ready,
  without blocking interaction.
- **Window closes or opens while the switcher is open**: The displayed groups and counts update so
  the user never selects a window that no longer exists; selecting a vanished window falls back
  gracefully (e.g. nearest remaining window or no-op) rather than erroring.
- **Very large numbers of windows/groups**: The overview and grids remain navigable and scroll
  rather than overflowing or overlapping.
- **Rapid repeated invocation**: Quickly triggering the shortcut several times does not open
  multiple overlapping switchers or leave the session in an inconsistent state.
- **Highlighted window spans multiple monitors / virtual desktops**: Selection still activates the
  correct window and brings it to the foreground on its monitor/desktop.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST present open windows grouped by their owning application, with
  exactly one group entry per application that has at least one eligible window.
- **FR-002**: Each application group MUST display the application's logo/icon, a human-readable
  application name, and the count of windows it contains.
- **FR-003**: The system MUST order application groups in a consistent, predictable sequence and
  default the initial highlight to the application most likely to be wanted next (most-recently-used
  ordering).
- **FR-004**: Users MUST be able to move the highlight across groups using both the switcher
  shortcut (advancing forward) and directional keys, and MUST be able to operate the switcher with
  the mouse (hover/click) as well as the keyboard.
- **FR-005**: Users MUST be able to enter a highlighted application group to reveal a preview of
  every window belonging to that application, and MUST be able to return to the grouped overview
  from within a group.
- **FR-006**: Window previews within a group MUST be laid out in a uniform, aligned grid with
  consistent thumbnail sizing, spacing, and a stable ordering — previews MUST NOT overlap or be
  positioned arbitrarily.
- **FR-007**: When a group contains more window previews than fit in the visible area, the system
  MUST provide scrolling so that every window remains reachable, while preserving the grid
  alignment.
- **FR-008**: The switcher MUST remain open and interactive after the user releases the
  invocation keys; it MUST NOT auto-dismiss on key release or after a timeout.
- **FR-009**: Pressing the bound switcher shortcut again while the switcher is open MUST advance the
  highlight forward to the next group, wrapping to the first group after the last; it MUST NOT
  dismiss the switcher. The switcher MUST be dismissed only by confirming a selection (which
  activates the chosen item per FR-010) or by a dedicated cancel action (Escape). The cancel action
  MUST close the switcher without changing the active window.
- **FR-010**: Confirming a selection MUST activate the chosen window — bringing it to the front,
  focusing it, and switching to its virtual desktop/monitor if needed — and then close the
  switcher. Confirming a group (rather than an individual window) MUST activate that application's
  most-recently-used window.
- **FR-011**: The system MUST begin generating window thumbnails as soon as the switcher is
  activated, not deferring capture until a group is opened, so previews are ready (or nearly ready)
  by the time the user drills in.
- **FR-012**: While a thumbnail is still being produced, the system MUST show a non-blocking
  placeholder and replace it with the real preview when ready, without freezing the interface.
- **FR-013**: Window previews MUST reasonably represent the current contents of each window at the
  time the switcher was invoked.
- **FR-014**: The switcher's appearance MUST follow the system light/dark theme by default, and
  MUST reflect the system's active mode when shown.
- **FR-015**: The switcher MUST present a clean, modern overlay in which logos, application names,
  window titles/labels, and thumbnails are legible with adequate contrast in both light and dark
  modes.
- **FR-016**: The invocation shortcut MUST be configurable / re-bindable, and all described
  behaviors MUST apply to whatever shortcut is bound (the design MUST NOT assume a literal Alt+Tab).
- **FR-017**: Windows that cannot be associated with a recognizable application MUST be grouped
  under a clearly labeled fallback group with a placeholder logo rather than being hidden.
- **FR-018**: The set of windows shown MUST update to reflect windows that open or close while the
  switcher is open, so counts and selections stay accurate.
- **FR-019**: Invoking the switcher when no eligible windows exist MUST present a clear empty state
  and dismiss cleanly without activating anything.
- **FR-020**: The switcher MUST show all eligible windows across every virtual desktop, activity,
  and monitor. When the user confirms a window that lives on a different virtual desktop or
  activity, activation MUST switch to that desktop/activity as part of bringing the window to the
  foreground.

### Key Entities *(include if feature involves data)*

- **Application Group**: A collection of all eligible windows owned by a single application.
  Attributes: application identity, display name, logo/icon, ordered list of member windows,
  window count, and most-recently-used member.
- **Window Entry**: A single open window. Attributes: title, owning application, a preview
  thumbnail (with a ready/not-ready state), last-used recency, and location (monitor / virtual
  desktop).
- **Switcher Session**: The transient state of an open switcher instance. Attributes: current view
  level (grouped overview vs. inside a group), current highlight/selection, scroll position, and
  the window that was active before the switcher opened (for cancel/restore).
- **Theme State**: The active appearance mode (light/dark) derived from the system, used to style
  the overlay.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: When invoked, the switcher overlay becomes visible within 100 milliseconds for at
  least 95% of invocations.
- **SC-002**: Cycling between groups and scrolling through window previews appears smooth, with no
  user-perceptible stutter, while displaying at least 50 windows across at least 10 application
  groups.
- **SC-003**: For at least 90% of invocations, every window preview is fully rendered (no
  placeholder remaining) by the moment the user enters its group.
- **SC-004**: In a usability test, users locate and switch to a specific target window among 30+
  open windows in under 5 seconds on their first attempt at least 85% of the time.
- **SC-005**: Window previews are presented in a grid where every thumbnail shares the same size
  and alignment, with zero overlapping or arbitrarily positioned previews across all tested window
  counts.
- **SC-006**: The switcher remains open and responsive after key release for an indefinite idle
  period and closes only via the defined dismissal/cancel actions in 100% of trials.
- **SC-007**: The switcher's light/dark appearance matches the system setting in 100% of trials
  across both modes, with all text and icons meeting standard legibility/contrast expectations.
- **SC-008**: Selecting a group or a window activates the intended window (correct window focused
  and frontmost) in at least 99% of selections, including windows on other monitors or virtual
  desktops.

## Assumptions

- **Application identity**: "Application type" means the owning application of a window (e.g. all
  windows of one browser, all windows of one editor), determined from the window's application
  association; it does not mean categorizing applications into types like "browsers" vs "editors".
- **Window eligibility**: Normal, user-facing top-level windows are shown; utility, dock/panel, and
  the switcher's own overlay are excluded. Per FR-020, windows from all virtual desktops,
  activities, and monitors are included.
- **Ordering**: Groups and windows default to most-recently-used ordering so the prior window/app
  is fastest to reach; users are not required to configure ordering for v1.
- **Logos**: Application logos come from each application's declared icon; a neutral placeholder is
  used when none is available.
- **Thumbnails reflect invocation time**: Previews capture window contents around the moment of
  invocation and are not required to update live frame-by-frame while the switcher is open.
- **Single instance**: Only one switcher overlay exists at a time; re-invocation manipulates the
  existing session rather than stacking overlays.
- **Configuration scope**: v1 includes a configurable invocation shortcut and system-following
  theme; broader visual customization (custom colors, layout density options) is out of scope for
  v1.
- **Platform**: The switcher targets the KDE Plasma desktop on both Wayland and X11 sessions, per
  the project constitution; behaviors that cannot be supported on a given session type will be
  documented during planning.
- **Performance targets**: Latency and smoothness targets in Success Criteria align with the
  project constitution's performance principles (sub-100 ms invocation, 60 fps interaction).
