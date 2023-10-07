include(CTest)
include(GoogleTest)

set(BINARY sqlcipherxx)

find_package(GTest REQUIRED)

file(GLOB_RECURSE TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *-gtest.cc)
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_EXECUTABLE ${TEST_SOURCE} NAME_WE)
    message(STATUS "Found Google Test: ${TEST_SOURCE}")
    add_executable(${TEST_EXECUTABLE})
    target_compile_definitions(${TEST_EXECUTABLE} PRIVATE "BOOST_ALLOW_DEPRECATED_HEADERS")
    target_include_directories(${TEST_EXECUTABLE} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${TEST_EXECUTABLE} PUBLIC ${BINARY}-shared)
    target_link_libraries(${TEST_EXECUTABLE} PUBLIC GTest::GTest GTest::Main)
    target_sources(${TEST_EXECUTABLE} PRIVATE ${TEST_SOURCE})
    gtest_add_tests(TARGET ${TEST_EXECUTABLE} "" AUTO)
    ## This requires test can be started without crash, but the zonda logger(a.k.a. "logger") static
    ## library with precompiled initialization crash all the way around. So switch to gtest_add_tests
    #gtest_discover_tests(${TEST_EXECUTABLE})
endforeach()
