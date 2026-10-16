/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: particle: trail.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 15/10/2026
     Desc: Trail APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/trail.h>
#include <Vital.sandbox/API/base/node_3d.h>


// TODO: Improve
///////////////////////////
// Vital: API: Trail //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Trail : vm_module {
        inline static const std::vector<std::string> base_scope = {"particle", "trail"};
        inline static constexpr bool has_streamed = true;
        using base_class = Vital::Engine::Trail;

        inline static const std::vector<std::pair<std::string, godot::BaseMaterial3D::BlendMode>> blend_mode_registry = {
            { "MIX", godot::BaseMaterial3D::BLEND_MODE_MIX },
            { "ADD", godot::BaseMaterial3D::BLEND_MODE_ADD },
            { "SUB", godot::BaseMaterial3D::BLEND_MODE_SUB },
            { "MUL", godot::BaseMaterial3D::BLEND_MODE_MUL }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Trail;
            base_class* trail = nullptr;

            auto get_node() {
                return trail;
            }

            bool is_streamed() const {
                return Node_3D::is_streamed(trail);
            }

            bool is_alive() const {
                return trail ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> trail) {
                    instance -> trail -> destroy();
                    instance -> trail = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<Trail>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> trail = base_class::create();
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance, Node_3D::Type::Spatial>(vm);

            // Getters //
            {
                vm_module::bind_method<Instance>(vm, "get_lifetime", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> trail -> get_lifetime());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_width", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> trail -> get_width());
                    return 1;
                });

                // Returns the color at the start of the ribbon, then the color at the end of it
                vm_module::bind_method<Instance>(vm, "get_color", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> trail -> get_color_from());
                    vm -> push_value(self -> trail -> get_color_to());
                    return 2;
                });

                vm_module::bind_method<Instance>(vm, "get_blend_mode", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(static_cast<int>(self -> trail -> get_blend_mode()));
                    return 1;
                });
            }

            // Setters //
            {
                // How long the ribbon takes to fade out behind the node in seconds, at least 0.01
                vm_module::bind_method<Instance>(vm, "set_lifetime", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(seconds)", true)
                        .require(2, &Machine::is_number);

                    self -> trail -> set_lifetime(vm -> get_float(2));
                    vm -> push_value(true);
                    return 1;
                });

                // Ribbon width at its start, it thins out to nothing at the end
                vm_module::bind_method<Instance>(vm, "set_width", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(width)", true)
                        .require(2, &Machine::is_number);

                    self -> trail -> set_width(vm -> get_float(2));
                    vm -> push_value(true);
                    return 1;
                });

                // Color at the start and at the end of the ribbon, e.g. set_color({1, 0.6, 0.1, 1}, {1, 0.1, 0, 0})
                // Without `to` the ribbon fades out in the `from` color (alpha 0)
                vm_module::bind_method<Instance>(vm, "set_color", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(from, to = from)", true)
                        .require(2, &Machine::is_color)
                        .optional(3, &Machine::is_color);

                    auto from = vm -> get_color(2);
                    auto to = vm -> is_color(3) ? vm -> get_color(3) : godot::Color(from.r, from.g, from.b, 0.0f);
                    self -> trail -> set_colors(from, to);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_blend_mode", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(mode)", true)
                        .require_enum(2, blend_mode_registry);

                    self -> trail -> set_blend_mode(static_cast<godot::BaseMaterial3D::BlendMode>(vm -> get_int(2)));
                    vm -> push_value(true);
                    return 1;
                });
            }

            // Misc //
            {
                // Drops the current ribbon, e.g. after repositioning the node
                vm_module::bind_method<Instance>(vm, "clear", [](auto vm, auto self, auto& id) -> int {
                    self -> trail -> clear();
                    vm -> push_value(true);
                    return 1;
                });
            }
        }

        static void inject(Machine* vm) {
            API::Node_3D::inject<Instance>(vm);
            vm -> scope_set_enum(base_scope, "blend_mode", blend_mode_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Trail : vm_module {};
}
#endif
