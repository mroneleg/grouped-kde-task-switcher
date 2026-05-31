#pragma once

#include "core/WindowSource.h"

#include <QList>
#include <QUuid>

namespace gks {

/**
 * Deterministic test double for WindowSource: scripted descriptor lists, on-demand
 * change-signal emission, and recorded activate()/restoreActive() calls
 * (contracts/window-source.md "Test double").
 */
class FakeWindowSource : public WindowSource
{
    Q_OBJECT
public:
    using WindowSource::WindowSource;

    QList<WindowDescriptor> snapshot() const override { return m_windows; }

    bool activate(const QUuid &id) override
    {
        m_activated.append(id);
        return containsId(id);
    }

    void restoreActive(const QUuid &id) override { m_restored.append(id); }

    QUuid previouslyActive() const override { return m_previous; }

    // --- test helpers ---
    void setWindows(const QList<WindowDescriptor> &windows) { m_windows = windows; }
    void setPreviouslyActive(const QUuid &id) { m_previous = id; }

    void addWindow(const WindowDescriptor &window)
    {
        m_windows.append(window);
        Q_EMIT windowAdded(window);
    }

    void removeWindow(const QUuid &id)
    {
        for (int i = 0; i < m_windows.size(); ++i) {
            if (m_windows[i].id == id) {
                m_windows.removeAt(i);
                break;
            }
        }
        Q_EMIT windowRemoved(id);
    }

    QList<QUuid> activated() const { return m_activated; }
    QList<QUuid> restored() const { return m_restored; }

private:
    bool containsId(const QUuid &id) const
    {
        for (const WindowDescriptor &w : m_windows) {
            if (w.id == id) {
                return true;
            }
        }
        return false;
    }

    QList<WindowDescriptor> m_windows;
    QUuid m_previous;
    QList<QUuid> m_activated;
    QList<QUuid> m_restored;
};

} // namespace gks
