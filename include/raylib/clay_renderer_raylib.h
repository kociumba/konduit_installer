#ifndef KONDUIT_INSTALLER_CLAY_RENDERER_RAYLIB_H
#define KONDUIT_INSTALLER_CLAY_RENDERER_RAYLIB_H

#include "../clay.h"
#include "raylib.h"
#include "raymath.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

Clay_Dimensions Raylib_MeasureText(
    Clay_StringSlice text,
    Clay_TextElementConfig* config,
    void* userData
);

void Clay_Raylib_Initialize(
    int width,
    int height,
    const char* title,
    unsigned int flags
);

void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands, Font* fonts);

void Clay_Raylib_Close();

#ifdef __cplusplus
}
#endif

#endif  // KONDUIT_INSTALLER_CLAY_RENDERER_RAYLIB_H
