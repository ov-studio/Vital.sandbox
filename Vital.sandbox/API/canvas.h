/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: canvas.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Canvas APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/canvas.h>


/////////////////////////
// Vital: API: Canvas //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Canvas : vm_module {
        inline static const std::string base_name = "engine";
        using base_class = Vital::Engine::Canvas;

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "get_resolution", [](auto vm) -> int {
                auto result = base_class::get_singleton() -> get_resolution();
                vm -> push_value(result);
                return 1;
            });

            API::bind(vm, {base_name}, "world_to_screen", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_vector3(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto position = vm -> get_vector3(1);
                auto padding = vm -> is_number(2) ? vm -> get_float(2) : 0.0f;
                auto result = base_class::get_singleton() -> world_to_screen(position, padding);
                vm -> push_value(godot::Vector2(result.x, result.y));
                vm -> push_value(result.z);
                return 2;
            });

            API::bind(vm, {base_name}, "screen_to_world", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_vector2(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto position = vm -> get_vector2(1);
                auto depth = vm -> is_number(2) ? vm -> get_float(2) : 1.0f;
                vm -> push_value(base_class::get_singleton() -> screen_to_world(position, depth));
                return 1;
            });

            API::bind(vm, {base_name}, "draw_line", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_vector2_array(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto points = vm -> get_vector2_array(1);
                auto stroke = vm -> is_number(2) ? vm -> get_float(2) : 0.0f;
                auto color = vm -> is_color(3) ? vm -> get_color(3) : godot::Color{1, 1, 1, 1};
                base_class::get_singleton() -> draw_line(points, stroke, color);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "draw_polygon", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_vector2_array(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
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

            API::bind(vm, {base_name}, "draw_rectangle", [](auto vm) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_vector2(1)) || (!vm -> is_vector2(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
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

            API::bind(vm, {base_name}, "draw_circle", [](auto vm) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_vector2(1)) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
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

            API::bind(vm, {base_name}, "draw_image", [](auto vm) -> int {
                if ((vm -> get_count() < 3) || (!vm -> is_vector2(1)) || (!vm -> is_vector2(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto position = vm -> get_vector2(1);
                auto size = vm -> get_vector2(2);
                auto rotation = vm -> is_number(4) ? vm -> get_float(4) : 0.0f;
                auto pivot = vm -> is_vector2(5) ? vm -> get_vector2(5) : godot::Vector2{0.0f, 0.0f};
                auto color = vm -> is_color(6) ? vm -> get_color(6) : godot::Color{1, 1, 1, 1};
                if (vm -> is_string(3)) {
                    auto path = vm -> get_string(3);
                    base_class::get_singleton() -> draw_image(position, size, path, rotation, pivot, color);
                }
                else if (vm_module::is_userdata<Vital::Engine::Texture>(vm, "texture", 3)) {
                    auto texture = static_cast<Vital::Engine::Texture*>(vm -> get_userdata(3));
                    base_class::get_singleton() -> draw_image(position, size, texture, rotation, pivot, color);
                }
                else if (vm_module::is_userdata<Vital::Engine::Rendertarget>(vm, "rendertarget", 3)) {
                    auto rt = static_cast<Vital::Engine::Rendertarget*>(vm -> get_userdata(3));
                    base_class::get_singleton() -> draw_image(position, size, rt, rotation, pivot, color);
                }
                else if (vm_module::is_userdata<Vital::Engine::Texture>(vm, "svg", 3)) {
                    auto svg = static_cast<Vital::Engine::Texture*>(vm -> get_userdata(3));
                    base_class::get_singleton() -> draw_image(position, size, svg, rotation, pivot, color);
                }
                else throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "draw_text", [](auto vm) -> int {
                if ((vm -> get_count() < 5) || (!vm -> is_string(1)) || (!vm -> is_vector2(2)) || (!vm -> is_vector2(3)) || (!vm_module::is_userdata<Vital::Engine::Font>(vm, "font", 4)) || (!vm -> is_number(5))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto text = vm -> get_string(1);
                auto start_at = vm -> get_vector2(2);
                auto end_at = vm -> get_vector2(3);
                auto font = static_cast<Vital::Engine::Font*>(vm -> get_userdata(4));
                auto font_size = vm -> get_int(5);
                auto color = vm -> is_color(6) ? vm -> get_color(6) : godot::Color{1, 1, 1, 1};
                std::pair<godot::HorizontalAlignment, godot::VerticalAlignment> alignment = {godot::HORIZONTAL_ALIGNMENT_LEFT, godot::VERTICAL_ALIGNMENT_TOP};
                if (vm -> is_table(7)) {
                    vm -> get_table_field(1, 7);
                    alignment.first = vm -> get_horizontal_alignment(-1);
                    vm -> pop(1);
                    vm -> get_table_field(2, 7);
                    alignment.second = vm -> get_vertical_alignment(-1);
                    vm -> pop(1);
                }
                auto clip = vm -> is_bool(8) ? vm -> get_bool(8) : false;
                auto wordwrap = vm -> is_bool(9) ? vm -> get_bool(9) : false;
                auto stroke = vm -> is_number(10) ? vm -> get_int(10) : 0;
                auto stroke_color = vm -> is_color(11) ? vm -> get_color(11) : godot::Color{1, 1, 1, 1};
                auto rotation = vm -> is_number(12) ? vm -> get_float(12) : 0.0f;
                auto pivot = vm -> is_vector2(13) ? vm -> get_vector2(13) : godot::Vector2{0.0f, 0.0f};
                base_class::get_singleton() -> draw_text(text, start_at, end_at, font, font_size, color, alignment, clip, wordwrap, stroke, stroke_color, rotation, pivot);
                vm -> push_value(true);
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Canvas : vm_module {};
}
#endif