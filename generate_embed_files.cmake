get_filename_component(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets" ABSOLUTE)
set(GENERATED_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/embed.h")
set(GENERATED_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/embed.c")
set(INPUT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/embed.h.in")
set(INPUT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/embed.c.in")

file(GLOB_RECURSE ASSET_FILES "${ASSETS_DIR}/*")

set(EMBED_HEADERS "")
set(EMBED_SOURCES "")

set(BUNDLE_FILE "test_assets/test.rres" CACHE FILEPATH "Bundle path")

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
