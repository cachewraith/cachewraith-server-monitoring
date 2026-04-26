#pragma once

#include <optional>
#include <string>

namespace cachewraith {

class TelegramNotifier {
public:
    TelegramNotifier();

    bool configured() const;
    bool sendMessage(const std::string& text, std::optional<long long> message_thread_id = std::nullopt) const;
    bool sendTestMessage(const std::string& server_name, std::optional<long long> message_thread_id = std::nullopt) const;

private:
    std::optional<std::string> token_;
    std::optional<std::string> chat_id_;
};

} // namespace cachewraith
