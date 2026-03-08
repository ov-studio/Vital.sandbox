/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: canvas.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Canvas APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.extension/Engine/public/canvas.h>


//////////////////////////////////
// Vital: Sandbox: API: Canvas //
//////////////////////////////////

namespace Vital::Sandbox::API {
    struct Canvas : vm_module {
        inline static const std::string base_name = "engine";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "draw_line", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_vector2_array(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto points = vm -> get_vector2_array(1);
                float stroke = vm -> is_number(2) ? vm -> get_float(2) : 0.0f;
                godot::Color color = vm -> is_color(3) ? vm -> get_color(3) : {1, 1, 1, 1};
                Vital::Engine::Canvas::get_singleton() -> draw_line(points, stroke, color);
                vm -> push_bool(true);
                return 1;
            });

            API::bind(vm, {base_name}, "draw_polygon", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_vector2_array(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto points = vm -> get_vector2_array(1);
                godot::Color color = vm -> is_color(2) ? vm -> get_color(2) : {1, 1, 1, 1};
                float stroke = vm -> is_number(3) ? vm -> get_float(3) : 0.0f;
                godot::Color stroke_color = vm -> is_color(4) ? vm -> get_color(4) : {1, 1, 1, 1};
                float rotation = vm -> is_number(5) ? vm -> get_float(5) : 0.0f;
                godot::Vector2 pivot = vm -> is_vector2(6) ? vm -> get_vector2(6) : {0.0f, 0.0f};
                Vital::Engine::Canvas::get_singleton() -> draw_polygon(points, color, stroke, stroke_color, rotation, pivot);
                vm -> push_bool(true);
                return 1;
            });

            API::bind(vm, {base_name}, "draw_rectangle", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_vector2(1)) || (!vm -> is_vector2(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto position = vm -> get_vector2(1);
                auto size = vm -> get_vector2(2);
                godot::Color color = vm -> is_color(3) ? vm -> get_color(3) : {1, 1, 1, 1};
                float stroke = vm -> is_number(4) ? vm -> get_float(4) : 0.0f;
                godot::Color stroke_color = vm -> is_color(5) ? vm -> get_color(5) : {1, 1, 1, 1};
                float rotation = vm -> is_number(6) ? vm -> get_float(6) : 0.0f;
                godot::Vector2 pivot = vm -> is_vector2(7) ? vm -> get_vector2(7) : {0.0f, 0.0f};
                Vital::Engine::Canvas::get_singleton() -> draw_rectangle(position, size, color, stroke, stroke_color, rotation, pivot);
                vm -> push_bool(true);
                return 1;
            });

            API::bind(vm, {base_name}, "draw_circle", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_vector2(1)) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto position = vm -> get_vector2(1);
                float radius = vm -> get_float(2);
                godot::Color color = vm -> is_color(3) ? vm -> get_color(3) : godot::Color{1, 1, 1, 1};
                float stroke = vm -> is_number(4) ? vm -> get_float(4) : 0.0f;
                godot::Color stroke_color = vm -> is_color(5) ? vm -> get_color(5) : godot::Color{1, 1, 1, 1};
                float rotation = vm -> is_number(6) ? vm -> get_float(6) : 0.0f;
                godot::Vector2 pivot = vm -> is_vector2(7) ? vm -> get_vector2(7) : godot::Vector2{0.0f, 0.0f};
                Vital::Engine::Canvas::get_singleton() -> draw_circle(position, radius, color, stroke, stroke_color, rotation, pivot);
                vm -> push_bool(true);
                return 1;
            });

            API::bind(vm, {base_name}, "draw_image", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 3) || (!vm -> is_vector2(1)) || (!vm -> is_vector2(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto position = vm -> get_vector2(1);
                auto size = vm -> get_vector2(2);
                float rotation = vm -> is_number(4) ? vm -> get_float(4) : 0.0f;
                godot::Vector2 pivot = vm -> is_vector2(5) ? vm -> get_vector2(5) : godot::Vector2{0.0f, 0.0f};
                godot::Color color = vm -> is_color(6) ? vm -> get_color(6) : godot::Color{1, 1, 1, 1};
                if (vm -> is_string(3)) {
                    auto path = vm -> get_string(3);
                    Vital::Engine::Canvas::get_singleton() -> draw_image(position, size, path, rotation, pivot, color);
                }
                else if (vm_module::is_userdata<Vital::Engine::Texture>(vm, "texture", 3)) {
                    auto* texture = static_cast<Vital::Engine::Texture*>(vm -> get_userdata(3));
                    Vital::Engine::Canvas::get_singleton() -> draw_image(position, size, texture, rotation, pivot, color);
                }
                else if (vm_module::is_userdata<Vital::Engine::Rendertarget>(vm, "rendertarget", 3)) {
                    auto* rt = static_cast<Vital::Engine::Rendertarget*>(vm -> get_userdata(3));
                    Vital::Engine::Canvas::get_singleton() -> draw_image(position, size, rt, rotation, pivot, color);
                }
                else if (vm_module::is_userdata<Vital::Engine::SVG>(vm, "svg", 3)) {
                    auto* svg = static_cast<Vital::Engine::SVG*>(vm -> get_userdata(3));
                    Vital::Engine::Canvas::get_singleton() -> draw_image(position, size, svg, rotation, pivot, color);
                }
                else throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                vm -> push_bool(true);
                return 1;
            });

            // draw_text(text, start_at, end_at, font, font_size, color?, alignment?, clip?, wordwrap?, stroke?, stroke_color?, rotation?, pivot?) //
            API::bind(vm, {base_name}, "draw_text", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 5) || (!vm -> is_string(1)) || (!vm -> is_vector2(2)) || (!vm -> is_vector2(3)) || (!vm -> is_userdata(4)) || (!vm -> is_number(5))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto text = vm -> get_string(1);
                auto start_at = vm -> get_vector2(2);
                auto end_at = vm -> get_vector2(3);
                auto* font_raw = static_cast<godot::Font*>(vm -> get_userdata(4));
                if (!font_raw) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                godot::Ref<godot::Font> font(font_raw);
                int font_size = vm -> get_int(5);
                godot::Color color = vm -> is_color(6) ? vm -> get_color(6) : godot::Color{1, 1, 1, 1};
                std::pair<godot::HorizontalAlignment, godot::VerticalAlignment> alignment = {godot::HORIZONTAL_ALIGNMENT_LEFT, godot::VERTICAL_ALIGNMENT_CENTER};
                if (vm -> is_table(7)) {
                    vm -> get_table_index(1, 7);
                    alignment.first = vm -> get_horizontal_alignment(-1);
                    vm -> pop();
                    vm -> get_table_index(2, 7);
                    alignment.second = vm -> get_vertical_alignment(-1);
                    vm -> pop();
                }
                bool clip = vm -> is_bool(8)  ? vm -> get_bool(8)  : false;
                bool wordwrap = vm -> is_bool(9)  ? vm -> get_bool(9)  : false;
                int stroke = vm -> is_number(10) ? vm -> get_int(10) : 0;
                godot::Color stroke_color = vm -> is_color(11) ? vm -> get_color(11) : godot::Color{1, 1, 1, 1};
                float rotation = vm -> is_number(12) ? vm -> get_float(12) : 0.0f;
                godot::Vector2 pivot = vm -> is_vector2(13) ? vm -> get_vector2(13) : godot::Vector2{0.0f, 0.0f};
                Vital::Engine::Canvas::get_singleton() -> draw_text(text, start_at, end_at, font, font_size, color, alignment, clip, wordwrap, stroke, stroke_color, rotation, pivot);
                vm -> push_bool(true);
                return 1;
            });
        }
    };
}