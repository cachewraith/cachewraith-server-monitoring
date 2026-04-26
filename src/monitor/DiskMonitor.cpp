#include "cachewraith/monitor/DiskMonitor.h"

#include "cachewraith/util/Logger.h"

#include <sys/statvfs.h>

namespace cachewraith {

DiskMonitor::DiskMonitor(Config config) : config_(std::move(config)) {}

std::string DiskMonitor::name() const { return "disk"; }

std::vector<Alert> DiskMonitor::check() {
    std::vector<Alert> alerts;
    for (const auto& path : config_.disk_paths) {
        struct statvfs stats {};
        if (statvfs(path.c_str(), &stats) != 0) {
            Logger::instance().warning("Unable to read disk stats for " + path.string());
            continue;
        }
        if (stats.f_blocks == 0) {
            continue;
        }
        const auto used_blocks = stats.f_blocks - stats.f_bfree;
        const auto used_percent = static_cast<int>((used_blocks * 100) / stats.f_blocks);
        if (used_percent >= config_.thresholds.disk_percent) {
            Alert alert;
            alert.type = "disk_high";
            alert.key = path.string();
            alert.title = "DISK USAGE HIGH";
            alert.severity = AlertSeverity::Critical;
            alert.message = "Server: " + config_.server_name + "\nPath: " + path.string() +
                            "\nUsage: " + std::to_string(used_percent) +
                            "%\nThreshold: " + std::to_string(config_.thresholds.disk_percent) + "%";
            alerts.push_back(std::move(alert));
        }
    }
    return alerts;
}

} // namespace cachewraith
