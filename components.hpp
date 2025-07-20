#ifndef KONDUIT_INSTALLER_COMPONENTS_HPP
#define KONDUIT_INSTALLER_COMPONENTS_HPP

#include <cmath>
#include <map>
#include "main.hpp"
#include "utils.hpp"

struct ProgressItems {
    int done;
    int all;
};

bool button(std::string text, Vector2 size = {80, 30});

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

bool checkbox(std::string label, bool* toggle);

#endif  // KONDUIT_INSTALLER_COMPONENTS_HPP
