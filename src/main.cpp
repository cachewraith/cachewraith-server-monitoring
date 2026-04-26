#include "cachewraith/alert/TelegramNotifier.h"
#include "cachewraith/app/Application.h"
#include "cachewraith/app/Config.h"
#include "cachewraith/util/Logger.h"

#include <csignal>
#include <filesystem>
#include <iostream>
#include <memory>

namespace {

std::unique_ptr<cachewraith::Application> g_app;

void handleSignal(int) {
    if (g_app) {
        g_app->stop();
    }
}

void printHelp() {
    std::cout << "CacheWraith Server Monitoring Agent\n\n"
              << "Usage: cachewraith-monitor [options]\n\n"
              << "Options:\n"
              << "  --config PATH       Load config from PATH (default: ./config.json)\n"
              << "  --test-telegram     Send a test Telegram alert and exit\n"
              << "  --once              Run one monitoring cycle and exit\n"
              << "  --version           Print version and exit\n"
              << "  --help              Show this help\n";
}

} // namespace

int main(int argc, char** argv) {
    std::filesystem::path config_path = "./config.json";
    bool once = false;
    bool test_telegram = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--config") {
            if (i + 1 >= argc) {
                std::cerr << "--config requires a path\n";
                return 2;
            }
            config_path = argv[++i];
        } else if (arg == "--once") {
            once = true;
        } else if (arg == "--test-telegram") {
            test_telegram = true;
        } else if (arg == "--version") {
            std::cout << "cachewraith-monitor 1.0.0\n";
            return 0;
        } else if (arg == "--help") {
            printHelp();
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printHelp();
            return 2;
        }
    }

    try {
        auto config = cachewraith::Config::load(config_path);
        cachewraith::Logger::instance().initialize(config.log_file);

        if (test_telegram) {
            cachewraith::TelegramNotifier notifier;
            return notifier.sendTestMessage(config.server_name, config.telegram.default_thread_id) ? 0 : 1;
        }

        std::signal(SIGINT, handleSignal);
        std::signal(SIGTERM, handleSignal);

        g_app = std::make_unique<cachewraith::Application>(std::move(config));
        g_app->run(once);
        g_app.reset();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "CacheWraith error: " << ex.what() << "\n";
        return 1;
    }
}
