#pragma once

#include "cachewraith/app/Config.h"
#include "cachewraith/monitor/IMonitor.h"

namespace cachewraith {

class DiskMonitor final : public IMonitor {
public:
    explicit DiskMonitor(Config config);
    std::vector<Alert> check() override;
    std::string name() const override;

private:
    Config config_;
};

} // namespace cachewraith
