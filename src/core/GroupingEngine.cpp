#include "core/GroupingEngine.h"

#include "core/ApplicationResolver.h" // FallbackKey

#include <QHash>

#include <algorithm>

namespace gks {

QList<ApplicationGroup> GroupingEngine::buildGroups(const QList<WindowDescriptor> &windows) const
{
    const QString fallbackKey = QString::fromLatin1(ApplicationResolver::FallbackKey);

    QHash<QString, ApplicationGroup> byKey;
    QList<QString> firstSeen; // preserve discovery order before sorting

    for (const WindowDescriptor &w : windows) {
        auto it = byKey.find(w.appKey);
        if (it == byKey.end()) {
            ApplicationGroup group;
            group.appKey = w.appKey;
            group.displayName = w.appDisplayName;
            group.iconName = w.appIconName;
            group.isFallback = (w.appKey == fallbackKey);
            group.windows.append(w);
            byKey.insert(w.appKey, group);
            firstSeen.append(w.appKey);
        } else {
            it->windows.append(w);
        }
    }

    QList<ApplicationGroup> groups;
    groups.reserve(byKey.size());
    for (const QString &key : firstSeen) {
        groups.append(byKey.value(key));
    }

    // MRU order within each group.
    for (ApplicationGroup &group : groups) {
        std::stable_sort(group.windows.begin(), group.windows.end(),
                         [](const WindowDescriptor &a, const WindowDescriptor &b) {
                             return a.lastUsedAt > b.lastUsedAt;
                         });
    }

    // Order groups by their most-recent window (desc); fallback group always last.
    std::stable_sort(groups.begin(), groups.end(),
                     [](const ApplicationGroup &a, const ApplicationGroup &b) {
                         if (a.isFallback != b.isFallback) {
                             return !a.isFallback; // non-fallback sorts before fallback
                         }
                         const qint64 ra = a.windows.isEmpty() ? 0 : a.windows.first().lastUsedAt;
                         const qint64 rb = b.windows.isEmpty() ? 0 : b.windows.first().lastUsedAt;
                         return ra > rb;
                     });

    return groups;
}

} // namespace gks
