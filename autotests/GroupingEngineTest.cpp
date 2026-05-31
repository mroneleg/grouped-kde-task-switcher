#include <QtTest>

#include "core/ApplicationResolver.h"
#include "core/GroupingEngine.h"

using namespace gks;

namespace {
WindowDescriptor mk(const char *key, const char *name, qint64 t,
                    QList<int> desktops = {1}, const char *activity = "")
{
    WindowDescriptor w;
    w.id = QUuid::createUuid();
    w.appKey = QString::fromUtf8(key);
    w.appDisplayName = QString::fromUtf8(name);
    w.appIconName = QString::fromUtf8(key);
    w.caption = QString::fromUtf8(name) + QStringLiteral(" window");
    w.lastUsedAt = t;
    w.desktops = std::move(desktops);
    w.activity = QString::fromUtf8(activity);
    return w;
}
} // namespace

class GroupingEngineTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void oneGroupPerAppOrderedByRecency()
    {
        GroupingEngine engine;
        const auto groups = engine.buildGroups({
            mk("chrome", "Chrome", 10),
            mk("chrome", "Chrome", 30),
            mk("code", "Code", 20),
        });
        QCOMPARE(groups.size(), 2);
        QCOMPARE(groups[0].appKey, QStringLiteral("chrome")); // most-recent window = 30
        QCOMPARE(groups[0].count(), 2);
        QCOMPARE(groups[1].appKey, QStringLiteral("code"));
    }

    void windowsAreMruWithinGroup()
    {
        GroupingEngine engine;
        const auto groups = engine.buildGroups({
            mk("chrome", "Chrome", 10),
            mk("chrome", "Chrome", 30),
        });
        QCOMPARE(groups[0].windows.first().lastUsedAt, qint64(30));
        QCOMPARE(groups[0].mostRecentWindow().lastUsedAt, qint64(30));
    }

    void fallbackGroupSortsLast()
    {
        GroupingEngine engine;
        const auto groups = engine.buildGroups({
            mk(ApplicationResolver::FallbackKey, "Unidentified", 99),
            mk("code", "Code", 20),
        });
        QCOMPARE(groups.last().appKey, QString::fromLatin1(ApplicationResolver::FallbackKey));
        QVERIFY(groups.last().isFallback);
        QVERIFY(!groups.first().isFallback);
    }

    void singleWindowGroup()
    {
        GroupingEngine engine;
        const auto groups = engine.buildGroups({mk("term", "Terminal", 5)});
        QCOMPARE(groups.size(), 1);
        QCOMPARE(groups[0].count(), 1);
    }

    void includesAllDesktopsActivitiesMonitors() // T050 / FR-020
    {
        GroupingEngine engine;
        auto a = mk("chrome", "Chrome", 10, {1}, "activity-A");
        auto b = mk("chrome", "Chrome", 20, {3}, "activity-B");
        b.screenName = QStringLiteral("HDMI-1");
        auto c = mk("code", "Code", 15, {2}, "activity-A");

        const auto groups = engine.buildGroups({a, b, c});

        int total = 0;
        for (const auto &g : groups) {
            total += g.count();
        }
        QCOMPARE(total, 3); // nothing filtered out by desktop/activity/monitor
        QCOMPARE(groups.size(), 2);
    }
};

QTEST_GUILESS_MAIN(GroupingEngineTest)
#include "GroupingEngineTest.moc"
