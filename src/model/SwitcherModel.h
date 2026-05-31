#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QByteArray>
#include <QVariant>

namespace gks {

class SessionController;

/**
 * Read-only QML-facing model for the grouped overview (contracts/switcher-model.md).
 * Exposes the controller's current application groups; resets whenever the
 * controller's state changes.
 */
class SwitcherModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        AppDisplayNameRole = Qt::UserRole + 1,
        AppIconNameRole,
        WindowCountRole,
        IsFallbackRole,
        AppKeyRole,
    };

    explicit SwitcherModel(SessionController *controller, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private Q_SLOTS:
    void refresh();

private:
    SessionController *m_controller;
};

} // namespace gks
