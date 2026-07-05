/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: http.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: HTTP Tools
 ----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <httplib.h>


////////////////////////
// Vital: Tool: HTTP //
////////////////////////

namespace Vital::Tool::HTTP {
    using http_headers = std::vector<std::string>;

    inline httplib::Client make_client(const std::string& url, std::string& out_path, int connect_timeout = 10, int timeout = 30, bool follow_redirects = true) {
        size_t protocol_end = url.find("://");
        if (protocol_end == std::string::npos) throw std::runtime_error("Invalid URL format");
        std::string scheme = url.substr(0, protocol_end);
        std::string rest = url.substr(protocol_end + 3);
        size_t path_start = rest.find("/");
        size_t port_pos = rest.find(":");
        std::string host_part = (path_start != std::string::npos) ? rest.substr(0, path_start) : rest;
        out_path = (path_start != std::string::npos) ? rest.substr(path_start) : "/";
        int port;
        std::string host;
        if (port_pos != std::string::npos && (path_start == std::string::npos || port_pos < path_start)) {
            host = host_part.substr(0, port_pos);
            try { port = std::stoi(host_part.substr(port_pos + 1)); }
            catch (const std::exception&) { throw std::runtime_error("Invalid URL format"); }
        }
        else {
            host = host_part;
            port = (scheme == "https") ? 443 : 80;
        }
        httplib::Client cli(scheme + "://" + host + ":" + std::to_string(port));
        cli.set_connection_timeout(connect_timeout, 0);
        cli.set_read_timeout(timeout, 0);
        cli.set_follow_location(follow_redirects);
        return cli;
    }

    inline httplib::Headers make_headers(const http_headers& headers, std::string* out_content_type = nullptr) {
        httplib::Headers result;
        result.insert({ "User-Agent", "Vital.sandbox/" + Vital::Build.to_string() });
        for (const auto& h : headers) {
            size_t colon_pos = h.find(":");
            if (colon_pos == std::string::npos) continue;
            std::string key = h.substr(0, colon_pos);
            std::string value = h.substr(colon_pos + 1);
            size_t value_start = value.find_first_not_of(" \t");
            if (value_start != std::string::npos) value = value.substr(value_start);
            if (out_content_type && (key == "Content-Type" || key == "content-type")) *out_content_type = value;
            else result.insert({key, value});
        }
        return result;
    }

    inline std::string get(const std::string& url, const http_headers& headers = {}, int timeout = 60, bool follow_redirects = true, const std::atomic<bool>* cancelled = nullptr) {
        std::string path;
        auto cli = make_client(url, path, 10, timeout, follow_redirects);
        auto httplib_headers = make_headers(headers);
        std::string buffer;
        auto res = cli.Get(path.c_str(), httplib_headers, [&buffer, cancelled](const char* data, size_t len) {
            if (cancelled && cancelled->load()) return false;
            buffer.append(data, len);
            return true;
        });
        if (!res) {
            if (cancelled && res.error() == httplib::Error::Canceled) return "";
            throw std::runtime_error("Request failed: " + httplib::to_string(res.error()));
        }
        if (res -> status != 200) throw std::runtime_error("HTTP error: " + std::to_string(res -> status));
        return buffer;
    }

    inline std::string post(const std::string& url, const std::string& body, const http_headers& headers = {}, int timeout = 60) {
        std::string path;
        auto cli = make_client(url, path, 10, timeout, false);
        std::string content_type = "application/json";
        auto httplib_headers = make_headers(headers, &content_type);
        auto res = cli.Post(path.c_str(), httplib_headers, body.c_str(), body.size(), content_type.c_str());
        if (!res) throw std::runtime_error("Request failed: " + httplib::to_string(res.error()));
        if (res -> status != 200) throw std::runtime_error("HTTP error: " + std::to_string(res -> status));
        return res -> body;
    }

    class Server {
        private:
            std::unique_ptr<httplib::Server> server;
            std::thread thread;
            std::atomic<bool> running { false };
            std::string bind_address = "127.0.0.1";
            int port = 0;
            std::string label = "HTTP";
        public:
            Server() = default;
            ~Server() { stop(); }
            Server(const Server&) = delete;
            Server& operator=(const Server&) = delete;

            bool is_running() const { 
                return running.load();
            }

            void set_bind_address(const std::string& address) { 
                bind_address = address;
            }

            void set_port(int p) { 
                port = p;
            }

            void set_label(const std::string& l) { 
                label = l;
            }

            int get_port() const { 
                return port;
            }

            std::string get_url(const std::string& path = "") const {
                std::string base = "http://" + bind_address + ":" + std::to_string(port);
                if (path.empty()) return base;
                return base + (path.front() == '/' ? path : "/" + path);
            }
        private:
            bool is_port_taken(int check_port) const {
                const std::string probe_host = (bind_address == "0.0.0.0") ? "127.0.0.1" : bind_address;
                httplib::Client probe(probe_host, check_port);
                probe.set_connection_timeout(0, 200000);
                probe.set_read_timeout(0, 200000);
                auto res = probe.Get("/");
                return res.error() != httplib::Error::Connection;
            }
        public:
            template<typename Handler>
            void get(const std::string& pattern, Handler handler) {
                if (!server) server = std::make_unique<httplib::Server>();
                server -> Get(pattern, std::move(handler));
            }
            
            void add_mount(const std::string& prefix, const std::string& directory) {
                if (!server) server = std::make_unique<httplib::Server>();
                server -> set_mount_point(prefix, directory);
            }
    
            bool start(bool random_port = false) {
                if (running.load()) return true;
                if (!server) server = std::make_unique<httplib::Server>();

                if (random_port) {
                    int bound_port = server -> bind_to_any_port(bind_address);
                    if (bound_port <= 0) {
                        Tool::print("error", fmt::format("{}: HTTP server failed to bind to an available port", label));
                        return false;
                    }
                    port = bound_port;
                }
                else {
                    if (is_port_taken(port)) {
                        Tool::print("error", fmt::format("{}: HTTP server failed to bind port {} — already in use", label, port));
                        return false;
                    }
                    if (!server -> bind_to_port(bind_address, port)) {
                        Tool::print("error", fmt::format("{}: HTTP server failed to bind port {}", label, port));
                        return false;
                    }
                }

                running.store(true);
                thread = std::thread([this]() {
                    Tool::print("sbox", fmt::format("{}: HTTP server starting on port {}", label, port));
                    server -> listen_after_bind();
                    Tool::print("sbox", fmt::format("{}: HTTP server stopped", label));
                });
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                Tool::print("sbox", fmt::format("{}: HTTP server running on port {}", label, port));
                return true;
            }

            void stop() {
                if (!running.load()) return;
                running.store(false);
                if (server) server -> stop();
                if (thread.joinable()) thread.join();
                server.reset();
            }
    };
}