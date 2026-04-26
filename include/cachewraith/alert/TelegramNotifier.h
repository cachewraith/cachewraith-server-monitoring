#pragma once

#include <memory>
#include <optional>
#include <string>

namespace cachewraith {

class TelegramNotifier {
public:
    TelegramNotifier();

    bool configured() const;
    bool sendMessage(const std::string& text, std::optional<long long> message_thread_id = std::nullopt) const;
    bool sendMessageAsync(const std::string& text, std::optional<long long> message_thread_id = std::nullopt) const;
    bool sendTestMessage(const std::string& server_name, std::optional<long long> message_thread_id = std::nullopt) const;

private:
    struct QueuedMessage;
    struct State;

    static bool performSend(const std::string& token,
                            const std::string& chat_id,
                            const std::string& text,
                            std::optional<long long> message_thread_id);

    std::shared_ptr<State> state_;
};

} // namespace cachewraith
