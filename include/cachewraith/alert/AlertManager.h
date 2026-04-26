#pragma once

#include "cachewraith/alert/CooldownManager.h"
#include "cachewraith/alert/TelegramNotifier.h"
#include "cachewraith/app/Config.h"

#include <vector>

namespace cachewraith {

class AlertManager {
public:
    AlertManager(const Config& config, TelegramNotifier notifier);

    void handle(const std::vector<Alert>& alerts);

private:
    Config config_;
    TelegramNotifier notifier_;
    CooldownManager cooldowns_;
};

} // namespace cachewraith
