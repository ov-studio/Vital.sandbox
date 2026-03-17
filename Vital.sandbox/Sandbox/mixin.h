/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: mixin.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Mixin Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/index.h>


////////////////////////////
// Vital: Sandbox: Mixin //
////////////////////////////

namespace Vital::Sandbox {
    template<typename Derived>
    struct Mixin {
        private:
            Derived* self() { return static_cast<Derived*>(this); }
        public:
            // Dispatchers //
            void push_value(bool value) { self() -> push_bool(value); }
            void push_value(int value) { self() -> push_number(value); }
            void push_value(float value) { self() -> push_number(value); }
            void push_value(double value) { self() -> push_number(value); }
            void push_value(const std::string& value) { self() -> push_string(value); }
            void push_value(vm_exec& value) { self() -> push_function(value); }
            void push_value(const godot::Color& value) { self() -> push_color(value); }
            void push_value(const godot::Vector2& value) { self() -> push_vector2(value); }
            void push_value(const godot::PackedVector2Array& value) { self() -> push_vector2_array(value); }
            void push_value(const godot::Vector3& value) { self() -> push_vector3(value); }
            void push_value(const godot::PackedVector3Array& value) { self() -> push_vector3_array(value); }
            void push_value(godot::HorizontalAlignment value) { self() -> push_horizontal_alignment(value); }
            void push_value(godot::VerticalAlignment value) { self() -> push_vertical_alignment(value); }
            void push_value(const Vital::Tool::StackValue& value) {
                std::visit([this](auto&& v) {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::nullptr_t>)
                        self() -> push_nil();
                    else if constexpr (std::is_same_v<T, bool>)
                        self() -> push_bool(v);
                    else if constexpr (std::is_same_v<T, int32_t>)
                        self() -> push_number(static_cast<int>(v));
                    else if constexpr (std::is_same_v<T, int64_t>)
                        self() -> push_number(static_cast<double>(v));
                    else if constexpr (std::is_same_v<T, float>)
                        self() -> push_number(v);
                    else if constexpr (std::is_same_v<T, double>)
                        self() -> push_number(v);
                    else if constexpr (std::is_same_v<T, std::string>)
                        self() -> push_string(v);
                    else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                        self() -> create_table();
                        for (int i = 0; i < static_cast<int>(v.size()); ++i) {
                            self() -> push_string(v[i]);
                            self() -> set_table_field(i + 1, -2);
                        }
                    }
                }, value.value);
            }
        

            // Pushers //
            void table_push_nil(const std::string& nspace = "") {
                if (!nspace.empty()) self() -> create_namespace(nspace);
                self() -> push_nil();
                self() -> set_table_field(self() -> get_length(-2) + 1, -2);
                if (!nspace.empty()) self() -> pop();
            }
            template<typename T>
            void table_push(T value, const std::string& nspace = "") {
                if (!nspace.empty()) self() -> create_namespace(nspace);
                push_value(value);
                self() -> set_table_field(self() -> get_length(-2) + 1, -2);
                if (!nspace.empty()) self() -> pop();
            }
            void table_push_bool(bool value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_string(const std::string& value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_number(int value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_number(float value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_number(double value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_function(vm_exec& value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_color(const godot::Color& value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_vector2(const godot::Vector2& value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_vector2_array(const godot::PackedVector2Array& value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_vector3(const godot::Vector3& value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_vector3_array(const godot::PackedVector3Array& value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_horizontal_alignment(godot::HorizontalAlignment value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_vertical_alignment(godot::VerticalAlignment value, const std::string& nspace = "") { table_push(value, nspace); }
            void table_push_table(const std::string& nspace = "") {
                if (!nspace.empty()) {
                    self() -> create_namespace(nspace);
                    self() -> get_table(-2);
                }
                self() -> set_table_field(self() -> get_length(-2) + 1, -2);
                if (!nspace.empty()) self() -> pop(2);
            }


            // Setters //
            void table_set_nil(const std::string& index, const std::string& nspace = "") {
                if (!nspace.empty()) self() -> create_namespace(nspace);
                self() -> push_nil();
                self() -> set_table_field(index, -2);
                if (!nspace.empty()) self() -> pop();
            }
            template<typename T>
            void table_set(const std::string& index, T value, const std::string& nspace = "") {
                if (!nspace.empty()) self() -> create_namespace(nspace);
                push_value(value);
                self() -> set_table_field(index, -2);
                if (!nspace.empty()) self() -> pop();
            }
            void table_set_bool(const std::string& index, bool value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_string(const std::string& index, const std::string& value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_number(const std::string& index, int value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_number(const std::string& index, float value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_number(const std::string& index, double value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_function(const std::string& index, vm_exec& value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_color(const std::string& index, const godot::Color& value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_vector2(const std::string& index, const godot::Vector2& value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_vector2_array(const std::string& index, const godot::PackedVector2Array& value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_vector3(const std::string& index, const godot::Vector3& value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_vector3_array(const std::string& index, const godot::PackedVector3Array& value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_horizontal_alignment(const std::string& index, godot::HorizontalAlignment value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_vertical_alignment(const std::string& index, godot::VerticalAlignment value, const std::string& nspace = "") { table_set(index, value, nspace); }
            void table_set_table(const std::string& index, const std::string& nspace = "") {
                if (!nspace.empty()) {
                    self() -> create_namespace(nspace);
                    self() -> get_table(-2);
                }
                self() -> set_table_field(index, -2);
                if (!nspace.empty()) self() -> pop(2);
            }
    };
}