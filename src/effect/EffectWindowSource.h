#pragma once

#include "core/ApplicationResolver.h"
#include "core/WindowSource.h"

#include <QUuid>

#include <memory>

namespace KWin
{
class EffectWindow;
}

namespace gks {

/**
 * Live WindowSource backed by KWin's effects API (EffectWindow / the `effects`
 * global). The only compositor-coupled implementation; everything it produces is
 * a plain WindowDescriptor so the core stays testable (contracts/window-source.md).
 *
 * Enumerates stackingOrder() across all virtual desktops, activities, and monitors
 * (FR-020), grouping key from windowClass() (KWin 5.27 has no desktopFileName()).
 */
class EffectWindowSource : public WindowSource
{
    Q_OBJECT
public:
    explicit EffectWindowSource(QObject *parent = nullptr);
    ~EffectWindowSource() override;

    QList<WindowDescriptor> snapshot() const override;
    bool activate(const QUuid &id) override;
    void restoreActive(const QUuid &id) override;
    QUuid previouslyActive() const override;

    /// Record the active window so cancel() can restore it. Call on open.
    void capturePreviousActive();

private Q_SLOTS:
    void onWindowAdded(KWin::EffectWindow *w);
    void onWindowClosed(KWin::EffectWindow *w);
    void onWindowMinimizedChanged(KWin::EffectWindow *w);

private:
    bool isEligible(KWin::EffectWindow *w) const;
    WindowDescriptor describe(KWin::EffectWindow *w, qint64 recency) const;

    std::unique_ptr<AppInfoProvider> m_provider;
    ApplicationResolver m_resolver;
    QUuid m_previousActive;
};

} // namespace gks
