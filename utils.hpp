#ifndef KONDUIT_INSTALLER_UTILS_HPP
#define KONDUIT_INSTALLER_UTILS_HPP

#include <chrono>
#include <functional>
#include <iostream>
// #include <mutex>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "main.hpp"

Clay_Sizing center_percent();

struct DebounceState {
    std::chrono::milliseconds initial_delay;
    std::chrono::milliseconds repeat_interval;
    std::chrono::milliseconds current_interval;
    std::chrono::steady_clock::time_point last_trigger_time;
    bool first_action_made;
    long long triggered_count;

    DebounceState(long long init_ms, long long repeat_ms)
        : initial_delay(init_ms),
          repeat_interval(repeat_ms),
          current_interval(init_ms),
          last_trigger_time(std::chrono::steady_clock::time_point::min()),
          first_action_made(false),
          triggered_count(0) {}
};

static std::unordered_map<std::string, DebounceState>& get_debounce_states();

bool debounce_action(
    const std::string& id,
    const std::function<void()>& action,
    long long initial_delay_ms = 200,
    long long repeat_interval_ms = 25
);

void reset_debounce(const std::string& id);

long long get_debounce_count(const std::string& id);

struct DirectoryValidationResult {
    bool exists_and_is_dir = false;
    bool empty_initially = false;
    bool usable = true;
    std::string error_message;
};

bool is_directory_empty(const std::filesystem::path& p, std::error_code& ec);

DirectoryValidationResult validate_path(const std::string& path_str);

static inline Color to_raylib_color(const Clay_Color& clayColor) {
    return {
        static_cast<unsigned char>(clayColor.r),
        static_cast<unsigned char>(clayColor.g),
        static_cast<unsigned char>(clayColor.b),
        static_cast<unsigned char>(clayColor.a)
    };
}

enum class TransitionType { BINARY, MULTI };

struct ColorCondition {
    bool condition;
    Clay_Color color;
};

struct ColorTransitionState {
    TransitionType type;
    float duration;
    float progress;
    bool isTransitioning;
    Clay_Color currentColor;
    Clay_Color sourceColor;
    Clay_Color targetColor;

    bool lastCondition;
    Clay_Color trueColor;
    Clay_Color falseColor;

    std::vector<ColorCondition> conditions;
    Clay_Color defaultColor;

    explicit ColorTransitionState(
        float dur,
        TransitionType t = TransitionType::BINARY
    )
        : type(t),
          duration(dur),
          progress(0.0f),
          isTransitioning(false),
          currentColor({0.0f, 0.0f, 0.0f, 255.0f}),
          sourceColor({0.0f, 0.0f, 0.0f, 255.0f}),
          targetColor({0.0f, 0.0f, 0.0f, 255.0f}),
          lastCondition(false) {}
};

static std::unordered_map<std::string, ColorTransitionState>&
get_color_states();

Clay_Color color_transition(
    const std::string& id,
    bool condition,
    const Clay_Color& trueColor,
    const Clay_Color& falseColor,
    float duration = 0.15f
);

Clay_Color color_transition(
    const std::string& id,
    const std::vector<ColorCondition>& conditions,
    const Clay_Color& defaultColor,
    float duration = 0.15f
);

void reset_color_transition(const std::string& id);

struct FloatCondition {
    bool condition;
    float value;
};

struct IntCondition {
    bool condition;
    int value;
};

struct ValueTransitionState {
    TransitionType type;
    float duration;
    float progress;
    bool isTransitioning;

    union {
        float currentFloat;
        int currentInt;
    };

    union {
        float sourceFloat;
        int sourceInt;
    };

    union {
        float targetFloat;
        int targetInt;
    };

    // Binary-specific
    bool lastCondition;
    float trueFloat;
    float falseFloat;
    int trueInt;
    int falseInt;

    // Multi-specific
    std::vector<FloatCondition> floatConditions;
    float defaultFloat;
    std::vector<IntCondition> intConditions;
    int defaultInt;

    explicit ValueTransitionState(float dur, TransitionType t)
        : type(t),
          duration(dur),
          progress(0.0f),
          isTransitioning(false),
          lastCondition(false) {}
};

static std::unordered_map<std::string, ValueTransitionState>&
get_value_states();

float lerp_float(float a, float b, float t);

int lerp_int(int a, int b, float t);

float float_transition(
    const std::string& id,
    bool condition,
    float trueVal,
    float falseVal,
    float duration = 0.15f
);

int int_transition(
    const std::string& id,
    bool condition,
    int trueVal,
    int falseVal,
    float duration = 0.15f
);

float float_transition(
    const std::string& id,
    const std::vector<FloatCondition>& conditions,
    float defaultVal,
    float duration = 0.15f
);

int int_transition(
    const std::string& id,
    const std::vector<IntCondition>& conditions,
    int defaultVal,
    float duration = 0.15f
);

#endif  // KONDUIT_INSTALLER_UTILS_HPP
