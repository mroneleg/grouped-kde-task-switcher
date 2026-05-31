#include "effect/EffectWindowSource.h"

#include <kwineffects.h>

#include <KService>

namespace gks {

namespace {

/// Resolves an application's display name from its WM_CLASS via KService.
class KServiceAppInfoProvider : public AppInfoProvider
{
public:
    QString displayNameForClass(const QString &wmClass) const override
    {
        if (wmClass.isEmpty()) {
            return {};
        }
        // The WM_CLASS class often matches a desktop-file base name.
        if (KService::Ptr s = KService::serviceByDesktopName(wmClass.toLower())) {
            return s->name();
        }
        if (KService::Ptr s = KService::serviceByStorageId(wmClass.toLower() + QStringLiteral(".desktop"))) {
            return s->name();
        }
        return {};
    }
};

} // namespace

EffectWindowSource::EffectWindowSource(QObject *parent)
    : WindowSource(parent)
    , m_provider(std::make_unique<KServiceAppInfoProvider>())
    , m_resolver(m_provider.get())
{
    connect(KWin::effects, &KWin::EffectsHandler::windowAdded, this,
            &EffectWindowSource::onWindowAdded);
    connect(KWin::effects, &KWin::EffectsHandler::windowClosed, this,
            &EffectWindowSource::onWindowClosed);
    connect(KWin::effects, &KWin::EffectsHandler::windowMinimized, this,
            &EffectWindowSource::onWindowMinimizedChanged);
    connect(KWin::effects, &KWin::EffectsHandler::windowUnminimized, this,
            &EffectWindowSource::onWindowMinimizedChanged);
}

EffectWindowSource::~EffectWindowSource() = default;

bool EffectWindowSource::isEligible(KWin::EffectWindow *w) const
{
    return w && w->isNormalWindow() && w->isManaged() && !w->isDeleted()
        && !w->isSpecialWindow();
}

WindowDescriptor EffectWindowSource::describe(KWin::EffectWindow *w, qint64 recency) const
{
    WindowDescriptor d;
    d.id = w->internalId();

    // EffectWindow::windowClass() is the WM_CLASS pair "instance class"; the
    // grouping key is the class (last token).
    const QString wmClass = w->windowClass().section(QLatin1Char(' '), -1);
    const AppIdentity app = m_resolver.resolve(wmClass, w->icon().name());
    d.appKey = app.key;
    d.appDisplayName = app.displayName;
    d.appIconName = app.iconName;

    d.caption = w->caption();
    const QVector<uint> desks = w->desktops();
    for (uint u : desks) {
        d.desktops.append(int(u));
    }
    d.activity = w->activities().value(0);
    d.screenName = w->screen() ? w->screen()->name() : QString();
    d.minimized = w->isMinimized();
    d.lastUsedAt = recency;
    return d;
}

QList<WindowDescriptor> EffectWindowSource::snapshot() const
{
    QList<WindowDescriptor> result;
    const KWin::EffectWindowList stack = KWin::effects->stackingOrder();
    // stackingOrder() is bottom→top; higher index ≈ more recently raised, so use
    // the index as the recency rank (the active window sorts most-recent).
    qint64 recency = 0;
    for (KWin::EffectWindow *w : stack) {
        if (isEligible(w)) {
            result.append(describe(w, recency));
        }
        ++recency;
    }
    return result;
}

bool EffectWindowSource::activate(const QUuid &id)
{
    KWin::EffectWindow *w = KWin::effects->findWindow(id);
    if (!w) {
        return false;
    }
    // Switch to the window's virtual desktop if it is not on the current one (FR-020).
    if (!w->isOnAllDesktops()) {
        const QVector<uint> desks = w->desktops();
        if (!desks.isEmpty() && !desks.contains(uint(KWin::effects->currentDesktop()))) {
            KWin::effects->setCurrentDesktop(int(desks.first()));
        }
    }
    KWin::effects->activateWindow(w);
    return true;
}

void EffectWindowSource::restoreActive(const QUuid &id)
{
    if (id.isNull()) {
        return;
    }
    if (KWin::EffectWindow *w = KWin::effects->findWindow(id)) {
        KWin::effects->activateWindow(w);
    }
}

QUuid EffectWindowSource::previouslyActive() const
{
    return m_previousActive;
}

void EffectWindowSource::capturePreviousActive()
{
    KWin::EffectWindow *w = KWin::effects->activeWindow();
    m_previousActive = w ? w->internalId() : QUuid();
}

void EffectWindowSource::onWindowAdded(KWin::EffectWindow *w)
{
    if (isEligible(w)) {
        Q_EMIT windowAdded(describe(w, 0));
    }
}

void EffectWindowSource::onWindowClosed(KWin::EffectWindow *w)
{
    if (w) {
        Q_EMIT windowRemoved(w->internalId());
    }
}

void EffectWindowSource::onWindowMinimizedChanged(KWin::EffectWindow *w)
{
    if (isEligible(w)) {
        Q_EMIT windowChanged(describe(w, 0));
    }
}

} // namespace gks
