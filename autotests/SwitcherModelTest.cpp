#include <QtTest>

#include "core/SessionController.h"
#include "fakes/FakeWindowSource.h"
#include "model/SwitcherModel.h"

using namespace gks;

namespace {
WindowDescriptor mk(const char *key, const char *name, const char *icon, qint64 t)
{
    WindowDescriptor w;
    w.id = QUuid::createUuid();
    w.appKey = QString::fromUtf8(key);
    w.appDisplayName = QString::fromUtf8(name);
    w.appIconName = QString::fromUtf8(icon);
    w.caption = QString::fromUtf8(name);
    w.lastUsedAt = t;
    w.desktops = {1};
    return w;
}
} // namespace

class SwitcherModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void exposesGroupRoles()
    {
        FakeWindowSource src;
        src.setWindows({
            mk("chrome", "Chrome", "google-chrome", 30),
            mk("code", "Code", "code", 20),
        });
        SessionController ctrl(&src);
        SwitcherModel model(&ctrl);
        ctrl.open();

        QCOMPARE(model.rowCount(), 2);
        const QModelIndex idx0 = model.index(0);
        QCOMPARE(model.data(idx0, SwitcherModel::AppDisplayNameRole).toString(),
                 QStringLiteral("Chrome"));
        QCOMPARE(model.data(idx0, SwitcherModel::AppIconNameRole).toString(),
                 QStringLiteral("google-chrome"));
        QCOMPARE(model.data(idx0, SwitcherModel::WindowCountRole).toInt(), 1);
        QCOMPARE(model.data(idx0, SwitcherModel::IsFallbackRole).toBool(), false);
    }

    void resetsOnStateChange()
    {
        FakeWindowSource src;
        src.setWindows({mk("a", "A", "a", 30)});
        SessionController ctrl(&src);
        SwitcherModel model(&ctrl);

        QCOMPARE(model.rowCount(), 0); // closed -> no groups yet
        ctrl.open();
        QCOMPARE(model.rowCount(), 1);
    }
};

QTEST_GUILESS_MAIN(SwitcherModelTest)
#include "SwitcherModelTest.moc"
