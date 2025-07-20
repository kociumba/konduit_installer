cmake_minimum_required(VERSION 3.31)
project(konduit_installer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
    endif ()
endif ()

set(C_SOURCES
        include/raylib/rres_impl.c
        include/lz4/lz4.c
        include/raylib/rres-raylib_impl.c)
set(CXX_SOURCES main.cpp
        main.hpp
        clayman.cpp
        components.cpp
        components.hpp
        include/tinyfiledialogs/tinyfiledialogs.c
        utils.hpp)

add_executable(konduit_installer ${C_SOURCES} ${CXX_SOURCES})

set_source_files_properties(${C_SOURCES} PROPERTIES LANGUAGE C)

target_include_directories(konduit_installer PUBLIC include)
target_link_libraries(konduit_installer PUBLIC raylib)

if (WIN32)
    target_link_libraries(konduit_installer PRIVATE Comdlg32.lib Ole32.lib)
endif ()
