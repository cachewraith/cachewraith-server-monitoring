#include "cachewraith/security/LogTailer.h"

#include "cachewraith/util/Logger.h"

#include <fstream>

namespace cachewraith {

LogTailer::LogTailer(std::filesystem::path path) : path_(std::move(path)) {}

const std::filesystem::path& LogTailer::path() const { return path_; }

std::vector<std::string> LogTailer::readNewLines() {
    std::error_code ec;
    if (!std::filesystem::exists(path_, ec)) {
        if (initialized_) {
            Logger::instance().warning("Log file is missing: " + path_.string());
        }
        return {};
    }

    const auto size = std::filesystem::file_size(path_, ec);
    if (ec) {
        Logger::instance().warning("Unable to stat log file: " + path_.string());
        return {};
    }

    if (!initialized_) {
        offset_ = size;
        last_size_ = size;
        initialized_ = true;
        return {};
    }

    if (size < last_size_) {
        Logger::instance().info("Log rotation detected for " + path_.string());
        offset_ = 0;
    }

    std::ifstream input(path_);
    if (!input) {
        Logger::instance().warning("Unable to read log file: " + path_.string());
        return {};
    }
    input.seekg(static_cast<std::streamoff>(offset_));

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        lines.push_back(line);
    }
    offset_ = static_cast<std::uintmax_t>(input.tellg() >= 0 ? input.tellg() : static_cast<std::streampos>(size));
    last_size_ = size;
    return lines;
}

} // namespace cachewraith
