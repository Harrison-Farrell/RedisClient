cmake_minimum_required(VERSION 3.10...3.21)

project(REDIS_CLIENT)

set(CMAKE_CXX_STANDARD 17)

set(LIBRARY_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/redisClient.h
)

set(LIBRARY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/redisClient.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCES}
)

add_library(AVS::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PUBLIC
    hiredis::hiredis
    spdlog::spdlog
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(libraries)
add_subdirectory(examples)