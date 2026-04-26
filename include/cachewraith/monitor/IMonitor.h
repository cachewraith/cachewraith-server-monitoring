#pragma once

#include "cachewraith/alert/Alert.h"

#include <string>
#include <vector>

namespace cachewraith {

class IMonitor {
public:
    virtual ~IMonitor() = default;
    virtual std::vector<Alert> check() = 0;
    virtual std::string name() const = 0;
};

} // namespace cachewraith
