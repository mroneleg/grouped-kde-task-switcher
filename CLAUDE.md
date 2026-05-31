<!-- SPECKIT START -->
For additional context about technologies to be used, project structure,
shell commands, and other important information, read the current plan:
[specs/001-grouped-window-switcher/plan.md](specs/001-grouped-window-switcher/plan.md)

Active feature: **Grouped Window Switcher** (`001-grouped-window-switcher`).
Stack: C++20 + Qt 5 + KDE Frameworks 5, built as a **KWin 5.27** `QuickSceneEffect`
plugin (CMake + extra-cmake-modules; `find_package(KWin)` → `kwineffects`),
QML overlay (`PlasmaCore.ColorScope` theming; `org.kde.kwin 3.0`
`WindowThumbnailItem { wId }`), targeting **Plasma 5.27 LTS** (Ubuntu 24.04) on
Wayland and X11. Shortcut via `KGlobalAccel` + `QAction`; group by
`EffectWindow::windowClass()`. Behavioral logic lives in a compositor-free core
(`src/core`, `src/model`) behind a `WindowSource` seam so it is unit-testable
(QTest, offscreen QPA); KWin coupling is confined to `src/effect`. See
research.md §"Retarget to Plasma 5.27", data-model.md, and contracts/.
<!-- SPECKIT END -->
