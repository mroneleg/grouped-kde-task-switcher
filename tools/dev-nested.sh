#!/usr/bin/env bash
#
# Nested-compositor dev harness for the Grouped Window Switcher effect.
#
# Launches a throwaway kwin_wayland in a window on your current session, with our
# effect enabled, an isolated config, and its own D-Bus + KGlobalAccel daemon.
# A crash here kills ONLY the nested instance — your real desktop is untouched.
# All nested-KWin output (incl. QML errors / crashes) goes to the log file.
#
# Usage:  tools/dev-nested.sh
# Stop:   Ctrl+C in this terminal (or close the nested window).
#
# Iterate: edit -> cmake --build build -> sudo cmake --install build -> re-run this.
set -u

LOG=/tmp/gks-nested.log
CFG=/tmp/gks-nested-config
: > "$LOG"
rm -rf "$CFG"; mkdir -p "$CFG"

need() { command -v "$1" >/dev/null 2>&1; }
PATH="/usr/bin:$PATH"
for bin in dbus-run-session kwin_wayland kwriteconfig5 qdbus; do
    need "$bin" || { echo "ERROR: '$bin' not found in PATH." >&2; exit 1; }
done

pick() { for c in "$@"; do command -v "$c" >/dev/null 2>&1 && { echo "$c"; return; }; done; }
TERM_APP=$(pick konsole xterm alacritty xfce4-terminal)
EDIT_APP=$(pick kate kwrite gedit mousepad)
MISC_APP=$(pick dolphin kcalc systemsettings5 kfind)

if [ -z "$TERM_APP" ] && [ -z "$EDIT_APP" ] && [ -z "$MISC_APP" ]; then
    echo "ERROR: no GUI apps found to populate the nested session." >&2; exit 1
fi

# Two windows of the terminal (a group of 2, to exercise drill-in) plus one each
# of the others (single-window groups) — so grouping + the thumbnail grid both
# have something to show.
APPSCMD=""
[ -n "$TERM_APP" ] && APPSCMD+="$TERM_APP & sleep 1; $TERM_APP & sleep 1; "
[ -n "$EDIT_APP" ] && APPSCMD+="$EDIT_APP & sleep 1; "
[ -n "$MISC_APP" ] && APPSCMD+="$MISC_APP & sleep 1; "
APPSCMD+="wait"

echo "Nested KWin log: $LOG"
echo "Apps inside nested session: ${TERM_APP:-} x2, ${EDIT_APP:-}, ${MISC_APP:-}"
echo "Starting… (a 1600x900 window will open; give it a few seconds)"

dbus-run-session -- bash -s "$CFG" "$LOG" "$APPSCMD" <<'INNER'
set -u
CFG="$1"; LOG="$2"; APPSCMD="$3"
export XDG_CONFIG_HOME="$CFG"

# Enable our effect in the isolated config + give it a shortcut for in-window use.
kwriteconfig5 --file kwinrc --group Plugins --key groupedswitcherEnabled true >>"$LOG" 2>&1
kwriteconfig5 --file kglobalshortcutsrc --group kwin --key GroupedWindowSwitcher \
    "Meta+Tab,Meta+Tab,Toggle Grouped Window Switcher" >>"$LOG" 2>&1

# KGlobalAccel daemon so the effect's QAction shortcut registers and is invokable.
if   command -v kglobalacceld6 >/dev/null; then kglobalacceld6 >>"$LOG" 2>&1 &
elif command -v kglobalaccel5  >/dev/null; then kglobalaccel5  >>"$LOG" 2>&1 &
elif command -v kglobalacceld  >/dev/null; then kglobalacceld  >>"$LOG" 2>&1 & fi
sleep 1

echo "== launching nested kwin_wayland ==" >>"$LOG"
kwin_wayland --xwayland --width 1600 --height 900 -- bash -c "$APPSCMD" >>"$LOG" 2>&1 &
KW=$!

sleep 6
echo "== auto-invoking GroupedWindowSwitcher ==" >>"$LOG"
qdbus org.kde.kglobalaccel /component/kwin invokeShortcut GroupedWindowSwitcher >>"$LOG" 2>&1 \
    || echo "(invokeShortcut failed; press Meta+Tab inside the nested window instead)" >>"$LOG"

echo "Nested KWin running (pid $KW). Click into the window; Tab/arrows navigate,"
echo "Enter activates, Escape closes. Ctrl+C here to stop."
wait "$KW"
INNER

echo "Nested session ended. Full log: $LOG"
