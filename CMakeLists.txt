cmake_minimum_required(VERSION 3.8)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (APPLE)
    if (EXISTS /usr/local/opt/openssl)
        set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    endif()
endif()

project(sqlcipher-lock-demo VERSION 1.0)

find_package(Boost REQUIRED COMPONENTS thread chrono filesystem)
find_package(OpenSSL REQUIRED)

add_subdirectory(thirdparty/sqlcipher)
add_subdirectory(src)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()
