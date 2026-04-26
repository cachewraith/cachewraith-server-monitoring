#pragma once

#include "cachewraith/app/Config.h"
#include "cachewraith/monitor/IMonitor.h"
#include "cachewraith/security/LogTailer.h"

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>

namespace cachewraith {

struct NginxAccessEvent {
    std::string ip;
    std::string method;
    std::string path;
    int status{0};
};

std::optional<NginxAccessEvent> parseNginxAccessLine(const std::string& line);
bool isSuspiciousNginxPath(const std::string& path, const std::vector<std::string>& suspicious_paths);

class NginxLogMonitor final : public IMonitor {
public:
    explicit NginxLogMonitor(Config config);
    std::vector<Alert> check() override;
    std::string name() const override;

private:
    struct Counter {
        int requests{0};
        int not_found{0};
        int suspicious{0};
        std::string last_suspicious_path;
        std::chrono::steady_clock::time_point window_start{};
    };

    Config config_;
    LogTailer tailer_;
    std::unordered_map<std::string, Counter> by_ip_;
};

} // namespace cachewraith
