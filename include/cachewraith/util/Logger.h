#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

namespace cachewraith {

enum class LogLevel { Debug, Info, Warning, Error };

class Logger {
public:
    static Logger& instance();

    void initialize(const std::filesystem::path& file);
    void log(LogLevel level, const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void debug(const std::string& message);

private:
    Logger() = default;

    std::mutex mutex_;
    std::ofstream stream_;
    bool initialized_{false};
};

} // namespace cachewraith
