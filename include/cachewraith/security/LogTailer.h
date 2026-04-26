#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace cachewraith {

class LogTailer {
public:
    explicit LogTailer(std::filesystem::path path);

    std::vector<std::string> readNewLines();
    const std::filesystem::path& path() const;

private:
    std::filesystem::path path_;
    std::uintmax_t offset_{0};
    std::uintmax_t last_size_{0};
    bool initialized_{false};
};

} // namespace cachewraith
