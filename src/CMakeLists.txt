set(BINARY sqlcipherxx)

add_library(${BINARY}-shared SHARED)
add_library(${BINARY}-static STATIC)

file(GLOB_RECURSE LIBRARY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

target_sources(${BINARY}-shared PUBLIC ${LIBRARY_SOURCES})
target_sources(${BINARY}-static PUBLIC ${LIBRARY_SOURCES})

target_link_libraries(${BINARY}-shared PRIVATE sqlcipher-static)
target_link_libraries(${BINARY}-static PRIVATE sqlcipher-static)

target_include_directories(${BINARY}-shared
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PUBLIC $<INSTALL_INTERFACE:include>
)
target_include_directories(${BINARY}-static
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PUBLIC $<INSTALL_INTERFACE:include>
)

set_target_properties(${BINARY}-shared PROPERTIES OUTPUT_NAME ${BINARY})
set_target_properties(${BINARY}-static PROPERTIES OUTPUT_NAME ${BINARY})
set_target_properties(${BINARY}-static PROPERTIES POSITION_INDEPENDENT_CODE ON)

