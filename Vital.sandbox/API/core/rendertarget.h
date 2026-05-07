/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: rendertarget.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rendertarget APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/rendertarget.h>


///////////////////////////////
// Vital: API: Rendertarget //
///////////////////////////////

namespace Vital::Sandbox::API {
    struct Rendertarget : vm_module {
        inline static const std::string base_name = "rendertarget";
        using base_class = Vital::Engine::Rendertarget;

        struct Instance : vm_instance<Instance> {
            using Owner = Rendertarget;
            base_class* rendertarget = nullptr;
        };
        inline static std::mutex mutex;
        inline static std::unordered_map<int, std::shared_ptr<Instance>> buffer;
        inline static std::atomic<int> next_id { 1 };

        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!Instance::erase(instance)) return;
            if (instance -> rendertarget) {
                instance -> rendertarget -> destroy();
                instance -> rendertarget = nullptr;
            }
            Instance::release(instance);
        }

        static std::shared_ptr<Instance> find_by_ptr(base_class* ptr) {
            if (!ptr) return nullptr;
            std::lock_guard<std::mutex> lock(mutex);
            for (auto& [id, instance] : buffer) {
                if (!instance -> destroyed && instance -> rendertarget == ptr) return instance;
            }
            return nullptr;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Rendertarget>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(size, transparent = false)")
                    .require(1, &Machine::is_vector2)
                    .optional(2, &Machine::is_bool);

                auto size = vm -> get_vector2(1);
                auto transparent = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                auto instance = Instance::init(vm);
                instance -> rendertarget = base_class::create(size, transparent);
                Instance::store(instance);
                vm -> create_object(base_name, instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(vm, -1);
                instance -> set_ref(instance -> self_reference(), -1);
                return 1;
            });

            API::bind(vm, {base_name}, "set_active", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(rendertarget, clear = false, instant = false)")
                    .optional(1, [](Machine* vm, int index) { return vm_module::is_userdata<Instance>(vm, base_name, index); })
                    .optional(2, &Machine::is_bool)
                    .optional(3, &Machine::is_bool);

                std::shared_ptr<Instance> instance = vm -> is_nil(1) ? nullptr : vm_module::get_userdata_object<Instance>(vm, 1);
                auto clear = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                auto instant = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                base_class::set_active(instance ? instance -> rendertarget : nullptr, clear, instant);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_active", [](auto vm, auto& id) -> int {
                auto ptr = base_class::get_active();
                auto instance = find_by_ptr(ptr);
                if (!instance) vm -> push_value(false);
                else instance -> vm -> get_reference(instance -> self_reference(), true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "get_size", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> rendertarget -> get_size());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "is_active", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> rendertarget -> is_active());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "set_active", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(clear = false, instant = false)")
                    .optional(2, &Machine::is_bool)
                    .optional(3, &Machine::is_bool);

                auto clear = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                auto instant = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                base_class::set_active(self -> rendertarget, clear, instant);
                vm -> push_value(true);
                return 1;
            });
        }

        static void clean(const std::string& env) {
            vm_module::collect_env<Instance>(mutex, buffer, env, clean_instance);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Rendertarget : vm_module {};
}
#endif