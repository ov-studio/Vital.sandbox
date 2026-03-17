namespace Vital::Sandbox {
    template<typename Derived>
    struct Mixin {
        private:
            Derived* self() { return static_cast<Derived*>(this); }
        public:
            // Dispatchers //
            void push_value(bool value) { self() -> push_bool(value); }
            template<typename T, typename = std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_enum_v<T>>>
            void push_value(T value) { self() -> push_number(static_cast<int64_t>(value)); }
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
                    else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                        self() -> create_table();
                        for (int i = 0; i < static_cast<int>(v.size()); ++i) {
                            self() -> push_string(v[i]);
                            self() -> set_table_field(i + 1, -2);
                        }
                    }
                    else push_value(v);
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
            void table_push_value(T value, const std::string& nspace = "") {
                if (!nspace.empty()) self() -> create_namespace(nspace);
                push_value(value);
                self() -> set_table_field(self() -> get_length(-2) + 1, -2);
                if (!nspace.empty()) self() -> pop();
            }
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
            void table_set_value(const std::string& index, T value, const std::string& nspace = "") {
                if (!nspace.empty()) self() -> create_namespace(nspace);
                push_value(value);
                self() -> set_table_field(index, -2);
                if (!nspace.empty()) self() -> pop();
            }
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