#pragma once

#include "core/WindowDescriptor.h"

#include <QList>
#include <QObject>
#include <QUuid>

namespace gks {

/**
 * The single boundary between the compositor-free core and the live compositor
 * (contracts/window-source.md). The core depends only on this interface, so all
 * behavioral logic is testable with a fake. The live adapter (EffectWindowSource)
 * wraps KWin; tests use FakeWindowSource.
 */
class WindowSource : public QObject
{
    Q_OBJECT
public:
    explicit WindowSource(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~WindowSource() override = default;

    /// Eligible windows across all virtual desktops, activities, and monitors
    /// (FR-020), already filtered to user-facing top-level windows.
    virtual QList<WindowDescriptor> snapshot() const = 0;

    /// Activate a window (raise, focus, switch desktop/activity). Returns false
    /// if the window no longer exists (FR-010/018/020).
    virtual bool activate(const QUuid &id) = 0;

    /// Restore focus to a previously-active window on cancel (FR-009).
    virtual void restoreActive(const QUuid &id) = 0;

    /// The window active immediately before the switcher opened.
    virtual QUuid previouslyActive() const = 0;

Q_SIGNALS:
    void windowAdded(const gks::WindowDescriptor &window);
    void windowRemoved(const QUuid &id);
    void windowChanged(const gks::WindowDescriptor &window);
};

} // namespace gks
