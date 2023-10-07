cmake_minimum_required(VERSION 3.16)

project(sqlcipher VERSION 4.5.1)

set(BINARY sqlcipher)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

if (NOT BUILD_NAR)
    add_library(${BINARY}-shared SHARED)
endif()
add_library(${BINARY}-static STATIC)
if (NOT BUILD_NAR)
    set_target_properties(${BINARY}-shared PROPERTIES OUTPUT_NAME ${BINARY})
endif()
set_target_properties(${BINARY}-static PROPERTIES OUTPUT_NAME ${BINARY})
set_target_properties(${BINARY}-static PROPERTIES POSITION_INDEPENDENT_CODE ON)

if (NOT BUILD_NAR)
    target_sources(${BINARY}-shared PRIVATE sqlite3.c)
endif()
target_sources(${BINARY}-static PRIVATE sqlite3.c)

if (NOT BUILD_NAR)
    target_compile_definitions(${BINARY}-shared PRIVATE "SQLITE_HAS_CODEC")
    target_compile_definitions(${BINARY}-shared PRIVATE "SQLCIPHER_CRYPTO_OPENSSL")
    target_compile_definitions(${BINARY}-shared PRIVATE "SQLITE_OS_UNIX=1")
    target_compile_definitions(${BINARY}-shared PRIVATE "SQLITE_THREADSAFE=1")
    target_compile_definitions(${BINARY}-shared PRIVATE "SQLITE_ENABLE_MATH_FUNCTIONS")
    target_compile_definitions(${BINARY}-shared PRIVATE "SQLITE_HAVE_ZLIB=1")
endif()

target_compile_definitions(${BINARY}-static PRIVATE "SQLITE_HAS_CODEC")
target_compile_definitions(${BINARY}-static PRIVATE "SQLCIPHER_CRYPTO_OPENSSL")
target_compile_definitions(${BINARY}-static PRIVATE "SQLITE_OS_UNIX=1")
target_compile_definitions(${BINARY}-static PRIVATE "SQLITE_THREADSAFE=1")
target_compile_definitions(${BINARY}-static PRIVATE "SQLITE_ENABLE_MATH_FUNCTIONS")
target_compile_definitions(${BINARY}-static PRIVATE "SQLITE_HAVE_ZLIB=1")

if (NOT BUILD_NAR)
    target_include_directories(${BINARY}-shared
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC $<INSTALL_INTERFACE:include>
    )
endif()
target_include_directories(${BINARY}-static
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC $<INSTALL_INTERFACE:include>
)

if (NOT BUILD_NAR)
    target_link_libraries(${BINARY}-shared PRIVATE m)
    target_link_libraries(${BINARY}-shared PRIVATE OpenSSL::Crypto)
    target_link_libraries(${BINARY}-shared PRIVATE ZLIB::ZLIB)
    target_link_libraries(${BINARY}-shared PRIVATE Threads::Threads)
    target_link_libraries(${BINARY}-shared PRIVATE ${CMAKE_LD_LIBS})
endif()

target_link_libraries(${BINARY}-static PRIVATE m)
target_link_libraries(${BINARY}-static PRIVATE OpenSSL::Crypto)
target_link_libraries(${BINARY}-static PRIVATE ZLIB::ZLIB)
target_link_libraries(${BINARY}-static PRIVATE Threads::Threads)
target_link_libraries(${BINARY}-static PRIVATE ${CMAKE_LD_LIBS})

include(GNUInstallDirs)
if (NOT BUILD_NAR)
    install(TARGETS ${BINARY}-shared EXPORT ${BINARY}-targets LIBRARY DESTINATION lib)
endif()
install(TARGETS ${BINARY}-static EXPORT ${BINARY}-targets ARCHIVE DESTINATION lib)

install(
    DIRECTORY .
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(
    EXPORT ${BINARY}-targets
    FILE ${BINARY}-config.cmake
    NAMESPACE ${BINARY}::
    DESTINATION lib/cmake/${BINARY}
)
