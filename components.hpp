#ifndef KONDUIT_INSTALLER_COMPONENTS_HPP
#define KONDUIT_INSTALLER_COMPONENTS_HPP

#include <algorithm>
#include <cmath>
#include <map>
#include "main.hpp"
#include "utils.hpp"

struct ProgressItems {
    int done;
    int all;
};

bool button(std::string text, Vector2 size = {80, 30});

struct InputState {
    std::string name;
    Vector2 size;
    std::string* input;
    size_t cursor_pos;
    bool cursor_moving;
    double last_blink;
    bool blink;
    bool selected;
};

bool text_input(
    std::string label,
    std::string* input,
    Vector2 size = {150, 30}
);

/// 0.0..1.0 % range based
///
/// pass {-1 , number} to use adaptable width
void progress_bar(
    float percent,
    std::string text,
    ProgressItems items = {-1, -1},
    Vector2 size = {-1, 30}
);

enum CheckboxType {
    SQUARE,
    CIRCLE,
};

static std::map<CheckboxType, Clay_CornerRadius> checkbox_rounding_styles = {
    {CheckboxType::SQUARE, {4, 4, 4, 4}},
    {CheckboxType::CIRCLE, {15, 15, 15, 15}}
};

bool checkbox(std::string label, bool* toggle);

struct RadioOption {
    std::string label;
    bool* toggle;
};

bool radio_selection(std::string label, std::vector<RadioOption> options);

bool popup(std::string id, bool* open, std::function<void()> content);

#endif  // KONDUIT_INSTALLER_COMPONENTS_HPP
