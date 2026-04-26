#include "cachewraith/alert/TelegramNotifier.h"

#include "cachewraith/util/Env.h"
#include "cachewraith/util/Logger.h"
#include "cachewraith/util/StringUtils.h"

#include <curl/curl.h>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>

namespace cachewraith {

struct TelegramNotifier::QueuedMessage {
    std::string text;
    std::optional<long long> message_thread_id;
};

struct TelegramNotifier::State {
    State(std::optional<std::string> token_value, std::optional<std::string> chat_id_value)
        : token(std::move(token_value)), chat_id(std::move(chat_id_value)) {
        if (configured()) {
            worker = std::thread([this] { run(); });
        }
    }

    ~State() {
        {
            std::lock_guard lock(mutex);
            stopping = true;
        }
        cv.notify_one();
        if (worker.joinable()) {
            worker.join();
        }
    }

    bool configured() const {
        return token.has_value() && chat_id.has_value();
    }

    bool enqueue(QueuedMessage message) {
        if (!configured()) {
            Logger::instance().warning("Telegram is not configured; set CACHEWRAITH_TELEGRAM_BOT_TOKEN and CACHEWRAITH_TELEGRAM_CHAT_ID");
            return false;
        }

        {
            std::lock_guard lock(mutex);
            queue.push_back(std::move(message));
        }
        cv.notify_one();
        return true;
    }

    void run() {
        while (true) {
            QueuedMessage message;
            {
                std::unique_lock lock(mutex);
                cv.wait(lock, [this] { return stopping || !queue.empty(); });
                if (stopping && queue.empty()) {
                    return;
                }
                message = std::move(queue.front());
                queue.pop_front();
            }

            if (!TelegramNotifier::performSend(*token, *chat_id, message.text, message.message_thread_id)) {
                Logger::instance().error("Telegram background delivery failed");
            }
        }
    }

    std::optional<std::string> token;
    std::optional<std::string> chat_id;
    std::mutex mutex;
    std::condition_variable cv;
    std::deque<QueuedMessage> queue;
    bool stopping{false};
    std::thread worker;
};

TelegramNotifier::TelegramNotifier()
    : state_(std::make_shared<State>(getEnv("CACHEWRAITH_TELEGRAM_BOT_TOKEN"),
                                     getEnv("CACHEWRAITH_TELEGRAM_CHAT_ID"))) {}

bool TelegramNotifier::configured() const {
    return state_ && state_->configured();
}

bool TelegramNotifier::performSend(const std::string& token,
                                    const std::string& chat_id,
                                    const std::string& text,
                                    std::optional<long long> message_thread_id) {
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        Logger::instance().error("Unable to initialize libcurl for Telegram request");
        return false;
    }

    const std::string url = "https://api.telegram.org/bot" + token + "/sendMessage";
    std::string body = "chat_id=" + urlEncodeComponent(chat_id) +
                       "&disable_web_page_preview=true&text=" + urlEncodeComponent(text);
    if (message_thread_id) {
        body += "&message_thread_id=" + std::to_string(*message_thread_id);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 8L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cachewraith-monitor/1.0");

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    const CURLcode result = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK) {
        Logger::instance().error(std::string("Telegram request failed: ") + curl_easy_strerror(result));
        return false;
    }
    if (status < 200 || status >= 300) {
        Logger::instance().error("Telegram returned HTTP status " + std::to_string(status));
        return false;
    }
    return true;
}

bool TelegramNotifier::sendMessage(const std::string& text, std::optional<long long> message_thread_id) const {
    if (!state_ || !state_->configured()) {
        Logger::instance().warning("Telegram is not configured; set CACHEWRAITH_TELEGRAM_BOT_TOKEN and CACHEWRAITH_TELEGRAM_CHAT_ID");
        return false;
    }

    return performSend(*state_->token, *state_->chat_id, text, message_thread_id);
}

bool TelegramNotifier::sendMessageAsync(const std::string& text, std::optional<long long> message_thread_id) const {
    return state_ && state_->enqueue(QueuedMessage{text, message_thread_id});
}

bool TelegramNotifier::sendTestMessage(const std::string& server_name, std::optional<long long> message_thread_id) const {
    return sendMessage("CacheWraith test alert\nServer: " + server_name + "\nStatus: Telegram notifications are working",
                       message_thread_id);
}

} // namespace cachewraith
