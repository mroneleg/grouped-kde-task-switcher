#pragma once

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QVariant>

namespace gks {

class SessionController;

/**
 * Read-only model of the windows in the currently-open group (the in-group grid,
 * contracts/switcher-model.md). Exposes each window's internalId for binding to
 * a QML WindowThumbnailItem.wId. Empty unless the controller is InGroup.
 */
class WindowEntryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        CaptionRole = Qt::UserRole + 1,
        InternalIdRole, ///< QUuid, for WindowThumbnailItem.wId
        IconNameRole,
        MinimizedRole,
    };

    explicit WindowEntryModel(SessionController *controller, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private Q_SLOTS:
    void refresh();

private:
    SessionController *m_controller;
};

} // namespace gks
