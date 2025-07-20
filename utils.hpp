#ifndef KONDUIT_INSTALLER_UTILS_HPP
#define KONDUIT_INSTALLER_UTILS_HPP

#include <chrono>
#include <functional>
#include <iostream>
// #include <mutex>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

inline Clay_Sizing center_percent() {
    return Clay_Sizing{
        .width = {.size = {.percent = 0.5}, .type = CLAY__SIZING_TYPE_PERCENT},
        .height = {.size = {.percent = 0.5}, .type = CLAY__SIZING_TYPE_PERCENT},
    };
}

class Debouncer {
   public:
    explicit Debouncer(long long initial_delay_ms, long long repeat_interval_ms)
        : initial_delay_(std::chrono::milliseconds(initial_delay_ms)),
          repeat_interval_(std::chrono::milliseconds(repeat_interval_ms)),
          current_interval_(initial_delay_),
          last_trigger_time_(std::chrono::steady_clock::time_point::min()),
          first_action_made_(false),
          triggered_count_(0) {
        if (initial_delay_ms < 0 || repeat_interval_ms < 0) {
            throw std::invalid_argument("Delays must be non-negative");
        }
    }

    void reset() {
        current_interval_ = initial_delay_;
        last_trigger_time_ = std::chrono::steady_clock::time_point::min();
        first_action_made_ = false;
        triggered_count_ = 0;
    }

    [[nodiscard]] long long count() const { return triggered_count_; }

    bool throttle(const std::function<void()>& action) {
        auto now = std::chrono::steady_clock::now();

        if (last_trigger_time_ ==
            std::chrono::steady_clock::time_point::min()) {
            action();
            last_trigger_time_ = now;
            triggered_count_++;
            first_action_made_ = true;
            return true;
        }

        if (now - last_trigger_time_ >= current_interval_) {
            action();
            last_trigger_time_ = now;
            triggered_count_++;

            if (first_action_made_ && triggered_count_ == 2) {
                current_interval_ = repeat_interval_;
            }
            return true;
        }
        return false;
    }

   private:
    std::chrono::milliseconds initial_delay_;
    std::chrono::milliseconds repeat_interval_;
    std::chrono::milliseconds current_interval_;
    std::chrono::steady_clock::time_point last_trigger_time_;
    bool first_action_made_;
    long long triggered_count_;
};

struct DirectoryValidationResult {
    bool exists_and_is_dir = false;
    bool empty_initially = false;
    bool usable = true;
    std::string error_message;
};

inline bool
is_directory_empty(const std::filesystem::path& p, std::error_code& ec) {
    auto begin = std::filesystem::directory_iterator(p, ec);
    if (ec) {
        return false;
    }
    return begin == std::filesystem::directory_iterator{};
}

inline DirectoryValidationResult validate_path(const std::string& path_str) {
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

#endif  // KONDUIT_INSTALLER_UTILS_HPP
