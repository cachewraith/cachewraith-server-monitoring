#include "cachewraith/monitor/ServiceMonitor.h"

#include "cachewraith/util/Logger.h"
#include "cachewraith/util/StringUtils.h"

#include <sys/wait.h>
#include <unistd.h>

namespace cachewraith {

ServiceMonitor::ServiceMonitor(Config config) : config_(std::move(config)) {}

std::string ServiceMonitor::name() const { return "service"; }

namespace {

bool isActive(const std::string& service) {
    const pid_t pid = fork();
    if (pid == -1) {
        return false;
    }
    if (pid == 0) {
        execlp("systemctl", "systemctl", "is-active", "--quiet", service.c_str(), static_cast<char*>(nullptr));
        _exit(127);
    }
    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        return false;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

} // namespace

std::vector<Alert> ServiceMonitor::check() {
    std::vector<Alert> alerts;
    for (const auto& service : config_.services) {
        if (!isSafeSystemdServiceName(service)) {
            Logger::instance().warning("Skipping unsafe service name: " + service);
            continue;
        }
        if (!isActive(service)) {
            Alert alert;
            alert.type = "service_down";
            alert.key = service;
            alert.title = "SERVICE DOWN";
            alert.severity = AlertSeverity::Critical;
            alert.message = "Server: " + config_.server_name + "\nService: " + service + "\nStatus: not active";
            alerts.push_back(std::move(alert));
        }
    }
    return alerts;
}

} // namespace cachewraith
