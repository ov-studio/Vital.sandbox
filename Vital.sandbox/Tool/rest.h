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
    // TODO: Improve
    using rest_headers = std::vector<std::string>;

    inline std::string get(const std::string& url, const rest_headers& headers = {}) {
        std::string buffer;
        
        // Parse URL to extract host, port, path, and scheme
        std::string scheme, host, path;
        int port = 80;
        
        size_t protocol_end = url.find("://");
        if (protocol_end != std::string::npos) {
            scheme = url.substr(0, protocol_end);
            std::string rest = url.substr(protocol_end + 3);
            
            // Check for port in host:port format
            size_t path_start = rest.find("/");
            size_t port_pos = rest.find(":");
            
            std::string host_part;
            if (path_start != std::string::npos) {
                host_part = rest.substr(0, path_start);
                path = rest.substr(path_start);
            } else {
                host_part = rest;
                path = "/";
            }
            
            // Extract port if present
            if (port_pos != std::string::npos && (path_start == std::string::npos || port_pos < path_start)) {
                host = host_part.substr(0, port_pos);
                port = std::stoi(host_part.substr(port_pos + 1));
            } else {
                host = host_part;
                if (scheme == "https") port = 443;
            }
        } else {
            throw std::runtime_error("Invalid URL format");
        }
        
        // Build scheme://host:port for httplib Client constructor
        std::string scheme_host_port = scheme + "://" + host + ":" + std::to_string(port);
        
        // Client auto-detects SSL based on scheme (https://)
        httplib::Client cli(scheme_host_port);
        cli.set_connection_timeout(30, 0);
        cli.set_read_timeout(60, 0);
        
        // Convert headers
        httplib::Headers httplib_headers;
        for (const auto& h : headers) {
            size_t colon_pos = h.find(":");
            if (colon_pos != std::string::npos) {
                httplib_headers.insert({h.substr(0, colon_pos), h.substr(colon_pos + 1)});
            }
        }
        
        auto res = cli.Get(path.c_str(), httplib_headers, [&buffer](const char* data, size_t len) {
            buffer.append(data, len);
            return true;
        });
        
        if (!res) {
            throw std::runtime_error("Request failed: " + httplib::to_string(res.error()));
        }
        
        if (res->status != 200) {
            throw std::runtime_error("HTTP error: " + std::to_string(res->status));
        }
        
        return buffer;
    }

    inline std::string post(const std::string& url, const std::string& body, const rest_headers& headers = {}) {
        std::string buffer;
        
        // Parse URL to extract host, port, path, and scheme
        std::string scheme, host, path;
        int port = 80;
        
        size_t protocol_end = url.find("://");
        if (protocol_end != std::string::npos) {
            scheme = url.substr(0, protocol_end);
            std::string rest = url.substr(protocol_end + 3);
            
            // Check for port in host:port format
            size_t path_start = rest.find("/");
            size_t port_pos = rest.find(":");
            
            std::string host_part;
            if (path_start != std::string::npos) {
                host_part = rest.substr(0, path_start);
                path = rest.substr(path_start);
            } else {
                host_part = rest;
                path = "/";
            }
            
            // Extract port if present
            if (port_pos != std::string::npos && (path_start == std::string::npos || port_pos < path_start)) {
                host = host_part.substr(0, port_pos);
                port = std::stoi(host_part.substr(port_pos + 1));
            } else {
                host = host_part;
                if (scheme == "https") port = 443;
            }
        } else {
            throw std::runtime_error("Invalid URL format");
        }
        
        // Build scheme://host:port for httplib Client constructor
        std::string scheme_host_port = scheme + "://" + host + ":" + std::to_string(port);
        
        // Client auto-detects SSL based on scheme (https://)
        httplib::Client cli(scheme_host_port);
        cli.set_connection_timeout(30, 0);
        cli.set_read_timeout(60, 0);
        
        // Convert headers (add Content-Type if empty for JSON POST)
        httplib::Headers httplib_headers;
        bool has_content_type = false;
        for (const auto& h : headers) {
            size_t colon_pos = h.find(":");
            if (colon_pos != std::string::npos) {
                std::string key = h.substr(0, colon_pos);
                std::string value = h.substr(colon_pos + 1);
                httplib_headers.insert({key, value});
                if (key == "Content-Type" || key == "content-type") {
                    has_content_type = true;
                }
            }
        }
        
        // Add default Content-Type if not provided
        if (!has_content_type) {
            httplib_headers.insert({"Content-Type", "application/json"});
        }
        
        auto res = cli.Post(path.c_str(), httplib_headers, body.c_str(), body.size());
        
        if (!res) {
            throw std::runtime_error("Request failed: " + httplib::to_string(res.error()));
        }
        
        if (res->status != 200) {
            throw std::runtime_error("HTTP error: " + std::to_string(res->status));
        }
        
        return res->body;
    }
}