#pragma once

#include <string>
#include <string_view>

namespace cachewraith {

std::string trim(std::string_view value);
std::string toLower(std::string_view value);
bool containsCaseInsensitive(std::string_view haystack, std::string_view needle);
bool isSafeSystemdServiceName(std::string_view service);
std::string urlEncodeComponent(std::string_view value);

} // namespace cachewraith
