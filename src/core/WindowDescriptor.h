#pragma once

#include <QList>
#include <QString>
#include <QUuid>

namespace gks {

/**
 * Immutable snapshot of one eligible window at the moment the switcher opened.
 *
 * Carries no KWin/compositor types so the core (grouping, ordering, selection)
 * is unit-testable with fakes and no running compositor (Constitution III).
 */
struct WindowDescriptor
{
    QUuid id;                ///< KWin internalId(); WindowThumbnailItem.wId at runtime
    QString appKey;          ///< grouping key: lowercased WM_CLASS class, or the fallback key
    QString appDisplayName;  ///< human-readable application name
    QString appIconName;     ///< icon name/path; placeholder when unknown
    QString caption;         ///< window title
    QList<int> desktops;     ///< virtual desktops the window is on (empty => all)
    QString activity;        ///< activity id (empty => all)
    QString screenName;      ///< monitor the window is on
    bool minimized = false;
    qint64 lastUsedAt = 0;   ///< recency rank; higher = more recently used

    bool operator==(const WindowDescriptor &other) const = default;
};

} // namespace gks
