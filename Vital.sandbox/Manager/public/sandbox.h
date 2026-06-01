/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: sandbox.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/canvas.h>


/////////////////////////////
// Vital: Manager: Sandbox //
/////////////////////////////

namespace Vital::Manager {
    class Sandbox;
    class Sandbox : public godot::Node {
        protected:
            inline static Sandbox* singleton = nullptr;
            inline static const std::string signal_reference = "vital.network:execute";
        private:
            Vital::Sandbox::Machine* vm = nullptr;
            mutable std::mutex mutex;
            std::unordered_map<std::string, std::unordered_map<std::string, int>> exports;
        public:
            // Instantiators //
            Sandbox();
            ~Sandbox();


            // Singleton //
            static Sandbox* get_singleton();
            static void free_singleton();


            // Managers //
            void ready();
            void drain();
            void process(double delta);
            #if defined(Vital_SDK_Client)
            void draw(Engine::Canvas* canvas);
            void input(godot::Ref<godot::InputEvent> event);
            #endif

            template<typename... Args>
            void signal(const std::string& name, Args&&... args) {
                static_assert((std::is_same_v<std::decay_t<Args>, Tool::StackValue> && ...), "Sandbox::signal() expects StackValue arguments");
                Tool::Stack stack;
                stack.array.reserve(sizeof...(Args));
                (stack.array.emplace_back(std::forward<Args>(args)), ...);
                Engine::Core::get_singleton() -> execute([this, name, stack = std::move(stack)]() {
                    Tool::Event::emit(name, stack);
                    if (!vm || !vm -> is_reference(signal_reference)) return;
                    vm -> get_reference(signal_reference, true);
                    vm -> push_value(name);
                    for (const auto& value : stack.array) vm -> push_value(value);
                    vm -> pcall(static_cast<int>(stack.array.size()) + 1, 0);
                });
            }


            // Getters //
            Vital::Sandbox::Machine* get_vm();


            // Exports //
			// TODO: Improve
            bool export_register(const std::string& resource, const std::string& fn_name, int lua_ref);
            void export_clear(const std::string& resource);
            std::vector<std::string> export_list(const std::string& resource) const;
            int export_get_ref(const std::string& resource, const std::string& fn_name) const;
    };
}