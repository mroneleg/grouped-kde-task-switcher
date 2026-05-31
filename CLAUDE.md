<!-- SPECKIT START -->
For additional context about technologies to be used, project structure,
shell commands, and other important information, read the current plan:
[specs/001-grouped-window-switcher/plan.md](specs/001-grouped-window-switcher/plan.md)

Active feature: **Grouped Window Switcher** (`001-grouped-window-switcher`).
Stack: C++20 + Qt 6 + KDE Frameworks 6, built as a KWin 6 `QuickSceneEffect`
plugin (CMake + extra-cmake-modules), QML overlay (Kirigami.Theme,
`org.kde.kwin` WindowThumbnail), targeting Plasma 6 on Wayland and X11.
Behavioral logic lives in a compositor-free core (`src/core`, `src/model`)
behind a `WindowSource` seam so it is unit-testable (QTest, offscreen QPA);
KWin coupling is confined to `src/effect`. See also research.md, data-model.md,
and contracts/ in the feature directory.
<!-- SPECKIT END -->
