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
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/texture.h>
#include <Vital.sandbox/API/utility/file.h>


/////////////////////////////
// Vital: API: Adjustment //
/////////////////////////////

namespace Vital::Sandbox::API {
    struct Adjustment : vm_module {
        inline static const std::vector<std::string> base_scope = {"gfx", "adjustment"};
        inline static const std::string lut_reference = fmt::format("{}:lut", vm_module::scope_id(base_scope));
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "is_enabled", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> is_adjustment_enabled());
                return 1;
            });

            API::bind(vm, base_scope, "get_brightness", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_adjustment_brightness());
                return 1;
            });

            API::bind(vm, base_scope, "get_contrast", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_adjustment_contrast());
                return 1;
            });

            API::bind(vm, base_scope, "get_saturation", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_adjustment_saturation());
                return 1;
            });

            API::bind(vm, base_scope, "get_lut", [](auto vm, auto& id) -> int {
                if (vm -> is_reference("sandbox", lut_reference)) vm -> get_reference("sandbox", lut_reference, true);
                else vm -> push_value(false);
                return 1;
            });
        
            API::bind(vm, base_scope, "set_enabled", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_adjustment_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_brightness", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_adjustment_brightness(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_contrast", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_adjustment_contrast(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_saturation", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_adjustment_saturation(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_lut", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                auto ref = path;
                auto base = API::File::assert_file(vm, path);
                auto lut_texture = Vital::Engine::Texture::get_from_reference(ref);
                if (!lut_texture) lut_texture = Vital::Engine::Texture::create_texture_2d(base, path, ref);
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
                if (lut_size == 0) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid lut dimensions");
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
                vm -> set_reference("sandbox", lut_reference, -1);
                vm -> pop(1);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "reset_lut", [](auto vm, auto& id) -> int {
                base_class::get_environment() -> set_adjustment_color_correction(godot::Ref<godot::Texture>());
                vm -> del_reference("sandbox", lut_reference);
                vm -> push_value(true);
                return 1;
            });
    
            API::bind(vm, base_scope, "get_tonemapper", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_tonemapper()));
                return 1;
            });

            API::bind(vm, base_scope, "set_tonemapper", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require(1, &Machine::is_number)
                    .validate_enum(1, godot::Environment::TONE_MAPPER_LINEAR, godot::Environment::TONE_MAPPER_ACES);

                auto mode = static_cast<godot::Environment::ToneMapper>(vm -> get_int(1));
                base_class::get_environment() -> set_tonemapper(mode);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_tonemap_exposure", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_tonemap_exposure());
                return 1;
            });

            API::bind(vm, base_scope, "set_tonemap_exposure", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_tonemap_exposure(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_tonemap_white", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_tonemap_white());
                return 1;
            });

            API::bind(vm, base_scope, "set_tonemap_white", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_tonemap_white(value);
                vm -> push_value(true);
                return 1;
            });
        

            // Background / Sky / Ambient / Tonemap //

            API::bind(vm, base_scope, "get_background", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_background()));
                return 1;
            });

            API::bind(vm, base_scope, "set_background", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require(1, &Machine::is_number)
                    .validate_enum(1, godot::Environment::BG_CLEAR_COLOR, godot::Environment::BG_CAMERA_FEED);

                auto mode = static_cast<godot::Environment::BGMode>(vm -> get_int(1));
                base_class::get_environment() -> set_background(mode);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_sky_rotation", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sky_rotation());
                return 1;
            });

            API::bind(vm, base_scope, "set_sky_rotation", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(euler_radians)")
                    .require(1, &Machine::is_vector3);

                auto value = vm -> get_vector3(1);
                base_class::get_environment() -> set_sky_rotation(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_bg_color", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_bg_color());
                return 1;
            });

            API::bind(vm, base_scope, "set_bg_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                base_class::get_environment() -> set_bg_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_bg_intensity", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_bg_intensity());
                return 1;
            });

            API::bind(vm, base_scope, "set_bg_intensity", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(energy)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_bg_intensity(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_canvas_max_layer", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_canvas_max_layer());
                return 1;
            });

            API::bind(vm, base_scope, "set_canvas_max_layer", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(layer)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_int(1);
                base_class::get_environment() -> set_canvas_max_layer(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_camera_feed_id", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_camera_feed_id());
                return 1;
            });

            API::bind(vm, base_scope, "set_camera_feed_id", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(id)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_int(1);
                base_class::get_environment() -> set_camera_feed_id(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_ambient_color", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ambient_light_color());
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_color", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(color)")
                    .require(1, &Machine::is_color);

                auto value = vm -> get_color(1);
                base_class::get_environment() -> set_ambient_light_color(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_ambient_source", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_ambient_source()));
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_source", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(source)")
                    .require(1, &Machine::is_number)
                    .validate_enum(1, godot::Environment::AMBIENT_SOURCE_BG, godot::Environment::AMBIENT_SOURCE_SKY);

                auto source = static_cast<godot::Environment::AmbientSource>(vm -> get_int(1));
                base_class::get_environment() -> set_ambient_source(source);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_ambient_energy", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ambient_light_energy());
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_energy", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(energy)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ambient_light_energy(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_ambient_sky_contribution", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_ambient_light_sky_contribution());
                return 1;
            });

            API::bind(vm, base_scope, "set_ambient_sky_contribution", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(ratio)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_ambient_light_sky_contribution(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_reflection_source", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_environment() -> get_reflection_source()));
                return 1;
            });

            API::bind(vm, base_scope, "set_reflection_source", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(source)")
                    .require(1, &Machine::is_number)
                    .validate_enum(1, godot::Environment::REFLECTION_SOURCE_BG, godot::Environment::REFLECTION_SOURCE_SKY);

                auto source = static_cast<godot::Environment::ReflectionSource>(vm -> get_int(1));
                base_class::get_environment() -> set_reflection_source(source);
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