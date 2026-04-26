#pragma once

#include "cachewraith/app/Config.h"
#include "cachewraith/monitor/IMonitor.h"
#include "cachewraith/security/LogTailer.h"

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>

namespace cachewraith {

struct SshEvent {
    enum class Kind { FailedPassword, AcceptedPassword, AcceptedPublicKey };
    Kind kind;
    std::string user;
    std::string ip;
    std::string method;
    std::string device_name;
};

std::optional<SshEvent> parseSshLogLine(const std::string& line);

class SshLogMonitor final : public IMonitor {
public:
    explicit SshLogMonitor(Config config);
    std::vector<Alert> check() override;
    std::string name() const override;

private:
    struct Counter {
        int count{0};
        std::chrono::steady_clock::time_point window_start{};
    };

    Config config_;
    LogTailer tailer_;
    std::unordered_map<std::string, Counter> failed_by_ip_;
};

} // namespace cachewraith
