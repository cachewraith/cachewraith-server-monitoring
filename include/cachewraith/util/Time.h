#pragma once

#include <chrono>
#include <string>

namespace cachewraith {

std::string formatTime(std::chrono::system_clock::time_point time);
std::chrono::steady_clock::time_point steadyNow();

} // namespace cachewraith
