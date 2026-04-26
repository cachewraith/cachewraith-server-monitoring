#pragma once

#include "cachewraith/alert/AlertManager.h"
#include "cachewraith/app/Config.h"
#include "cachewraith/monitor/IMonitor.h"

#include <atomic>
#include <memory>
#include <vector>

namespace cachewraith {

class Application {
public:
    explicit Application(Config config);

    void run(bool once);
    void stop();

private:
    struct MonitorSchedule {
        std::unique_ptr<IMonitor> monitor;
        int interval_seconds{1};
        int seconds_until_next_check{0};
    };

    void registerMonitors();
    void addMonitor(std::unique_ptr<IMonitor> monitor, int interval_seconds);
    void runOnce();
    void runScheduled();

    Config config_;
    TelegramNotifier notifier_;
    AlertManager alert_manager_;
    std::vector<MonitorSchedule> monitors_;
    std::atomic_bool running_{true};
};

} // namespace cachewraith
