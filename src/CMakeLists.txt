# Compositor-free behavioral core + Qt model layer (Constitution Principle III:
# testable without a running compositor). KWin coupling lives in src/effect.
add_library(gks_core STATIC
    core/WindowSource.cpp
    core/ApplicationResolver.cpp
    core/GroupingEngine.cpp
    core/SessionController.cpp
    model/SwitcherModel.cpp
)

target_include_directories(gks_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(gks_core PUBLIC Qt5::Core)
set_target_properties(gks_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
