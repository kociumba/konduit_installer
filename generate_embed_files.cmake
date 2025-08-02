get_filename_component(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets" ABSOLUTE)
set(GENERATED_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/embed.h")
set(GENERATED_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/embed.c")
set(INPUT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/embed.h.in")
set(INPUT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/embed.c.in")

file(GLOB_RECURSE ASSET_FILES "${ASSETS_DIR}/*")

set(EMBED_LIST_CURRENT "${CMAKE_CURRENT_BINARY_DIR}/embedded_files")
set(EMBED_LIST_CACHED "${CMAKE_CURRENT_BINARY_DIR}/embedded_files.cache")

set(BUNDLE_FILE "test_assets/GxOGUPjW4AMjYXV.zip" CACHE FILEPATH "Bundle path")

file(WRITE "${EMBED_LIST_CURRENT}" "")
foreach (ASSET_FILE ${ASSET_FILES})
    if (NOT IS_DIRECTORY "${ASSET_FILE}")
        file(APPEND "${EMBED_LIST_CURRENT}" "${ASSET_FILE}\n")
    endif ()
endforeach ()

if (BUNDLE_FILE AND EXISTS "${BUNDLE_FILE}" AND NOT IS_DIRECTORY "${BUNDLE_FILE}")
    file(APPEND "${EMBED_LIST_CURRENT}" "${BUNDLE_FILE}\n")
endif ()

if (EXISTS "${EMBED_LIST_CURRENT}" AND EXISTS "${EMBED_LIST_CACHED}")
    file(SHA256 "${EMBED_LIST_CURRENT}" CURRENT_HASH)
    file(SHA256 "${EMBED_LIST_CACHED}" CACHED_HASH)
    if (NOT "${CURRENT_HASH}" STREQUAL "${CACHED_HASH}")
        set(SHOULD_REGENERATE_EMBED TRUE)
    else ()
        set(SHOULD_REGENERATE_EMBED FALSE)
    endif ()
else ()
    set(SHOULD_REGENERATE_EMBED TRUE)
endif ()

function(embed_file FILE_PATH)
    get_filename_component(NAME_WE "${FILE_PATH}" NAME_WE)
    string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" C_IDENTIFIER "${NAME_WE}")
    string(TOLOWER "${C_IDENTIFIER}" C_IDENTIFIER)

    set(_headers "extern const unsigned char ${C_IDENTIFIER}_data[];\nextern const size_t ${C_IDENTIFIER}_size;\n\n")
    set(_sources "const unsigned char ${C_IDENTIFIER}_data[] = {\n#embed \"${FILE_PATH}\"\n};\nconst size_t ${C_IDENTIFIER}_size = sizeof(${C_IDENTIFIER}_data);\n\n")

    string(APPEND EMBED_HEADERS "${_headers}")
    string(APPEND EMBED_SOURCES "${_sources}")

    set(EMBED_HEADERS "${EMBED_HEADERS}" PARENT_SCOPE)
    set(EMBED_SOURCES "${EMBED_SOURCES}" PARENT_SCOPE)
endfunction()

if (SHOULD_REGENERATE_EMBED)
    message(STATUS "Regeneration required")

    set(EMBED_HEADERS "")
    set(EMBED_SOURCES "")

    foreach (ASSET_FILE ${ASSET_FILES})
        if (NOT IS_DIRECTORY "${ASSET_FILE}")
            embed_file("${ASSET_FILE}")
        endif ()
    endforeach ()

    if (BUNDLE_FILE AND EXISTS "${BUNDLE_FILE}" AND NOT IS_DIRECTORY "${BUNDLE_FILE}")
        message(STATUS "Embedding bundle file: ${BUNDLE_FILE}")
        embed_file("${BUNDLE_FILE}")
    endif ()

    configure_file("${INPUT_HEADER}" "${GENERATED_HEADER}" @ONLY)
    configure_file("${INPUT_SOURCE}" "${GENERATED_SOURCE}" @ONLY)

    file(COPY "${EMBED_LIST_CURRENT}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    file(RENAME "${CMAKE_CURRENT_BINARY_DIR}/embedded_files" "${EMBED_LIST_CACHED}")
endif ()