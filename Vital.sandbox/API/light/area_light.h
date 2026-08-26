/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: light: area_light.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Area Light APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/area_light.h>
#include <Vital.sandbox/API/light/light_3d.h>


/////////////////////////////
// Vital: API: Area_Light //
/////////////////////////////

namespace Vital::Sandbox::API {
    struct Area_Light : vm_module {
        inline static const std::vector<std::string> base_scope = {"light", "area"};
        using base_class = Vital::Engine::Area_Light;

        inline static const std::vector<std::pair<std::string, base_class::BakeMode>> bake_mode_registry = {
            { "DISABLED", base_class::BAKE_DISABLED },
            { "STATIC",   base_class::BAKE_STATIC   },
            { "DYNAMIC",  base_class::BAKE_DYNAMIC  }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Area_Light;
            base_class* light = nullptr;

            auto get_node() {
                return light;
            }

            bool is_alive() const {
                return light ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> light) {
                    instance -> light -> destroy();
                    instance -> light = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static std::shared_ptr<Instance> find_by_ptr(base_class* ptr) {
            if (!ptr) return nullptr;
            std::lock_guard<std::mutex> lock(registry.mutex);
            for (auto& [id, instance] : registry.buffer) {
                if (Instance::find_unlocked(instance) && (instance -> light == ptr)) return instance;
            }
            return nullptr;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Area_Light>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> light = base_class::create();
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Light_3D::methods<Instance, Light_3D::Type::Area>(vm);

            vm_module::bind_method<Instance>(vm, "is_area_normalizing_energy", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> is_area_normalizing_energy());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_area", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_area_size());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_texture", [](auto vm, auto self, auto& id) -> int {
                if (self -> vm -> is_reference("runtime", self -> value_reference("texture"))) self -> get_reference(self -> value_reference("texture"), true);
                else vm -> push_value(false);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_area", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size)", true)
                    .require(2, &Machine::is_vector2);

                auto size = vm -> get_vector2(2);
                self -> light -> set_area_size(size);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_area_normalize_energy", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> light -> set_area_normalize_energy(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_texture", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(path)", true)
                    .require(2, &Machine::is_string);

                auto path = vm -> get_string(2);
                auto ref = path;
                auto base = API::File::assert_file(vm, path);
                auto texture = Vital::Engine::Texture::get_from_reference(ref);
                if (!texture) texture = Vital::Engine::Image::create(base, path, true, ref);
                self -> light -> set_area_texture(texture -> get_texture());
                vm -> push_value(path);
                self -> set_reference(self -> value_reference("texture"), -1);
                vm -> pop(1);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "reset_texture", [](auto vm, auto self, auto& id) -> int {
                self -> light -> set_area_texture(godot::Ref<godot::Texture2D>());
                self -> del_reference(self -> value_reference("texture"));
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Light_3D::inject<Instance>(vm);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Area_Light : vm_module {};
}
#endif
