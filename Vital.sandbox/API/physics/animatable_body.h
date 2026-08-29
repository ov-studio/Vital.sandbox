/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: animatable_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Animatable Body APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/animatable_body.h>
#include <Vital.sandbox/API/core/physics_body.h>


//////////////////////////////////
// Vital: API: Animatable_Body //
//////////////////////////////////

namespace Vital::Sandbox::API {
    struct Animatable_Body : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "animatable"};
        using base_class = Vital::Engine::Animatable_Body;

        struct Instance : vm_instance<Instance> {
            using Owner = Animatable_Body;
            base_class* body = nullptr;

            auto get_node() {
                return body;
            }

            bool is_alive() const {
                return body ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> body) {
                    instance -> body -> destroy();
                    instance -> body = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static std::shared_ptr<Instance> find_by_ptr(base_class* ptr) {
            if (!ptr) return nullptr;
            std::lock_guard<std::mutex> lock(registry.mutex);
            for (auto& [id, instance] : registry.buffer) {
                if (Instance::find_unlocked(instance) && (instance -> body == ptr)) return instance;
            }
            return nullptr;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Animatable_Body>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> body = base_class::create();
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Physics_Body::methods<Instance, Physics_Body::Type::Animatable>(vm);
        }

        static void inject(Machine* vm) {
            API::Physics_Body::inject<Instance>(vm);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
