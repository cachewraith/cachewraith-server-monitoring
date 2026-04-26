#include "cachewraith/alert/AlertManager.h"

#include "cachewraith/util/Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

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

std::string severityName(AlertSeverity severity) {
    switch (severity) {
    case AlertSeverity::Info:
        return "INFO";
    case AlertSeverity::Warning:
        return "WARNING";
    case AlertSeverity::Critical:
        return "CRITICAL";
    }
    return "UNKNOWN";
}

std::string alertAction(const Alert& alert) {
    if (alert.type == "ssh_login") {
        return "Verify this user and source IP. Rotate keys/passwords if it was not expected.";
    }
    if (alert.type == "ssh_bruteforce") {
        return "Block or rate-limit the source IP and review SSH hardening.";
    }
    if (alert.type == "web_scan") {
        return "Check the request in Nginx logs and block the source IP if it keeps scanning.";
    }
    if (alert.type == "web_rate") {
        return "Inspect traffic from this IP and apply rate limiting if needed.";
    }
    if (alert.type == "web_404") {
        return "Review requested paths and block noisy scanners if confirmed.";
    }
    if (alert.type == "service_down") {
        return "Restart or inspect the service with systemctl and journalctl.";
    }
    if (alert.type == "website_down") {
        return "Check the site endpoint, upstream service, DNS, and network path.";
    }
    if (alert.type == "ram_high" || alert.type == "disk_high" || alert.type == "load_high") {
        return "Inspect the busiest processes and free capacity before the host degrades.";
    }
    return {};
}

std::string localTimestamp(std::chrono::system_clock::time_point time) {
    const auto raw_time = std::chrono::system_clock::to_time_t(time);
    std::tm local_time{};
#if defined(_WIN32)
    localtime_s(&local_time, &raw_time);
#else
    localtime_r(&raw_time, &local_time);
#endif

    std::ostringstream stream;
    stream << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S %Z");
    return stream.str();
}

std::string formatAlertText(const Alert& alert) {
    std::ostringstream text;
    text << "[" << severityName(alert.severity) << "] " << alert.title << '\n'
         << "Time: " << localTimestamp(alert.created_at) << '\n'
         << alert.message;

    const auto action = alertAction(alert);
    if (!action.empty()) {
        text << "\nAction: " << action;
    }
    return text.str();
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
        const std::string text = formatAlertText(alert);
        if (!notifier_.sendMessageAsync(text, findThreadId(config_, alert))) {
            Logger::instance().error("Failed to queue alert for Telegram: " + alert.type + " key=" + alert.key);
        }
    }
}

} // namespace cachewraith
