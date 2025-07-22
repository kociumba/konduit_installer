#include "components.hpp"

bool button(std::string text, Vector2 size) {
    std::string name = std::format("{}_button", text);
    clay.element(
        {.id = clay.hashID(name),
         .layout =
             {
                 .sizing = clay.fixedSize(size.x, size.y),
                 .childAlignment =
                     {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER},
             },
         .backgroundColor = color_transition(
             name + "_bg", clay.pointerOver(name), MAIN_COLOR, MAIN_DARK
         ),
         .cornerRadius = {4, 4, 4, 4},
         .border =
             {.color = color_transition(
                  name, clay.pointerOver(name), BORDER_LIGHT, BORDER_GRAY
              ),
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

static std::map<std::string, bool> active;

static std::map<std::string, InputState> states;

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
        state.selected = false;
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
                    .backgroundColor = color_transition(
                        name + "_bg",
                        {{active[name], MAIN_DARK},
                         {clay.pointerOver(name), MAIN_DARK}},
                        TEXT_DARK
                    ),
                    .cornerRadius = {4, 4, 4, 4},
                    .clip =
                        {
                            .horizontal = true,
                            .vertical = true,
                        },
                    .border =
                        {.color = color_transition(
                             name,
                             {{active[name], MAIN_COLOR},
                              {clay.pointerOver(name), BORDER_LIGHT}},
                             BORDER_GRAY
                         ),
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
                            .layout = {.sizing = clay.fixedSize(1, 18)},
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
                            .backgroundColor = color_transition(
                                name + "_text",
                                state.selected,
                                MAIN_COLOR,
                                TRANSPARENT,
                                0.1
                            ),
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
        state.selected = false;
        active[name] = false;
    }

    if (active[name]) {
        auto key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                state.selected = false;
                input->insert(state.cursor_pos, 1, (char)key);
                state.cursor_pos++;
            }
            key = GetCharPressed();
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE) && state.selected) {
        input->clear();
        state.cursor_pos = 0;
        state.selected = false;
    }
    if (IsKeyDown(KEY_BACKSPACE) && state.cursor_pos > 0) {
        state.selected = false;
        debounce_action(name + "backspace", [&] {
            input->erase(state.cursor_pos - 1, 1);
            state.cursor_pos--;
        });
    }
    if (IsKeyReleased(KEY_BACKSPACE)) {
        reset_debounce(name + "backspace");
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (IsKeyPressed(KEY_BACKSPACE) && state.cursor_pos > 0) {
            state.selected = false;
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

        if (IsKeyPressed(KEY_A) && !input->empty()) {
            state.selected = true;
            state.cursor_pos = input->length();
        }

        if (IsKeyPressed(KEY_C) && state.selected) {
            SetClipboardText(input->c_str());
            state.selected = false;
        }

        if (IsKeyPressed(KEY_V) && state.selected) {
            auto t = GetClipboardText();
            if (t) {
                state.cursor_pos =
                    std::clamp(state.cursor_pos, (size_t)0, strlen(t));
                *input = t;
                state.selected = false;
            }
        }

        if (IsKeyPressed(KEY_V)) {
            auto t = GetClipboardText();
            if (t) {
                input->insert(state.cursor_pos, t);
                state.cursor_pos += strlen(t);
            }
        }
    }

    if (IsKeyDown(KEY_LEFT) && state.cursor_pos > 0) {
        state.cursor_moving = true;
        debounce_action(name + "left", [&] { state.cursor_pos--; });
    }
    if (IsKeyReleased(KEY_LEFT)) {
        state.cursor_moving = false;
        reset_debounce(name + "left");
    }

    if (IsKeyDown(KEY_RIGHT) && state.cursor_pos < input->length()) {
        state.cursor_moving = true;
        debounce_action(name + "right", [&] { state.cursor_pos++; });
    }
    if (IsKeyReleased(KEY_RIGHT)) {
        state.cursor_moving = false;
        reset_debounce(name + "right");
    }

    if (IsKeyPressed(KEY_DOWN) && state.cursor_pos > 0) {
        state.cursor_moving = true;
        state.cursor_pos = 0;
    }

    if (IsKeyReleased(KEY_DOWN)) {
        state.cursor_moving = false;
    }

    if (IsKeyPressed(KEY_UP) && state.cursor_pos < input->length()) {
        state.cursor_moving = true;
        state.cursor_pos = input->length();
    }

    if (IsKeyReleased(KEY_UP)) {
        state.cursor_moving = false;
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
                    .backgroundColor = color_transition(
                        clickable_name + "_bg",
                        clay.pointerOver(clickable_name),
                        MAIN_DARK,
                        TEXT_DARK
                    ),
                    .cornerRadius = {4, 4, 4, 4},
                    .border =
                        {.color = color_transition(
                             clickable_name,
                             clay.pointerOver(clickable_name),
                             BORDER_LIGHT,
                             BORDER_GRAY
                         ),
                         .width = {1, 1, 1, 1, 0}},
                },
                [&] {
                    clay.element(
                        {
                            .id = clay.hashID(
                                std::format("{}_clicked", clickable_name)
                            ),
                            .layout =
                                {.sizing = clay.fixedSize(
                                     int_transition(
                                         clickable_name + "_clicked" + "_w",
                                         *toggle,
                                         20,
                                         1,
                                         0.1
                                     ),
                                     int_transition(
                                         clickable_name + "_clicked" + "_h",
                                         *toggle,
                                         20,
                                         1,
                                         0.1
                                     )
                                 )},
                            .backgroundColor = color_transition(
                                clickable_name + "_clicked",
                                *toggle,
                                MAIN_COLOR,
                                TRANSPARENT,
                                0.15
                            ),
                            .cornerRadius = {6, 6, 6, 6},
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
        drag_window = false;
        *toggle = !*toggle;
        return true;
    }

    return false;
}

// I can revisit this if I add bold fonts
// void text_outline(
//     const std::string& text,
//     const Clay_TextElementConfig textElementConfig) {
//     auto base_id = clay.hashID(std::format("text_base_{}", text));
//     auto outline_id = clay.hashID(std::format("text_outline_{}", text));
//     clay.element(
//         {
//             .id = base_id,
//         },
//         [&] { clay.textElement(text, textElementConfig); });
//     clay.element(
//         {.id = outline_id,
//          .floating =
//              {.offset = {0, 0},
//               .parentId = base_id.id,
//               .zIndex = -1,
//               .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID}},
//         [&] {
//             clay.textElement(
//                 text,
//                 {.userData = textElementConfig.userData,
//                  .textColor = K_BLACK,
//                  .fontId = textElementConfig.fontId,
//                  .fontSize = textElementConfig.fontSize,
//                  .letterSpacing = textElementConfig.letterSpacing,
//                  .lineHeight = textElementConfig.lineHeight,
//                  .wrapMode = textElementConfig.wrapMode,
//                  .textAlignment = textElementConfig.textAlignment});
//         });
// }