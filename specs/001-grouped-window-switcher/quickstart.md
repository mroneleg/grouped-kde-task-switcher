# Quickstart: Grouped Window Switcher

How to build, test, install, enable, and run the switcher during development. Targets **KDE Plasma
5.27 LTS / KWin 5.27** (Qt 5 / KF5) on Wayland or X11 (developed on Ubuntu 24.04).

> The KWin effects ABI is not stable across releases — build against, and run on, the **same** KWin
> version. Record your KWin version: `kwin_wayland --version` (or `kwin_x11 --version`).

## 1. Install build dependencies

On Ubuntu 24.04 (Plasma 5.27 LTS):

```bash
sudo apt update && sudo apt install -y \
  cmake extra-cmake-modules g++ \
  qtbase5-dev qtdeclarative5-dev \
  kwin-dev \
  libkf5config-dev libkf5coreaddons-dev libkf5i18n-dev \
  libkf5windowsystem-dev libkf5service-dev libkf5globalaccel-dev \
  libkf5configwidgets-dev kirigami2-dev \
  qml-module-org-kde-kwin qml-module-org-kde-kirigami2 \
  qml-module-org-kde-plasma-core qml-module-org-kde-plasma-components
```

(The QML runtime modules `org.kde.kwin`, `org.kde.plasma.*`, and Kirigami ship with the Plasma 5
desktop and are usually already present.)

## 2. Build

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

## 3. Run tests (TDD — write these first)

C++ logic tests are headless and compositor-free:

```bash
# Core/model/state-machine unit tests
QT_QPA_PLATFORM=offscreen LANG=C ctest --test-dir build --output-on-failure

# QML view-logic tests
QT_QPA_PLATFORM=offscreen qmltestrunner -input tests/qml
```

Anything that needs real OpenGL (live thumbnail rendering) is **not** covered headlessly — verify it
manually in step 6, or run that subset under `xvfb-run -a` with a software-GL stack.

## 4. Install the effect

```bash
cmake --install build            # installs to ${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins/
# (use a --prefix matching your system prefix, typically /usr)
```

## 5. Enable it and bind the shortcut

- Enable: **System Settings → Window Management → Desktop Effects** → enable "Grouped Window
  Switcher" (or `kwriteconfig5 --file kwinrc --group Plugins --key groupedswitcherEnabled true`).
- Reload KWin so it picks up the new plugin:
  - Wayland: log out/in (or restart the session) — KWin Wayland cannot fully hot-reload effects.
  - X11: `kwin_x11 --replace &`.
- Bind the shortcut: **System Settings → Shortcuts** → find the Grouped Window Switcher action and
  assign a key (e.g. Meta+Tab). The effect registers its action via `KGlobalAccel` (a `QAction`).

## 6. Try it (manual verification)

Open several windows across ≥3 apps (e.g. two browser windows, two editor windows, a terminal):

1. Press the shortcut → the overlay appears (target <100 ms) showing one entry per app with its logo
   and window count.
2. Release all keys → the overlay **stays open** (FR-008).
3. Press the shortcut again → highlight advances to the next group, wrapping (FR-009).
4. Enter a group → a uniform, scrollable grid of live thumbnails (FR-005/006/007).
5. Confirm (Enter/click) → that window is raised and focused, switching desktop/activity if needed;
   overlay closes (FR-010). Escape instead → closes with no switch (FR-009).
6. Switch the system color scheme light↔dark → re-open; the overlay matches (FR-014).

## 7. Troubleshooting

- **Effect not listed**: confirm the install path is under the running KWin's plugin dir; check
  `journalctl --user -u plasma-kwin_wayland` (or `_x11`) for load errors; verify `metadata.json`.
- **No thumbnails / icons only**: compositing unavailable, or windows are minimized (minimized
  windows fall back to the app icon by design — research §4).
- **Built against a different KWin version**: rebuild against the running KWin; symbol/ABI mismatches
  manifest as load failures.
