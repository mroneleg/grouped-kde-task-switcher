<!--
SYNC IMPACT REPORT
==================
Version change: (template / unversioned) → 1.0.0
Rationale: Initial ratification of the project constitution. MINOR/PATCH not
applicable; first concrete version is 1.0.0.

Modified principles: (none — initial definition)
Added principles:
  - I. KDE & Plasma Conventions
  - II. Performance & Responsiveness (Native-First)
  - III. Test-First (NON-NEGOTIABLE)
  - IV. Simplicity & UX Clarity
Added sections:
  - Technology & Architecture Constraints
  - Development Workflow & Quality Gates
Removed sections: (none)

Templates requiring updates:
  - .specify/templates/plan-template.md ✅ no change needed (Constitution Check
    gate references this file dynamically; principle-derived gates apply as-is)
  - .specify/templates/spec-template.md ✅ no change needed (no constitution refs)
  - .specify/templates/tasks-template.md ✅ no change needed (no constitution refs)
  - .specify/templates/checklist-template.md ✅ no change needed (no constitution refs)

Follow-up TODOs:
  - None. RATIFICATION_DATE set to first adoption date (2026-05-31).
-->

# Grouped KDE Task Switcher Constitution

## Core Principles

### I. KDE & Plasma Conventions

The project MUST integrate as a first-class citizen of the KDE Plasma desktop and
follow upstream norms rather than inventing parallel mechanisms.

- Window enumeration, activation, and stacking MUST go through supported KDE/Qt
  interfaces (KWindowSystem, KWin scripting/effects APIs, Wayland protocols, or
  D-Bus) — never through scraping or unsupported hacks.
- Both Wayland and X11 sessions MUST be supported; any feature that cannot work on
  Wayland MUST be explicitly documented as X11-only and degrade gracefully.
- Visual design MUST follow the KDE Human Interface Guidelines and respect the
  user's active Plasma theme, color scheme, fonts, icons, and accessibility
  settings — no hardcoded styling that ignores the system theme.
- Packaging, metadata, and installation MUST conform to KDE/Frameworks conventions
  so the switcher installs and is discoverable through standard Plasma mechanisms.

**Rationale**: A task switcher lives at the center of the desktop. Diverging from
KDE conventions produces a tool that looks alien, breaks on theme or session
changes, and is unmaintainable against upstream churn.

### II. Performance & Responsiveness (Native-First)

The switcher MUST feel instantaneous. Invocation latency and rendering jank are
correctness defects, not polish.

- The switcher overlay MUST become visible within 100 ms of the activating
  shortcut (p95), and key-repeat cycling MUST register within one frame.
- Animations and window-preview rendering MUST hold 60 fps (≤16 ms frame budget)
  on reference hardware; dropped frames during cycling MUST be treated as bugs.
- Native code (C++/Qt) is the DEFAULT for any path that enumerates windows,
  captures or scales previews, or runs per-frame. QML/JavaScript MAY be used for
  declarative layout and theming, but MUST NOT own hot paths. Any measured
  resource or latency problem in QML/JS MUST be moved to native code.
- Window-preview capture MUST be lazy and bounded: previews are produced only for
  visible entries, memory for thumbnails MUST be capped, and capture MUST never
  block the UI thread.
- Performance budgets above MUST be backed by a measurement before a feature is
  considered done; "feels fast" without numbers is not acceptable.

**Rationale**: A switcher is invoked dozens of times a day under user impatience.
Latency or stutter makes the whole desktop feel broken, so performance is a
non-negotiable acceptance criterion, not a later optimization.

### III. Test-First (NON-NEGOTIABLE)

Test-Driven Development is mandatory for all behavioral code.

- Tests MUST be written and MUST fail before the corresponding implementation is
  written. The Red-Green-Refactor cycle MUST be followed.
- Core logic — window grouping (e.g. grouping all Chrome windows, all VS Code
  windows by application), ordering, selection/cycling state, and preview
  lifecycle — MUST be unit-tested independently of the rendered UI.
- Grouping and selection logic MUST be structured so it is testable without a live
  compositor (pure functions / injectable window sources), enabling fast,
  deterministic tests.
- Bug fixes MUST start with a failing regression test that reproduces the defect.
- Performance budgets from Principle II SHOULD be guarded by automated benchmarks
  where feasible; where not, the manual measurement MUST be recorded.

**Rationale**: Grouping and cycling state is subtle and easy to regress. Tests
written first force testable design and lock in correct behavior across the wide
matrix of window/application combinations.

### IV. Simplicity & UX Clarity

The switcher MUST do one thing predictably: let users find and activate a window
fast, with windows grouped by application.

- YAGNI governs scope: features MUST be justified by a concrete user need.
  Speculative configurability MUST be rejected.
- Default behavior MUST be sensible with zero configuration. Every added setting
  MUST be justified against the cost of added complexity and surface area.
- Grouped switching behavior MUST be predictable: grouping rules, ordering, and
  what a keypress does MUST be consistent and explainable in one sentence.
- The implementation MUST prefer the simplest design that meets the principles
  above; added abstraction or indirection MUST be justified in the plan's
  Complexity Tracking section.
- Accessibility (keyboard-only operation, screen-reader labels where applicable,
  respect for reduced-motion settings) MUST be preserved, not bolted on.

**Rationale**: Task switchers accrete options until they are slow and confusing.
Constraining scope and keeping behavior predictable is what keeps the tool fast,
maintainable, and pleasant to use.

## Technology & Architecture Constraints

- **Platform**: KDE Plasma on Linux, supporting both Wayland and X11 sessions.
- **Native-first stack**: Performance-critical and system-integration code is
  C++/Qt against KDE Frameworks. QML is permitted for declarative UI/theming only,
  subject to Principle II.
- **Core features** (in scope by definition):
  - Application-based window grouping (all windows of an app presented as one
    group, e.g. all Chrome windows, all VS Code windows).
  - Live window previews/thumbnails for switcher entries, captured lazily and
    bounded in memory per Principle II.
- **Layering**: Grouping, ordering, and selection logic MUST be isolated from
  rendering and from the compositor integration so it can be unit-tested in
  isolation (Principle III).
- **No unsupported integration**: Direct manipulation of compositor internals or
  undocumented interfaces is prohibited (Principle I).

## Development Workflow & Quality Gates

- **Constitution Check gate**: Every implementation plan MUST pass the Constitution
  Check before Phase 0 research and re-verify it after Phase 1 design. Violations
  MUST be recorded and justified in the plan's Complexity Tracking table or the
  design MUST be revised.
- **Definition of done** for any behavioral change:
  1. Failing tests written first, then made to pass (Principle III).
  2. Relevant performance budgets measured and recorded (Principle II).
  3. Works on both Wayland and X11, or documented X11-only exception (Principle I).
  4. No new user-facing configuration without explicit justification (Principle IV).
- **Code review** MUST verify compliance with all four principles; a reviewer MUST
  block changes that regress latency/frame budgets or bypass KDE interfaces.

## Governance

This constitution supersedes other development practices for this project. Where a
practice conflicts with a principle here, the constitution wins.

- **Amendments** MUST be made by editing this file, accompanied by the Sync Impact
  Report, a version bump per the policy below, and propagation to any dependent
  templates and guidance docs.
- **Versioning policy** (semantic):
  - MAJOR: Backward-incompatible governance changes or removal/redefinition of a
    principle.
  - MINOR: A new principle or section is added, or guidance is materially expanded.
  - PATCH: Clarifications, wording, and non-semantic refinements.
- **Compliance review**: All PRs and reviews MUST verify adherence to the principles
  and the Definition of Done. Unjustified complexity MUST be rejected.
- **Runtime guidance**: Use `CLAUDE.md` and the active feature plan for day-to-day
  development guidance; both MUST remain consistent with this constitution.

**Version**: 1.0.0 | **Ratified**: 2026-05-31 | **Last Amended**: 2026-05-31
