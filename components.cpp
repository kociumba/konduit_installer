#include "components.hpp"

bool button(std::string text, Vector2 size) {
    auto name = std::format("{}_button", text);
    clay.element(
        {.id = clay.hashID(name),
         .layout =
             {
                 .sizing = clay.fixedSize(size.x, size.y),
                 .childAlignment =
                     {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER},
             },
         .backgroundColor = clay.pointerOver(name) ? MAIN_COLOR : MAIN_DARK,
         .cornerRadius = {4, 4, 4, 4},
         .border =
             {.color = clay.pointerOver(name) ? BORDER_LIGHT : BORDER_GRAY,
              .width = {1, 1, 1, 1}}},
        [&] {
            clay.textElement(
                text,
                {.textColor = K_WHITE,
                 .fontId = FONT_SIZE_18_ID,
                 .fontSize = 18}
            );
        }
    );
    if (clay.mousePressed() && clay.pointerOver(name)) {
        drag_window = false;
        return true;
    }
    return false;
}

std::map<std::string, bool> active;

struct InputState {
    std::string name;
    Vector2 size;
    std::string* input;
    size_t cursor_pos;
    bool cursor_moving;
    double last_blink;
    bool blink;
};

std::map<std::string, InputState> states;

Debouncer backspace_debouncer(200, 25);
Debouncer left_debouncer(200, 25);
Debouncer right_debouncer(200, 25);

bool text_input(std::string label, std::string* input, Vector2 size) {
    auto name = std::format("{}_input", label);
    auto& state = states[name];

    if (state.name != name) {
        state.name = name;
        state.size = size;
        state.input = input;
        state.cursor_moving = false;
        state.cursor_pos = input->length();
        state.last_blink = GetTime();
        state.blink = true;
    }

    std::string text_to_cursor = input->substr(0, state.cursor_pos);
    Vector2 text_size =
        MeasureTextEx(fonts[FONT_SIZE_18_ID], (text_to_cursor).c_str(), 18, 0);

    float scroll_offset = 0.0f;
    if (text_size.x > state.size.x - 8) {
        scroll_offset = -(text_size.x - (state.size.x - 8));
    }
    if (scroll_offset > 0)
        scroll_offset = 0;

    clay.element(
        {
            .id = clay.hashID(std::format("{}_clip_container", name)),
            .layout = {.sizing = clay.fixedSize(state.size.x, state.size.y)},
            .clip = {.horizontal = true, .vertical = true},
        },
        [&] {
            clay.element(
                {
                    .id = clay.hashID(name),
                    .layout =
                        {
                            .sizing =
                                clay.fixedSize(state.size.x, state.size.y),
                            .padding = clay.padLeft(4),
                            .childAlignment =
                                {.x = CLAY_ALIGN_X_LEFT,
                                 .y = CLAY_ALIGN_Y_CENTER},
                        },
                    .backgroundColor = TEXT_DARK,
                    .cornerRadius = {4, 4, 4, 4},
                    .clip =
                        {
                            .horizontal = true,
                            .vertical = true,
                        },
                    .border =
                        {.color = active[name]
                                      ? MAIN_COLOR
                                      : (clay.pointerOver(name) ? BORDER_LIGHT
                                                                : BORDER_GRAY),
                         .width = {1, 1, 1, 1, 0}},
                },
                [&] {
                    if (active[name]) {
                        double currentTime = GetTime();
                        if (currentTime - state.last_blink >= 0.3) {
                            state.blink = !state.blink;
                            state.last_blink = currentTime;
                        }

                        clay.element({
                            .id = clay.hashID(std::format("{}_cursor", name)),
                            .layout = {.sizing = clay.fixedSize(2, 18)},
                            .backgroundColor =
                                state.cursor_moving
                                    ? K_WHITE
                                    : (state.blink ? TRANSPARENT : K_WHITE),
                            .cornerRadius = {1, 1, 1, 1},
                            .floating = {
                                .offset =
                                    {text_size.x + 4 + scroll_offset,
                                     (state.size.y - 18) / 2},
                                .parentId = clay.hashID(name).id,
                                .zIndex = 1,
                                .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                                .clipTo = CLAY_CLIP_TO_ATTACHED_PARENT
                            },
                        });
                    }
                    clay.element(
                        {
                            .id = clay.hashID(std::format("{}_text", name)),
                            .floating =
                                {.offset =
                                     {scroll_offset + 4,
                                      (state.size.y - 18) / 2},
                                 .parentId = clay.hashID(name).id,
                                 .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                                 .clipTo = CLAY_CLIP_TO_ATTACHED_PARENT},
                        },
                        [&] {
                            clay.textElement(
                                input->empty() ? label : *input,
                                {.textColor =
                                     input->empty() ? TEXT_GRAY : K_WHITE,
                                 .fontId = FONT_SIZE_18_ID,
                                 .fontSize = 18}
                            );
                        }
                    );
                }
            );
        }
    );

    if (clay.pointerOver(name) && clay.mousePressed()) {
        active[name] = true;
        drag_window = false;
        state.cursor_pos = input->length();
        state.blink = false;
        state.last_blink = GetTime();
    }
    if (!clay.pointerOver(name) && clay.mousePressed()) {
        active[name] = false;
    }

    if (active[name]) {
        auto key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                input->insert(state.cursor_pos, 1, (char)key);
                state.cursor_pos++;
            }
            key = GetCharPressed();
        }
    }

    if (IsKeyDown(KEY_BACKSPACE) && state.cursor_pos > 0) {
        backspace_debouncer.throttle([&] {
            input->erase(state.cursor_pos - 1, 1);
            state.cursor_pos--;
        });
    }
    if (IsKeyReleased(KEY_BACKSPACE)) {
        backspace_debouncer.reset();
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (IsKeyPressed(KEY_BACKSPACE) && state.cursor_pos > 0) {
            size_t lastSpace =
                input->find_last_not_of(" \t\n\r", state.cursor_pos - 1);
            if (lastSpace != std::string::npos) {
                lastSpace = input->find_last_of(" \t\n\r", lastSpace);
                if (lastSpace != std::string::npos) {
                    input->erase(
                        lastSpace + 1, state.cursor_pos - (lastSpace + 1)
                    );
                    state.cursor_pos = lastSpace + 1;
                } else {
                    input->erase(0, state.cursor_pos);
                    state.cursor_pos = 0;
                }
            } else {
                input->clear();
                state.cursor_pos = 0;
            }
        }
    }

    if (IsKeyDown(KEY_LEFT) && state.cursor_pos > 0) {
        state.cursor_moving = true;
        left_debouncer.throttle([&] { state.cursor_pos--; });
    }
    if (IsKeyReleased(KEY_LEFT)) {
        state.cursor_moving = false;
        left_debouncer.reset();
    }

    if (IsKeyDown(KEY_RIGHT) && state.cursor_pos < input->length()) {
        state.cursor_moving = true;
        right_debouncer.throttle([&] { state.cursor_pos++; });
    }
    if (IsKeyReleased(KEY_RIGHT)) {
        state.cursor_moving = false;
        right_debouncer.reset();
    }

    if (active[name] && IsKeyPressed(KEY_ENTER)) {
        active[name] = false;
        return true;
    }

    return false;
}

void progress_bar(
    float percent,
    std::string id,
    ProgressItems items,
    Vector2 size
) {
    auto name = std::format("{}_progress", id);
    auto p = std::round(percent * 100);

    clay.element(
        {
            .id = clay.hashID(name),
            .layout =
                {.sizing =
                     (size.x == -1 ? clay.expandXfixedY(size.y)
                                   : clay.fixedSize(size.x, size.y)),
                 .childAlignment =
                     {.x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER}},
            .backgroundColor = TEXT_DARK,
            .cornerRadius = {4, 4, 4, 4},
            .clip = {.horizontal = true, .vertical = true},
            .border = {.color = BORDER_GRAY, .width = {1, 1, 1, 1, 0}},
        },
        [&] {
            clay.element(
                {
                    .id = clay.hashID(std::format("{}_inner", name)),
                    .layout =
                        {.sizing =
                             {.width =
                                  {.size = {.percent = percent},
                                   .type = CLAY__SIZING_TYPE_PERCENT},
                              .height =
                                  {.size =
                                       {.minMax =
                                            {.min = size.y, .max = size.y}}}}},
                    .backgroundColor = MAIN_COLOR,
                    .cornerRadius = {4, 4, 4, 4},
                },
                [&] {}
            );
            auto info = std::format("{}%", p);
            if (items.done != -1 || items.all != -1) {
                info = std::format("{} [{}/{}]", info, items.done, items.all);
            }
            auto info_size =
                MeasureTextEx(fonts[FONT_SIZE_18_ID], info.c_str(), 18, 0);
            auto data = Clay_GetElementData(clay.hashID(name));
            clay.element(
                {.id = clay.hashID(std::format("{}_text", name)),
                 .floating =
                     {.offset =
                          {.x = (data.boundingBox.width - info_size.x) / 2,
                           .y = (data.boundingBox.height - info_size.y) / 2},
                      .parentId = clay.hashID(name).id,
                      .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID}},
                [&] {
                    clay.textElement(
                        info,
                        {.textColor = K_WHITE,
                         .fontId = FONT_SIZE_18_ID,
                         .fontSize = 18}
                    );
                }
            );
        }
    );
}

bool checkbox(std::string label, bool* toggle) {
    auto name = std::format("{}_checkbox", label);
    auto clickable_name = std::format("{}_clickable", name);

    clay.element(
        {
            .id = clay.hashID(name),
            .layout =
                {.childGap = 8,
                 .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                 .layoutDirection = CLAY_LEFT_TO_RIGHT},
        },
        [&] {
            clay.element(
                {
                    .id = clay.hashID(clickable_name),
                    .layout =
                        {.sizing = clay.fixedSize(30, 30),
                         .childAlignment =
                             {.x = CLAY_ALIGN_X_CENTER,
                              .y = CLAY_ALIGN_Y_CENTER}},
                    .backgroundColor = clay.pointerOver(clickable_name)
                                           ? MAIN_DARK
                                           : TEXT_DARK,
                    .cornerRadius = {4, 4, 4, 4},
                    .border =
                        {.color = clay.pointerOver(clickable_name)
                                      ? BORDER_LIGHT
                                      : BORDER_GRAY,
                         .width = {1, 1, 1, 1, 0}},
                },
                [&] {
                    clay.element(
                        {
                            .id = clay.hashID(
                                std::format("{}_clicked", clickable_name)
                            ),
                            .layout = {.sizing = clay.fixedSize(20, 20)},
                            .backgroundColor =
                                *toggle ? MAIN_COLOR : TRANSPARENT,
                            .cornerRadius = {4, 4, 4, 4},
                        },
                        [&] {}
                    );
                }
            );
            clay.textElement(
                label,
                {.textColor = K_WHITE,
                 .fontId = FONT_SIZE_18_ID,
                 .fontSize = 18}
            );
        }
    );

    if (clay.pointerOver(clickable_name) && clay.mousePressed()) {
        *toggle = !*toggle;
        return true;
    }

    return false;
}