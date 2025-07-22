# ---------------------------------------------------------------------------------------
# Copyright(c) 2019-present by spdlog authors.
# Distributed under the MIT License (http://opensource.org/licenses/MIT)
# ---------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.23)
include(cmake/utils.cmake)
include(cmake/ide.cmake)
spdlog_extract_version()
project(spdlog VERSION ${SPDLOG_VERSION} LANGUAGES CXX)
include(GNUInstallDirs)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# ---------------------------------------------------------------------------------------
# Compiler config
# ---------------------------------------------------------------------------------------
# C++ standard >=17 is required
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
elseif (CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "Minimum supported CMAKE_CXX_STANDARD is 17, but it is set to ${CMAKE_CXX_STANDARD}")
endif ()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if (CMAKE_SYSTEM_NAME MATCHES "CYGWIN" OR CMAKE_SYSTEM_NAME MATCHES "MSYS" OR CMAKE_SYSTEM_NAME MATCHES "MINGW")
    set(CMAKE_CXX_EXTENSIONS ON)
endif ()

# ---------------------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------------------
option(SPDLOG_BUILD_ALL "Build all artifacts" OFF)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    option(SPDLOG_CLOCK_COARSE "Use CLOCK_REALTIME_COARSE instead of the regular clock," OFF)
else ()
    set(SPDLOG_CLOCK_COARSE OFF CACHE BOOL "non supported option" FORCE)
endif ()
if (SPDLOG_BUILD_SHARED)
    set(BUILD_SHARED_LIBS ON)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # place dlls and libs and executables in the same directory
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
    # make sure __cplusplus is defined
    add_compile_options(/Zc:__cplusplus)
    # enable parallel build for the solution
    add_compile_options(/MP)
endif ()
# ---------------------------------------------------------------------------------------
# Find {fmt} library
# ---------------------------------------------------------------------------------------
if (SPDLOG_FMT_EXTERNAL)
    find_package(fmt REQUIRED)
    message(STATUS "Using external fmt lib version: ${fmt_VERSION}")
else ()
    include(cmake/fmtlib.cmake)
endif ()
# ---------------------------------------------------------------------------------------
# Threads library is required
# ---------------------------------------------------------------------------------------
find_package(Threads REQUIRED)
# ---------------------------------------------------------------------------------------
# Library sources
# ---------------------------------------------------------------------------------------
set(SPDLOG_HEADERS
        "include/spdlog/common.h"
        "include/spdlog/formatter.h"
        "include/spdlog/fwd.h"
        "include/spdlog/logger.h"
        "include/spdlog/pattern_formatter.h"
        "include/spdlog/source_loc.h"
        "include/spdlog/spdlog.h"
        "include/spdlog/stopwatch.h"
        "include/spdlog/version.h"
        "include/spdlog/details/circular_q.h"
        "include/spdlog/details/file_helper.h"
        "include/spdlog/details/fmt_helper.h"
        "include/spdlog/details/log_msg.h"
        "include/spdlog/details/async_log_msg.h"
        "include/spdlog/details/mpmc_blocking_q.h"
        "include/spdlog/details/null_mutex.h"
        "include/spdlog/details/os.h"
        "include/spdlog/details/err_helper.h"
        "include/spdlog/bin_to_hex.h"
        "include/spdlog/sinks/android_sink.h"
        "include/spdlog/sinks/base_sink.h"
        "include/spdlog/sinks/basic_file_sink.h"
        "include/spdlog/sinks/callback_sink.h"
        "include/spdlog/sinks/daily_file_sink.h"
        "include/spdlog/sinks/dist_sink.h"
        "include/spdlog/sinks/dup_filter_sink.h"
        "include/spdlog/sinks/hourly_file_sink.h"
        "include/spdlog/sinks/kafka_sink.h"
        "include/spdlog/sinks/mongo_sink.h"
        "include/spdlog/sinks/msvc_sink.h"
        "include/spdlog/sinks/null_sink.h"
        "include/spdlog/sinks/ostream_sink.h"
        "include/spdlog/sinks/qt_sinks.h"
        "include/spdlog/sinks/ringbuffer_sink.h"
        "include/spdlog/sinks/rotating_file_sink.h"
        "include/spdlog/sinks/sink.h"
        "include/spdlog/sinks/stdout_color_sinks.h"
        "include/spdlog/sinks/stdout_sinks.h"
        "include/spdlog/sinks/syslog_sink.h"
        "include/spdlog/sinks/systemd_sink.h"
        "include/spdlog/sinks/tcp_sink.h"
        "include/spdlog/sinks/udp_sink.h"
        "include/spdlog/sinks/async_sink.h")
set(SPDLOG_SRCS
        "src/common.cpp"
        "src/logger.cpp"
        "src/pattern_formatter.cpp"
        "src/spdlog.cpp"
        "src/details/file_helper.cpp"
        "src/details/os_filesystem.cpp"
        "src/details/log_msg.cpp"
        "src/details/async_log_msg.cpp"
        "src/details/err_helper.cpp"
        "src/sinks/base_sink.cpp"
        "src/sinks/basic_file_sink.cpp"
        "src/sinks/rotating_file_sink.cpp"
        "src/sinks/stdout_sinks.cpp"
        "src/sinks/async_sink.cpp")
if (WIN32)
    list(APPEND SPDLOG_SRCS
            "src/details/os_windows.cpp"
            "src/sinks/wincolor_sink.cpp")
    list(APPEND SPDLOG_HEADERS
            "include/spdlog/sinks/wincolor_sink.h"
            "include/spdlog/details/tcp_client_windows.h"
            "include/spdlog/details/udp_client_windows.h"
            "include/spdlog/details/windows_include.h"
            "include/spdlog/sinks/win_eventlog_sink.h")
else ()
    list(APPEND SPDLOG_SRCS
            "src/details/os_unix.cpp"
            "src/sinks/ansicolor_sink.cpp")
    list(APPEND SPDLOG_HEADERS
            "include/spdlog/details/tcp_client_unix.h"
            "include/spdlog/details/udp_client_unix.h"
            "include/spdlog/sinks/ansicolor_sink.h")
endif ()
# ---------------------------------------------------------------------------------------
# Check if fwrite_unlocked/_fwrite_nolock is available
# ---------------------------------------------------------------------------------------
include(CheckSymbolExists)
if (WIN32)
    check_symbol_exists(_fwrite_nolock "stdio.h" HAVE_FWRITE_UNLOCKED)
else ()
    check_symbol_exists(fwrite_unlocked "stdio.h" HAVE_FWRITE_UNLOCKED)
endif ()
if (HAVE_FWRITE_UNLOCKED)
    set(SPDLOG_FWRITE_UNLOCKED 1)
endif ()
# ---------------------------------------------------------------------------------------
# spdlog library
# ---------------------------------------------------------------------------------------
if (BUILD_SHARED_LIBS)
    if (WIN32)
        set(VERSION_RC ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc @ONLY)
    endif ()
    add_library(spdlog SHARED ${VERSION_RC})
    target_compile_definitions(spdlog PUBLIC SPDLOG_SHARED_LIB)
    if (MSVC)
        # disable dlls related warnings on msvc
        target_compile_options(spdlog PUBLIC $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<NOT:$<COMPILE_LANGUAGE:CUDA>>>:/wd4251 /wd4275>)
    endif ()
else ()
    add_library(spdlog STATIC)
endif ()
set_target_properties(spdlog PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
add_library(spdlog::spdlog ALIAS spdlog)
target_sources(spdlog PRIVATE ${SPDLOG_SRCS})
target_sources(
        spdlog
        PUBLIC FILE_SET pub_headers
        TYPE HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES ${SPDLOG_HEADERS})
set(SPDLOG_INCLUDES_LEVEL "")
if (SPDLOG_SYSTEM_INCLUDES)
    set(SPDLOG_INCLUDES_LEVEL "SYSTEM")
endif ()
target_include_directories(spdlog ${SPDLOG_INCLUDES_LEVEL} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(spdlog PUBLIC Threads::Threads)
target_link_libraries(spdlog PUBLIC fmt::fmt)
spdlog_enable_warnings(spdlog)
set_target_properties(spdlog PROPERTIES VERSION ${SPDLOG_VERSION} SOVERSION
        ${SPDLOG_VERSION_MAJOR}.${SPDLOG_VERSION_MINOR})
set(SPDLOG_NAME spdlog-${SPDLOG_VERSION_MAJOR})
set_target_properties(spdlog PROPERTIES DEBUG_POSTFIX "-${SPDLOG_VERSION_MAJOR}.${SPDLOG_VERSION_MINOR}d")
# ---------------------------------------------------------------------------------------
# Set prefix and source group for visual studio
# ---------------------------------------------------------------------------------------
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX include FILES ${SPDLOG_HEADERS})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX sources FILES ${SPDLOG_SRCS})
    source_group(sources FILES ${VERSION_RC})
endif ()
