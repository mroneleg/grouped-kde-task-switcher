#include "model/SwitcherModel.h"

#include "core/SessionController.h"

namespace gks {

SwitcherModel::SwitcherModel(SessionController *controller, QObject *parent)
    : QAbstractListModel(parent)
    , m_controller(controller)
{
    if (m_controller) {
        connect(m_controller, &SessionController::stateChanged, this, &SwitcherModel::refresh);
    }
}

int SwitcherModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_controller) {
        return 0;
    }
    return m_controller->groups().size();
}

QVariant SwitcherModel::data(const QModelIndex &index, int role) const
{
    if (!m_controller || !index.isValid()) {
        return {};
    }
    const QList<ApplicationGroup> &groups = m_controller->groups();
    if (index.row() < 0 || index.row() >= groups.size()) {
        return {};
    }
    const ApplicationGroup &group = groups[index.row()];
    switch (role) {
    case AppDisplayNameRole:
        return group.displayName;
    case AppIconNameRole:
        return group.iconName;
    case WindowCountRole:
        return group.count();
    case IsFallbackRole:
        return group.isFallback;
    case AppKeyRole:
        return group.appKey;
    default:
        return {};
    }
}

QHash<int, QByteArray> SwitcherModel::roleNames() const
{
    return {
        {AppDisplayNameRole, "appDisplayName"},
        {AppIconNameRole, "appIconName"},
        {WindowCountRole, "windowCount"},
        {IsFallbackRole, "isFallback"},
        {AppKeyRole, "appKey"},
    };
}

void SwitcherModel::refresh()
{
    beginResetModel();
    endResetModel();
}

} // namespace gks
