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


//////////////////////////
// Vital: API: Texture //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Texture {
        inline static const std::vector<std::pair<std::string, godot::CanvasItem::TextureFilter>> texture_filter_registry = {
            { "DEFAULT",                     godot::CanvasItem::TEXTURE_FILTER_PARENT_NODE                      },
            { "NEAREST",                     godot::CanvasItem::TEXTURE_FILTER_NEAREST                          },
            { "LINEAR",                      godot::CanvasItem::TEXTURE_FILTER_LINEAR                           },
            { "NEAREST_MIPMAP",              godot::CanvasItem::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS             },
            { "LINEAR_MIPMAP",               godot::CanvasItem::TEXTURE_FILTER_LINEAR_WITH_MIPMAPS              },
            { "NEAREST_MIPMAP_ANISOTROPIC",  godot::CanvasItem::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC },
            { "LINEAR_MIPMAP_ANISOTROPIC",   godot::CanvasItem::TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC  }
        };

        // TODO: RENAME LIKE AUDIO EFFECT.H
        template<typename BoundInstance>
        static void bind_filter_methods(Machine* vm) {
            vm_module::bind_method<BoundInstance>(vm, "has_mipmaps", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> texture -> has_mipmaps());
                return 1;
            });

            vm_module::bind_method<BoundInstance>(vm, "get_size", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> texture -> get_size());
                return 1;
            });

            vm_module::bind_method<BoundInstance>(vm, "get_filter", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(static_cast<int>(self -> texture -> get_filter()));
                return 1;
            });

            vm_module::bind_method<BoundInstance>(vm, "set_filter", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, texture_filter_registry);

                auto mode = static_cast<godot::CanvasItem::TextureFilter>(vm -> get_int(2));
                self -> texture -> set_filter(mode);
                vm -> push_value(true);
                return 1;
            });
        }
    };
}
#endif
