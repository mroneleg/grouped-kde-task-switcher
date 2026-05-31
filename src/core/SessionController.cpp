#include "core/SessionController.h"

#include "core/WindowSource.h"

namespace gks {

SessionController::SessionController(WindowSource *source, QObject *parent)
    : QObject(parent)
    , m_source(source)
{
    if (m_source) {
        connect(m_source, &WindowSource::windowAdded, this, &SessionController::onWindowAdded);
        connect(m_source, &WindowSource::windowRemoved, this, &SessionController::onWindowRemoved);
        connect(m_source, &WindowSource::windowChanged, this, &SessionController::onWindowChanged);
    }
}

void SessionController::open()
{
    // Single-instance guard: re-invoking while open advances the highlight
    // rather than starting a second session (FR-008/009, T051).
    if (m_level != Level::Closed) {
        advanceHighlight();
        return;
    }

    m_previousActive = m_source ? m_source->previouslyActive() : QUuid();
    m_groups = m_source ? m_engine.buildGroups(m_source->snapshot()) : QList<ApplicationGroup>{};
    m_openGroup = -1;
    m_highlightedWindow = 0;

    if (m_groups.isEmpty()) {
        // Empty state, then dismiss cleanly (FR-019).
        m_level = Level::Closed;
        Q_EMIT stateChanged();
        Q_EMIT closed();
        return;
    }

    m_level = Level::Overview;
    // Default highlight = previous application (the most-recently-used window
    // belongs to the current app at index 0), so Alt+Tab lands on index 1.
    m_highlightedGroup = (m_groups.size() > 1) ? 1 : 0;
    Q_EMIT stateChanged();
}

void SessionController::advanceHighlight()
{
    if (m_level == Level::Overview) {
        if (m_groups.isEmpty()) {
            return;
        }
        m_highlightedGroup = (m_highlightedGroup + 1) % m_groups.size();
        Q_EMIT stateChanged();
    } else if (m_level == Level::InGroup) {
        moveWindowHighlight(1);
    }
}

void SessionController::retreatHighlight()
{
    if (m_level == Level::Overview) {
        if (m_groups.isEmpty()) {
            return;
        }
        const int n = m_groups.size();
        m_highlightedGroup = (m_highlightedGroup - 1 + n) % n;
        Q_EMIT stateChanged();
    } else if (m_level == Level::InGroup) {
        moveWindowHighlight(-1);
    }
}

void SessionController::enterHighlightedGroup()
{
    if (m_level != Level::Overview) {
        return;
    }
    if (m_highlightedGroup < 0 || m_highlightedGroup >= m_groups.size()) {
        return;
    }
    if (m_groups[m_highlightedGroup].windows.isEmpty()) {
        return;
    }
    m_openGroup = m_highlightedGroup;
    m_highlightedWindow = 0;
    m_level = Level::InGroup;
    Q_EMIT stateChanged();
}

void SessionController::back()
{
    if (m_level != Level::InGroup) {
        return;
    }
    m_highlightedGroup = m_openGroup;
    m_openGroup = -1;
    m_highlightedWindow = 0;
    m_level = Level::Overview;
    Q_EMIT stateChanged();
}

void SessionController::moveWindowHighlight(int delta)
{
    if (m_level != Level::InGroup || m_openGroup < 0 || m_openGroup >= m_groups.size()) {
        return;
    }
    const int n = m_groups[m_openGroup].windows.size();
    if (n == 0) {
        return;
    }
    m_highlightedWindow = ((m_highlightedWindow + delta) % n + n) % n;
    Q_EMIT stateChanged();
}

void SessionController::confirm()
{
    if (m_level == Level::Overview) {
        if (m_source && m_highlightedGroup >= 0 && m_highlightedGroup < m_groups.size()) {
            const ApplicationGroup &g = m_groups[m_highlightedGroup];
            if (!g.windows.isEmpty()) {
                m_source->activate(g.windows.first().id);
            }
        }
        closeWith(Level::Closed);
    } else if (m_level == Level::InGroup) {
        if (m_source && m_openGroup >= 0 && m_openGroup < m_groups.size()) {
            const QList<WindowDescriptor> &ws = m_groups[m_openGroup].windows;
            if (m_highlightedWindow >= 0 && m_highlightedWindow < ws.size()) {
                m_source->activate(ws[m_highlightedWindow].id);
            }
        }
        closeWith(Level::Closed);
    }
}

void SessionController::cancel()
{
    if (m_level == Level::Closed) {
        return;
    }
    if (m_source) {
        m_source->restoreActive(m_previousActive);
    }
    closeWith(Level::Closed);
}

void SessionController::onWindowAdded(const WindowDescriptor &)
{
    handleLiveUpdate();
}

void SessionController::onWindowRemoved(const QUuid &)
{
    handleLiveUpdate();
}

void SessionController::onWindowChanged(const WindowDescriptor &)
{
    handleLiveUpdate();
}

void SessionController::handleLiveUpdate()
{
    if (m_level == Level::Closed) {
        return;
    }
    rebuildPreservingSelection();
    if (m_groups.isEmpty()) {
        closeWith(Level::Closed); // removing the last window closes (FR-019)
        return;
    }
    Q_EMIT stateChanged();
}

void SessionController::rebuildPreservingSelection()
{
    const QString hlKey = (m_highlightedGroup >= 0 && m_highlightedGroup < m_groups.size())
        ? m_groups[m_highlightedGroup].appKey
        : QString();
    const QString openKey = (m_openGroup >= 0 && m_openGroup < m_groups.size())
        ? m_groups[m_openGroup].appKey
        : QString();
    QUuid hlWindowId;
    if (m_level == Level::InGroup && m_openGroup >= 0 && m_openGroup < m_groups.size()) {
        const QList<WindowDescriptor> &ws = m_groups[m_openGroup].windows;
        if (m_highlightedWindow >= 0 && m_highlightedWindow < ws.size()) {
            hlWindowId = ws[m_highlightedWindow].id;
        }
    }

    m_groups = m_source ? m_engine.buildGroups(m_source->snapshot()) : QList<ApplicationGroup>{};

    if (!hlKey.isEmpty()) {
        for (int i = 0; i < m_groups.size(); ++i) {
            if (m_groups[i].appKey == hlKey) {
                m_highlightedGroup = i;
                break;
            }
        }
    }
    if (m_level == Level::InGroup) {
        int idx = -1;
        if (!openKey.isEmpty()) {
            for (int i = 0; i < m_groups.size(); ++i) {
                if (m_groups[i].appKey == openKey) {
                    idx = i;
                    break;
                }
            }
        }
        m_openGroup = idx;
        if (idx >= 0 && !hlWindowId.isNull()) {
            const QList<WindowDescriptor> &ws = m_groups[idx].windows;
            for (int j = 0; j < ws.size(); ++j) {
                if (ws[j].id == hlWindowId) {
                    m_highlightedWindow = j;
                    break;
                }
            }
        }
    }

    clampIndices();
}

void SessionController::clampIndices()
{
    if (m_groups.isEmpty()) {
        m_highlightedGroup = 0;
        m_openGroup = -1;
        m_highlightedWindow = 0;
        return;
    }
    if (m_highlightedGroup < 0) {
        m_highlightedGroup = 0;
    }
    if (m_highlightedGroup >= m_groups.size()) {
        m_highlightedGroup = m_groups.size() - 1;
    }
    if (m_level == Level::InGroup) {
        if (m_openGroup < 0 || m_openGroup >= m_groups.size()
            || m_groups[m_openGroup].windows.isEmpty()) {
            // The open group disappeared — fall back to the overview.
            m_level = Level::Overview;
            m_openGroup = -1;
            m_highlightedWindow = 0;
        } else {
            const int n = m_groups[m_openGroup].windows.size();
            if (m_highlightedWindow < 0) {
                m_highlightedWindow = 0;
            }
            if (m_highlightedWindow >= n) {
                m_highlightedWindow = n - 1;
            }
        }
    }
}

void SessionController::closeWith(Level next)
{
    m_level = next;
    m_openGroup = -1;
    m_highlightedWindow = 0;
    Q_EMIT stateChanged();
    Q_EMIT closed();
}

} // namespace gks
