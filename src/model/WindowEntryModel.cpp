#include "model/WindowEntryModel.h"

#include "core/SessionController.h"

namespace gks {

WindowEntryModel::WindowEntryModel(SessionController *controller, QObject *parent)
    : QAbstractListModel(parent)
    , m_controller(controller)
{
    if (m_controller) {
        connect(m_controller, &SessionController::stateChanged, this, &WindowEntryModel::refresh);
    }
}

static const ApplicationGroup *openGroup(const SessionController *c)
{
    if (!c || c->level() != SessionController::Level::InGroup) {
        return nullptr;
    }
    const int idx = c->openGroupIndex();
    if (idx < 0 || idx >= c->groups().size()) {
        return nullptr;
    }
    return &c->groups()[idx];
}

int WindowEntryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    const ApplicationGroup *g = openGroup(m_controller);
    return g ? g->windows.size() : 0;
}

QVariant WindowEntryModel::data(const QModelIndex &index, int role) const
{
    const ApplicationGroup *g = openGroup(m_controller);
    if (!g || !index.isValid() || index.row() < 0 || index.row() >= g->windows.size()) {
        return {};
    }
    const WindowDescriptor &w = g->windows[index.row()];
    switch (role) {
    case CaptionRole:
        return w.caption;
    case InternalIdRole:
        return QVariant::fromValue(w.id);
    case IconNameRole:
        return w.appIconName;
    case MinimizedRole:
        return w.minimized;
    default:
        return {};
    }
}

QHash<int, QByteArray> WindowEntryModel::roleNames() const
{
    return {
        {CaptionRole, "caption"},
        {InternalIdRole, "internalId"},
        {IconNameRole, "iconName"},
        {MinimizedRole, "minimized"},
    };
}

void WindowEntryModel::refresh()
{
    beginResetModel();
    endResetModel();
}

} // namespace gks
