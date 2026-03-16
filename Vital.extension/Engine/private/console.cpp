/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: console.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Console Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.extension/Engine/public/console.h>
#include <Vital.extension/Engine/public/core.h>
#include <Vital.extension/Engine/public/canvas.h>


/////////////////////////////
// Vital: Engine: Console //
/////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Console::Console() {
        webview = Webview::create();
        webview -> set_position({0, 0});
        webview -> set_visible(true);
        webview -> set_fullscreen(true);
        webview -> set_transparent(true);
        webview -> set_autoplay(false);
        webview -> set_zoomable(false);
        webview -> set_devtools_visible(false);
        webview -> load_html(Vital::Tool::fetch_module("console")); 
        webview -> set_message_handler([this](godot::String message) {
            this -> on_message(message);
        });
    }

    Console::~Console() {
        if (!webview) return;
        webview -> destroy(); 
        webview = nullptr;
    }


    // Utils //
    Console* Console::get_singleton() {
        if (!singleton) singleton = memnew(Console);
        return singleton;
    }

    void Console::free_singleton() {
        if (!singleton) return;
        memdelete(singleton);
        singleton = nullptr;
    }


    // APIs //
    void Console::print(const std::string& mode, const std::string& message) {
        if (mode.empty() || message.empty()) return;
        rapidjson::Document document;
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.SetObject();
        auto& alloc = document.GetAllocator();
        document.AddMember("action",  "print", alloc);
        document.AddMember("mode", rapidjson::Value(mode.c_str(), alloc), alloc);
        document.AddMember("message", rapidjson::Value(message.c_str(), alloc), alloc);
        document.Accept(writer);
        webview -> emit(buffer.GetString());
    }

    void Console::command(const std::string& input) {
        if (input.empty()) return;
        std::istringstream iss(input);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) tokens.push_back(token);
        if (tokens.empty()) return;
        Vital::Tool::Stack arguments;
        arguments.object["command"] = tokens[0];
        arguments.object["parameters"] = std::vector<std::string>(tokens.begin() + 1, tokens.end());
        Vital::Tool::Event::emit("vital.sandbox:console_input", arguments);
    }


    // Events //
    void Console::on_message(godot::String message) {
        rapidjson::Document document;
        document.Parse(to_std_string(message).c_str());
        if (document.HasParseError() || !document.HasMember("action")) return;
        std::string action = document["action"].GetString();
        if (action == "input") {
            if (!document.HasMember("message") || !document["message"].IsString()) return;
            command(document["message"].GetString());
        }
    }
}
#endif