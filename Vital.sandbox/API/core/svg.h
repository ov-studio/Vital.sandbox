/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: svg.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SVG APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/texture.h>
#include <Vital.sandbox/API/utility/file.h>


//////////////////////
// Vital: API: SVG //
//////////////////////

namespace Vital::Sandbox::API {
    struct SVG : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "svg"};
        using base_class = Vital::Engine::Texture;

        inline static const std::vector<std::pair<std::string, godot::CanvasItem::TextureFilter>> texture_filter_registry = {
            { "DEFAULT",                     godot::CanvasItem::TEXTURE_FILTER_PARENT_NODE                      },
            { "NEAREST",                     godot::CanvasItem::TEXTURE_FILTER_NEAREST                          },
            { "LINEAR",                      godot::CanvasItem::TEXTURE_FILTER_LINEAR                           },
            { "NEAREST_MIPMAP",              godot::CanvasItem::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS             },
            { "LINEAR_MIPMAP",               godot::CanvasItem::TEXTURE_FILTER_LINEAR_WITH_MIPMAPS              },
            { "NEAREST_MIPMAP_ANISOTROPIC",  godot::CanvasItem::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC },
            { "LINEAR_MIPMAP_ANISOTROPIC",   godot::CanvasItem::TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC  }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = SVG;
            base_class* texture = nullptr;

            bool is_alive() const { 
                return texture ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> texture) {
                    instance -> texture -> destroy();
                    instance -> texture = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<SVG>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path, mipmaps = false)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_bool);

                auto path = vm -> get_string(1);
                auto base = API::File::assert_file(vm, path);
                auto mipmaps = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                auto instance = Instance::init(vm);
                instance -> texture = base_class::create_svg(base, path, mipmaps);
                instance -> store(true);
                return 1;
            });

            API::bind(vm, base_scope, "create_from_raw", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(raw, mipmaps = false)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_bool);

                auto raw = vm -> get_string(1);
                auto mipmaps = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                auto instance = Instance::init(vm);
                instance -> texture = base_class::create_svg_from_raw(raw, mipmaps);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "get_size", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> texture -> get_size());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_filter", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(static_cast<int>(self -> texture -> get_filter()));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_filter", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, texture_filter_registry);

                auto mode = static_cast<godot::CanvasItem::TextureFilter>(vm -> get_int(2));
                self -> texture -> set_filter(mode);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "update", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(raw)", true)
                    .require(2, &Machine::is_string);

                auto raw = vm -> get_string(2);
                self -> texture -> update_svg_from_raw(raw);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "texture_filter", texture_filter_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct SVG : vm_module {};
}
#endif