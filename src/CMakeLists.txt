# Compositor-free behavioral core + Qt model layer (Constitution Principle III:
# testable without a running compositor). KWin coupling lives in the effect target.
add_library(gks_core STATIC
    core/WindowSource.cpp
    core/ApplicationResolver.cpp
    core/GroupingEngine.cpp
    core/SessionController.cpp
    model/SwitcherModel.cpp
    model/WindowEntryModel.cpp
)

target_include_directories(gks_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(gks_core PUBLIC Qt5::Core)
set_target_properties(gks_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# The KWin effect plugin (only built when the KWin effect headers are present).
if(TARGET KWinEffects::kwineffects)
    kcoreaddons_add_plugin(kwin_grouped_switcher
        SOURCES
            effect/main.cpp
            effect/GroupedSwitcherEffect.cpp
            effect/EffectWindowSource.cpp
        INSTALL_NAMESPACE "kwin/effects/plugins")

    target_include_directories(kwin_grouped_switcher PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(kwin_grouped_switcher PRIVATE
        gks_core
        KWinEffects::kwineffects
        Qt5::Quick
        Qt5::Gui
        Qt5::DBus
        KF5::GlobalAccel
        KF5::Service
        KF5::I18n
        KF5::CoreAddons)

    # QML overlay → /usr/share/kwin/effects/groupedswitcher/qml/
    install(DIRECTORY ui/
        DESTINATION ${KDE_INSTALL_DATADIR}/kwin/effects/groupedswitcher/qml
        FILES_MATCHING PATTERN "*.qml")
else()
    message(STATUS "KWinEffects not found — building core + tests only (effect skipped)")
endif()
