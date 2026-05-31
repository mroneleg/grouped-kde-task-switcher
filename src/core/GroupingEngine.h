#pragma once

#include "core/ApplicationGroup.h"
#include "core/WindowDescriptor.h"

#include <QList>

namespace gks {

/**
 * Builds application groups from window descriptors.
 *
 * No desktop/activity/monitor filtering — every eligible window is included
 * (FR-020 scope). Windows within a group are MRU-ordered; groups are ordered by
 * their most-recent window (MRU desc), with the fallback group always last
 * (FR-001/002/003/017).
 */
class GroupingEngine
{
public:
    QList<ApplicationGroup> buildGroups(const QList<WindowDescriptor> &windows) const;
};

} // namespace gks
