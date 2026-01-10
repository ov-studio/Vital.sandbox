/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: public: rest.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rest Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/System/public/vital.h>
#include <curl/curl.h>


////////////////////////
// Vital: Tool: Rest //
////////////////////////

namespace Vital::System::Rest {
    struct CurlGlobal {
        CurlGlobal() { 
            if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
                throw std::runtime_error("curl_global_init failed");
        }
        ~CurlGlobal() { curl_global_cleanup(); }
    };

    static inline CurlGlobal global;
    static inline size_t CallbackHandle(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        static_cast<std::string*>(userp)->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    static inline std::string get(const std::string& url) {
        std::string buffer;
        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("curl_easy_init failed");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CallbackHandle);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "vital.sdk/1.0");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::string err = curl_easy_strerror(res);
            curl_easy_cleanup(curl);
            throw std::runtime_error("Request failed: " + err);
        }
        curl_easy_cleanup(curl);
        return buffer;
    }
}