#pragma once

#include <QString>

namespace gks {

/// Resolved identity of an application for one window.
struct AppIdentity
{
    QString key;          ///< grouping key
    QString displayName;  ///< human-readable name
    QString iconName;     ///< icon name/path
};

/**
 * Resolves an application's display name from its WM_CLASS. Abstracted behind an
 * interface so the core is testable without hitting the live KService database;
 * the production implementation (effect layer) is backed by KService.
 */
class AppInfoProvider
{
public:
    virtual ~AppInfoProvider() = default;
    /// Display name for a WM_CLASS class string, or empty if unknown.
    virtual QString displayNameForClass(const QString &wmClass) const = 0;
};

/**
 * Maps a window's WM_CLASS (+ its own icon name) to an AppIdentity. On KWin 5.27
 * EffectWindow has no desktopFileName(), so grouping is keyed on windowClass()
 * (research.md §"Retarget to Plasma 5.27").
 */
class ApplicationResolver
{
public:
    static constexpr const char *FallbackKey = "__unidentified__";

    explicit ApplicationResolver(const AppInfoProvider *provider = nullptr);

    /// @param wmClass        WM_CLASS class string (resourceClass); may be empty
    /// @param windowIconName icon name reported by the window; may be empty
    AppIdentity resolve(const QString &wmClass, const QString &windowIconName) const;

private:
    const AppInfoProvider *m_provider;
};

} // namespace gks
