/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: rest.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rest Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <curl/curl.h>


////////////////////////
// Vital: Tool: Rest //
////////////////////////

namespace Vital::Tool::Rest {
    using rest_headers = std::vector<std::string>;

    struct CurlGlobal {
        CurlGlobal() { 
            if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
                throw std::runtime_error("curl_global_init failed");
        }
        ~CurlGlobal() { curl_global_cleanup(); }
    };

    inline CurlGlobal global;
    inline size_t CallbackHandle(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size*nmemb;
        static_cast<std::string*>(userp)->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    inline curl_slist* ApplyHeaders(const rest_headers &headers) {
        curl_slist* list = nullptr;
        for (const auto& header : headers) {
            list = curl_slist_append(list, header.c_str());
        }
        return list;
    }

    inline std::string get(const std::string& url, const rest_headers& headers = {}) {
        std::string buffer;
        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("curl_easy_init failed");
        curl_slist* rq_headers = ApplyHeaders(headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, rq_headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CallbackHandle);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, Vital::Signature);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(rq_headers);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            std::string err = curl_easy_strerror(res);
            throw std::runtime_error("Request failed: " + err);
        }
        return buffer;
    }

    inline std::string post(const std::string& url, const std::string& body, const char* content_type = "application/json") {
        std::string buffer;
        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("curl_easy_init failed");
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, content_type);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CallbackHandle);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, Vital::Signature);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            std::string err = curl_easy_strerror(res);
            throw std::runtime_error("Request failed: " + err);
        }
        return buffer;
    }
}