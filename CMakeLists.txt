cmake_minimum_required(VERSION 3.31)
project(konduit_installer LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 23)
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
            CMAKE_ARGS
            -DBUILD_EXAMPLES=OFF
            -DGLFW_BUILD_OSMESA=OFF
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
    endif ()
endif ()

set(C_SOURCES include/tinyfiledialogs/tinyfiledialogs.c
        include/raylib/clay_renderer_raylib.c
        include/raylib/rres_impl.c
        include/lz4/lz4.c
        include/raylib/rres-raylib_impl.c
        embed.h
        embed.c)
set(CXX_SOURCES main.cpp
        main.hpp
        clayman.cpp
        components.cpp
        components.hpp
        utils.hpp
        utils.cpp
        #        win32_entry.cpp
        #        entry.h
)

add_executable(konduit_installer ${C_SOURCES} ${CXX_SOURCES})

set_source_files_properties(${C_SOURCES} WIN32 PROPERTIES LANGUAGE C)
if (MSVC)
    set_source_files_properties(${C_SOURCES} PROPERTIES COMPILE_FLAGS "/TC")
endif ()

target_include_directories(konduit_installer PUBLIC include)
target_link_libraries(konduit_installer PUBLIC raylib)

if (WIN32)
    target_link_libraries(konduit_installer PRIVATE Comdlg32.lib Ole32.lib user32.lib gdi32.lib)
    if (NOT MSVC)
        target_link_options(konduit_installer PRIVATE -static)
    endif ()
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (MSVC)
        # optional for prod pdb
        # /Zi
        target_compile_options(konduit_installer PRIVATE "/O2" "/GL")
        # /DEBUG
        target_link_options(konduit_installer PRIVATE "/LTCG" "/OPT:REF" "/OPT:ICF" "/INCREMENTAL:NO")
        target_link_options(konduit_installer PRIVATE "/SUBSYSTEM:WINDOWS")
    else ()
        target_compile_options(konduit_installer PRIVATE "-O2" "-g0")
        target_link_options(konduit_installer PRIVATE -mwindows)
        target_link_options(konduit_installer PRIVATE -flto)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set_target_properties(konduit_installer PROPERTIES
                    LINK_FLAGS_RELEASE "-Wl,--strip-all"
            )
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|IntelLLVM")
            set_target_properties(konduit_installer PROPERTIES
                    LINK_FLAGS_RELEASE "-Wl,-S"
            )
        endif ()
    endif ()
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (MSVC)
        target_compile_options(konduit_installer PRIVATE "/Od" "/Zi" "/RTC1")
        target_link_options(konduit_installer PRIVATE "/DEBUG" "/INCREMENTAL")
        #        target_link_options(konduit_installer PRIVATE "/SUBSYSTEM:CONSOLE")
    else ()
        #        target_link_options(konduit_installer PRIVATE -mconsole -e main)
        target_link_options(konduit_installer PRIVATE -Wl,--Map=${CMAKE_PROJECT_NAME}.map)
    endif ()
endif ()
