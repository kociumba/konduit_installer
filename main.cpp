#include "main.hpp"
#include "components.hpp"
#include "include/raylib/clay_renderer_raylib.h"

ClayMan* g_clayManInstance = nullptr;

ClayMan& getGlobalClayManInstance() {
    assert(g_clayManInstance != nullptr && "ClayMan instance not initialized");
    return *g_clayManInstance;
}

#define clay getGlobalClayManInstance()

Vector2 mousePosition = {0, 0};
Vector2 windowPosition = {0, 0};
Vector2 windowSize = {600, 400};
Vector2 panOffset = mousePosition;
bool drag_window = false;
bool should_close = false;
bool debug = false;
unsigned int window_flags = FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT |
                            FLAG_WINDOW_TRANSPARENT | FLAG_VSYNC_HINT;

struct Install_data {
    std::string input_buffer;
    std::string install_path;
    void set_install_path(const std::string& path) {
        validation = validate_path(path);
        install_path = path;
    }
    DirectoryValidationResult validation;
    bool test_toggle = false;

    bool radio_test1 = false;
    bool radio_test2 = false;

    std::string temp_path;

    bool show_popup = false;

    std::string popup_input_buffer;
} data;

void drag() {
#if defined(_WIN32)
    mousePosition = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !drag_window) {
        windowPosition = GetWindowPosition();
        panOffset = mousePosition;
        drag_window = true;
    }

    if (drag_window) {
        windowPosition.x += (mousePosition.x - panOffset.x);
        windowPosition.y += (mousePosition.y - panOffset.y);
        SetWindowPosition((int)windowPosition.x, (int)windowPosition.y);

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            drag_window = false;
        }
    }
#endif
}

void input() {
    if (IsKeyPressed(KEY_F3)) {
        debug = !debug;
    }
    if (IsKeyPressed(KEY_F11)) {
        Clay_SetDebugModeEnabled(!Clay_IsDebugModeEnabled());
    }
}

void debug_ui() {
    auto debug_string =
        std::format("FPS {} | GL version: {}", GetFPS(), rlGetVersion());
    clay.element(
        {
            .id = clay.hashID("debug_container"),
            .layout =
                {.sizing =
                     {.width = {.type = CLAY__SIZING_TYPE_FIT},
                      .height = {.size{.minMax = {.min = 35, .max = 35}}}},
                 .padding = clay.padAll(8),
                 .childGap = 16,
                 .layoutDirection = CLAY_LEFT_TO_RIGHT},
            .backgroundColor = K_BLACK,
            .cornerRadius = {0, 0, 10, 0},
            .floating =
                {.offset = {1, windowSize.y - 36},
                 .attachTo = CLAY_ATTACH_TO_PARENT},
            .border = {.color = K_WHITE, .width = {1, 1, 1, 1, 0}},
        },
        [&] {
            clay.textElement(
                debug_string,
                {.textColor = K_WHITE,
                 .fontId = FONT_SIZE_18_ID,
                 .fontSize = 18}
            );
        }
    );
}

void installer_ui() {
    clay.element(
        {.id = clay.hashID("installer_root"),
         .layout =
             {.sizing = clay.expandXY(),
              .padding = clay.padAll(12),
              .childGap = 12,
              .layoutDirection = CLAY_TOP_TO_BOTTOM},
         .backgroundColor = BACKGROUND_MAIN,
         .cornerRadius = {10, 10, 10, 10},
         .border = {.color = MAIN_COLOR, .width = {1, 1, 1, 1}}},
        [&] {
            clay.textElement(
                "Installer",
                {.textColor = K_WHITE,
                 .fontId = FONT_SIZE_24_ID,
                 .fontSize = 24,
                 .textAlignment = CLAY_TEXT_ALIGN_CENTER}
            );

            clay.element(
                {.id = clay.hashID("content_area"),
                 .layout =
                     {.sizing = clay.expandXY(),
                      .padding = clay.padAll(8),
                      .childGap = 8,
                      .layoutDirection = CLAY_TOP_TO_BOTTOM},
                 .backgroundColor = SECONDARY_BG,
                 .cornerRadius = {6, 6, 6, 6},
                 .border = {.color = BORDER_GRAY, .width = {1, 1, 1, 1}}},
                [&] {
                    progress_bar(fmod(GetTime(), 2.0) / 2.0, "progress_test");
                    checkbox("toggle", &data.test_toggle);
                    radio_selection(
                        "one of these",
                        {{"first", &data.radio_test1},
                         {"second", &data.radio_test2}}
                    );
                    clay.textElement(
                        std::format(
                            "Select directory: \"{}\"", data.install_path
                        ),
                        {.textColor = K_WHITE,
                         .fontId = FONT_SIZE_18_ID,
                         .fontSize = 18}
                    );
                    if (text_input("~/dir", &data.input_buffer, {200, 30})) {
                        data.set_install_path(data.input_buffer);
                        data.input_buffer.clear();
                    }
                    if (!data.validation.usable) {
                        std::string reason;
                        if (!data.validation.exists_and_is_dir) {
                            reason = "it does not exist or is not a directory";
                        } else if (!data.validation.empty_initially) {
                            reason = "it is not empty";
                        } else {
                            reason = "unknown reason";
                        }
                        std::string err = std::format(
                            "the selected path cannot be used: {}\n(err: "
                            "{})",
                            reason,
                            data.validation.error_message
                        );
                        clay.textElement(
                            err,
                            {
                                .textColor = ERROR,
                                .fontId = FONT_SIZE_18_ID,
                                .fontSize = 18,
                            }
                        );
                    }
                    if (button("Browse")) {
                        char* p = tinyfd_selectFolderDialog(
                            "select the installation folder", nullptr
                        );
                        if (p != nullptr) {
                            data.set_install_path(p);
                        }
                    }
                }
            );

            clay.element(
                {.id = clay.hashID("control_buttons"),
                 .layout =
                     {.sizing = clay.expandXfixedY(40),
                      .padding = clay.padAll(4),
                      .childGap = 8,
                      .layoutDirection = CLAY_LEFT_TO_RIGHT},
                 .backgroundColor = TRANSPARENT},
                [&] {
                    clay.element(
                        {.id = clay.hashID("filler"),
                         .layout = {.sizing = clay.expandX()}}
                    );
                    if (button("Cancel")) {
                        should_close = true;
                    }
                    if (button("Next")) {
                        data.temp_path =
                            write_to_temp_file(test_data, test_size);
                        info(data.temp_path.c_str());
                        auto t = load_resource(data.temp_path);
                        data.show_popup = true;
                    }
                    popup("destination selection", &data.show_popup, [&] {
                        text_input("dummy input", &data.popup_input_buffer);
                    });
                }
            );
        }
    );
}

void ui() {
    // only used for corner rounding
    clay.element(
        Clay_ElementDeclaration{
            .id = clay.hashID("root_container"),
            .layout =
                {.sizing = clay.expandXY(),
                 .padding = clay.padAll(0),
                 .childGap = 0,
                 .layoutDirection = CLAY_LEFT_TO_RIGHT},
            .backgroundColor = TRANSPARENT,
            .cornerRadius = {10, 10, 10, 10},
            .border = {.color = TRANSPARENT, .width = {1, 1, 1, 1, 0}},
        },
        [&] {
            installer_ui();
            if (debug) {
                debug_ui();
            }
        }
    );
}

Font fonts[3];
std::vector<uint32_t> icon_codepoints = {
    codepoint(ICON_FA_FOLDER),
    codepoint(ICON_FA_EYE),
};

int main() {
    g_clayManInstance =
        new ClayMan(windowSize.x, windowSize.y, Raylib_MeasureText, fonts);

#ifdef _WIN32
    window_flags = window_flags | FLAG_WINDOW_UNDECORATED;
#endif

    Clay_Raylib_Initialize(
        clay.getWindowWidth(),
        clay.getWindowHeight(),
        "... installer",
        window_flags
    );

    fonts[0] = LoadFontFromMemory(
        ".ttf", roboto_regular_data, roboto_regular_size, 24, nullptr, 400
    );
    GenTextureMipmaps(&fonts[0].texture);
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_TRILINEAR);
    fonts[1] = LoadFontFromMemory(
        ".ttf", roboto_regular_data, roboto_regular_size, 18, nullptr, 400
    );
    GenTextureMipmaps(&fonts[1].texture);
    SetTextureFilter(fonts[1].texture, TEXTURE_FILTER_TRILINEAR);
    fonts[2] = LoadFontFromMemory(
        ".ttf",
        fa_regular_400_data,
        fa_regular_400_size,
        24,
        (int*)(icon_codepoints.data()),
        icon_codepoints.size()
    );
    GenTextureMipmaps(&fonts[2].texture);
    SetTextureFilter(fonts[2].texture, TEXTURE_FILTER_TRILINEAR);

    //    std::printf("eye: U+%04X\n", codepoint(ICON_FA_EYE));

    //    ExportImage(LoadImageFromTexture(fonts[2].texture), "fa_font_18.png");

    auto m = GetCurrentMonitor();
    auto m_w = GetMonitorWidth(m);
    auto m_h = GetMonitorHeight(m);

    windowPosition.x = (m_w - clay.getWindowWidth()) / 2;
    windowPosition.y = (m_h - clay.getWindowHeight()) / 2;

    SetWindowPosition(
        windowPosition.x, windowPosition.y
    );  // glazewm still breaks this

    while (!WindowShouldClose() && !should_close) {
        drag();
        input();
        Vector2 mousePosition = GetMousePosition();
        Vector2 scrollDelta = GetMouseWheelMoveV();

        clay.updateClayState(
            GetScreenWidth(),
            GetScreenHeight(),
            mousePosition.x,
            mousePosition.y,
            scrollDelta.x,
            scrollDelta.y,
            GetFrameTime(),
            IsMouseButtonDown(0)
        );

        clay.beginLayout();
        ui();
        Clay_RenderCommandArray renderCommands = clay.endLayout();

        BeginDrawing();
        ClearBackground(BLANK);
        Clay_Raylib_Render(renderCommands, fonts);
        EndDrawing();
    }

    remove_all_temp_files();
    delete g_clayManInstance;
    for (const auto& font : fonts) {
        UnloadFont(font);
    }
    Clay_Raylib_Close();
    return 0;
}
