get_filename_component(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets" ABSOLUTE)
set(GENERATED_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/embed.h")
set(GENERATED_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/embed.c")
set(INPUT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/embed.h.in")
set(INPUT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/embed.c.in")

file(GLOB_RECURSE ASSET_FILES "${ASSETS_DIR}/*")

set(EMBED_HEADERS "")
set(EMBED_SOURCES "")

foreach (ASSET_FILE ${ASSET_FILES})
    if (NOT IS_DIRECTORY "${ASSET_FILE}")
        get_filename_component(NAME_WE ${ASSET_FILE} NAME_WE)
        string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" C_IDENTIFIER "${NAME_WE}")
        string(TOLOWER "${C_IDENTIFIER}" C_IDENTIFIER)

        string(APPEND EMBED_HEADERS
                "extern const unsigned char ${C_IDENTIFIER}_data[];\n"
                "extern const size_t ${C_IDENTIFIER}_size;\n\n"
        )

        string(APPEND EMBED_SOURCES
                "const unsigned char ${C_IDENTIFIER}_data[] = {\n"
                "#embed \"${ASSET_FILE}\"\n"
                "};\n"
                "const size_t ${C_IDENTIFIER}_size = sizeof(${C_IDENTIFIER}_data);\n\n"
        )
    endif ()
endforeach ()

configure_file("${INPUT_HEADER}" "${GENERATED_HEADER}" @ONLY)
configure_file("${INPUT_SOURCE}" "${GENERATED_SOURCE}" @ONLY)
