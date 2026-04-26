#include "cachewraith/util/Env.h"

#include <cstdlib>

namespace cachewraith {

std::optional<std::string> getEnv(const char* name) {
    if (const char* value = std::getenv(name); value != nullptr && *value != '\0') {
        return std::string(value);
    }
    return std::nullopt;
}

} // namespace cachewraith
