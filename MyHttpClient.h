#ifndef SCRAPER_MYHTTPCLIENT_H
#define SCRAPER_MYHTTPCLIENT_H

#ifdef HAVE_CURL

#include "tgbot/net/HttpClient.h"
#include "tgbot/net/Url.h"
#include "tgbot/net/HttpReqArg.h"
#include "tgbot/net/HttpParser.h"

#include <curl/curl.h>

#include <string>
#include <vector>

class MyHttpClient : public TgBot::HttpClient {

public:
    MyHttpClient();
    ~MyHttpClient() override;

    /**
     * @brief Sends a request to the url.
     *
     * If there's no args specified, a GET request will be sent, otherwise a POST request will be sent.
     * If at least 1 arg is marked as file, the content type of a request will be multipart/form-data, otherwise it will be application/x-www-form-urlencoded.
     */
    std::string makeRequest(const TgBot::Url& url, const std::vector<TgBot::HttpReqArg>& args) const override;

    /**
     * @brief Raw curl settings storage for fine tuning.
     */
    CURL* curlSettings;

    std::string makeRequest(const std::string &url, const std::vector<TgBot::HttpReqArg> &args) const;

private:
    const TgBot::HttpParser _httpParser;

};


MyHttpClient::MyHttpClient() : _httpParser() {
    curlSettings = curl_easy_init();

    curl_easy_setopt(curlSettings, CURLOPT_CONNECTTIMEOUT, 20);
    curl_easy_setopt(curlSettings, CURLOPT_TIMEOUT, 25);
}

MyHttpClient::~MyHttpClient() {
    curl_easy_cleanup(curlSettings);
}

static std::size_t curlWriteString(char* ptr, std::size_t size, std::size_t nmemb, void* userdata) {
    static_cast<std::string*>(userdata)->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string MyHttpClient::makeRequest(const TgBot::Url& url, const std::vector<TgBot::HttpReqArg>& args) const {
    return "";
}

std::string MyHttpClient::makeRequest(const std::string& url, const std::vector<TgBot::HttpReqArg>& args) const {
    // Copy settings for each call because we change CURLOPT_URL and other stuff.
    // This also protects multithreaded case.
    auto curl = curl_easy_duphandle(curlSettings);

//    std::string u = url.protocol + "://" + url.host + url.path;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // disable keep-alive
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Connection: close");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_mime* mime;
    curl_mimepart* part;
    mime = curl_mime_init(curl);
    if (!args.empty()) {
        for (const TgBot::HttpReqArg& a : args) {
            part = curl_mime_addpart(mime);

            curl_mime_data(part, a.value.c_str(), a.value.size());
            curl_mime_type(part, a.mimeType.c_str());
            curl_mime_name(part, a.name.c_str());
            if (a.isFile) {
                curl_mime_filename(part, a.fileName.c_str());
            }
        }
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteString);

    char errbuf[CURL_ERROR_SIZE] {};
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    auto res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_mime_free(mime);

    // If the request did not complete correctly, show the error
    // information. If no detailed error information was written to errbuf
    // show the more generic information from curl_easy_strerror instead.
    if (res != CURLE_OK) {
        size_t len = strlen(errbuf);
        std::string errmsg;
        if (len) {
            errmsg = std::string(errbuf) + ((errbuf[len - 1] != '\n') ? "\n" : "");
        }
        else {
            errmsg = curl_easy_strerror(res);
        }
        throw std::runtime_error(std::string("curl error: ") + errmsg);
    }

    return _httpParser.extractBody(response);
}


#endif //SCRAPER_MYHTTPCLIENT_H
#endif