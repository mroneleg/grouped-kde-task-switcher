#include <QtTest>

#include "core/ApplicationResolver.h"

using namespace gks;

namespace {
class FakeProvider : public AppInfoProvider
{
public:
    QHash<QString, QString> map;
    QString displayNameForClass(const QString &wmClass) const override
    {
        return map.value(wmClass);
    }
};
} // namespace

class ApplicationResolverTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void resolvesViaProvider()
    {
        FakeProvider provider;
        provider.map.insert(QStringLiteral("Code"), QStringLiteral("Visual Studio Code"));
        ApplicationResolver resolver(&provider);

        const AppIdentity id = resolver.resolve(QStringLiteral("Code"), QStringLiteral("code-oss"));
        QCOMPARE(id.key, QStringLiteral("code"));
        QCOMPARE(id.displayName, QStringLiteral("Visual Studio Code"));
        QCOMPARE(id.iconName, QStringLiteral("code-oss"));
    }

    void prettifiesWhenNoProvider()
    {
        ApplicationResolver resolver(nullptr);
        const AppIdentity id = resolver.resolve(QStringLiteral("firefox"), QString());
        QCOMPARE(id.key, QStringLiteral("firefox"));
        QCOMPARE(id.displayName, QStringLiteral("Firefox"));
        QCOMPARE(id.iconName, QStringLiteral("firefox"));
    }

    void fallbackForEmptyClass()
    {
        ApplicationResolver resolver(nullptr);
        const AppIdentity id = resolver.resolve(QString(), QString());
        QCOMPARE(id.key, QString::fromLatin1(ApplicationResolver::FallbackKey));
        QVERIFY(!id.displayName.isEmpty());
        QCOMPARE(id.iconName, QStringLiteral("application-x-executable"));
    }
};

QTEST_GUILESS_MAIN(ApplicationResolverTest)
#include "ApplicationResolverTest.moc"
