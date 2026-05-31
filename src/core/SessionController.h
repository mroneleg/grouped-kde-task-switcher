#pragma once

#include "core/ApplicationGroup.h"
#include "core/GroupingEngine.h"
#include "core/WindowDescriptor.h"

#include <QList>
#include <QObject>
#include <QUuid>

namespace gks {

class WindowSource;

/**
 * The interaction state machine (contracts/interaction-state-machine.md):
 * Closed → Overview → InGroup, selection/cycling, confirm/cancel, single-instance
 * re-invocation, and live updates. Compositor-free and fully unit-testable.
 */
class SessionController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int level READ levelInt NOTIFY stateChanged)
    Q_PROPERTY(int highlightedGroupIndex READ highlightedGroupIndex NOTIFY stateChanged)
    Q_PROPERTY(int openGroupIndex READ openGroupIndex NOTIFY stateChanged)
    Q_PROPERTY(int highlightedWindowIndex READ highlightedWindowIndex NOTIFY stateChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY stateChanged)

public:
    enum class Level { Closed, Overview, InGroup };
    Q_ENUM(Level)

    explicit SessionController(WindowSource *source, QObject *parent = nullptr);

    Level level() const { return m_level; }
    int levelInt() const { return int(m_level); }
    const QList<ApplicationGroup> &groups() const { return m_groups; }
    int highlightedGroupIndex() const { return m_highlightedGroup; }
    int openGroupIndex() const { return m_openGroup; }
    int highlightedWindowIndex() const { return m_highlightedWindow; }
    bool isEmpty() const { return m_groups.isEmpty(); }

public Q_SLOTS:
    /// Open the switcher; re-invoking while open advances the highlight
    /// (single-instance guard, FR-008/009, T051).
    void open();
    void advanceHighlight(); ///< forward; wraps (FR-009)
    void retreatHighlight(); ///< backward; wraps
    void enterHighlightedGroup(); ///< Overview → InGroup (FR-005)
    void back();                  ///< InGroup → Overview, keep origin highlighted (FR-005)
    void moveWindowHighlight(int delta); ///< within the grid (FR-006)
    void confirm(); ///< activate selection, close (FR-010)
    void cancel();  ///< Escape → restore previous active, close (FR-009)

Q_SIGNALS:
    void stateChanged();
    void closed();

private Q_SLOTS:
    void onWindowAdded(const gks::WindowDescriptor &window);
    void onWindowRemoved(const QUuid &id);
    void onWindowChanged(const gks::WindowDescriptor &window);

private:
    void rebuildPreservingSelection();
    void clampIndices();
    void closeWith(Level next);
    void handleLiveUpdate();

    WindowSource *m_source;
    GroupingEngine m_engine;
    QList<ApplicationGroup> m_groups;
    Level m_level = Level::Closed;
    int m_highlightedGroup = 0;
    int m_openGroup = -1;
    int m_highlightedWindow = 0;
    QUuid m_previousActive;
};

} // namespace gks
