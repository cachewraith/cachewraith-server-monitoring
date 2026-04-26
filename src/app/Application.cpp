#include "cachewraith/app/Application.h"

#include "cachewraith/monitor/CpuMonitor.h"
#include "cachewraith/monitor/DiskMonitor.h"
#include "cachewraith/monitor/RamMonitor.h"
#include "cachewraith/monitor/ServiceMonitor.h"
#include "cachewraith/monitor/WebsiteMonitor.h"
#include "cachewraith/security/NginxLogMonitor.h"
#include "cachewraith/security/SshLogMonitor.h"
#include "cachewraith/util/Logger.h"

#include <algorithm>
#include <thread>

namespace cachewraith {

Application::Application(Config config)
    : config_(std::move(config)), notifier_(), alert_manager_(config_, notifier_) {
    registerMonitors();
}

void Application::registerMonitors() {
    addMonitor(std::make_unique<SshLogMonitor>(config_), 1);
    addMonitor(std::make_unique<NginxLogMonitor>(config_), 1);
    addMonitor(std::make_unique<RamMonitor>(config_), config_.interval_seconds);
    addMonitor(std::make_unique<DiskMonitor>(config_), config_.interval_seconds);
    addMonitor(std::make_unique<CpuMonitor>(config_), config_.interval_seconds);
    addMonitor(std::make_unique<ServiceMonitor>(config_), config_.interval_seconds);
    addMonitor(std::make_unique<WebsiteMonitor>(config_), config_.interval_seconds);
}

void Application::addMonitor(std::unique_ptr<IMonitor> monitor, int interval_seconds) {
    monitors_.push_back(MonitorSchedule{std::move(monitor), std::max(1, interval_seconds), 0});
}

void Application::runOnce() {
    std::vector<Alert> alerts;
    for (auto& schedule : monitors_) {
        try {
            auto monitor_alerts = schedule.monitor->check();
            alerts.insert(alerts.end(), monitor_alerts.begin(), monitor_alerts.end());
        } catch (const std::exception& ex) {
            Logger::instance().error("Monitor failed: " + schedule.monitor->name() + ": " + ex.what());
        }
    }
    alert_manager_.handle(alerts);
}

void Application::runScheduled() {
    for (auto& schedule : monitors_) {
        if (schedule.seconds_until_next_check > 0) {
            --schedule.seconds_until_next_check;
            continue;
        }

        try {
            alert_manager_.handle(schedule.monitor->check());
        } catch (const std::exception& ex) {
            Logger::instance().error("Monitor failed: " + schedule.monitor->name() + ": " + ex.what());
        }
        schedule.seconds_until_next_check = schedule.interval_seconds - 1;
    }
}

void Application::run(bool once) {
    Logger::instance().info("CacheWraith monitor started");
    if (once) {
        runOnce();
    } else {
        while (running_) {
            runScheduled();
            if (running_) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
    Logger::instance().info("CacheWraith monitor stopped");
}

void Application::stop() {
    running_ = false;
}

} // namespace cachewraith
