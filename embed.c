#include "embed.h"

const unsigned char roboto_font[] = {
#embed "assets/Roboto-Regular.ttf"
};
const size_t roboto_font_size = sizeof(roboto_font);

const unsigned char fa_icon_font[] = {
#embed "assets/fa-regular-400.ttf"
};
const size_t fa_icon_font_size = sizeof(fa_icon_font);