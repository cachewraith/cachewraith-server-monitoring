#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cachewraith {

struct Thresholds {
    int ram_percent{90};
    int disk_percent{90};
    double load_average_1m{4.0};
    int ssh_failed_attempts{10};
    int nginx_requests_per_minute{120};
    int nginx_404_per_minute{30};
    long website_timeout_seconds{5};
};

struct Cooldowns {
    int default_seconds{900};
    int resource_alert_seconds{900};
    int website_alert_seconds{300};
    int service_alert_seconds{300};
    int ssh_bruteforce_seconds{600};
    int web_scan_seconds{600};
};

struct LogPaths {
    std::filesystem::path ssh_auth{"/var/log/auth.log"};
    std::filesystem::path nginx_access{"/var/log/nginx/access.log"};
};

struct TelegramTopics {
    std::optional<long long> default_thread_id;
    std::unordered_map<std::string, long long> alert_threads;
};

struct Config {
    std::string server_name{"cachewraith-vps-01"};
    int interval_seconds{60};
    std::filesystem::path log_file{"/var/log/cachewraith-monitor.log"};
    Thresholds thresholds;
    Cooldowns cooldowns;
    TelegramTopics telegram;
    std::vector<std::filesystem::path> disk_paths{"/"};
    std::vector<std::string> services;
    std::vector<std::string> websites;
    LogPaths logs;
    std::vector<std::string> nginx_suspicious_paths{
        "/.env", "/wp-admin", "/xmlrpc.php", "/phpmyadmin", "/.git", "../", "union select", "<script"};

    static Config load(const std::filesystem::path& path);
    void validate() const;
};

} // namespace cachewraith
