#include "utils.hpp"

#include <utility>

Clay_Sizing center_percent() {
    return Clay_Sizing{
        .width = {.size = {.percent = 0.5}, .type = CLAY__SIZING_TYPE_PERCENT},
        .height = {.size = {.percent = 0.5}, .type = CLAY__SIZING_TYPE_PERCENT},
    };
}

static std::unordered_map<std::string, DebounceState>& get_debounce_states() {
    static std::unordered_map<std::string, DebounceState> states;
    return states;
}

bool debounce_action(
    const std::string& id,
    const std::function<void()>& action,
    long long initial_delay_ms,
    long long repeat_interval_ms
) {
    auto& states = get_debounce_states();

    auto it = states.find(id);
    if (it == states.end()) {
        states.emplace(id, DebounceState(initial_delay_ms, repeat_interval_ms));
        it = states.find(id);
    }

    auto& state = it->second;
    auto now = std::chrono::steady_clock::now();

    if (state.last_trigger_time ==
        std::chrono::steady_clock::time_point::min()) {
        action();
        state.last_trigger_time = now;
        state.triggered_count++;
        state.first_action_made = true;
        return true;
    }

    if (now - state.last_trigger_time >= state.current_interval) {
        action();
        state.last_trigger_time = now;
        state.triggered_count++;

        if (state.first_action_made && state.triggered_count == 2) {
            state.current_interval = state.repeat_interval;
        }
        return true;
    }
    return false;
}

void reset_debounce(const std::string& id) {
    auto& states = get_debounce_states();
    auto it = states.find(id);
    if (it != states.end()) {
        it->second.current_interval = it->second.initial_delay;
        it->second.last_trigger_time =
            std::chrono::steady_clock::time_point::min();
        it->second.first_action_made = false;
        it->second.triggered_count = 0;
    }
}

long long get_debounce_count(const std::string& id) {
    auto& states = get_debounce_states();
    auto it = states.find(id);
    return it != states.end() ? it->second.triggered_count : 0;
}

bool is_directory_empty(const std::filesystem::path& p, std::error_code& ec) {
    auto begin = std::filesystem::directory_iterator(p, ec);
    if (ec) {
        return false;
    }
    return begin == std::filesystem::directory_iterator{};
}

DirectoryValidationResult validate_path(const std::string& path_str) {
    DirectoryValidationResult result;

    if (path_str.empty() ||
        path_str.find_first_not_of(" \t\r\n") == std::string::npos) {
        result.error_message = "path is empty or contains only whitespace.";
        result.usable = false;
        return result;
    }

    std::filesystem::path p(path_str);
    std::error_code ec;

    if (!p.is_absolute()) {
        result.error_message =
            "path must be absolute, relative paths are not allowed.";
        result.usable = false;
        return result;
    }

    bool path_exists = std::filesystem::exists(p, ec);
    if (ec) {
        result.error_message =
            std::format("error checking path existence: {}", ec.message());
        result.usable = false;
        return result;
    }

    if (path_exists) {
        bool is_dir = std::filesystem::is_directory(p, ec);
        if (ec) {
            result.error_message = std::format(
                "error checking if path is a directory: {}", ec.message()
            );
            result.usable = false;
            return result;
        }

        if (is_dir) {
            result.exists_and_is_dir = true;

            bool is_empty = is_directory_empty(p, ec);
            if (ec) {
                result.error_message = std::format(
                    "error checking if directory is empty: {}", ec.message()
                );
                result.usable = false;
                return result;
            }
            result.empty_initially = is_empty;
            if (is_empty) {
                result.usable = true;
            } else {
                result.usable = false;
                result.error_message =
                    "directory exists but is not empty. cannot proceed.";
            }
        } else {
            result.error_message =
                "path exists but is not a directory. cannot proceed.";
            result.usable = false;
            return result;
        }
    } else {
        result.exists_and_is_dir = false;
        result.empty_initially = true;
        result.usable = true;
    }

    return result;
}

static std::unordered_map<std::string, ColorTransitionState>&
get_color_states() {
    static std::unordered_map<std::string, ColorTransitionState> states;
    return states;
}

Clay_Color color_transition(
    const std::string& id,
    bool condition,
    const Clay_Color& trueColor,
    const Clay_Color& falseColor,
    float duration,
    EasingFunction easingFunc
) {
    auto& states = get_color_states();

    auto it = states.find(id);
    if (it == states.end() || it->second.type != TransitionType::BINARY) {
        states.emplace(
            id, ColorTransitionState(duration, TransitionType::BINARY)
        );
        it = states.find(id);
    }

    auto& state = it->second;
    state.trueColor = trueColor;
    state.falseColor = falseColor;
    state.easingFunc = std::move(easingFunc);
    Clay_Color newTarget = condition ? trueColor : falseColor;

    if (newTarget.r != state.targetColor.r ||
        newTarget.g != state.targetColor.g ||
        newTarget.b != state.targetColor.b ||
        newTarget.a != state.targetColor.a ||
        state.lastCondition != condition) {
        state.sourceColor = state.currentColor;
        state.targetColor = newTarget;
        state.progress = 0.0f;
        state.isTransitioning = true;
        state.lastCondition = condition;
    }

    if (state.isTransitioning) {
        state.progress += GetFrameTime() / state.duration;
        if (state.progress >= 1.0f) {
            state.progress = 1.0f;
            state.isTransitioning = false;
            state.currentColor = state.targetColor;
        } else {
            float easedProgress =
                state.easingFunc(state.progress, 0.0f, 1.0f, 1.0f);

            Color raylibSource = to_raylib_color(state.sourceColor);
            Color raylibTarget = to_raylib_color(state.targetColor);
            Color raylibResult =
                ColorLerp(raylibSource, raylibTarget, easedProgress);
            state.currentColor = {
                static_cast<float>(raylibResult.r),
                static_cast<float>(raylibResult.g),
                static_cast<float>(raylibResult.b),
                static_cast<float>(raylibResult.a)
            };
        }
    }

    return state.currentColor;
}

Clay_Color color_transition(
    const std::string& id,
    const std::vector<ColorCondition>& conditions,
    const Clay_Color& defaultColor,
    float duration,
    EasingFunction easingFunc
) {
    auto& states = get_color_states();

    auto it = states.find(id);
    if (it == states.end() || it->second.type != TransitionType::MULTI) {
        states.emplace(
            id, ColorTransitionState(duration, TransitionType::MULTI)
        );
        it = states.find(id);
    }

    auto& state = it->second;
    state.conditions = conditions;
    state.defaultColor = defaultColor;
    state.easingFunc = std::move(easingFunc);

    Clay_Color newTarget = defaultColor;
    for (const auto& cond : conditions) {
        if (cond.condition) {
            newTarget = cond.color;
            break;
        }
    }

    if (newTarget.r != state.targetColor.r ||
        newTarget.g != state.targetColor.g ||
        newTarget.b != state.targetColor.b ||
        newTarget.a != state.targetColor.a) {
        state.sourceColor = state.currentColor;
        state.targetColor = newTarget;
        state.progress = 0.0f;
        state.isTransitioning = true;
    }

    if (state.isTransitioning) {
        state.progress += GetFrameTime() / state.duration;
        if (state.progress >= 1.0f) {
            state.progress = 1.0f;
            state.isTransitioning = false;
            state.currentColor = state.targetColor;
        } else {
            float easedProgress =
                state.easingFunc(state.progress, 0.0f, 1.0f, 1.0f);

            Color raylibSource = to_raylib_color(state.sourceColor);
            Color raylibTarget = to_raylib_color(state.targetColor);
            Color raylibResult =
                ColorLerp(raylibSource, raylibTarget, easedProgress);
            state.currentColor = {
                static_cast<float>(raylibResult.r),
                static_cast<float>(raylibResult.g),
                static_cast<float>(raylibResult.b),
                static_cast<float>(raylibResult.a)
            };
        }
    }

    return state.currentColor;
}

void reset_color_transition(const std::string& id) {
    auto& states = get_color_states();
    auto it = states.find(id);
    if (it != states.end()) {
        it->second.progress = 0.0f;
        it->second.isTransitioning = false;
        it->second.currentColor = {0.0f, 0.0f, 0.0f, 255.0f};
        it->second.sourceColor = {0.0f, 0.0f, 0.0f, 255.0f};
        it->second.targetColor = {0.0f, 0.0f, 0.0f, 255.0f};
        if (it->second.type == TransitionType::BINARY) {
            it->second.lastCondition = false;
        }
    }
}

static std::unordered_map<std::string, ValueTransitionState>&
get_value_states() {
    static std::unordered_map<std::string, ValueTransitionState> states;
    return states;
}

float lerp_float(float a, float b, float t) {
    return a + (b - a) * t;
}

int lerp_int(int a, int b, float t) {
    return static_cast<int>(a + (b - a) * t + 0.5f);
}

float float_transition(
    const std::string& id,
    bool condition,
    float trueVal,
    float falseVal,
    float duration,
    EasingFunction easingFunc
) {
    auto& states = get_value_states();
    auto it = states.find(id);

    if (it == states.end() || it->second.type != TransitionType::BINARY) {
        states.emplace(
            id, ValueTransitionState(duration, TransitionType::BINARY)
        );
        it = states.find(id);
    }

    auto& state = it->second;
    state.trueFloat = trueVal;
    state.falseFloat = falseVal;
    state.easingFunc = std::move(easingFunc);

    float newTarget = condition ? trueVal : falseVal;

    if (newTarget != state.targetFloat || state.lastCondition != condition) {
        state.sourceFloat = state.currentFloat;
        state.targetFloat = newTarget;
        state.progress = 0.0f;
        state.isTransitioning = true;
        state.lastCondition = condition;
    }

    if (state.isTransitioning) {
        state.progress += GetFrameTime() / state.duration;
        if (state.progress >= 1.0f) {
            state.progress = 1.0f;
            state.isTransitioning = false;
            state.currentFloat = state.targetFloat;
        } else {
            float changeAmount = state.targetFloat - state.sourceFloat;
            state.currentFloat = state.easingFunc(
                state.progress * state.duration,
                state.sourceFloat,
                changeAmount,
                state.duration
            );
        }
    }

    return state.currentFloat;
}

int int_transition(
    const std::string& id,
    bool condition,
    int trueVal,
    int falseVal,
    float duration,
    EasingFunction easingFunc
) {
    auto& states = get_value_states();
    auto it = states.find(id);

    if (it == states.end() || it->second.type != TransitionType::BINARY) {
        states.emplace(
            id, ValueTransitionState(duration, TransitionType::BINARY)
        );
        it = states.find(id);
    }

    auto& state = it->second;
    state.trueInt = trueVal;
    state.falseInt = falseVal;
    state.easingFunc = std::move(easingFunc);

    int newTarget = condition ? trueVal : falseVal;

    if (newTarget != state.targetInt || state.lastCondition != condition) {
        state.sourceInt = state.currentInt;
        state.targetInt = newTarget;
        state.progress = 0.0f;
        state.isTransitioning = true;
        state.lastCondition = condition;
    }

    if (state.isTransitioning) {
        state.progress += GetFrameTime() / state.duration;
        if (state.progress >= 1.0f) {
            state.progress = 1.0f;
            state.isTransitioning = false;
            state.currentInt = state.targetInt;
        } else {
            auto changeAmount =
                static_cast<float>(state.targetInt - state.sourceInt);
            float easedValue = state.easingFunc(
                state.progress * state.duration,
                static_cast<float>(state.sourceInt),
                changeAmount,
                state.duration
            );
            state.currentInt = lround(easedValue);
        }
    }

    return state.currentInt;
}

float float_transition(
    const std::string& id,
    const std::vector<FloatCondition>& conditions,
    float defaultVal,
    float duration,
    EasingFunction easingFunc
) {
    auto& states = get_value_states();
    auto it = states.find(id);

    if (it == states.end() || it->second.type != TransitionType::MULTI) {
        states.emplace(
            id, ValueTransitionState(duration, TransitionType::MULTI)
        );
        it = states.find(id);
    }

    auto& state = it->second;
    state.floatConditions = conditions;
    state.defaultFloat = defaultVal;
    state.easingFunc = std::move(easingFunc);

    float newTarget = defaultVal;
    for (const auto& cond : conditions) {
        if (cond.condition) {
            newTarget = cond.value;
            break;
        }
    }

    if (newTarget != state.targetFloat) {
        state.sourceFloat = state.currentFloat;
        state.targetFloat = newTarget;
        state.progress = 0.0f;
        state.isTransitioning = true;
    }

    if (state.isTransitioning) {
        state.progress += GetFrameTime() / state.duration;
        if (state.progress >= 1.0f) {
            state.progress = 1.0f;
            state.isTransitioning = false;
            state.currentFloat = state.targetFloat;
        } else {
            float changeAmount = state.targetFloat - state.sourceFloat;
            state.currentFloat = state.easingFunc(
                state.progress * state.duration,
                state.sourceFloat,
                changeAmount,
                state.duration
            );
        }
    }

    return state.currentFloat;
}

int int_transition(
    const std::string& id,
    const std::vector<IntCondition>& conditions,
    int defaultVal,
    float duration,
    EasingFunction easingFunc
) {
    auto& states = get_value_states();
    auto it = states.find(id);

    if (it == states.end() || it->second.type != TransitionType::MULTI) {
        states.emplace(
            id, ValueTransitionState(duration, TransitionType::MULTI)
        );
        it = states.find(id);
    }

    auto& state = it->second;
    state.intConditions = conditions;
    state.defaultInt = defaultVal;
    state.easingFunc = easingFunc;

    int newTarget = defaultVal;
    for (const auto& cond : conditions) {
        if (cond.condition) {
            newTarget = cond.value;
            break;
        }
    }

    if (newTarget != state.targetInt) {
        state.sourceInt = state.currentInt;
        state.targetInt = newTarget;
        state.progress = 0.0f;
        state.isTransitioning = true;
    }

    if (state.isTransitioning) {
        state.progress += GetFrameTime() / state.duration;
        if (state.progress >= 1.0f) {
            state.progress = 1.0f;
            state.isTransitioning = false;
            state.currentInt = state.targetInt;
        } else {
            float changeAmount =
                static_cast<float>(state.targetInt - state.sourceInt);
            float easedValue = state.easingFunc(
                state.progress * state.duration,
                static_cast<float>(state.sourceInt),
                changeAmount,
                state.duration
            );
            state.currentInt = lround(easedValue);
        }
    }

    return state.currentInt;
}

uint32_t codepoint(const char* str) {
    const auto* s = (const unsigned char*)str;

    if ((s[0] & 0xF8) == 0xF0) {
        return ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) |
               ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
    } else if ((s[0] & 0xF0) == 0xE0) {
        return ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
    } else if ((s[0] & 0xE0) == 0xC0) {
        return ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
    } else if ((s[0] & 0x80) == 0x00) {
        return s[0];
    }

    return 0xFFFD;
}