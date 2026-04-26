#include "cachewraith/alert/CooldownManager.h"

#include "cachewraith/util/Time.h"

namespace cachewraith {

CooldownManager::CooldownManager(Cooldowns cooldowns) : cooldowns_(cooldowns) {}

int CooldownManager::cooldownSecondsFor(const Alert& alert) const {
    if (alert.type == "ram_high" || alert.type == "disk_high" || alert.type == "load_high") {
        return cooldowns_.resource_alert_seconds;
    }
    if (alert.type == "website_down") {
        return cooldowns_.website_alert_seconds;
    }
    if (alert.type == "service_down") {
        return cooldowns_.service_alert_seconds;
    }
    if (alert.type == "ssh_bruteforce") {
        return cooldowns_.ssh_bruteforce_seconds;
    }
    if (alert.type == "web_scan" || alert.type == "web_rate" || alert.type == "web_404") {
        return cooldowns_.web_scan_seconds;
    }
    return cooldowns_.default_seconds;
}

bool CooldownManager::shouldSend(const Alert& alert) {
    if (alert.bypass_cooldown) {
        return true;
    }

    const auto seconds = cooldownSecondsFor(alert);
    if (seconds <= 0) {
        return true;
    }

    const auto key = alert.type + ":" + alert.key;
    const auto now = steadyNow();
    const auto found = last_sent_.find(key);
    if (found == last_sent_.end() ||
        now - found->second >= std::chrono::seconds(seconds)) {
        last_sent_[key] = now;
        return true;
    }
    return false;
}

} // namespace cachewraith
