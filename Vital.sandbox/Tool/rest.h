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
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <Vital.sandbox/Tool/index.h>
#include <httplib.h>


////////////////////////
// Vital: Tool: Rest //
////////////////////////

namespace Vital::Tool::Rest {
    using rest_headers = std::vector<std::string>;

    inline httplib::Client make_client(const std::string& url, std::string& out_path, int connect_timeout = 10, int read_timeout = 30, bool follow_redirects = true) {
        size_t protocol_end = url.find("://");
        if (protocol_end == std::string::npos) throw std::runtime_error("Invalid URL format");
        std::string scheme = url.substr(0, protocol_end);
        std::string rest   = url.substr(protocol_end + 3);
        size_t path_start  = rest.find("/");
        size_t port_pos    = rest.find(":");
        std::string host_part = (path_start != std::string::npos) ? rest.substr(0, path_start) : rest;
        out_path = (path_start != std::string::npos) ? rest.substr(path_start) : "/";
        int port;
        std::string host;
        if (port_pos != std::string::npos && (path_start == std::string::npos || port_pos < path_start)) {
            host = host_part.substr(0, port_pos);
            port = std::stoi(host_part.substr(port_pos + 1));
        } else {
            host = host_part;
            port = (scheme == "https") ? 443 : 80;
        }
        httplib::Client cli(scheme + "://" + host + ":" + std::to_string(port));
        cli.set_connection_timeout(connect_timeout, 0);
        cli.set_read_timeout(read_timeout, 0);
        cli.set_follow_location(follow_redirects);
        return cli;
    }

    inline httplib::Headers make_headers(const rest_headers& headers, std::string* out_content_type = nullptr) {
        httplib::Headers result;
        for (const auto& h : headers) {
            size_t colon_pos = h.find(":");
            if (colon_pos == std::string::npos) continue;
            std::string key   = h.substr(0, colon_pos);
            std::string value = h.substr(colon_pos + 1);
            size_t value_start = value.find_first_not_of(" \t");
            if (value_start != std::string::npos) value = value.substr(value_start);
            if (out_content_type && (key == "Content-Type" || key == "content-type")) *out_content_type = value;
            else result.insert({key, value});
        }
        return result;
    }

    inline std::string get(const std::string& url, const rest_headers& headers = {}, int read_timeout = 60, bool follow_redirects = true) {
        std::string path;
        auto cli = make_client(url, path, 10, read_timeout, follow_redirects);
        auto httplib_headers = make_headers(headers);
        std::string buffer;
        auto res = cli.Get(path.c_str(), httplib_headers, [&buffer](const char* data, size_t len) {
            buffer.append(data, len);
            return true;
        });
        if (!res) throw std::runtime_error("Request failed: " + httplib::to_string(res.error()));
        if (res -> status != 200) throw std::runtime_error("HTTP error: " + std::to_string(res -> status));
        return buffer;
    }

    inline std::string post(const std::string& url, const std::string& body, const rest_headers& headers = {}, int read_timeout = 60) {
        std::string path;
        auto cli = make_client(url, path, 10, read_timeout, false);
        std::string content_type = "application/json";
        auto httplib_headers = make_headers(headers, &content_type);
        auto res = cli.Post(path.c_str(), httplib_headers, body.c_str(), body.size(), content_type.c_str());
        if (!res) throw std::runtime_error("Request failed: " + httplib::to_string(res.error()));
        if (res -> status != 200) throw std::runtime_error("HTTP error: " + std::to_string(res -> status));
        return res -> body;
    }
}