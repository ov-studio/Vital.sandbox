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
#include <Vital.extension/Engine/public/console.h>


/////////////////////////////
// Vital: Engine: Console //
/////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Console::Console() {
        #if defined(Vital_SDK_Client)
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
        #else
            #if defined(Vital_SDK_WINDOWS)
                AllocConsole();
                SetConsoleTitleA("Vital.server");
                SetConsoleOutputCP(CP_UTF8);
                SetConsoleCP(CP_UTF8);
                freopen("CONOUT$", "w", stdout);
                freopen("CONOUT$", "w", stderr);
                freopen("CONIN$",  "r", stdin);
                HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
                DWORD mode;
                GetConsoleMode(hStdin, &mode);
                SetConsoleMode(hStdin, mode | ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
            #elif defined(Vital_SDK_MACOS) || defined(Vital_SDK_LINUX)
                tcgetattr(STDIN_FILENO, &stdin_termios);
                struct termios term = stdin_termios;
                term.c_lflag |= (ICANON | ECHO | ECHOE | ECHOK | ISIG);
                tcsetattr(STDIN_FILENO, TCSANOW, &term);
            #endif
            stdin_running = true;
            stdin_thread = std::thread([this]() {
                std::string line;
                while (stdin_running) {
                    std::cout << "> " << std::flush;
                    if (!std::getline(std::cin, line)) break;
                    command(line);
                }
            });
            stdin_thread.detach();
        #endif
    }

    Console::~Console() {
        #if defined(Vital_SDK_Client)
            if (!webview) return;
            webview -> destroy();
            webview = nullptr;
        #else
            stdin_running = false;
            #if defined(Vital_SDK_WINDOWS)
                FreeConsole();
            #elif defined(Vital_SDK_MACOS) || defined(Vital_SDK_LINUX)
                tcsetattr(STDIN_FILENO, TCSANOW, &stdin_termios);
            #endif
        #endif
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
        #if defined(Vital_SDK_Client)
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
        #else
            godot::UtilityFunctions::print(to_godot_string(message));
        #endif
    }

    void Console::command(const std::string& input) {
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
    #if defined(Vital_SDK_Client)
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
    #endif
}