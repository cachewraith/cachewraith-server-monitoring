#include "cachewraith/monitor/WebsiteMonitor.h"

#include <curl/curl.h>

namespace cachewraith {

WebsiteMonitor::WebsiteMonitor(Config config) : config_(std::move(config)) {}

std::string WebsiteMonitor::name() const { return "website"; }

namespace {

size_t discardBody(char* ptr, size_t size, size_t nmemb, void*) {
    (void)ptr;
    return size * nmemb;
}

} // namespace

std::vector<Alert> WebsiteMonitor::check() {
    std::vector<Alert> alerts;
    for (const auto& url : config_.websites) {
        CURL* curl = curl_easy_init();
        if (curl == nullptr) {
            continue;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discardBody);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, config_.thresholds.website_timeout_seconds);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, config_.thresholds.website_timeout_seconds);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "cachewraith-monitor/1.0");

        const CURLcode result = curl_easy_perform(curl);
        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_easy_cleanup(curl);

        if (result != CURLE_OK || status < 200 || status >= 400) {
            Alert alert;
            alert.type = "website_down";
            alert.key = url;
            alert.title = "WEBSITE HEALTH CHECK FAILED";
            alert.severity = AlertSeverity::Critical;
            alert.message = "Server: " + config_.server_name + "\nURL: " + url +
                            "\nResult: " + (result == CURLE_OK ? "HTTP " + std::to_string(status)
                                                                 : curl_easy_strerror(result));
            alerts.push_back(std::move(alert));
        }
    }
    return alerts;
}

} // namespace cachewraith
