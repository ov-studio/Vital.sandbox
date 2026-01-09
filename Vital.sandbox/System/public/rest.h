/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: rest.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: REST System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/System/public/vital.h>
#include <curl/curl.h>


//////////////////////////
// Vital: System: REST //
//////////////////////////

namespace Vital::System::REST {
    struct Curl {
        Curl()  { curl_global_init(CURL_GLOBAL_DEFAULT); }
        ~Curl() { curl_global_cleanup(); }
    };
    
    struct Request {
        CURL* curl = nullptr;
        std::string response;
        std::promise<std::string> promise;
        std::function<void(std::string)> callback;
    };

    inline Curl curl_global_guard{};
    inline size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        auto* out = static_cast<std::string*>(userdata);
        out -> append(ptr, size*nmemb);
        return size*nmemb;
    }

    inline void set_curl_options(CURL* curl, std::string* response) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, Vital::Signature);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    }

    class Engine {
        private:
            CURLM* multi = nullptr;
            std::thread worker;
            std::atomic<bool> running{false};
            std::mutex queue_mutex;
            std::queue<std::unique_ptr<Request>> pending;
            std::unordered_map<CURL*, std::unique_ptr<Request>> requests;

            void loop() {
                int active = 0;
                while (running) {
                    {
                        std::lock_guard lock(queue_mutex);
                        while (!pending.empty()) {
                            auto req = std::move(pending.front());
                            pending.pop();
                            curl_multi_add_handle(multi, req -> curl);
                            requests[req -> curl] = std::move(req);
                        }
                    }
                    curl_multi_perform(multi, &active);
                    int msgs;
                    while (auto* msg = curl_multi_info_read(multi, &msgs)) {
                        if (msg -> msg == CURLMSG_DONE) {
                            auto it = requests.find(msg->easy_handle);
                            if (it != requests.end()) {
                                auto& req = it -> second;
                                if (msg -> data.result == CURLE_OK) {
                                    req -> promise.set_value(std::move(req -> response));
                                    if (req -> callback) req -> callback(req -> response);
                                }
                                else req -> promise.set_exception(std::make_exception_ptr(Vital::Error::fetch("request-failed", curl_easy_strerror(msg -> data.result))));
                                curl_multi_remove_handle(multi, req -> curl);
                                curl_easy_cleanup(req -> curl);
                                requests.erase(it);
                            }
                        }
                    }
                    curl_multi_poll(multi, nullptr, 0, 100, nullptr);
                }
            }
        public:
            Engine() {
                multi = curl_multi_init();
                running = true;
                worker = std::thread(&Engine::loop, this);
            }

            ~Engine() {
                running = false;
                if (worker.joinable()) worker.join();
                curl_multi_cleanup(multi);
            }

            std::future<std::string> get_async(std::string_view url, std::function<void(std::string)> callback = nullptr) {
                auto req = std::make_unique<Request>();
                req -> curl = curl_easy_init();
                req -> response.reserve(4096);
                curl_easy_setopt(req -> curl, CURLOPT_URL, std::string(url).c_str());
                set_curl_options(req -> curl, &req -> response);
                auto fut = req -> promise.get_future();
                req -> callback = std::move(callback);
                {
                    std::lock_guard lock(queue_mutex);
                    pending.push(std::move(req));
                }
                return fut;
            }

            [[nodiscard]]
            std::string get_sync(std::string_view url) {
                std::string response;
                CURL* curl = curl_easy_init();
                if (!curl) throw Vital::Error::fetch("curl-init-failed", "curl_easy_init failed");
                curl_easy_setopt(curl, CURLOPT_URL, std::string(url).c_str());
                set_curl_options(curl, &response);
                CURLcode result = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                if (result != CURLE_OK) {
                    throw Vital::Error::fetch("request-failed", curl_easy_strerror(result));
                }
                return response;
            }
    };
    inline Engine engine{};

    [[nodiscard]]
    inline std::string get_sync(std::string_view url) {
        return engine.get_sync(url);
    }

    [[nodiscard]]
    inline std::future<std::string> get_async(std::string_view url, std::function<void(std::string)> callback = nullptr) {
        return engine.get_async(url);
    }
}
