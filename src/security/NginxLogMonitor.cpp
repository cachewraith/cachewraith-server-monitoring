#include "cachewraith/security/NginxLogMonitor.h"

#include "cachewraith/util/StringUtils.h"

#include <regex>
#include <sstream>

namespace cachewraith {

namespace {

std::string configuredWebsiteText(const std::vector<std::string>& websites) {
    if (websites.empty()) {
        return "none configured";
    }

    std::ostringstream text;
    for (std::size_t i = 0; i < websites.size(); ++i) {
        if (i > 0) {
            text << ", ";
        }
        text << websites[i];
    }
    return text.str();
}

} // namespace

std::optional<NginxAccessEvent> parseNginxAccessLine(const std::string& line) {
    static const std::regex common(R"(^([0-9a-fA-F:\.]+)\s+\S+\s+\S+\s+\[[^\]]+\]\s+"([A-Z]+)\s+([^"\s]+)[^"]*"\s+([0-9]{3}))");
    std::smatch match;
    if (!std::regex_search(line, match, common)) {
        return std::nullopt;
    }
    return NginxAccessEvent{match[1], match[2], match[3], std::stoi(match[4])};
}

bool isSuspiciousNginxPath(const std::string& path, const std::vector<std::string>& suspicious_paths) {
    for (const auto& suspect : suspicious_paths) {
        if (containsCaseInsensitive(path, suspect)) {
            return true;
        }
    }
    return false;
}

NginxLogMonitor::NginxLogMonitor(Config config)
    : config_(std::move(config)), tailer_(config_.logs.nginx_access) {}

std::string NginxLogMonitor::name() const { return "nginx-log"; }

std::vector<Alert> NginxLogMonitor::check() {
    std::vector<Alert> alerts;
    const auto now = std::chrono::steady_clock::now();
    for (const auto& line : tailer_.readNewLines()) {
        const auto event = parseNginxAccessLine(line);
        if (!event) {
            continue;
        }

        auto& counter = by_ip_[event->ip];
        if (counter.window_start.time_since_epoch().count() == 0 ||
            now - counter.window_start > std::chrono::minutes(1)) {
            counter = Counter{0, 0, 0, "", now};
        }

        ++counter.requests;
        if (event->status == 404) {
            ++counter.not_found;
        }
        if (isSuspiciousNginxPath(event->path, config_.nginx_suspicious_paths)) {
            ++counter.suspicious;
            counter.last_suspicious_path = event->path;
        }

        if (counter.suspicious > 0) {
            Alert alert;
            alert.type = "web_scan";
            alert.key = event->ip + ":suspicious";
            alert.title = "WEB SCAN DETECTED";
            alert.severity = AlertSeverity::Warning;
            alert.message = "Server: " + config_.server_name + "\nSource IP: " + event->ip +
                            "\nWebsite URL: " + configuredWebsiteText(config_.websites) +
                            "\nReason: suspicious path requested\nPath: " + counter.last_suspicious_path +
                            "\nCount: " + std::to_string(counter.suspicious);
            alerts.push_back(std::move(alert));
            counter.suspicious = 0;
        }

        if (counter.requests >= config_.thresholds.nginx_requests_per_minute) {
            Alert alert;
            alert.type = "web_rate";
            alert.key = event->ip;
            alert.title = "HIGH WEB REQUEST RATE";
            alert.severity = AlertSeverity::Warning;
            alert.message = "Server: " + config_.server_name + "\nSource IP: " + event->ip +
                            "\nWebsite URL: " + configuredWebsiteText(config_.websites) +
                            "\nRequests: " + std::to_string(counter.requests) + "\nWindow: 1 minute";
            alerts.push_back(std::move(alert));
            counter.requests = 0;
        }

        if (counter.not_found >= config_.thresholds.nginx_404_per_minute) {
            Alert alert;
            alert.type = "web_404";
            alert.key = event->ip;
            alert.title = "WEB 404 BURST";
            alert.severity = AlertSeverity::Warning;
            alert.message = "Server: " + config_.server_name + "\nSource IP: " + event->ip +
                            "\nWebsite URL: " + configuredWebsiteText(config_.websites) +
                            "\n404 Responses: " + std::to_string(counter.not_found) + "\nWindow: 1 minute";
            alerts.push_back(std::move(alert));
            counter.not_found = 0;
        }
    }
    return alerts;
}

} // namespace cachewraith
