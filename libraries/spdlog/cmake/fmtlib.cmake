include(FetchContent)

FetchContent_Declare(
    fmt
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    URL https://github.com/fmtlib/fmt/archive/refs/tags/11.1.4.tar.gz
    URL_HASH SHA256=ac366b7b4c2e9f0dde63a59b3feb5ee59b67974b14ee5dc9ea8ad78aa2c1ee1e)

FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
    # We do not require os features of fmt
    set(FMT_OS OFF CACHE BOOL "Disable FMT_OS" FORCE)
    FetchContent_MakeAvailable(fmt)
    set_target_properties(fmt PROPERTIES FOLDER "third-party")
endif()
