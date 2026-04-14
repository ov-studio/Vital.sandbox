/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: adjustment.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Adjustment APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/texture.h>


/////////////////////////////
// Vital: API: Adjustment //
/////////////////////////////

// TODO: Update API
namespace Vital::Sandbox::API {
    struct Adjustment : vm_module {
        inline static const std::string base_name = "gfx";
        inline static const std::string lut_reference = fmt::format("{}.adjustment:lut", base_name);
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, {base_name, "adjustment"}, "is_enabled", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> is_adjustment_enabled());
                return 1;
            });
    
            API::bind(vm, {base_name, "adjustment"}, "set_enabled", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_adjustment_enabled(state);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "adjustment"}, "set_brightness", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_adjustment_brightness(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "adjustment"}, "get_brightness", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> get_adjustment_brightness());
                return 1;
            });
        
            API::bind(vm, {base_name, "adjustment"}, "set_contrast", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_adjustment_contrast(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "adjustment"}, "get_contrast", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> get_adjustment_contrast());
                return 1;
            });
        
            API::bind(vm, {base_name, "adjustment"}, "set_saturation", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_adjustment_saturation(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "adjustment"}, "get_saturation", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> get_adjustment_saturation());
                return 1;
            });

            API::bind(vm, {base_name, "adjustment"}, "set_lut", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                auto lut_texture = Vital::Engine::Texture::get_from_reference(path);
                if (!lut_texture) lut_texture = Vital::Engine::Texture::create_texture_2d(path, path);
                godot::Ref<godot::Image> lut_image = lut_texture -> get_texture() -> get_image();
                int img_w = lut_image -> get_width();
                int img_h = lut_image -> get_height();
                int lut_size = 0;
                int grid_cols = 0;
                int grid_rows = 0;
                for (int size = 64; size >= 2; size /= 2) {
                    int grid = (int)godot::Math::round(godot::Math::sqrt((double)size));
                    if ((grid*grid == size) && (img_w == size*grid) && (img_h == size*grid)) {
                        lut_size = size;
                        grid_cols = grid;
                        grid_rows = grid;
                        break;
                    }
                    if ((img_w == size*size) && (img_h == size)) {
                        lut_size = size;
                        grid_cols = size;
                        grid_rows = 1;
                        break;
                    }
                }
                if (lut_size == 0) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error, "Invalid lut dimensions");
                godot::TypedArray<godot::Image> lut_slices;
                lut_slices.resize(lut_size);
                for (int b = 0; b < lut_size; b++) {
                    int tile_x = (b%grid_cols)*lut_size;
                    int tile_y = (b/grid_cols)*lut_size;
                    godot::Ref<godot::Image> slice = godot::Image::create(lut_size, lut_size, false, godot::Image::FORMAT_RGBA8);
                    for (int g = 0; g < lut_size; g++) {
                        for (int r = 0; r < lut_size; r++) {
                            godot::Color color = lut_image -> get_pixel(tile_x + r, tile_y + g);
                            slice -> set_pixel(r, g, color);
                        }
                    }
                    lut_slices[b] = slice;
                }
                godot::Ref<godot::ImageTexture3D> lut_texture_3d;
                lut_texture_3d.instantiate();
                lut_texture_3d -> create(godot::Image::FORMAT_RGBA8, lut_size, lut_size, lut_size, false, lut_slices);
                base_class::get_environment() -> set_adjustment_color_correction(lut_texture_3d);
                vm -> push_value(path);
                vm -> set_reference(lut_reference, -1);
                vm -> pop(1);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name, "adjustment"}, "get_lut", [](auto vm) -> int {
                if (vm -> is_reference(lut_reference)) vm -> get_reference(lut_reference, true);
                else vm -> push_value(false);
                return 1;
            });

            API::bind(vm, {base_name, "adjustment"}, "reset_lut", [](auto vm) -> int {
                base_class::get_environment() -> set_adjustment_color_correction(godot::Ref<godot::Texture>());
                vm -> del_reference(lut_reference);
                vm -> push_value(true);
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Adjustment : vm_module {};
}
#endif