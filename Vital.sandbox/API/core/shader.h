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
#include <Vital.sandbox/API/core/model.h>
#include <Vital.sandbox/API/core/image.h>
#include <Vital.sandbox/API/utility/file.h>


/////////////////////////
// Vital: API: Shader //
/////////////////////////

// TODO: Improve

namespace Vital::Sandbox::API {
    struct Shader : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "shader"};
        using base_class = Vital::Engine::Shader;

        struct Instance : vm_instance<Instance> {
            using Owner = Shader;
            base_class* shader = nullptr;

            auto get_node() {
                return shader;
            }

            bool is_alive() const {
                return shader ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> shader) {
                    // Remove all persistent registrations owned by this shader
                    // instance before tearing it down so we don't leave dangling
                    // material overrides on live models.
                    Shader::purge_registrations(instance.get());
                    instance -> shader -> destroy();
                    instance -> shader = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;


        // ------------------------------------------------------------------
        // Shader registry — persistent material assignments
        //
        // Each entry records a (model_pattern, component_pattern,
        // material_pattern) triple together with the owning shader instance
        // and the Lua environment it was created from.  Whenever a new model
        // spawns (entity:spawned) every live entry whose model_pattern matches
        // the new model's name is re-applied automatically.
        // ------------------------------------------------------------------
        struct Registration {
            std::string env;              // Lua resource env that created this
            std::string model_pattern;    // e.g. "*", "player_body", "vehicle_*"
            std::string component_pattern;// e.g. "*", "Body"
            std::string material_pattern; // e.g. "*", "body_paint", "body_*"
            Instance*   owner = nullptr;  // raw back-pointer; valid while shader alive
        };

        inline static std::mutex             registrations_mutex;
        inline static std::vector<Registration> registrations;

        // Apply a single registration to one model (silently skips mismatches).
        static void apply_registration(const Registration& reg, Vital::Engine::Model* model) {
            if (!reg.owner || !reg.owner -> shader) return;
            if (!Tool::match_wildcard(reg.model_pattern, model -> get_model_name())) return;
            try {
                model -> apply_material_shader(
                    reg.component_pattern,
                    reg.material_pattern,
                    reg.owner -> shader -> get_surface_factory()
                );
            } catch (...) {}
        }

        // Remove all registrations owned by a particular Instance* and replay
        // surviving registrations on every model that was affected, so other
        // shaders that overlap the same surfaces remain intact.
        static void purge_registrations(Instance* owner) {
            std::lock_guard<std::mutex> lock(registrations_mutex);

            // Collect the patterns being removed so we know which models need recompute.
            std::vector<Registration> removed;
            for (const auto& r : registrations)
                if (r.owner == owner) removed.push_back(r);

            registrations.erase(
                std::remove_if(registrations.begin(), registrations.end(),
                    [owner](const Registration& r) { return r.owner == owner; }),
                registrations.end()
            );

            // For each affected model: clear removed surfaces, then replay survivors.
            for (auto& [mid, inst] : Model::registry.buffer) {
                if (!inst || !inst -> is_alive()) continue;
                auto* model = inst -> get_node();
                if (!model) continue;

                bool affected = false;
                for (const auto& reg : removed) {
                    if (!Tool::match_wildcard(reg.model_pattern, model -> get_model_name())) continue;
                    affected = true;
                    try { model -> apply_material_shader(reg.component_pattern, reg.material_pattern, godot::Ref<godot::ShaderMaterial>()); }
                    catch (...) {}
                }
                if (!affected) continue;

                for (const auto& reg : registrations) {
                    if (!reg.owner || !reg.owner -> shader) continue;
                    if (!Tool::match_wildcard(reg.model_pattern, model -> get_model_name())) continue;
                    try { model -> apply_material_shader(reg.component_pattern, reg.material_pattern, reg.owner -> shader -> get_surface_factory()); }
                    catch (...) {}
                }
            }
        }

        // Remove registrations by (owner, model, component, material) key.
        static void remove_registration(Instance* owner,
                                        const std::string& model_pattern,
                                        const std::string& component_pattern,
                                        const std::string& material_pattern) {
            std::lock_guard<std::mutex> lock(registrations_mutex);
            registrations.erase(
                std::remove_if(registrations.begin(), registrations.end(),
                    [&](const Registration& r) {
                        return r.owner            == owner
                            && r.model_pattern     == model_pattern
                            && r.component_pattern == component_pattern
                            && r.material_pattern  == material_pattern;
                    }),
                registrations.end()
            );
        }

        static void init(Machine* vm) {
            // Hook entity:spawned so every new model gets shader assignments
            // that match its name applied automatically.
            static Tool::Event::Handle spawned_binding;
            spawned_binding.bind("entity:spawned", [](Tool::Stack args) {
                if (args.array.size() < 1) return;
                if (!args.array[0].is_raw_ptr<Vital::Engine::Model>()) return;
                auto entity = args.array[0].as_raw_ptr<Vital::Engine::Model>();

                const godot::ObjectID oid(entity -> get_instance_id());
                Vital::Engine::Core::get_singleton() -> enqueue([oid]() {
                    godot::Object* obj = godot::ObjectDB::get_instance(oid);
                    if (!obj) return;
                    auto model = godot::Object::cast_to<Vital::Engine::Model>(obj);
                    if (!model) return;

                    std::lock_guard<std::mutex> lock(registrations_mutex);
                    for (const auto& reg : registrations) {
                        apply_registration(reg, model);
                    }
                });
            });
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Shader>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path, mode)")
                    .require(1, &Machine::is_string)
                    .require_enum(2, base_class::mode_registry);

                auto path = vm -> get_string(1);
                auto base = API::File::assert_file(vm, path);
                auto mode = static_cast<base_class::Mode>(vm -> get_int(2));
                auto instance = Instance::init(vm);
                instance -> shader = base_class::create(base, path, mode);
                instance -> store(true);
                return 1;
            });

            API::bind(vm, base_scope, "create_from_raw", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(raw, mode)")
                    .require(1, &Machine::is_string)
                    .require_enum(2, base_class::mode_registry);

                auto raw = vm -> get_string(1);
                auto mode = static_cast<base_class::Mode>(vm -> get_int(2));
                auto instance = Instance::init(vm);
                instance -> shader = base_class::create_from_raw(raw, mode);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "get_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> shader -> get_mode());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_code", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> shader -> get_code());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_param", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, value)", true)
                    .require(2, &Machine::is_string)
                    .require(3, [](Machine* vm, int idx) {
                        return vm -> is_bool(idx)
                            || vm -> is_number(idx)
                            || vm -> is_vector2(idx) 
                            || vm -> is_vector3(idx);
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

            vm_module::bind_method<Instance>(vm, "set_param_color", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, color)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_color);

                auto name = vm -> get_string(2);
                auto color = vm -> get_color(3);
                vm -> push_value(self -> shader -> set_param(name, godot::Variant(color)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_param_texture", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, texture)", true)
                    .require(2, &Machine::is_string)
                    .require(3, [](Machine* vm, int idx) { return vm_module::is_userdata<API::Image::Instance>(vm, idx); });

                auto name = vm -> get_string(2);
                auto image = vm_module::get_userdata_object<API::Image::Instance>(vm, 3);
                vm -> push_value(self -> shader -> set_param_texture(name, image -> get_node() -> get_canvas_texture()));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_param_rt", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, rendertarget)", true)
                    .require(2, &Machine::is_string)
                    .require(3, [](Machine* vm, int idx) { return vm_module::is_userdata<API::Rendertarget::Instance>(vm, idx); });

                auto name = vm -> get_string(2);
                auto rendertarget = vm_module::get_userdata_object<API::Rendertarget::Instance>(vm, 3);
                vm -> push_value(self -> shader -> set_param_viewport_texture(name, rendertarget -> get_node() -> get_texture()));
                return 1;
            });

            // apply_to_material(model, component, material)
            //
            // Registers a persistent shader assignment and immediately applies
            // it to matching models.
            //
            // model     — a model instance (applies only to that model, no persistent
            //             registration for future spawns), or the string "*" to match
            //             every model (registers persistently for future spawns too).
            //             Any other value is an error.
            // component — mesh component name or wildcard
            // material  — surface/material name or wildcard
            //
            // Returns the number of surfaces updated on already-live models.
            vm_module::bind_method<Instance>(vm, "apply_to_material", [](auto vm, auto self, auto& id) -> int {
                bool is_model_instance = vm_module::is_userdata<API::Model::Instance>(vm, 2);
                bool is_wildcard       = !is_model_instance && vm -> is_string(2) && vm -> get_string(2) == "*";
                if (!is_model_instance && !is_wildcard)
                    return vm -> throw_error(id, "(model, component, material)", "bad argument #1 'model' (model instance or \"*\" expected)");

                vm_args(vm, id, "(model, component, material)", true)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_string);

                auto component = vm -> get_string(3);
                auto material  = vm -> get_string(4);
                auto env       = vm -> get_environment_id();

                int count = 0;

                if (is_model_instance) {
                    // Direct model instance — apply immediately, no persistent registration
                    auto model_inst = vm_module::get_userdata_object<API::Model::Instance>(vm, 2);
                    if (model_inst && model_inst -> is_alive()) {
                        auto* model = model_inst -> get_node();
                        if (model) {
                            try { count += model -> apply_material_shader(component, material, self -> shader -> get_surface_factory()); }
                            catch (...) {}
                        }
                    }
                } else {
                    // Wildcard "*" — register persistently for future spawns
                    const std::string model_pattern = "*";
                    {
                        std::lock_guard<std::mutex> lock(registrations_mutex);
                        bool exists = false;
                        for (const auto& r : registrations) {
                            if (r.owner == self.get()
                                && r.model_pattern     == model_pattern
                                && r.component_pattern == component
                                && r.material_pattern  == material) { exists = true; break; }
                        }
                        if (!exists) registrations.push_back({ env, model_pattern, component, material, self.get() });
                    }

                    Registration reg { env, model_pattern, component, material, self.get() };
                    for (auto& [mid, instance] : Model::registry.buffer) {
                        if (!instance || !instance -> is_alive()) continue;
                        auto* model = instance -> get_node();
                        if (!model) continue;
                        try { count += model -> apply_material_shader(component, material, self -> shader -> get_surface_factory()); }
                        catch (...) {}
                    }
                }

                vm -> push_value(count);
                return 1;
            });

            // remove_from_material(model, component, material)
            //
            // Removes a shader override applied by apply_to_material.
            //
            // model     — a model instance (clears the override on that model only),
            //             or the string "*" (removes the persistent registration and
            //             clears overrides on all currently live models).
            //             Any other value is an error.
            // component — mesh component name or wildcard
            // material  — surface/material name or wildcard
            //
            // Returns the number of surfaces whose override was changed.
            vm_module::bind_method<Instance>(vm, "remove_from_material", [](auto vm, auto self, auto& id) -> int {
                bool is_model_instance = vm_module::is_userdata<API::Model::Instance>(vm, 2);
                bool is_wildcard       = !is_model_instance && vm -> is_string(2) && vm -> get_string(2) == "*";
                if (!is_model_instance && !is_wildcard)
                    return vm -> throw_error(id, "(model, component, material)", "bad argument #1 'model' (model instance or \"*\" expected)");

                vm_args(vm, id, "(model, component, material)", true)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_string);

                auto component = vm -> get_string(3);
                auto material  = vm -> get_string(4);

                int count = 0;

                if (is_model_instance) {
                    // Direct model instance — clear override on that model only
                    auto model_inst = vm_module::get_userdata_object<API::Model::Instance>(vm, 2);
                    if (model_inst && model_inst -> is_alive()) {
                        auto* model = model_inst -> get_node();
                        if (model) {
                            try { model -> apply_material_shader(component, material, godot::Ref<godot::ShaderMaterial>()); count++; }
                            catch (...) {}

                            // Re-apply surviving registrations on this model
                            std::lock_guard<std::mutex> lock(registrations_mutex);
                            for (const auto& reg : registrations) {
                                if (!reg.owner || !reg.owner -> shader) continue;
                                if (!Tool::match_wildcard(reg.model_pattern, model -> get_model_name())) continue;
                                try { model -> apply_material_shader(reg.component_pattern, reg.material_pattern, reg.owner -> shader -> get_surface_factory()); }
                                catch (...) {}
                            }
                        }
                    }
                } else {
                    // Wildcard "*" — remove persistent registration and clear all live models
                    const std::string model_pattern = "*";
                    remove_registration(self.get(), model_pattern, component, material);

                    std::lock_guard<std::mutex> lock(registrations_mutex);
                    for (auto& [mid, inst] : Model::registry.buffer) {
                        if (!inst || !inst -> is_alive()) continue;
                        auto* model = inst -> get_node();
                        if (!model) continue;

                        try { model -> apply_material_shader(component, material, godot::Ref<godot::ShaderMaterial>()); count++; }
                        catch (...) {}

                        for (const auto& reg : registrations) {
                            if (!reg.owner || !reg.owner -> shader) continue;
                            if (!Tool::match_wildcard(reg.model_pattern, model -> get_model_name())) continue;
                            try { model -> apply_material_shader(reg.component_pattern, reg.material_pattern, reg.owner -> shader -> get_surface_factory()); }
                            catch (...) {}
                        }
                    }
                }

                vm -> push_value(count);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "shader_mode", base_class::mode_registry);
        }

        static void clean(const std::string& env) {
            // Drop registrations for this env and replay survivors on affected models,
            // so shaders from other envs that overlap the same surfaces remain intact.
            {
                std::lock_guard<std::mutex> lock(registrations_mutex);
                std::vector<Registration> removed;
                for (const auto& r : registrations)
                    if (r.env == env) removed.push_back(r);

                registrations.erase(
                    std::remove_if(registrations.begin(), registrations.end(), [&env](const Registration& r) { return r.env == env; }),
                    registrations.end()
                );

                for (auto& [mid, inst] : Model::registry.buffer) {
                    if (!inst || !inst -> is_alive()) continue;
                    auto* model = inst -> get_node();
                    if (!model) continue;

                    bool affected = false;
                    for (const auto& reg : removed) {
                        if (!Tool::match_wildcard(reg.model_pattern, model -> get_model_name())) continue;
                        affected = true;
                        try { model -> apply_material_shader(reg.component_pattern, reg.material_pattern, godot::Ref<godot::ShaderMaterial>()); }
                        catch (...) {}
                    }
                    if (!affected) continue;

                    for (const auto& reg : registrations) {
                        if (!reg.owner || !reg.owner -> shader) continue;
                        if (!Tool::match_wildcard(reg.model_pattern, model -> get_model_name())) continue;
                        try { model -> apply_material_shader(reg.component_pattern, reg.material_pattern, reg.owner -> shader -> get_surface_factory()); }
                        catch (...) {}
                    }
                }
            }
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Shader : vm_module {};
}
#endif
