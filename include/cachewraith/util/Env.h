#pragma once

#include <optional>
#include <string>

namespace cachewraith {

std::optional<std::string> getEnv(const char* name);

} // namespace cachewraith
