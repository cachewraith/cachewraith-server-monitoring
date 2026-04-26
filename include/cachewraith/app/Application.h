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
    void registerMonitors();
    void runOnce();

    Config config_;
    TelegramNotifier notifier_;
    AlertManager alert_manager_;
    std::vector<std::unique_ptr<IMonitor>> monitors_;
    std::atomic_bool running_{true};
};

} // namespace cachewraith
