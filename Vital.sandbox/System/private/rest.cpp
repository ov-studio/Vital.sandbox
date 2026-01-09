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
#include <Vital.sandbox/System/public/rest.h>
#include <curl/curl.h>


//////////////////////////
// Vital: System: REST //
//////////////////////////

namespace Vital::System::REST {
    size_t CallbackHandle(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalsize = size*nmemb;
        userp -> append((char*)contents, size*nmemb);
        return totalsize;
    }

    std::string get(std::string& url) {
        try {
            std::string buffer;
            curl_global_init(CURL_GLOBAL_DEFAULT);
            CURL* curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CallbackHandle);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "vital.sdk/1.0");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            CURLcode result = curl_easy_perform(curl);
            if (result != CURLE_OK) throw Vital::Error::fetch("request-failed", curl_easy_strerror(result));
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return buffer;
        }
        catch(...) { std::rethrow_exception(std::current_exception()); }
    }
}
