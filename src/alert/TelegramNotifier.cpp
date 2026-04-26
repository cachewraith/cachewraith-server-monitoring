#include "cachewraith/alert/TelegramNotifier.h"

#include "cachewraith/util/Env.h"
#include "cachewraith/util/Logger.h"
#include "cachewraith/util/StringUtils.h"

#include <curl/curl.h>

namespace cachewraith {

TelegramNotifier::TelegramNotifier()
    : token_(getEnv("CACHEWRAITH_TELEGRAM_BOT_TOKEN")),
      chat_id_(getEnv("CACHEWRAITH_TELEGRAM_CHAT_ID")) {}

bool TelegramNotifier::configured() const {
    return token_.has_value() && chat_id_.has_value();
}

bool TelegramNotifier::sendMessage(const std::string& text, std::optional<long long> message_thread_id) const {
    if (!configured()) {
        Logger::instance().warning("Telegram is not configured; set CACHEWRAITH_TELEGRAM_BOT_TOKEN and CACHEWRAITH_TELEGRAM_CHAT_ID");
        return false;
    }

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        Logger::instance().error("Unable to initialize libcurl for Telegram request");
        return false;
    }

    const std::string url = "https://api.telegram.org/bot" + *token_ + "/sendMessage";
    std::string body = "chat_id=" + urlEncodeComponent(*chat_id_) +
                       "&disable_web_page_preview=true&text=" + urlEncodeComponent(text);
    if (message_thread_id) {
        body += "&message_thread_id=" + std::to_string(*message_thread_id);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
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

bool TelegramNotifier::sendTestMessage(const std::string& server_name, std::optional<long long> message_thread_id) const {
    return sendMessage("CacheWraith test alert\nServer: " + server_name + "\nStatus: Telegram notifications are working",
                       message_thread_id);
}

} // namespace cachewraith
