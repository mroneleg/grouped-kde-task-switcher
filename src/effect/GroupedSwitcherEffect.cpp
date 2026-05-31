#include "effect/GroupedSwitcherEffect.h"

#include "core/SessionController.h"
#include "effect/EffectWindowSource.h"
#include "model/SwitcherModel.h"
#include "model/WindowEntryModel.h"

#include <kwineffects.h>

#include <KGlobalAccel>
#include <KLocalizedString>

#include <QAction>
#include <QStandardPaths>
#include <QUrl>

namespace gks {

GroupedSwitcherEffect::GroupedSwitcherEffect()
    : m_source(new EffectWindowSource(this))
    , m_controller(new SessionController(m_source, this))
    , m_groupModel(new SwitcherModel(m_controller, this))
    , m_entryModel(new WindowEntryModel(m_controller, this))
    , m_toggleAction(new QAction(this))
{
    connect(m_controller, &SessionController::closed, this,
            &GroupedSwitcherEffect::onControllerClosed);

    // Global toggle shortcut (KGlobalAccel — KWin 5.27 has no
    // EffectsHandler::registerGlobalShortcut). Default Meta+Tab.
    m_toggleAction->setObjectName(QStringLiteral("GroupedWindowSwitcher"));
    m_toggleAction->setText(i18n("Toggle Grouped Window Switcher"));
    const QKeySequence def(Qt::META | Qt::Key_Tab);
    KGlobalAccel::self()->setDefaultShortcut(m_toggleAction, {def});
    KGlobalAccel::self()->setShortcut(m_toggleAction, {def});
    connect(m_toggleAction, &QAction::triggered, this, &GroupedSwitcherEffect::toggle);

    const QString qml = QStandardPaths::locate(
        QStandardPaths::GenericDataLocation,
        QStringLiteral("kwin/effects/groupedswitcher/qml/main.qml"));
    if (!qml.isEmpty()) {
        setSource(QUrl::fromLocalFile(qml));
    }
}

GroupedSwitcherEffect::~GroupedSwitcherEffect() = default;

QVariantMap GroupedSwitcherEffect::initialProperties(KWin::EffectScreen *screen)
{
    Q_UNUSED(screen)
    return {
        {QStringLiteral("effect"), QVariant::fromValue<QObject *>(this)},
        {QStringLiteral("controller"), QVariant::fromValue<QObject *>(m_controller)},
        {QStringLiteral("groupModel"), QVariant::fromValue<QObject *>(m_groupModel)},
        {QStringLiteral("entryModel"), QVariant::fromValue<QObject *>(m_entryModel)},
    };
}

void GroupedSwitcherEffect::toggle()
{
    if (m_active) {
        // Re-invoking while open advances the highlight (single-instance, FR-009).
        m_controller->open();
        return;
    }
    activateOverlay();
}

void GroupedSwitcherEffect::activateOverlay()
{
    m_source->capturePreviousActive();
    m_controller->open();
    if (m_controller->level() == SessionController::Level::Closed) {
        return; // no eligible windows (FR-019)
    }
    m_active = true;
    setRunning(true);
}

void GroupedSwitcherEffect::onControllerClosed()
{
    m_active = false;
    // closed() is emitted from within a QML signal handler (a key press / click).
    // Tearing down the scene synchronously would delete the QQuickItem whose
    // handler is still running and crash KWin, so defer it to the next event loop tick.
    QMetaObject::invokeMethod(
        this, [this] { setRunning(false); }, Qt::QueuedConnection);
}

} // namespace gks
