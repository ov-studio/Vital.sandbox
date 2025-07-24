/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: rest.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: REST System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/rest.h>
#include <curl/curl.h>


/////////////////////////
// Vital: Type: Event //
/////////////////////////

namespace Vital::System::REST {
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalSize = size*nmemb;
        userp -> append((char*)contents, totalSize);
        return totalSize;
    }

    std::string get(const std::string& url) {
        std::string buffer;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        CURL* curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "vital.sdk/1.0");
        CURLcode result = curl_easy_perform(curl);
        if (result != CURLE_OK) throw std::runtime_error(curl_easy_strerror(result))
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return buffer;
    }
}
