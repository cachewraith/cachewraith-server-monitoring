#pragma once

#include <chrono>
#include <string>

namespace cachewraith {

enum class AlertSeverity { Info, Warning, Critical };

struct Alert {
    std::string type;
    std::string key;
    std::string title;
    std::string message;
    AlertSeverity severity{AlertSeverity::Warning};
    bool bypass_cooldown{false};
    std::chrono::system_clock::time_point created_at{std::chrono::system_clock::now()};
};

} // namespace cachewraith
