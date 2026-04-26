#include "cachewraith/security/SshLogMonitor.h"

#include <regex>

namespace cachewraith {

namespace {

std::string parseSyslogDeviceName(const std::string& line) {
    static const std::regex syslog_prefix(R"(^[A-Z][a-z]{2}\s+\d{1,2}\s+\d{2}:\d{2}:\d{2}\s+([^\s]+))");
    std::smatch match;
    if (std::regex_search(line, match, syslog_prefix)) {
        return match[1];
    }
    return {};
}

} // namespace

std::optional<SshEvent> parseSshLogLine(const std::string& line) {
    static const std::regex failed_invalid(R"(Failed password for invalid user ([^\s]+) from ([0-9a-fA-F:\.]+))");
    static const std::regex failed(R"(Failed password for ([^\s]+) from ([0-9a-fA-F:\.]+))");
    static const std::regex accepted(R"(Accepted (password|publickey) for ([^\s]+) from ([0-9a-fA-F:\.]+))");

    std::smatch match;
    const auto device_name = parseSyslogDeviceName(line);
    if (std::regex_search(line, match, accepted)) {
        const std::string method = match[1];
        return SshEvent{method == "password" ? SshEvent::Kind::AcceptedPassword : SshEvent::Kind::AcceptedPublicKey,
                        match[2], match[3], method, device_name};
    }
    if (std::regex_search(line, match, failed_invalid)) {
        return SshEvent{SshEvent::Kind::FailedPassword, match[1], match[2], "password", device_name};
    }
    if (std::regex_search(line, match, failed)) {
        return SshEvent{SshEvent::Kind::FailedPassword, match[1], match[2], "password", device_name};
    }
    return std::nullopt;
}

SshLogMonitor::SshLogMonitor(Config config)
    : config_(std::move(config)), tailer_(config_.logs.ssh_auth) {}

std::string SshLogMonitor::name() const { return "ssh-log"; }

std::vector<Alert> SshLogMonitor::check() {
    std::vector<Alert> alerts;
    const auto now = std::chrono::steady_clock::now();
    for (const auto& line : tailer_.readNewLines()) {
        const auto event = parseSshLogLine(line);
        if (!event) {
            continue;
        }

        if (event->kind == SshEvent::Kind::AcceptedPassword || event->kind == SshEvent::Kind::AcceptedPublicKey) {
            const auto device_name = event->device_name.empty() ? config_.server_name : event->device_name;
            Alert alert;
            alert.type = "ssh_login";
            alert.key = event->ip + ":" + event->user + ":" + event->method;
            alert.title = "SSH LOGIN SUCCESS";
            alert.severity = AlertSeverity::Info;
            alert.bypass_cooldown = true;
            alert.message = "Device: " + device_name + "\nConfigured Server: " + config_.server_name +
                            "\nUser: " + event->user + "\nFrom: " + event->ip +
                            "\nAuth Method: " + event->method;
            alerts.push_back(std::move(alert));
            continue;
        }

        auto& counter = failed_by_ip_[event->ip];
        if (counter.window_start.time_since_epoch().count() == 0 ||
            now - counter.window_start > std::chrono::minutes(5)) {
            counter.window_start = now;
            counter.count = 0;
        }
        ++counter.count;
        if (counter.count >= config_.thresholds.ssh_failed_attempts) {
            const auto device_name = event->device_name.empty() ? config_.server_name : event->device_name;
            Alert alert;
            alert.type = "ssh_bruteforce";
            alert.key = event->ip;
            alert.title = "SSH BRUTE FORCE DETECTED";
            alert.severity = AlertSeverity::Critical;
            alert.message = "Device: " + device_name + "\nConfigured Server: " + config_.server_name +
                            "\nFrom: " + event->ip +
                            "\nFailed Attempts: " + std::to_string(counter.count) + "\nWindow: 5 minutes";
            alerts.push_back(std::move(alert));
            counter.count = 0;
            counter.window_start = now;
        }
    }
    return alerts;
}

} // namespace cachewraith
