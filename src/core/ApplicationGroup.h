#pragma once

#include "core/WindowDescriptor.h"

#include <QList>
#include <QString>

namespace gks {

/**
 * All eligible windows owned by a single application. Built by GroupingEngine
 * from descriptors sharing an appKey (data-model.md).
 */
struct ApplicationGroup
{
    QString appKey;
    QString displayName;
    QString iconName;
    QList<WindowDescriptor> windows; ///< MRU order (most recent first)
    bool isFallback = false;

    int count() const { return int(windows.size()); }

    /// Target when the group (rather than a window) is confirmed (FR-010).
    /// Precondition: !windows.isEmpty().
    const WindowDescriptor &mostRecentWindow() const { return windows.first(); }
};

} // namespace gks
