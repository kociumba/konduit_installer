#ifndef KONDUIT_INSTALLER_EMBED_H
#define KONDUIT_INSTALLER_EMBED_H

#include <stddef.h>

#ifndef __has_embed
#define TO_STRING(x) #x
#define STRINGIFY(x) TO_STRING(x)

#if defined(__clang__)
#define COMPILER_INFO                                  \
    "Clang " STRINGIFY(__clang_major__) "." STRINGIFY( \
        __clang_minor__                                \
    ) "." STRINGIFY(__clang_patchlevel__)
#elif defined(__GNUC__)
#define COMPILER_INFO                                                       \
    "GCC " STRINGIFY(__GNUC__) "." STRINGIFY(__GNUC_MINOR__) "." STRINGIFY( \
        __GNUC_PATCHLEVEL__                                                 \
    )
#elif defined(_MSC_VER)
#define COMPILER_INFO "MSVC " STRINGIFY(_MSC_FULL_VER)
#else
#define COMPILER_INFO "Unknown compiler"
#endif

#pragma message(                                                     \
    "konduit requires your compiler to support c23 #embed. "         \
    "Your compiler is: " COMPILER_INFO ", which does not support it" \
)

#error your compiler is insufficient for konduit, see message above for more details (message only displayed if your compiler supports #pragma message)
#endif

// the #embeds are done in extern couse static embeds brick most C intelisense
// providers

extern const unsigned char roboto_font[];
extern const size_t roboto_font_size;

extern const unsigned char fa_icon_font[];
extern const size_t fa_icon_font_size;

#endif  // KONDUIT_INSTALLER_EMBED_H
