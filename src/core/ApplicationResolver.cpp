#include "core/ApplicationResolver.h"

namespace gks {

ApplicationResolver::ApplicationResolver(const AppInfoProvider *provider)
    : m_provider(provider)
{
}

static QString prettify(const QString &wmClass)
{
    if (wmClass.isEmpty()) {
        return {};
    }
    QString s = wmClass;
    s[0] = s[0].toUpper();
    return s;
}

AppIdentity ApplicationResolver::resolve(const QString &wmClass, const QString &windowIconName) const
{
    AppIdentity id;
    const QString cls = wmClass.trimmed();

    if (cls.isEmpty()) {
        id.key = QString::fromLatin1(FallbackKey);
        id.displayName = QStringLiteral("Unidentified");
        id.iconName = windowIconName.isEmpty() ? QStringLiteral("application-x-executable")
                                               : windowIconName;
        return id;
    }

    id.key = cls.toLower();

    QString resolvedName;
    if (m_provider) {
        resolvedName = m_provider->displayNameForClass(cls);
    }
    id.displayName = resolvedName.isEmpty() ? prettify(cls) : resolvedName;

    // Icon themes are usually keyed by the lowercased class when the window
    // does not report its own icon name.
    id.iconName = windowIconName.isEmpty() ? cls.toLower() : windowIconName;

    return id;
}

} // namespace gks
