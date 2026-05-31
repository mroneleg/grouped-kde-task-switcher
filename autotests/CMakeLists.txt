# Test fakes (FakeWindowSource is a QObject → needs its own TU for AUTOMOC).
add_library(gks_testfakes STATIC fakes/FakeWindowSource.cpp)
target_include_directories(gks_testfakes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gks_testfakes PUBLIC gks_core Qt5::Core)

# QTEST_GUILESS_MAIN tests run headless (no display / offscreen QPA needed).
ecm_add_test(ApplicationResolverTest.cpp
    TEST_NAME ApplicationResolverTest
    LINK_LIBRARIES gks_core Qt5::Test)

ecm_add_test(GroupingEngineTest.cpp
    TEST_NAME GroupingEngineTest
    LINK_LIBRARIES gks_core Qt5::Test)

ecm_add_test(SessionControllerTest.cpp
    TEST_NAME SessionControllerTest
    LINK_LIBRARIES gks_core gks_testfakes Qt5::Test)

ecm_add_test(SwitcherModelTest.cpp
    TEST_NAME SwitcherModelTest
    LINK_LIBRARIES gks_core gks_testfakes Qt5::Test)
