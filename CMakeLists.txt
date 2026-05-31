cmake_minimum_required(VERSION 3.16)

project(grouped-kde-task-switcher VERSION 0.1.0 LANGUAGES CXX)

find_package(ECM 5.102 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)

# KWin 5.27 builds at C++20; match it (set after KDECompilerSettings so it wins).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS Core Test)

# The compositor-free core + Qt model layer (unit-testable; no KWin).
add_subdirectory(src)

enable_testing()
add_subdirectory(autotests)

# NOTE: the KWin-coupled effect (src/effect) and QML overlay (src/ui) are added
# in a later increment — they require KWin/Qt5 Quick and a live compositor to
# validate, so they are intentionally not part of this headless-testable build.
