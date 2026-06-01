#pragma once

#include <kwinquickeffect.h>

class QAction;

namespace gks {

class EffectWindowSource;
class SessionController;
class SwitcherModel;
class WindowEntryModel;

/**
 * The KWin 5.27 effect that hosts the grouped switcher overlay (a QuickSceneEffect,
 * like KWin's Overview). It owns the live window source, the state machine, and the
 * QML-facing models, registers the toggle shortcut via KGlobalAccel, and stays open
 * until the controller closes (confirm/cancel) — never on modifier release (FR-008).
 */
class GroupedSwitcherEffect : public KWin::QuickSceneEffect
{
    Q_OBJECT
public:
    GroupedSwitcherEffect();
    ~GroupedSwitcherEffect() override;

protected:
    QVariantMap initialProperties(KWin::EffectScreen *screen) override;

public Q_SLOTS:
    /// Open/advance the switcher. Exposed on D-Bus (org.kde.KWin
    /// /GroupedWindowSwitcher) for testing/automation, in addition to the
    /// KGlobalAccel shortcut.
    Q_SCRIPTABLE void toggle();

private Q_SLOTS:
    void onControllerClosed();

private:
    void activateOverlay();

    EffectWindowSource *m_source;
    SessionController *m_controller;
    SwitcherModel *m_groupModel;
    WindowEntryModel *m_entryModel;
    QAction *m_toggleAction;
    bool m_active = false;
};

} // namespace gks
