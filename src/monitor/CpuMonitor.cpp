#include "cachewraith/monitor/CpuMonitor.h"

#include "cachewraith/util/Logger.h"

#include <fstream>

namespace cachewraith {

CpuMonitor::CpuMonitor(Config config) : config_(std::move(config)) {}

std::string CpuMonitor::name() const { return "cpu"; }

std::vector<Alert> CpuMonitor::check() {
    std::ifstream input("/proc/loadavg");
    if (!input) {
        Logger::instance().warning("Unable to read /proc/loadavg");
        return {};
    }

    double load1 = 0.0;
    input >> load1;
    if (load1 < config_.thresholds.load_average_1m) {
        return {};
    }

    Alert alert;
    alert.type = "load_high";
    alert.key = "loadavg1m";
    alert.title = "CPU LOAD HIGH";
    alert.severity = AlertSeverity::Warning;
    alert.message = "Server: " + config_.server_name + "\nLoad Average 1m: " + std::to_string(load1) +
                    "\nThreshold: " + std::to_string(config_.thresholds.load_average_1m);
    return {alert};
}

} // namespace cachewraith
