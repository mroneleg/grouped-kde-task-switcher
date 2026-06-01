#include <QtTest>

#include "core/SessionController.h"
#include "fakes/FakeWindowSource.h"
#include "model/WindowEntryModel.h"

using namespace gks;

namespace {
WindowDescriptor mk(const char *key, const char *caption, qint64 t, bool minimized = false)
{
    WindowDescriptor w;
    w.id = QUuid::createUuid();
    w.appKey = QString::fromUtf8(key);
    w.appDisplayName = QString::fromUtf8(key);
    w.appIconName = QString::fromUtf8(key);
    w.caption = QString::fromUtf8(caption);
    w.lastUsedAt = t;
    w.minimized = minimized;
    w.desktops = {1};
    return w;
}
using L = SessionController::Level;
} // namespace

class WindowEntryModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void emptyWhenNotInGroup()
    {
        FakeWindowSource src;
        src.setWindows({mk("a", "a", 30), mk("b", "b", 20)});
        SessionController ctrl(&src);
        WindowEntryModel model(&ctrl);

        QCOMPARE(model.rowCount(), 0); // Closed
        ctrl.open(); // Overview, still not InGroup
        QCOMPARE(ctrl.level(), L::Overview);
        QCOMPARE(model.rowCount(), 0);
    }

    void exposesOpenGroupWindowsWithRoles()
    {
        const auto b1 = mk("b", "b-one", 20);
        const auto b2 = mk("b", "b-two", 25, /*minimized=*/true);
        FakeWindowSource src;
        src.setWindows({mk("a", "a", 30), b1, b2});
        SessionController ctrl(&src);
        WindowEntryModel model(&ctrl);

        ctrl.open();                  // highlight = previous app = index 1 = group "b"
        ctrl.enterHighlightedGroup(); // InGroup on "b"
        QCOMPARE(ctrl.level(), L::InGroup);

        QCOMPARE(model.rowCount(), 2);
        // MRU order within the group: b2 (25) before b1 (20).
        const QModelIndex i0 = model.index(0);
        QCOMPARE(model.data(i0, WindowEntryModel::CaptionRole).toString(), QStringLiteral("b-two"));
        QCOMPARE(model.data(i0, WindowEntryModel::InternalIdRole).value<QUuid>(), b2.id);
        QCOMPARE(model.data(i0, WindowEntryModel::MinimizedRole).toBool(), true);
        QCOMPARE(model.data(i0, WindowEntryModel::IconNameRole).toString(), QStringLiteral("b"));

        const QModelIndex i1 = model.index(1);
        QCOMPARE(model.data(i1, WindowEntryModel::CaptionRole).toString(), QStringLiteral("b-one"));
        QCOMPARE(model.data(i1, WindowEntryModel::InternalIdRole).value<QUuid>(), b1.id);
        QCOMPARE(model.data(i1, WindowEntryModel::MinimizedRole).toBool(), false);
    }

    void resetsToEmptyOnBack()
    {
        FakeWindowSource src;
        src.setWindows({mk("a", "a", 30), mk("b", "b", 20)});
        SessionController ctrl(&src);
        WindowEntryModel model(&ctrl);

        ctrl.open();
        ctrl.enterHighlightedGroup();
        QVERIFY(model.rowCount() > 0);
        ctrl.back(); // back to Overview
        QCOMPARE(model.rowCount(), 0);
    }
};

QTEST_GUILESS_MAIN(WindowEntryModelTest)
#include "WindowEntryModelTest.moc"
