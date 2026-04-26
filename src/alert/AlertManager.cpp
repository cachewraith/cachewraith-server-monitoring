#include "cachewraith/alert/AlertManager.h"

#include "cachewraith/util/Logger.h"

namespace cachewraith {

namespace {

std::optional<long long> findThreadId(const Config& config, const Alert& alert) {
    const auto exact = config.telegram.alert_threads.find(alert.type);
    if (exact != config.telegram.alert_threads.end()) {
        return exact->second;
    }

    auto category = std::string{};
    if (alert.type == "ram_high" || alert.type == "disk_high" || alert.type == "load_high") {
        category = "resource";
    } else if (alert.type == "service_down") {
        category = "service";
    } else if (alert.type == "website_down") {
        category = "website";
    } else if (alert.type == "ssh_login" || alert.type == "ssh_bruteforce") {
        category = "ssh";
    } else if (alert.type == "web_scan" || alert.type == "web_rate" || alert.type == "web_404") {
        category = "web";
    }

    if (!category.empty()) {
        const auto grouped = config.telegram.alert_threads.find(category);
        if (grouped != config.telegram.alert_threads.end()) {
            return grouped->second;
        }
    }

    const auto fallback = config.telegram.alert_threads.find("default");
    if (fallback != config.telegram.alert_threads.end()) {
        return fallback->second;
    }
    return config.telegram.default_thread_id;
}

} // namespace

AlertManager::AlertManager(const Config& config, TelegramNotifier notifier)
    : config_(config), notifier_(std::move(notifier)), cooldowns_(config.cooldowns) {}

void AlertManager::handle(const std::vector<Alert>& alerts) {
    for (const auto& alert : alerts) {
        Logger::instance().warning("Alert generated: " + alert.type + " key=" + alert.key + " title=" + alert.title);
        if (!cooldowns_.shouldSend(alert)) {
            Logger::instance().info("Alert suppressed by cooldown: " + alert.type + " key=" + alert.key);
            continue;
        }
        const std::string text = alert.title + "\n" + alert.message;
        if (!notifier_.sendMessage(text, findThreadId(config_, alert))) {
            Logger::instance().error("Failed to send alert to Telegram: " + alert.type + " key=" + alert.key);
        }
    }
}

} // namespace cachewraith
