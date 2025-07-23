#ifndef KONDUIT_INSTALLER_EMBED_H
#define KONDUIT_INSTALLER_EMBED_H

#include <stddef.h>

// the #embeds are done in extern couse static embeds brick most C intelisense
// providers

extern const unsigned char roboto_font[];
extern const size_t roboto_font_size;

extern const unsigned char fa_icon_font[];
extern const size_t fa_icon_font_size;

#endif  // KONDUIT_INSTALLER_EMBED_H
