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

find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Quick Test)
find_package(KF5 REQUIRED COMPONENTS CoreAddons Config I18n Service WindowSystem GlobalAccel)

# The KWin 5.27 effect library (KWinEffects::kwineffects). Optional so the
# compositor-free core + tests still build where KWin headers are absent.
find_package(KWinEffects QUIET)
if(KWinEffects_FOUND)
    # KWinEffects' link interface references XCB::XCB / epoxy targets.
    find_package(XCB COMPONENTS XCB)
    find_package(epoxy)
endif()

# The compositor-free behavioral core + Qt model layer (testable; no KWin).
add_subdirectory(src)

enable_testing()
add_subdirectory(autotests)
