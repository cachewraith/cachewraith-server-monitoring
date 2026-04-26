#include "cachewraith/app/Application.h"

#include "cachewraith/monitor/CpuMonitor.h"
#include "cachewraith/monitor/DiskMonitor.h"
#include "cachewraith/monitor/RamMonitor.h"
#include "cachewraith/monitor/ServiceMonitor.h"
#include "cachewraith/monitor/WebsiteMonitor.h"
#include "cachewraith/security/NginxLogMonitor.h"
#include "cachewraith/security/SshLogMonitor.h"
#include "cachewraith/util/Logger.h"

#include <thread>

namespace cachewraith {

Application::Application(Config config)
    : config_(std::move(config)), notifier_(), alert_manager_(config_, notifier_) {
    registerMonitors();
}

void Application::registerMonitors() {
    monitors_.push_back(std::make_unique<RamMonitor>(config_));
    monitors_.push_back(std::make_unique<DiskMonitor>(config_));
    monitors_.push_back(std::make_unique<CpuMonitor>(config_));
    monitors_.push_back(std::make_unique<ServiceMonitor>(config_));
    monitors_.push_back(std::make_unique<WebsiteMonitor>(config_));
    monitors_.push_back(std::make_unique<SshLogMonitor>(config_));
    monitors_.push_back(std::make_unique<NginxLogMonitor>(config_));
}

void Application::runOnce() {
    std::vector<Alert> alerts;
    for (auto& monitor : monitors_) {
        try {
            auto monitor_alerts = monitor->check();
            alerts.insert(alerts.end(), monitor_alerts.begin(), monitor_alerts.end());
        } catch (const std::exception& ex) {
            Logger::instance().error("Monitor failed: " + monitor->name() + ": " + ex.what());
        }
    }
    alert_manager_.handle(alerts);
}

void Application::run(bool once) {
    Logger::instance().info("CacheWraith monitor started");
    do {
        runOnce();
        if (once) {
            break;
        }
        for (int i = 0; running_ && i < config_.interval_seconds; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } while (running_);
    Logger::instance().info("CacheWraith monitor stopped");
}

void Application::stop() {
    running_ = false;
}

} // namespace cachewraith
