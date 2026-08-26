/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: shader.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 22/08/2026
     Desc: Shader APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/shader.h>
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/Engine/public/rendertarget.h>
#include <Vital.sandbox/API/core/image.h>
#include <Vital.sandbox/API/utility/file.h>


//////////////////////////
// Vital: API: Shader  //
//////////////////////////

// TODO: Improve

namespace Vital::Sandbox::API {
    struct Shader : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "shader"};
        using base_class = Vital::Engine::Shader;

        inline static const std::vector<std::pair<std::string, int>> shader_type_registry = {
            { "CANVAS_ITEM", static_cast<int>(base_class::Type::CanvasItem) },
            { "SPATIAL",     static_cast<int>(base_class::Type::Spatial)    }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Shader;
            base_class* shader = nullptr;

            bool is_alive() const {
                return shader ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> shader) {
                    instance -> shader -> destroy();
                    instance -> shader = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<Shader>(vm);

            // core.shader.create(code, type = "CANVAS_ITEM") -> shader
            //
            // Creates a shader from a GDShader source string.
            // A `shader_type` declaration is injected automatically if absent.
            //
            // For canvas / postprocess draws (engine.draw_shader):
            //   type = "CANVAS_ITEM"
            //   Minimal body: "void fragment() { COLOR = vec4(1, 0, 0, 1); }"
            //
            //   Useful built-ins:  COLOR, UV, TEXTURE, SCREEN_TEXTURE (enable via hint),
            //                      TIME, FRAGCOORD
            //   Read modulate:     "uniform vec4 modulate;" — engine fills this from
            //                      the draw_shader `color` argument.
            //   Feed an RT:        use set_param_rt("u_screen", rt) then in shader:
            //                      "uniform sampler2D u_screen : hint_screen_texture;"
            //
            // For 3-D mesh surface overrides (shader:apply_to_model):
            //   type = "SPATIAL"
            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(code, type = \"CANVAS_ITEM\")")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_string);

                auto code = vm -> get_string(1);
                auto type = base_class::Type::CanvasItem;
                if (vm -> is_string(2)) {
                    auto type_str = vm -> get_string(2);
                    for (auto& [k, v] : shader_type_registry) {
                        if (k == type_str) { type = static_cast<base_class::Type>(v); break; }
                    }
                }
                auto instance = Instance::init(vm);
                instance -> shader = base_class::create(code, type);
                instance -> store(true);
                return 1;
            });

            // core.shader.create_from_file(path, type = "CANVAS_ITEM") -> shader
            API::bind(vm, base_scope, "create_from_file", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path, type = \"CANVAS_ITEM\")")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_string);

                auto path = vm -> get_string(1);
                API::File::assert_file(vm, path);
                godot::Ref<godot::FileAccess> fa = godot::FileAccess::open(
                    godot::String(path.c_str()), godot::FileAccess::READ);
                if (!fa.is_valid())
                    luaL_error(vm -> get_state(), "shader: cannot open file: %s", path.c_str());
                std::string code = std::string(fa -> get_as_text().utf8().get_data());

                auto type = base_class::Type::CanvasItem;
                if (vm -> is_string(2)) {
                    auto type_str = vm -> get_string(2);
                    for (auto& [k, v] : shader_type_registry) {
                        if (k == type_str) { type = static_cast<base_class::Type>(v); break; }
                    }
                }
                auto instance = Instance::init(vm);
                instance -> shader = base_class::create(code, type);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {

            // :is_valid() -> bool
            vm_module::bind_method<Instance>(vm, "is_valid", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> shader -> is_valid());
                return 1;
            });

            // :get_type() -> "CANVAS_ITEM" | "SPATIAL"
            vm_module::bind_method<Instance>(vm, "get_type", [](auto vm, auto self, auto& id) -> int {
                auto type = static_cast<int>(self -> shader -> get_type());
                for (auto& [k, v] : shader_type_registry) {
                    if (v == type) { vm -> push_value(k); return 1; }
                }
                vm -> push_value(std::string("CANVAS_ITEM"));
                return 1;
            });

            // :get_code() -> string
            vm_module::bind_method<Instance>(vm, "get_code", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> shader -> get_code());
                return 1;
            });

            // :set_code(code) -> bool
            // Hot-reloads the shader source.  Existing params are preserved.
            vm_module::bind_method<Instance>(vm, "set_code", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(code)", true)
                    .require(2, &Machine::is_string);
                vm -> push_value(self -> shader -> set_code(vm -> get_string(2)));
                return 1;
            });

            // :set_param(name, value) -> bool
            // Set a scalar/vector uniform.  value: number, bool, Vector2, Vector3.
            vm_module::bind_method<Instance>(vm, "set_param", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, value)", true)
                    .require(2, &Machine::is_string)
                    .require(3, [](Machine* m, int i) {
                        return m -> is_number(i) || m -> is_bool(i)
                            || m -> is_vector2(i) || m -> is_vector3(i);
                    });

                auto name = vm -> get_string(2);
                godot::Variant value;
                if (vm -> is_number(3))  value = vm -> get_float(3);
                else if (vm -> is_bool(3))    value = vm -> get_bool(3);
                else if (vm -> is_vector2(3)) value = vm -> get_vector2(3);
                else if (vm -> is_vector3(3)) value = vm -> get_vector3(3);
                vm -> push_value(self -> shader -> set_param(name, value));
                return 1;
            });

            // :set_param_color(name, r, g, b, a = 1.0) -> bool
            vm_module::bind_method<Instance>(vm, "set_param_color", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, r, g, b, a = 1.0)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_number)
                    .require(4, &Machine::is_number)
                    .require(5, &Machine::is_number)
                    .optional(6, &Machine::is_number);

                auto name = vm -> get_string(2);
                float r = vm -> get_float(3);
                float g = vm -> get_float(4);
                float b = vm -> get_float(5);
                float a = vm -> is_number(6) ? vm -> get_float(6) : 1.0f;
                vm -> push_value(self -> shader -> set_param(name, godot::Variant(godot::Color(r, g, b, a))));
                return 1;
            });

            // :set_param_texture(name, texture) -> bool
            // Feed a Texture instance as a sampler2D uniform.
            vm_module::bind_method<Instance>(vm, "set_param_texture", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, texture)", true)
                    .require(2, &Machine::is_string)
                    .require(3, [](Machine* m, int i) { return vm_module::is_userdata<API::Image::Instance>(m, i); });

                auto name = vm -> get_string(2);
                auto instance = vm_module::get_userdata_object<API::Image::Instance>(vm, 3);
                auto ref = instance -> texture -> get_canvas_texture();
                vm -> push_value(self -> shader -> set_param_texture(name, ref));
                return 1;
            });

            // :set_param_rt(name, rendertarget) -> bool
            // Feed a Rendertarget's SubViewport texture as a sampler2D uniform.
            // Declare in shader: "uniform sampler2D u_rt : hint_screen_texture;"
            // (or without the hint for a plain sampler).
            vm_module::bind_method<Instance>(vm, "set_param_rt", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, rendertarget)", true)
                    .require(2, &Machine::is_string)
                    .require(3, [](Machine* m, int i) { return vm_module::is_userdata<API::Rendertarget::Instance>(m, i); });

                auto name = vm -> get_string(2);
                auto instance = vm_module::get_userdata_object<API::Rendertarget::Instance>(vm, 3);
                auto vp_tex = instance -> rendertarget -> get_texture();
                vm -> push_value(self -> shader -> set_param_viewport_texture(name, vp_tex));
                return 1;
            });

            // :apply_to_model(model) -> number
            // Applies this shader (must be SPATIAL) as a surface override on every
            // mesh surface of a spawned Model.  Returns the count of surfaces patched.
            vm_module::bind_method<Instance>(vm, "apply_to_model", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(model)", true)
                    .require(2, [](Machine* m, int i) { return m -> is_userdata(i); });

                using ModelInstance = Vital::Sandbox::API::Model::Instance;
                auto instance = vm_module::get_userdata_object<ModelInstance>(vm, 2);
                if (!instance || !instance -> model) vm -> push_value(false);
                else vm -> push_value(self -> shader -> apply_to_node(instance -> model));
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "shader_type", shader_type_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Shader : vm_module {};
}
#endif
