set(_SCRIPT_DIR "${CMAKE_CURRENT_LIST_DIR}")

get_filename_component(ASSETS_DIR "${_SCRIPT_DIR}/assets" ABSOLUTE)
set(GENERATED_HEADER "${_SCRIPT_DIR}/embed.h")
set(GENERATED_SOURCE "${_SCRIPT_DIR}/embed.c")
set(INPUT_HEADER "${_SCRIPT_DIR}/embed.h.in")
set(INPUT_SOURCE "${_SCRIPT_DIR}/embed.c.in")

file(GLOB_RECURSE ASSET_FILES "${ASSETS_DIR}/*")

set(EMBED_LIST_CURRENT "${CMAKE_CURRENT_BINARY_DIR}/embedded_files")
set(EMBED_LIST_CACHED "${CMAKE_CURRENT_BINARY_DIR}/embedded_files.cache")

set(BUNDLE_FILE "${_SCRIPT_DIR}/test_assets/GxOGUPjW4AMjYXV.zip" CACHE FILEPATH "Bundle path")

set(ALL_EMBED_FILES)
foreach (ASSET_FILE ${ASSET_FILES})
    if (NOT IS_DIRECTORY "${ASSET_FILE}")
        list(APPEND ALL_EMBED_FILES "${ASSET_FILE}")
    endif ()
endforeach ()

if (BUNDLE_FILE AND EXISTS "${BUNDLE_FILE}" AND NOT IS_DIRECTORY "${BUNDLE_FILE}")
    list(APPEND ALL_EMBED_FILES "${BUNDLE_FILE}")
endif ()

file(WRITE "${EMBED_LIST_CURRENT}" "")
foreach (EMBED_FILE ${ALL_EMBED_FILES})
    file(APPEND "${EMBED_LIST_CURRENT}" "${EMBED_FILE}\n")
endforeach ()

set(SHOULD_REGENERATE_EMBED FALSE)

if (NOT EXISTS "${GENERATED_HEADER}" OR NOT EXISTS "${GENERATED_SOURCE}")
    message(STATUS "Generated embed files missing, regeneration required")
    set(SHOULD_REGENERATE_EMBED TRUE)
elseif (EXISTS "${EMBED_LIST_CACHED}")
    file(SHA256 "${EMBED_LIST_CURRENT}" CURRENT_HASH)
    file(SHA256 "${EMBED_LIST_CACHED}" CACHED_HASH)
    if (NOT "${CURRENT_HASH}" STREQUAL "${CACHED_HASH}")
        message(STATUS "Embed file list changed, regeneration required")
        set(SHOULD_REGENERATE_EMBED TRUE)
    endif ()
else ()
    message(STATUS "Cache file missing, regeneration required")
    set(SHOULD_REGENERATE_EMBED TRUE)
endif ()

function(embed_file FILE_PATH)
    get_filename_component(NAME_WE "${FILE_PATH}" NAME_WE)
    string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" C_IDENTIFIER "${NAME_WE}")
    string(TOLOWER "${C_IDENTIFIER}" C_IDENTIFIER)

    get_filename_component(_ext "${FILE_PATH}" EXT)

    set(_headers "extern const unsigned char ${C_IDENTIFIER}_data[];\nextern const size_t ${C_IDENTIFIER}_size;\nextern const char ${C_IDENTIFIER}_ext[];\n\n")
    set(_sources "const unsigned char ${C_IDENTIFIER}_data[] = {\n#embed \"${FILE_PATH}\"\n};\nconst size_t ${C_IDENTIFIER}_size = sizeof(${C_IDENTIFIER}_data);\nconst char ${C_IDENTIFIER}_ext[] = \"${_ext}\";\n\n")

    string(APPEND EMBED_HEADERS "${_headers}")
    string(APPEND EMBED_SOURCES "${_sources}")

    set(EMBED_HEADERS "${EMBED_HEADERS}" PARENT_SCOPE)
    set(EMBED_SOURCES "${EMBED_SOURCES}" PARENT_SCOPE)
endfunction()

if (SHOULD_REGENERATE_EMBED)
    message(STATUS "Regenerating embed files...")

    set(EMBED_HEADERS "")
    set(EMBED_SOURCES "")

    foreach (EMBED_FILE ${ALL_EMBED_FILES})
        message(STATUS "Embedding file: ${EMBED_FILE}")
        embed_file("${EMBED_FILE}")
    endforeach ()

    configure_file("${INPUT_HEADER}" "${GENERATED_HEADER}" @ONLY)
    configure_file("${INPUT_SOURCE}" "${GENERATED_SOURCE}" @ONLY)

    file(COPY "${EMBED_LIST_CURRENT}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    file(RENAME "${CMAKE_CURRENT_BINARY_DIR}/embedded_files" "${EMBED_LIST_CACHED}")

    message(STATUS "Embed file generation complete")
else ()
    message(STATUS "Embed files up to date, skipping regeneration")
endif ()