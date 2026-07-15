/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: canvas.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Canvas APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/canvas.h>
#include <Vital.sandbox/API/core/texture.h>
#include <Vital.sandbox/API/core/rendertarget.h>


/////////////////////////
// Vital: API: Canvas //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Canvas : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "engine"};
        using base_class = Vital::Engine::Canvas;

        inline static const std::vector<std::pair<std::string, int>> horizontal_alignment_registry = {
            { "LEFT",   godot::HORIZONTAL_ALIGNMENT_LEFT   },
            { "CENTER", godot::HORIZONTAL_ALIGNMENT_CENTER },
            { "RIGHT",  godot::HORIZONTAL_ALIGNMENT_RIGHT  },
            { "FILL",   godot::HORIZONTAL_ALIGNMENT_FILL   }
        };

        inline static const std::vector<std::pair<std::string, int>> vertical_alignment_registry = {
            { "TOP",    godot::VERTICAL_ALIGNMENT_TOP    },
            { "CENTER", godot::VERTICAL_ALIGNMENT_CENTER },
            { "BOTTOM", godot::VERTICAL_ALIGNMENT_BOTTOM },
            { "FILL",   godot::VERTICAL_ALIGNMENT_FILL   }
        };

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "world_to_screen", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(position, padding = 0)")
                    .require(1, &Machine::is_vector3)
                    .optional(2, &Machine::is_number);

                auto position = vm -> get_vector3(1);
                auto padding = vm -> is_number(2) ? vm -> get_float(2) : 0.0f;
                auto result = base_class::get_singleton() -> world_to_screen(position, padding);
                vm -> push_value(godot::Vector2(result.x, result.y));
                vm -> push_value(result.z);
                return 2;
            });

            API::bind(vm, base_scope, "screen_to_world", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(position, depth = 1)")
                    .require(1, &Machine::is_vector2)
                    .optional(2, &Machine::is_number);

                auto position = vm -> get_vector2(1);
                auto depth = vm -> is_number(2) ? vm -> get_float(2) : 1.0f;
                vm -> push_value(base_class::get_singleton() -> screen_to_world(position, depth));
                return 1;
            });

            API::bind(vm, base_scope, "draw_line", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(points, thickness, color = {1, 1, 1, 1})")
                    .require(1, &Machine::is_vector2_array)
                    .optional(2, &Machine::is_number)
                    .optional(3, &Machine::is_color);

                auto points = vm -> get_vector2_array(1);
                auto thickness = vm -> is_number(2) ? vm -> get_float(2) : 0.0f;
                auto color = vm -> is_color(3) ? vm -> get_color(3) : godot::Color{1, 1, 1, 1};
                base_class::get_singleton() -> draw_line(points, thickness, color);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "draw_polygon", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(points, color = {1, 1, 1, 1}, stroke = 0, stroke_color = {1, 1, 1, 1}, rotation = 0, pivot = {0, 0})")
                    .require(1, &Machine::is_vector2_array)
                    .optional(2, &Machine::is_color)
                    .optional(3, &Machine::is_number)
                    .optional(4, &Machine::is_color)
                    .optional(5, &Machine::is_number)
                    .optional(6, &Machine::is_vector2);

                auto points = vm -> get_vector2_array(1);
                auto color = vm -> is_color(2) ? vm -> get_color(2) : godot::Color{1, 1, 1, 1};
                auto stroke = vm -> is_number(3) ? vm -> get_float(3) : 0.0f;
                auto stroke_color = vm -> is_color(4) ? vm -> get_color(4) : godot::Color{1, 1, 1, 1};
                auto rotation = vm -> is_number(5) ? vm -> get_float(5) : 0.0f;
                auto pivot = vm -> is_vector2(6) ? vm -> get_vector2(6) : godot::Vector2{0.0f, 0.0f};
                base_class::get_singleton() -> draw_polygon(points, color, stroke, stroke_color, rotation, pivot);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "draw_rectangle", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(position, size, color = {1, 1, 1, 1}, stroke = 0, stroke_color = {1, 1, 1, 1}, rotation = 0, pivot = {0, 0})")
                    .require(1, &Machine::is_vector2)
                    .require(2, &Machine::is_vector2)
                    .optional(3, &Machine::is_color)
                    .optional(4, &Machine::is_number)
                    .optional(5, &Machine::is_color)
                    .optional(6, &Machine::is_number)
                    .optional(7, &Machine::is_vector2);

                auto position = vm -> get_vector2(1);
                auto size = vm -> get_vector2(2);
                auto color = vm -> is_color(3) ? vm -> get_color(3) : godot::Color{1, 1, 1, 1};
                auto stroke = vm -> is_number(4) ? vm -> get_float(4) : 0.0f;
                auto stroke_color = vm -> is_color(5) ? vm -> get_color(5) : godot::Color{1, 1, 1, 1};
                auto rotation = vm -> is_number(6) ? vm -> get_float(6) : 0.0f;
                auto pivot = vm -> is_vector2(7) ? vm -> get_vector2(7) : godot::Vector2{0.0f, 0.0f};
                base_class::get_singleton() -> draw_rectangle(position, size, color, stroke, stroke_color, rotation, pivot);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "draw_circle", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(position, radius, color = {1, 1, 1, 1}, stroke = 0, stroke_color = {1, 1, 1, 1}, rotation = 0, pivot = {0, 0})")
                    .require(1, &Machine::is_vector2)
                    .require(2, &Machine::is_number)
                    .optional(3, &Machine::is_color)
                    .optional(4, &Machine::is_number)
                    .optional(5, &Machine::is_color)
                    .optional(6, &Machine::is_number)
                    .optional(7, &Machine::is_vector2);

                auto position = vm -> get_vector2(1);
                auto radius = vm -> get_float(2);
                auto color = vm -> is_color(3) ? vm -> get_color(3) : godot::Color{1, 1, 1, 1};
                auto stroke = vm -> is_number(4) ? vm -> get_float(4) : 0.0f;
                auto stroke_color = vm -> is_color(5) ? vm -> get_color(5) : godot::Color{1, 1, 1, 1};
                auto rotation = vm -> is_number(6) ? vm -> get_float(6) : 0.0f;
                auto pivot = vm -> is_vector2(7) ? vm -> get_vector2(7) : godot::Vector2{0.0f, 0.0f};
                base_class::get_singleton() -> draw_circle(position, radius, color, stroke, stroke_color, rotation, pivot);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "draw_image", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(position, size, material, rotation = 0, pivot = {0, 0}, color = {1, 1, 1, 1})")
                    .require(1, &Machine::is_vector2)
                    .require(2, &Machine::is_vector2)
                    .require(3, [](Machine* vm, int idx) {
                        return vm -> is_string(idx)
                            || vm_module::is_userdata<API::Texture::Instance>(vm, idx)
                            || vm_module::is_userdata<API::Rendertarget::Instance>(vm, idx)
                            || vm_module::is_userdata<API::SVG::Instance>(vm, idx);
                    })
                    .optional(4, &Machine::is_number)
                    .optional(5, &Machine::is_vector2)
                    .optional(6, &Machine::is_color);

                auto position = vm -> get_vector2(1);
                auto size = vm -> get_vector2(2);
                auto rotation = vm -> is_number(4) ? vm -> get_float(4) : 0.0f;
                auto pivot = vm -> is_vector2(5) ? vm -> get_vector2(5) : godot::Vector2{0.0f, 0.0f};
                auto color = vm -> is_color(6) ? vm -> get_color(6) : godot::Color{1, 1, 1, 1};
                if (vm -> is_string(3)) {
                    base_class::get_singleton() -> draw_image(position, size, vm -> get_string(3), rotation, pivot, color);
                }
                else if (vm_module::is_userdata<API::Texture::Instance>(vm, 3)) {
                    auto instance = vm_module::get_userdata_object<API::Texture::Instance>(vm, 3);
                    base_class::get_singleton() -> draw_image(position, size, instance -> texture, rotation, pivot, color);
                }
                else if (vm_module::is_userdata<API::Rendertarget::Instance>(vm, 3)) {
                    auto instance = vm_module::get_userdata_object<API::Rendertarget::Instance>(vm, 3);
                    base_class::get_singleton() -> draw_image(position, size, instance -> rendertarget, rotation, pivot, color);
                }
                else {
                    auto instance = vm_module::get_userdata_object<API::SVG::Instance>(vm, 3);
                    base_class::get_singleton() -> draw_image(position, size, instance -> texture, rotation, pivot, color);
                }
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "draw_text", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(text, start_at, end_at, font, font_size, color = {1, 1, 1, 1}, horizontal_alignment = core.engine.horizontal_alignment.LEFT, vertical_alignment = core.engine.horizontal_alignment.TOP, clip = false, wordwrap = false, stroke = 0, stroke_color = {1, 1, 1, 1}, rotation = 0, pivot = {0, 0})")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_vector2)
                    .require(3, &Machine::is_vector2)
                    .require(4, [](Machine* vm, int idx) { return vm_module::is_userdata<API::Font::Instance>(vm, idx); })
                    .require(5, &Machine::is_number)
                    .optional(6, &Machine::is_color)
                    .optional_enum(7, horizontal_alignment_registry)
                    .optional_enum(8, vertical_alignment_registry)
                    .optional(9, &Machine::is_bool)
                    .optional(10, &Machine::is_bool)
                    .optional(11, &Machine::is_number)
                    .optional(12, &Machine::is_color)
                    .optional(13, &Machine::is_number)
                    .optional(14, &Machine::is_vector2);

                auto text = vm -> get_string(1);
                auto start_at = vm -> get_vector2(2);
                auto end_at = vm -> get_vector2(3);
                auto font_instance = vm_module::get_userdata_object<API::Font::Instance>(vm, 4);
                auto font_size = vm -> get_int(5);
                auto color = vm -> is_color(6) ? vm -> get_color(6) : godot::Color{1, 1, 1, 1};
                std::pair<godot::HorizontalAlignment, godot::VerticalAlignment> alignment = {godot::HORIZONTAL_ALIGNMENT_LEFT, godot::VERTICAL_ALIGNMENT_TOP};
                if (vm -> is_number(7)) alignment.first = static_cast<godot::HorizontalAlignment>(vm -> get_int(7));
                if (vm -> is_number(8)) alignment.second = static_cast<godot::VerticalAlignment>(vm -> get_int(8));
                auto clip = vm -> is_bool(9) ? vm -> get_bool(9) : false;
                auto wordwrap = vm -> is_bool(10) ? vm -> get_bool(10) : false;
                auto stroke = vm -> is_number(11) ? vm -> get_int(11) : 0;
                auto stroke_color = vm -> is_color(12) ? vm -> get_color(12) : godot::Color{1, 1, 1, 1};
                auto rotation = vm -> is_number(13) ? vm -> get_float(13) : 0.0f;
                auto pivot = vm -> is_vector2(14) ? vm -> get_vector2(14) : godot::Vector2{0.0f, 0.0f};
                base_class::get_singleton() -> draw_text(text, start_at, end_at, font_instance -> font, font_size, color, alignment, clip, wordwrap, stroke, stroke_color, rotation, pivot);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "horizontal_alignment", horizontal_alignment_registry);
            vm -> scope_set_enum(base_scope, "vertical_alignment", vertical_alignment_registry);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Canvas : vm_module {};
}
#endif