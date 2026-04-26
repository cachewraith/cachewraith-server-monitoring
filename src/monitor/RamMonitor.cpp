#include "cachewraith/monitor/RamMonitor.h"

#include "cachewraith/util/Logger.h"

#include <fstream>
#include <sstream>
#include <unordered_map>

namespace cachewraith {

RamMonitor::RamMonitor(Config config) : config_(std::move(config)) {}

std::string RamMonitor::name() const { return "ram"; }

std::vector<Alert> RamMonitor::check() {
    std::ifstream input("/proc/meminfo");
    if (!input) {
        Logger::instance().warning("Unable to read /proc/meminfo");
        return {};
    }

    std::unordered_map<std::string, long long> values;
    std::string key;
    long long value = 0;
    std::string unit;
    while (input >> key >> value >> unit) {
        if (!key.empty() && key.back() == ':') {
            key.pop_back();
        }
        values[key] = value;
    }

    const auto total = values["MemTotal"];
    const auto available = values.contains("MemAvailable") ? values["MemAvailable"] : values["MemFree"];
    if (total <= 0 || available < 0) {
        return {};
    }

    const auto used_percent = static_cast<int>(((total - available) * 100) / total);
    if (used_percent < config_.thresholds.ram_percent) {
        return {};
    }

    Alert alert;
    alert.type = "ram_high";
    alert.key = "memory";
    alert.title = "RAM USAGE HIGH";
    alert.severity = AlertSeverity::Critical;
    alert.message = "Server: " + config_.server_name + "\nUsage: " + std::to_string(used_percent) +
                    "%\nThreshold: " + std::to_string(config_.thresholds.ram_percent) + "%";
    return {alert};
}

} // namespace cachewraith
