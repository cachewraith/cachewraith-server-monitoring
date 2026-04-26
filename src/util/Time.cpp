#include "cachewraith/util/Time.h"

#include <iomanip>
#include <sstream>

namespace cachewraith {

std::string formatTime(std::chrono::system_clock::time_point time) {
    const auto tt = std::chrono::system_clock::to_time_t(time);
    std::tm tm{};
    localtime_r(&tt, &tm);
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%d %H:%M:%S %z");
    return out.str();
}

std::chrono::steady_clock::time_point steadyNow() {
    return std::chrono::steady_clock::now();
}

} // namespace cachewraith
