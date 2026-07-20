/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: mixin.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Mixins
----------------------------------------------------------------*/


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
            template<typename T, typename = std::enable_if_t<(std::is_integral_v<T> && !std::is_same_v<T, bool>) || std::is_enum_v<T>>>
            void push_value(T value) { self() -> push_number(static_cast<int64_t>(value)); }
            void push_value(float value) { self() -> push_number(value); }
            void push_value(double value) { self() -> push_number(value); }
            void push_value(const std::string& value) { self() -> push_string(value); }
            void push_value(void* value) { self() -> push_userdata(value); }
            void push_value(vm_exec& value) { self() -> push_function(value); }
            void push_value(const godot::Color& value) { self() -> push_color(value); }
            void push_value(const godot::Vector2& value) { self() -> push_vector2(value); }
            void push_value(const godot::Vector2i& value) { self() -> push_vector2(godot::Vector2(value)); }
            void push_value(const godot::PackedVector2Array& value) { self() -> push_vector2_array(value); }
            void push_value(const godot::Vector3& value) { self() -> push_vector3(value); }
            void push_value(const godot::Vector3i& value) { self() -> push_vector3(godot::Vector3(value)); }
            void push_value(const godot::PackedVector3Array& value) { self() -> push_vector3_array(value); }
            void push_value(const godot::Vector4& value) { self() -> push_vector4(value); }
            void push_value(const godot::Vector4i& value) { self() -> push_vector4(godot::Vector4(value)); }
            void push_value(const godot::PackedVector4Array& value) { self() -> push_vector4_array(value); }
            void push_value(const godot::Transform3D& value) { self() -> push_transform3d(value); }
            void push_value(const godot::Projection& value) { self() -> push_projection(value); }
            void push_value(const godot::Variant& value) { push_value(Tool::StackValue::from_variant(value)); }

            void push_value(const Tool::StackValue& value) {
                std::visit([this](auto&& v) {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::nullptr_t>) self() -> push_nil();
                    else if constexpr (std::is_same_v<T, std::shared_ptr<void>>) {
                        if (!v) self() -> push_nil();
                        else {
                            auto base_ptr = std::static_pointer_cast<Vital::Sandbox::vm_instance_base>(v);
                            if (base_ptr) base_ptr -> push_self(self());
                            else self() -> push_nil();
                        }
                    }
                    else if constexpr (std::is_same_v<T, std::shared_ptr<Tool::Stack>>) {
                        self() -> create_table();
                        if (!v) return;
                        for (int i = 0; i < static_cast<int>(v -> array.size()); ++i) {
                            push_value(v -> array[i]);
                            self() -> set_table_field(i + 1, -2);
                        }
                        for (const auto& [key, sv] : v -> object) {
                            push_value(sv);
                            self() -> set_table_field(key, -2);
                        }
                    }
                    else push_value(v);
                }, value.value);
            }


            // Table //
            void table_get_value(int value, int idx = 1) { self() -> get_table_field(value, idx); }
            void table_get_value(const std::string& value, int idx = 1) { self() -> get_table_field(value, idx); }

            void table_push_nil(const std::string& nspace = "") {
                if (!nspace.empty()) self() -> create_namespace(nspace);
                self() -> push_nil();
                self() -> set_table_field(self() -> get_length(-2) + 1, -2);
                if (!nspace.empty()) self() -> pop(1);
            }

            template<typename T>
            void table_push_value(T value, const std::string& nspace = "") {
                if (!nspace.empty()) self() -> create_namespace(nspace);
                push_value(value);
                self() -> set_table_field(self() -> get_length(-2) + 1, -2);
                if (!nspace.empty()) self() -> pop(1);
            }

            void table_push_table(const std::string& nspace = "") {
                if (!nspace.empty()) {
                    self() -> create_namespace(nspace);
                    self() -> get_table(-2);
                }
                self() -> set_table_field(self() -> get_length(-2) + 1, -2);
                if (!nspace.empty()) self() -> pop(2);
            }

            void table_set_nil(const std::string& idx, const std::string& nspace = "") {
                if (!nspace.empty()) self() -> create_namespace(nspace);
                self() -> push_nil();
                self() -> set_table_field(idx, -2);
                if (!nspace.empty()) self() -> pop(1);
            }

            template<typename T>
            void table_set_value(const std::string& idx, T value, const std::string& nspace = "") {
                if (!nspace.empty()) self() -> create_namespace(nspace);
                push_value(value);
                self() -> set_table_field(idx, -2);
                if (!nspace.empty()) self() -> pop(1);
            }

            void table_set_table(const std::string& idx, const std::string& nspace = "") {
                if (!nspace.empty()) {
                    self() -> create_namespace(nspace);
                    self() -> get_table(-2);
                }
                self() -> set_table_field(idx, -2);
                if (!nspace.empty()) self() -> pop(2);
            }


            // Scope //
            void scope_with(const std::vector<std::string>& scope, std::function<void(Derived*)> exec) {
                self() -> get_global(scope[0]);
                for (std::size_t i = 1; i < scope.size(); ++i) {
                    if (!self() -> is_table(-1)) {
                        self() -> pop(static_cast<int>(i));
                        return;
                    }
                    self() -> get_table_field(scope[i], -1);
                }
                if (self() -> is_table(-1)) exec(self());
                self() -> pop(static_cast<int>(scope.size()));
            }

            void scope_set(const std::vector<std::string>& scope) {
                self() -> create_namespace(scope[0]);
                for (std::size_t i = 1; i < scope.size(); ++i) {
                    self() -> get_table_field(scope[i], -1);
                    if (!self() -> is_table(-1)) {
                        self() -> pop(1);
                        self() -> create_table();
                        self() -> push(-1);
                        self() -> set_table_field(scope[i], -3);
                    }
                }
            }

            template<typename T>
            void scope_set_enum(const std::vector<std::string>& scope, const std::string& field, const std::vector<std::pair<std::string, T>>& registry) {
                scope_with(scope, [&](auto vm) {
                    vm -> create_table();
                    for (const auto& [name, value] : registry) vm -> table_set_value(name, static_cast<int>(value));
                    vm -> set_table_field(field, -2);
                });
            }

            void scope_move_global(const std::vector<std::string>& scope, const std::string& name, bool nil_source = false) {
                if (scope.size() < 2) return;
                self() -> create_namespace(scope[0]);
                for (std::size_t i = 1; i < scope.size() - 1; ++i) {
                    self() -> get_table_field(scope[i], -1);
                    if (!self() -> is_table(-1)) {
                        self() -> pop(1);
                        self() -> create_table();
                        self() -> push(-1);
                        self() -> set_table_field(scope[i], -3);
                    }
                }
                self() -> get_global(name);
                self() -> set_table_field(scope.back(), -2);
                self() -> pop(static_cast<int>(scope.size() - 1));
                if (nil_source) {
                    self() -> push_nil();
                    self() -> push_global(name);
                }
            }

            void scope_nil_field(const std::vector<std::string>& scope, const std::string& name) {
                self() -> get_global(scope[0]);
                for (std::size_t i = 1; i < scope.size(); ++i) {
                    if (!self() -> is_table(-1)) {
                        self() -> pop(static_cast<int>(i));
                        return;
                    }
                    self() -> get_table_field(scope[i], -1);
                }
                if (self() -> is_table(-1)) {
                    self() -> push_nil();
                    self() -> set_table_field(name, -2);
                }
                self() -> pop(static_cast<int>(scope.size()));
            }


            // Collectors //
            Tool::StackValue collect_value(int idx, std::unordered_set<const void*>& visited, int depth = 0) {
                if (self() -> is_nil(idx)) return Tool::StackValue(nullptr);
                if (self() -> is_bool(idx)) return Tool::StackValue(self() -> get_bool(idx));
                if (self() -> is_number(idx)) return Tool::StackValue(self() -> get_double(idx));
                if (self() -> is_string(idx)) return Tool::StackValue(self() -> get_string(idx));
                if (self() -> is_table(idx)) return Tool::StackValue(collect_table(idx, visited, depth));
                return Tool::StackValue(nullptr);
            }

            Tool::Stack collect_args(int idx) {
                Tool::Stack payload;
                std::unordered_set<const void*> visited;
                for (int i = idx; i <= self() -> get_count(); ++i) payload.array.emplace_back(collect_value(i, visited));
                return payload;
            }

            std::shared_ptr<Tool::Stack> collect_table(int idx, std::unordered_set<const void*>& visited, int depth = 0) {
                auto stack = std::make_shared<Tool::Stack>();
                if (depth > 32) return stack;

                auto ptr = self() -> get_pointer(idx);
                if (!ptr || visited.count(ptr)) return stack;
                visited.insert(ptr);

                self() -> push_nil();
                while (self() -> next(idx)) {
                    bool key_is_number = self() -> is_number(-2);
                    bool key_is_string = self() -> is_string(-2);
                    bool val_is_collectible = self() -> is_nil(-1) || self() -> is_bool(-1) || self() -> is_number(-1) || self() -> is_string(-1) || self() -> is_table(-1);
                    if (key_is_number) {
                        auto key_idx = self() -> get_int(-2);
                        if (key_idx >= 1) {
                            auto val = collect_value(self() -> get_count(), visited, depth + 1);
                            if (static_cast<int>(stack -> array.size()) < key_idx) stack -> array.resize(key_idx, Tool::StackValue(nullptr));
                            stack -> array[key_idx - 1] = val;
                        }
                    }
                    else if (key_is_string && val_is_collectible) stack -> object[self() -> get_string(-2)] = collect_value(self() -> get_count(), visited, depth + 1);
                    self() -> pop(1);
                }
                visited.erase(ptr);
                return stack;
            }
    };
}
