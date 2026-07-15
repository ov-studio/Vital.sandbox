/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: texture.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Texture APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/texture.h>
#include <Vital.sandbox/API/utility/file.h>


//////////////////////////
// Vital: API: Texture //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Texture : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "texture"};
        using base_class = Vital::Engine::Texture;

        inline static const std::vector<std::pair<std::string, godot::Image::Format>> texel_format_registry = {
            { "RGBA8",    godot::Image::FORMAT_RGBA8    },
            { "RGB8",     godot::Image::FORMAT_RGB8     },
            { "RGBA4444", godot::Image::FORMAT_RGBA4444 },
            { "RGB565",   godot::Image::FORMAT_RGB565   },
            { "LA8",      godot::Image::FORMAT_LA8      },
            { "L8",       godot::Image::FORMAT_L8       }
        };

        inline static const std::vector<std::pair<std::string, godot::Image::CompressMode>> compression_mode_registry = {
            { "S3TC", godot::Image::COMPRESS_S3TC },
            { "ETC2", godot::Image::COMPRESS_ETC2 },
            { "BPTC", godot::Image::COMPRESS_BPTC },
            { "ASTC", godot::Image::COMPRESS_ASTC }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Texture;
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
            vm_module::register_type<Texture>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                auto base = API::File::assert_file(vm, path);
                auto instance = Instance::init(vm);
                instance -> texture = base_class::create_texture_2d(base, path);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "is_compressed", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> texture -> is_compressed());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_size", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> texture -> get_size());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "convert", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(format)", true)
                    .require_enum(2, texel_format_registry);

                auto format = static_cast<godot::Image::Format>(vm -> get_int(2));
                self -> texture -> convert(format);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "compress", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, compression_mode_registry);

                auto mode = static_cast<godot::Image::CompressMode>(vm -> get_int(2));
                self -> texture -> compress(mode);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "texel_format", texel_format_registry);
            vm -> scope_set_enum(base_scope, "compression_mode", compression_mode_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Texture : vm_module {};
}
#endif