/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: masterlist.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Masterlist Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

// TODO: Improve
#pragma once
#include <Vital.sandbox/Manager/public/masterlist.h>
#if !defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Manager/public/kit.h>
#include <Vital.sandbox/Tool/http.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>


///////////////////////////////////
// Vital: Manager: Masterlist //
///////////////////////////////////

// TOOD: Improve
namespace Vital::Manager {
    // Internal //
    void Masterlist::send_heartbeat() const {
        if (!server_config) return;

        auto nm = Network::get_singleton();
        rapidjson::Document document;
        document.SetObject();
        auto& alloc = document.GetAllocator();
        document.AddMember(rapidjson::StringRef("token"), rapidjson::Value(server_config -> get_masterlist_token().c_str(), alloc), alloc);
        document.AddMember(rapidjson::StringRef("name"), rapidjson::Value(server_config -> get_server_name().c_str(), alloc), alloc);
        document.AddMember(rapidjson::StringRef("ip"), rapidjson::Value(nm -> get_server_ip().c_str(), alloc), alloc);
        document.AddMember(rapidjson::StringRef("port"), rapidjson::Value(server_config -> get_network_port()), alloc);
        document.AddMember(rapidjson::StringRef("httpPort"), rapidjson::Value(server_config -> get_http_port()), alloc);
        document.AddMember(rapidjson::StringRef("players"), rapidjson::Value(nm -> get_peer_count()), alloc);
        document.AddMember(rapidjson::StringRef("maxPlayers"), rapidjson::Value(server_config -> get_max_clients()), alloc);
        document.AddMember(rapidjson::StringRef("version"), rapidjson::Value(server_config -> get_server_version().c_str(), alloc), alloc);
        document.AddMember(rapidjson::StringRef("description"), rapidjson::Value(server_config -> get_server_description().c_str(), alloc), alloc);
        document.AddMember(rapidjson::StringRef("discord"), rapidjson::Value(server_config -> get_discord().c_str(), alloc), alloc);
        document.AddMember(rapidjson::StringRef("website"), rapidjson::Value(server_config -> get_website().c_str(), alloc), alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        try { Tool::HTTP::post(server_config -> get_masterlist_url() + "/heartbeat", buffer.GetString(), {}, 15); }
        catch (const std::exception& e) { log("warn", fmt::format("heartbeat failed — {}", e.what())); }
        catch (...) { log("warn", "heartbeat failed — unknown error"); }
    }

    void Masterlist::send_offline() const {
        if (!server_config) return;

        rapidjson::Document document;
        document.SetObject();
        auto& alloc = document.GetAllocator();
        document.AddMember(rapidjson::StringRef("token"), rapidjson::Value(server_config -> get_masterlist_token().c_str(), alloc), alloc);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        try { Tool::HTTP::del(server_config -> get_masterlist_url() + "/heartbeat", buffer.GetString(), {}, 10); }
        catch (const std::exception& e) { log("warn", fmt::format("deregister failed — {}", e.what())); }
    }


    // Helpers //
    int Masterlist::get_interval_seconds() {
        int32_t value = Manager::Kit::fetch_json_value("config/masterlist", "interval").as<int32_t>();
        return value > 0 ? value : 300;
    }

    int Masterlist::get_debounce_seconds() {
        int32_t value = Manager::Kit::fetch_json_value("config/masterlist", "debounce").as<int32_t>();
        return value > 0 ? value : 5;
    }


    // Managers //
    void Masterlist::start(const Config::Server& config) {
        if (active) return;
        if (!config.get_masterlist_enabled()) return;
        if (config.get_masterlist_token().empty()) {
            log("warn", "masterlist enabled but token missing in config.yaml — skipping");
            return;
        }

        server_config = &config;
        active = true;
        send_heartbeat();

        const int interval_s = get_interval_seconds();
        timer = Tool::Timer::create([this](Tool::Timer*, int) {
            Engine::Core::execute([this]() { send_heartbeat(); });
        }, interval_s * 1000, 0);
        log("sbox", fmt::format("reporting to masterlist every {}s", interval_s));
    }

    void Masterlist::refresh() {
        if (!active) return;
        std::lock_guard<std::mutex> lock(debounce_mutex);
        if (debounce_timer) return;

        const int debounce_ms = get_debounce_seconds() * 1000;
        debounce_timer = Tool::Timer::create([this](Tool::Timer*, int) {
            Engine::Core::execute([this]() {
                send_heartbeat();
                std::lock_guard<std::mutex> inner_lock(debounce_mutex);
                debounce_timer = nullptr;
            });
        }, debounce_ms, 1);
    }

    void Masterlist::stop() {
        if (!active) return;
        
        if (timer) {
            timer -> stop();
            timer = nullptr;
        }
        {
            std::lock_guard<std::mutex> lock(debounce_mutex);
            if (debounce_timer) {
                debounce_timer -> stop();
                debounce_timer = nullptr;
            }
        }
        send_offline();
        active = false;
        server_config = nullptr;
    }

    void Masterlist::teardown() {
        stop();
    }


    // State //
    bool Masterlist::is_active() const {
        return active;
    }
}
#endif