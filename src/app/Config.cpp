#include "cachewraith/app/Config.h"

#include "cachewraith/util/StringUtils.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace cachewraith {

namespace {

template <typename T>
void setIfPresent(const nlohmann::json& object, const char* key, T& target) {
    if (object.contains(key) && !object.at(key).is_null()) {
        target = object.at(key).get<T>();
    }
}

} // namespace

Config Config::load(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to open config file: " + path.string());
    }

    nlohmann::json json;
    try {
        input >> json;
    } catch (const nlohmann::json::exception& ex) {
        throw std::runtime_error("Invalid JSON in " + path.string() + ": " + ex.what());
    }

    Config config;
    setIfPresent(json, "server_name", config.server_name);
    setIfPresent(json, "interval_seconds", config.interval_seconds);
    setIfPresent(json, "log_file", config.log_file);
    setIfPresent(json, "disk_paths", config.disk_paths);
    setIfPresent(json, "services", config.services);
    setIfPresent(json, "websites", config.websites);
    setIfPresent(json, "nginx_suspicious_paths", config.nginx_suspicious_paths);

    if (json.contains("thresholds")) {
        const auto& t = json.at("thresholds");
        setIfPresent(t, "ram_percent", config.thresholds.ram_percent);
        setIfPresent(t, "disk_percent", config.thresholds.disk_percent);
        setIfPresent(t, "load_average_1m", config.thresholds.load_average_1m);
        setIfPresent(t, "ssh_failed_attempts", config.thresholds.ssh_failed_attempts);
        setIfPresent(t, "nginx_requests_per_minute", config.thresholds.nginx_requests_per_minute);
        setIfPresent(t, "nginx_404_per_minute", config.thresholds.nginx_404_per_minute);
        setIfPresent(t, "website_timeout_seconds", config.thresholds.website_timeout_seconds);
    }

    if (json.contains("cooldowns")) {
        const auto& c = json.at("cooldowns");
        setIfPresent(c, "default_seconds", config.cooldowns.default_seconds);
        setIfPresent(c, "resource_alert_seconds", config.cooldowns.resource_alert_seconds);
        setIfPresent(c, "website_alert_seconds", config.cooldowns.website_alert_seconds);
        setIfPresent(c, "service_alert_seconds", config.cooldowns.service_alert_seconds);
        setIfPresent(c, "ssh_bruteforce_seconds", config.cooldowns.ssh_bruteforce_seconds);
        setIfPresent(c, "web_scan_seconds", config.cooldowns.web_scan_seconds);
    }

    if (json.contains("telegram")) {
        const auto& telegram = json.at("telegram");
        if (telegram.contains("default_thread_id") && !telegram.at("default_thread_id").is_null()) {
            config.telegram.default_thread_id = telegram.at("default_thread_id").get<long long>();
        }
        if (telegram.contains("alert_threads")) {
            config.telegram.alert_threads = telegram.at("alert_threads").get<std::unordered_map<std::string, long long>>();
        }
    }

    if (json.contains("logs")) {
        const auto& logs = json.at("logs");
        setIfPresent(logs, "ssh_auth", config.logs.ssh_auth);
        setIfPresent(logs, "nginx_access", config.logs.nginx_access);
    }

    config.validate();
    return config;
}

void Config::validate() const {
    if (trim(server_name).empty()) {
        throw std::runtime_error("Config field 'server_name' must not be empty");
    }
    if (interval_seconds <= 0) {
        throw std::runtime_error("Config field 'interval_seconds' must be greater than zero");
    }
    if (thresholds.ram_percent < 1 || thresholds.ram_percent > 100 ||
        thresholds.disk_percent < 1 || thresholds.disk_percent > 100) {
        throw std::runtime_error("Percent thresholds must be between 1 and 100");
    }
    if (thresholds.website_timeout_seconds < 1 || thresholds.website_timeout_seconds > 120) {
        throw std::runtime_error("thresholds.website_timeout_seconds must be between 1 and 120");
    }
    for (const auto& service : services) {
        if (!isSafeSystemdServiceName(service)) {
            throw std::runtime_error("Unsafe systemd service name in config: " + service);
        }
    }
    if (telegram.default_thread_id && *telegram.default_thread_id <= 0) {
        throw std::runtime_error("telegram.default_thread_id must be a positive integer");
    }
    for (const auto& [key, thread_id] : telegram.alert_threads) {
        if (trim(key).empty()) {
            throw std::runtime_error("telegram.alert_threads contains an empty key");
        }
        if (thread_id <= 0) {
            throw std::runtime_error("telegram.alert_threads." + key + " must be a positive integer");
        }
    }
}

} // namespace cachewraith
