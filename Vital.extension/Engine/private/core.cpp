/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: core.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Core Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/core.h>
#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Sandbox/index.h>
#include <Vital.sandbox/Tool/event.h>


////////////////////////////
// Vital: Engine: Engine //
////////////////////////////

namespace Vital::Engine {
    // Instantiators //    
    void Core::_ready() {
        singleton = singleton ? singleton : this;
        set_process(true);
        #if defined(Vital_SDK_Client)
        set_process_unhandled_key_input(true);
        get_environment();
        #else
        start_console();
        #endif
        if (Vital::is_editor()) return;
        Vital::Tool::Event::emit("vital.core:ready");
    }

    void Core::_exit_tree() {
        #if defined(Vital_SDK_Client)
        free_environment();
        #else
        stop_console();
        #endif
        if (Vital::is_editor()) return;
        Vital::Tool::Event::emit("vital.core:free");
    }

    void Core::_process(double delta) {
        if (Vital::is_editor()) return;
        Sandbox::get_singleton() -> process(delta);
    }

    #if defined(Vital_SDK_Client)
    void Core::_unhandled_input(godot::Ref<godot::InputEvent> event) {
        if (Vital::is_editor()) return;
        Sandbox::get_singleton() -> input(event);
    }
    #else
    // TODO; Make it crossplatform
    void Core::start_console() {
        #if defined(_WIN32)
        // Allocate a real console window for the server
        AllocConsole();
        SetConsoleTitleA("Vital.server");
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        freopen("CONIN$",  "r", stdin);
    
        // Enable echo + line input so typing is visible
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, mode | ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
        #endif
    
        stdin_running = true;
        stdin_thread = std::thread([this]() {
            std::string line;
            while (stdin_running) {
                std::cout << "> " << std::flush;
                if (!std::getline(std::cin, line)) break;
                if (line.empty()) continue;
        
                std::istringstream iss(line);
                std::vector<std::string> tokens;
                std::string token;
                while (iss >> token) tokens.push_back(token);
                if (tokens.empty()) continue;
        
                std::vector<std::string> parameters(tokens.begin() + 1, tokens.end());
                Vital::Tool::Stack arguments;
                arguments.object["command"] = tokens[0];
                arguments.object["parameters"] = parameters;
                Vital::Tool::Event::emit("vital.sandbox:console_input", arguments);
            }
        });
        stdin_thread.detach();
    }
    
    void Core::stop_console() {
        stdin_running = false;
        #if defined(_WIN32)
        FreeConsole();
        #endif
    }
    #endif


    // Getters //
    Core* Core::get_singleton() {
        return singleton;
    }

    godot::Node* Core::get_root() {
        auto tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton() -> get_main_loop());
        return tree ? tree -> get_root() : nullptr;
    }
    
    #if defined(Vital_SDK_Client)
    godot::RenderingServer* Core::get_rendering_server() {
        return godot::RenderingServer::get_singleton();
    }
    
    godot::Ref<godot::Environment> Core::get_environment() {
        if (!environment) {
            environment = memnew(godot::WorldEnvironment);
            get_singleton() -> call_deferred("add_child", environment);
            godot::Ref<godot::Environment> env;
            env.instantiate();
            environment -> set_environment(env);
        }
        return environment -> get_environment();
    }

    void Core::free_environment() {
        if (!environment) return;
        environment -> queue_free();
        environment = nullptr;
    }
    #endif
}