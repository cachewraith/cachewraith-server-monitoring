#include "cachewraith/util/StringUtils.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace cachewraith {

std::string trim(std::string_view value) {
    auto begin = value.begin();
    auto end = value.end();
    while (begin != end && std::isspace(static_cast<unsigned char>(*begin))) {
        ++begin;
    }
    while (begin != end && std::isspace(static_cast<unsigned char>(*(end - 1)))) {
        --end;
    }
    return std::string(begin, end);
}

std::string toLower(std::string_view value) {
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

bool containsCaseInsensitive(std::string_view haystack, std::string_view needle) {
    return toLower(haystack).find(toLower(needle)) != std::string::npos;
}

bool isSafeSystemdServiceName(std::string_view service) {
    if (service.empty() || service.size() > 128) {
        return false;
    }
    return std::all_of(service.begin(), service.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '_' || c == '-' || c == '.' || c == '@';
    });
}

std::string urlEncodeComponent(std::string_view value) {
    std::ostringstream encoded;
    encoded << std::hex << std::uppercase;
    for (const unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        }
    }
    return encoded.str();
}

} // namespace cachewraith
