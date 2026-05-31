#include <QtTest>

#include "core/SessionController.h"
#include "fakes/FakeWindowSource.h"

using namespace gks;

namespace {
WindowDescriptor mk(const char *key, qint64 t)
{
    const QString k = QString::fromUtf8(key);
    WindowDescriptor w;
    w.id = QUuid::createUuid();
    w.appKey = k;
    w.appDisplayName = k;
    w.appIconName = k;
    w.caption = k;
    w.lastUsedAt = t;
    w.desktops = {1};
    return w;
}
using L = SessionController::Level;
} // namespace

class SessionControllerTest : public QObject
{
    Q_OBJECT

    FakeWindowSource *src = nullptr;
    SessionController *ctrl = nullptr;

    void setup(const QList<WindowDescriptor> &windows)
    {
        delete ctrl;
        delete src;
        src = new FakeWindowSource;
        src->setWindows(windows);
        ctrl = new SessionController(src);
    }

private Q_SLOTS:
    void cleanup()
    {
        delete ctrl;
        ctrl = nullptr;
        delete src;
        src = nullptr;
    }

    void openShowsGroupsWithPreviousAppHighlighted()
    {
        setup({mk("a", 30), mk("b", 20), mk("c", 10)});
        ctrl->open();
        QCOMPARE(ctrl->level(), L::Overview);
        QCOMPARE(ctrl->groups().size(), 3);
        QCOMPARE(ctrl->highlightedGroupIndex(), 1); // previous app default (FR-003)
    }

    void openEmptyClosesWithEmptyState() // FR-019
    {
        setup({});
        QSignalSpy closed(ctrl, &SessionController::closed);
        ctrl->open();
        QVERIFY(ctrl->isEmpty());
        QCOMPARE(ctrl->level(), L::Closed);
        QCOMPARE(closed.count(), 1);
    }

    void advanceWrapsAndNeverCloses() // FR-008/009
    {
        setup({mk("a", 30), mk("b", 20)});
        QSignalSpy closed(ctrl, &SessionController::closed);
        ctrl->open(); // highlight = 1
        ctrl->advanceHighlight(); // -> 0 (wrap)
        QCOMPARE(ctrl->highlightedGroupIndex(), 0);
        ctrl->advanceHighlight(); // -> 1
        QCOMPARE(ctrl->highlightedGroupIndex(), 1);
        QCOMPARE(closed.count(), 0);
        QCOMPARE(ctrl->level(), L::Overview);
    }

    void confirmActivatesGroupMostRecentWindow() // FR-010
    {
        const auto b0 = mk("b", 20);
        setup({mk("a", 30), b0});
        ctrl->open(); // highlight = 1 -> group "b"
        ctrl->confirm();
        QCOMPARE(src->activated().size(), 1);
        QCOMPARE(src->activated().first(), b0.id);
        QCOMPARE(ctrl->level(), L::Closed);
    }

    void enterAndBackKeepsOriginHighlighted() // FR-005
    {
        setup({mk("a", 30), mk("b", 20)});
        ctrl->open(); // overview, highlight = 1 (b)
        ctrl->enterHighlightedGroup();
        QCOMPARE(ctrl->level(), L::InGroup);
        QCOMPARE(ctrl->openGroupIndex(), 1);
        ctrl->back();
        QCOMPARE(ctrl->level(), L::Overview);
        QCOMPARE(ctrl->highlightedGroupIndex(), 1);
    }

    void confirmActivatesHighlightedWindowInGroup() // FR-010
    {
        const auto b2 = mk("b", 25); // most recent in group b
        setup({mk("a", 30), mk("b", 20), b2});
        ctrl->open(); // highlight = 1 (b)
        ctrl->enterHighlightedGroup(); // window highlight 0 -> b2
        ctrl->confirm();
        QCOMPARE(src->activated().size(), 1);
        QCOMPARE(src->activated().first(), b2.id);
    }

    void cancelRestoresPreviousAndActivatesNothing() // FR-009
    {
        const auto a0 = mk("a", 30);
        setup({a0, mk("b", 20)});
        src->setPreviouslyActive(a0.id);
        ctrl->open();
        ctrl->cancel();
        QCOMPARE(src->activated().size(), 0);
        QCOMPARE(src->restored().size(), 1);
        QCOMPARE(src->restored().first(), a0.id);
        QCOMPARE(ctrl->level(), L::Closed);
    }

    void reinvokeWhileOpenAdvancesSingleInstance() // T051
    {
        setup({mk("a", 30), mk("b", 20)});
        ctrl->open(); // highlight = 1
        ctrl->open(); // re-invoke -> advance -> 0 (wrap), one session
        QCOMPARE(ctrl->highlightedGroupIndex(), 0);
        QCOMPARE(ctrl->level(), L::Overview);
    }

    void liveRemoveClampsThenClosesOnEmpty() // FR-018/019
    {
        const auto a0 = mk("a", 30);
        const auto b0 = mk("b", 20);
        setup({a0, b0});
        ctrl->open();
        src->removeWindow(b0.id);
        QCOMPARE(ctrl->groups().size(), 1);
        QVERIFY(ctrl->highlightedGroupIndex() < ctrl->groups().size());
        QSignalSpy closed(ctrl, &SessionController::closed);
        src->removeWindow(a0.id); // last window gone -> close
        QCOMPARE(closed.count(), 1);
    }

    void liveAddInsertsGroup() // FR-018
    {
        setup({mk("a", 30)});
        ctrl->open();
        src->addWindow(mk("b", 40));
        QCOMPARE(ctrl->groups().size(), 2);
    }
};

QTEST_GUILESS_MAIN(SessionControllerTest)
#include "SessionControllerTest.moc"
