#include "cachewraith/util/Logger.h"
#include "cachewraith/util/Time.h"

#include <iostream>

namespace cachewraith {

namespace {

std::string levelName(LogLevel level) {
    switch (level) {
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    }
    return "INFO";
}

} // namespace

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::initialize(const std::filesystem::path& file) {
    std::lock_guard lock(mutex_);
    if (file.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(file.parent_path(), ec);
    }
    stream_.open(file, std::ios::app);
    initialized_ = stream_.is_open();
    if (!initialized_) {
        std::cerr << "CacheWraith: unable to open log file " << file << ", logging to stderr\n";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard lock(mutex_);
    const auto line = formatTime(std::chrono::system_clock::now()) + " [" + levelName(level) + "] " + message;
    if (initialized_) {
        stream_ << line << '\n';
        stream_.flush();
    } else {
        std::cerr << line << '\n';
    }
}

void Logger::info(const std::string& message) { log(LogLevel::Info, message); }
void Logger::warning(const std::string& message) { log(LogLevel::Warning, message); }
void Logger::error(const std::string& message) { log(LogLevel::Error, message); }
void Logger::debug(const std::string& message) { log(LogLevel::Debug, message); }

} // namespace cachewraith
