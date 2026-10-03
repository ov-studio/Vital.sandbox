/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: viewport.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Viewport APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////
// Vital: API: Viewport //
//////////////////////////

// TODO: Improve
namespace Vital::Sandbox::API {
    struct Viewport : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "viewport"};

        struct Instance : vm_instance<Instance> {
            using Owner = Viewport;

            godot::Ref<godot::ViewportTexture> get_texture() const {
                return Vital::Engine::Core::get_singleton() ->get_scene_root()->get_texture();
            }

            bool is_alive() const { return true; }

            // Screen is a global singleton — destroy() is a no-op.
            // Scripts cannot meaningfully destroy the main viewport texture.
            void clean() {}
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<Viewport>(vm);

            API::bind(vm, base_scope, "get_texture", [](auto vm, auto& id) -> int {
                std::shared_ptr<Instance> inst;
                {
                    std::lock_guard<std::mutex> lock(registry.mutex);
                    if (!registry.buffer.empty()) {
                        auto it = registry.buffer.begin();
                        if (it->second && it->second->is_alive()) inst = it->second;
                    }
                }
                if (!inst) Instance::make(vm, true);
                else inst -> push_self(vm);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::bind(vm, base_scope, "get_resolution", [](auto vm, auto& id) -> int {
                vm -> push_value(Vital::Engine::Core::get_singleton() -> get_resolution());
                return 1;
            });

            #if defined(VSDK_Client)
            API::bind(vm, base_scope, "screenshot", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                Vital::Engine::Core::get_singleton() -> capture_screenshot(API::File::get_base(vm, path, true), path);
                vm -> push_value(true);
                return 1;
            });
            #endif
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Viewport : vm_module {};
}
#endif