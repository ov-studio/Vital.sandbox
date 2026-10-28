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
#include <Vital.sandbox/API/core/display.h>
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

            auto& get_node() {
                return shader;
            }

            void destroy_resource() {
                if (!shader) return;
                purge_registrations(this);
                shader -> destroy();
                shader = nullptr;
            }
        };
        inline static vm_registry<Instance> registry;


        // ------------------------------------------------------------------
        // Shader registry - persistent material assignments with priority
        //
        // Every apply_to_material() call records one Registration. A surface
        // never keeps "whatever was applied last" directly. Instead the winner is
        // recomputed from the registry every time something changes (apply,
        // remove, destroy, resource stop, model spawn):
        //
        //   1. highest priority wins
        //   2. on equal priority, the most recently applied wins
        //
        // Because every assignment stays in the registry, removing or destroying
        // the winner automatically falls back to the next best one, and
        // re-applying a shader moves it to the top of its priority.
        //
        // target == 0 is the "*" assignment (every live model and every model
        // spawned later). Any other value is the instance id of one model; it
        // lasts as long as that model does.
        // ------------------------------------------------------------------
        // BEGIN pure
        struct Registration {
            uint64_t    seq = 0;          // apply order, bumped on every (re-)apply
            int         priority = 0;     // higher wins
            std::string env;              // Lua resource env that created this
            std::string component_pattern;// e.g. "*", "Body"
            std::string material_pattern; // e.g. "*", "body_paint", "body_*"
            uint64_t    target = 0;       // 0 = every model, otherwise one model's instance id
            Instance*   owner = nullptr;  // raw back-pointer; valid while shader alive
        };

        static bool is_candidate(const Registration& reg, uint64_t model_id, const std::string& component, const std::string& material) {
            if (!reg.owner || !reg.owner -> shader) return false;
            if (reg.target != 0 && reg.target != model_id) return false;
            return Tool::match_wildcard(reg.component_pattern, component)
                && Tool::match_wildcard(reg.material_pattern, material);
        }

        // Winner for one surface: highest priority, ties go to the latest applied.
        static const Registration* pick_winner(const std::vector<Registration>& list, uint64_t model_id, const std::string& component, const std::string& material) {
            const Registration* best = nullptr;
            for (const auto& reg : list) {
                if (!is_candidate(reg, model_id, component, material)) continue;
                if (!best || reg.priority > best -> priority || (reg.priority == best -> priority && reg.seq > best -> seq)) best = &reg;
            }
            return best;
        }

        // Adds an assignment, or re-prioritises the identical one (same shader, target and patterns).
        // Either way it becomes the latest apply.
        static void upsert(std::vector<Registration>& list, uint64_t& sequence, Instance* owner, const std::string& env, uint64_t target,
                           const std::string& component, const std::string& material, int priority) {
            for (auto& r : list) {
                if (r.owner == owner && r.target == target && r.component_pattern == component && r.material_pattern == material) {
                    r.priority = priority;
                    r.seq = ++sequence;
                    r.env = env;
                    return;
                }
            }
            list.push_back({ ++sequence, priority, env, component, material, target, owner });
        }

        // Erases this shader's assignments on `target` whose patterns are covered by the given ones
        // ("*" covers everything) and returns them so the caller can re-resolve their surfaces.
        static std::vector<Registration> erase_covered(std::vector<Registration>& list, Instance* owner, uint64_t target,
                                                       const std::string& component_pattern, const std::string& material_pattern) {
            std::vector<Registration> removed;
            for (auto it = list.begin(); it != list.end();) {
                if (it -> owner == owner && it -> target == target
                    && Tool::match_wildcard(component_pattern, it -> component_pattern)
                    && Tool::match_wildcard(material_pattern, it -> material_pattern)) {
                    removed.push_back(*it);
                    it = list.erase(it);
                }
                else ++it;
            }
            return removed;
        }
        // END pure

        inline static std::mutex                registrations_mutex;
        inline static std::vector<Registration> registrations;
        inline static uint64_t                  registration_sequence = 0;

        // Re-resolves every surface of one model matching the patterns.
        // clear_unmatched = true restores the mesh's own material on surfaces nobody claims;
        // false leaves those surfaces alone (used for spawns so we never wipe other overrides).
        // Surfaces that were visited are added to `touched`.
        static void resolve_model(Vital::Engine::Model* model, const std::string& component_pattern, const std::string& material_pattern,
                                  bool clear_unmatched, std::unordered_set<std::string>& touched) {
            const uint64_t model_id = model -> get_instance_id();

            std::vector<std::string> components;
            try { components = model -> get_components(); } catch (...) { return; }

            for (const auto& component : components) {
                if (!Tool::match_wildcard(component_pattern, component)) continue;
                std::vector<std::string> materials;
                try { materials = model -> get_materials(component); } catch (...) { continue; }

                for (const auto& material : materials) {
                    if (!Tool::match_wildcard(material_pattern, material)) continue;
                    const Registration* winner = pick_winner(registrations, model_id, component, material);
                    if (!winner && !clear_unmatched) continue;
                    try {
                        if (winner) model -> apply_material_shader(component, material, winner -> owner -> shader -> get_surface_factory());
                        else model -> apply_material_shader(component, material, godot::Ref<godot::ShaderMaterial>());
                        touched.insert(std::to_string(model_id) + "|" + component + "|" + material);
                    } catch (...) {}
                }
            }
        }

        // Re-resolves the surfaces matching the patterns on the registration's target.
        // Caller must hold registrations_mutex.
        static void refresh(uint64_t target, const std::string& component_pattern, const std::string& material_pattern,
                            bool clear_unmatched, std::unordered_set<std::string>& touched) {
            if (target != 0) {
                auto* object = godot::ObjectDB::get_instance(godot::ObjectID(target));
                auto* model = object ? godot::Object::cast_to<Vital::Engine::Model>(object) : nullptr;
                if (model) resolve_model(model, component_pattern, material_pattern, clear_unmatched, touched);
                return;
            }
            std::lock_guard<std::mutex> model_lock(Model::registry.mutex);
            for (auto& [index, instance] : Model::registry.buffer) {
                if (!instance || !instance -> is_alive()) continue;
                auto* model = instance -> get_node();
                if (!model) continue;
                resolve_model(model, component_pattern, material_pattern, clear_unmatched, touched);
            }
        }

        // Caller must hold registrations_mutex. Drops assignments whose model no longer exists.
        static void prune_dead_targets() {
            registrations.erase(
                std::remove_if(registrations.begin(), registrations.end(), [](const Registration& r) {
                    return r.target != 0 && !godot::ObjectDB::get_instance(godot::ObjectID(r.target));
                }),
                registrations.end()
            );
        }

        // Records (or re-prioritises) an assignment and resolves the surfaces it covers.
        // Returns the number of surfaces on live models that matched.
        static int assign(Instance* owner, const std::string& env, uint64_t target,
                          const std::string& component, const std::string& material, int priority) {
            std::lock_guard<std::mutex> lock(registrations_mutex);
            prune_dead_targets();

            upsert(registrations, registration_sequence, owner, env, target, component, material, priority);

            std::unordered_set<std::string> touched;
            refresh(target, component, material, false, touched);
            return static_cast<int>(touched.size());
        }

        // Removes this shader's assignments on `target` that are covered by the given patterns,
        // then hands the surfaces over to whatever is next in line.
        // Returns the number of surfaces that were re-resolved.
        static int unassign(Instance* owner, uint64_t target, const std::string& component_pattern, const std::string& material_pattern) {
            std::lock_guard<std::mutex> lock(registrations_mutex);
            prune_dead_targets();

            auto removed = erase_covered(registrations, owner, target, component_pattern, material_pattern);

            std::unordered_set<std::string> touched;
            for (const auto& r : removed) refresh(r.target, r.component_pattern, r.material_pattern, true, touched);
            return static_cast<int>(touched.size());
        }

        // Drops every assignment matching the predicate and resolves the surfaces they covered,
        // so lower priority shaders that overlap them take over again.
        template<typename Predicate>
        static void drop_registrations(Predicate&& predicate) {
            std::lock_guard<std::mutex> lock(registrations_mutex);
            prune_dead_targets();

            std::vector<Registration> removed;
            for (auto it = registrations.begin(); it != registrations.end();) {
                if (predicate(*it)) { removed.push_back(*it); it = registrations.erase(it); }
                else ++it;
            }

            std::unordered_set<std::string> touched;
            for (const auto& r : removed) refresh(r.target, r.component_pattern, r.material_pattern, true, touched);
        }

        // Called when a shader instance is destroyed: the next best shader takes over its surfaces
        static void purge_registrations(Instance* owner) {
            drop_registrations([owner](const Registration& r) { return r.owner == owner; });
        }

        static void init(Machine* vm) {
            // Hook entity:spawn so every new model gets the assignments that cover it
            static Tool::Event::Handle spawned_binding;
            spawned_binding.bind("entity:spawn", [](Tool::Stack args) {
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
                    std::unordered_set<std::string> touched;
                    resolve_model(model, "*", "*", false, touched);
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
                            || vm -> is_vector3(idx)
                            || vm -> is_color(idx)
                            || vm_module::is_userdata<API::Image::Instance>(vm, idx)
                            || vm_module::is_userdata<API::Rendertarget::Instance>(vm, idx)
                            || vm_module::is_userdata<API::Display::Instance>(vm, idx);
                    });

                auto name = vm -> get_string(2);

                if (vm_module::is_userdata<API::Image::Instance>(vm, 3)) {
                    auto image = vm_module::get_userdata_object<API::Image::Instance>(vm, 3);
                    vm -> push_value(self -> shader -> set_param_texture(name, image -> get_node() -> get_canvas_texture()));
                }
                else if (vm_module::is_userdata<API::Rendertarget::Instance>(vm, 3)) {
                    auto rendertarget = vm_module::get_userdata_object<API::Rendertarget::Instance>(vm, 3);
                    vm -> push_value(self -> shader -> set_param_viewport_texture(name, rendertarget -> get_node() -> get_texture()));
                }
                else if (vm_module::is_userdata<API::Display::Instance>(vm, 3)) {
                    // The root viewport can't be sampled while it renders, bind the last frame copy instead
                    vm -> push_value(self -> shader -> set_param_rid(name, Vital::Engine::Display::get_singleton() -> pin_last_frame_texture()));
                } 
                else if (vm -> is_color(3)) {
                    auto color = vm -> get_color(3);
                    vm -> push_value(self -> shader -> set_param(name, godot::Variant(color)));
                } 
                else {
                    godot::Variant value;
                    if      (vm -> is_number(3))   value = vm -> get_float(3);
                    else if (vm -> is_bool(3))     value = vm -> get_bool(3);
                    else if (vm -> is_vector3(3))  value = vm -> get_vector3(3);
                    else if (vm -> is_vector2(3))  value = vm -> get_vector2(3);
                    vm -> push_value(self -> shader -> set_param(name, value));
                }
                return 1;
            });

            // apply_to_material(model, component, material, priority = 0)
            //
            // Records a shader assignment and resolves the matching surfaces.
            //
            // model     - a model instance (that model only, lasts as long as the model),
            //             or the string "*" / nil to match every model (also covers models
            //             that spawn later). Any other value is an error.
            // component - mesh component name or wildcard
            // material  - surface/material name or wildcard
            // priority  - higher wins; on equal priority the most recently applied wins.
            //             Applying the same shader to the same target again just updates
            //             its priority and counts as the latest apply.
            //
            // Returns the number of surfaces on live models that matched.
            vm_module::bind_method<Instance>(vm, "apply_to_material", [](auto vm, auto self, auto& id) -> int {
                bool is_model_instance = vm_module::is_userdata<API::Model::Instance>(vm, 2);
                bool is_wildcard       = !is_model_instance && (vm -> is_nil(2) || (vm -> is_string(2) && vm -> get_string(2) == "*"));

                vm_args args_check(vm, id, "(model, component, material, priority = 0)", true);
                if (!is_model_instance && !is_wildcard)
                    args_check.validate(2, [](Machine*, int) { return false; }, "model instance, \"*\", or nil expected");
                args_check.require(3, &Machine::is_string)
                          .require(4, &Machine::is_string)
                          .optional(5, &Machine::is_number);

                auto component = vm -> get_string(3);
                auto material  = vm -> get_string(4);
                int  priority  = vm -> is_number(5) ? vm -> get_int(5) : 0;
                auto env       = vm -> get_environment_id();

                uint64_t target = 0;
                if (is_model_instance) {
                    auto model_inst = vm_module::get_userdata_object<API::Model::Instance>(vm, 2);
                    auto* model = (model_inst && model_inst -> is_alive()) ? model_inst -> get_node() : nullptr;
                    if (!model) {
                        vm -> push_value(0);
                        return 1;
                    }
                    target = model -> get_instance_id();
                }

                vm -> push_value(assign(self.get(), env, target, component, material, priority));
                return 1;
            });

            // remove_from_material(model, component, material)
            //
            // Removes this shader's assignments on the given target that are covered by the
            // component / material patterns, so remove_from_material(model, "*", "*") removes
            // all of them. The surfaces fall back to the next best assignment (priority, then
            // latest applied) or to the mesh's own material when nothing else claims them.
            //
            // model     - a model instance (assignments made for that model), or "*" / nil
            //             (assignments made with "*"). Any other value is an error.
            //
            // Returns the number of surfaces that were re-resolved.
            vm_module::bind_method<Instance>(vm, "remove_from_material", [](auto vm, auto self, auto& id) -> int {
                bool is_model_instance = vm_module::is_userdata<API::Model::Instance>(vm, 2);
                bool is_wildcard       = !is_model_instance && (vm -> is_nil(2) || (vm -> is_string(2) && vm -> get_string(2) == "*"));

                vm_args args_check(vm, id, "(model, component, material)", true);
                if (!is_model_instance && !is_wildcard)
                    args_check.validate(2, [](Machine*, int) { return false; }, "model instance, \"*\", or nil expected");
                args_check.require(3, &Machine::is_string)
                          .require(4, &Machine::is_string);

                auto component = vm -> get_string(3);
                auto material  = vm -> get_string(4);

                uint64_t target = 0;
                if (is_model_instance) {
                    auto model_inst = vm_module::get_userdata_object<API::Model::Instance>(vm, 2);
                    auto* model = (model_inst && model_inst -> is_alive()) ? model_inst -> get_node() : nullptr;
                    if (!model) {
                        vm -> push_value(0);
                        return 1;
                    }
                    target = model -> get_instance_id();
                }

                vm -> push_value(unassign(self.get(), target, component, material));
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "shader_mode", base_class::mode_registry);
        }

        static void clean(const std::string& env) {
            // Drop this env's assignments; shaders from other envs (or lower priorities)
            // that overlap the same surfaces take over again.
            drop_registrations([&env](const Registration& r) { return r.env == env; });
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Shader : vm_module {};
}
#endif
