#pragma once

#include "cachewraith/app/Config.h"
#include "cachewraith/monitor/IMonitor.h"

namespace cachewraith {

class CpuMonitor final : public IMonitor {
public:
    explicit CpuMonitor(Config config);
    std::vector<Alert> check() override;
    std::string name() const override;

private:
    Config config_;
};

} // namespace cachewraith
