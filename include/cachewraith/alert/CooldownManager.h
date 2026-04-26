#pragma once

#include "cachewraith/app/Config.h"
#include "cachewraith/alert/Alert.h"

#include <chrono>
#include <string>
#include <unordered_map>

namespace cachewraith {

class CooldownManager {
public:
    explicit CooldownManager(Cooldowns cooldowns);

    bool shouldSend(const Alert& alert);
    int cooldownSecondsFor(const Alert& alert) const;

private:
    Cooldowns cooldowns_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> last_sent_;
};

} // namespace cachewraith
