#ifndef KONDUIT_INSTALLER_MAIN_HPP
#define KONDUIT_INSTALLER_MAIN_HPP

#include <raylib/rres.h>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <format>
#include "clayman.hpp"
#include "raylib.h"

extern ClayMan* g_clayManInstance;

ClayMan& getGlobalClayManInstance();

#define clay getGlobalClayManInstance()

extern bool drag_window;
extern bool should_close;
extern Font fonts[3];

constexpr Clay_Color MAIN_COLOR = {115, 165, 88, 255};
constexpr Clay_Color MAIN_DARK = {60, 60, 60, 255};
constexpr Clay_Color BACKGROUND_MAIN = {30, 30, 30, 230};
constexpr Clay_Color TRANSPARENT = {0, 0, 0, 0};
constexpr Clay_Color K_BLACK = {0, 0, 0, 255};
constexpr Clay_Color K_WHITE = {250, 230, 230, 255};
constexpr Clay_Color ERROR = {200, 100, 100, 255};
constexpr Clay_Color TEXT_DARK = {50, 50, 50, 255};
constexpr Clay_Color TEXT_GRAY = {160, 160, 160, 200};
constexpr Clay_Color BORDER_GRAY = {80, 80, 80, 255};
constexpr Clay_Color BORDER_LIGHT = {100, 100, 100, 255};
constexpr Clay_Color SECONDARY_BG = {40, 40, 40, 230};

constexpr int FONT_SIZE_24_ID = 0;
constexpr int FONT_SIZE_18_ID = 1;
// constexpr int FONT_FA_ICONS_ID = 2;
//  constexpr int FONT_SIZE_16_ID = 2;

#endif  // KONDUIT_INSTALLER_MAIN_HPP
