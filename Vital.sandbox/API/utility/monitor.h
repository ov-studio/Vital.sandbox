/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: monitor.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Monitor APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


//////////////////////////
// Vital: API: Monitor //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Performance : vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "monitor"};

        inline static const std::vector<std::pair<std::string, int>> native_registry = {
            { "TIME_FPS",                             godot::Performance::TIME_FPS                             },
            { "TIME_PROCESS",                         godot::Performance::TIME_PROCESS                         },
            { "TIME_PHYSICS_PROCESS",                 godot::Performance::TIME_PHYSICS_PROCESS                 },
            { "TIME_NAVIGATION_PROCESS",              godot::Performance::TIME_NAVIGATION_PROCESS              },
            { "MEMORY_STATIC",                        godot::Performance::MEMORY_STATIC                        },
            { "MEMORY_STATIC_MAX",                    godot::Performance::MEMORY_STATIC_MAX                    },
            { "MEMORY_MESSAGE_BUFFER_MAX",            godot::Performance::MEMORY_MESSAGE_BUFFER_MAX            },
            { "OBJECT_COUNT",                         godot::Performance::OBJECT_COUNT                         },
            { "OBJECT_RESOURCE_COUNT",                godot::Performance::OBJECT_RESOURCE_COUNT                },
            { "OBJECT_NODE_COUNT",                    godot::Performance::OBJECT_NODE_COUNT                    },
            { "OBJECT_ORPHAN_NODE_COUNT",             godot::Performance::OBJECT_ORPHAN_NODE_COUNT             },
            { "RENDER_TOTAL_OBJECTS_IN_FRAME",        godot::Performance::RENDER_TOTAL_OBJECTS_IN_FRAME        },
            { "RENDER_TOTAL_PRIMITIVES_IN_FRAME",     godot::Performance::RENDER_TOTAL_PRIMITIVES_IN_FRAME     },
            { "RENDER_TOTAL_DRAW_CALLS_IN_FRAME",     godot::Performance::RENDER_TOTAL_DRAW_CALLS_IN_FRAME     },
            { "RENDER_VIDEO_MEM_USED",                godot::Performance::RENDER_VIDEO_MEM_USED                },
            { "RENDER_TEXTURE_MEM_USED",              godot::Performance::RENDER_TEXTURE_MEM_USED              },
            { "RENDER_BUFFER_MEM_USED",               godot::Performance::RENDER_BUFFER_MEM_USED               },
            { "PHYSICS_2D_ACTIVE_OBJECTS",            godot::Performance::PHYSICS_2D_ACTIVE_OBJECTS            },
            { "PHYSICS_2D_COLLISION_PAIRS",           godot::Performance::PHYSICS_2D_COLLISION_PAIRS           },
            { "PHYSICS_2D_ISLAND_COUNT",              godot::Performance::PHYSICS_2D_ISLAND_COUNT              },
            { "PHYSICS_3D_ACTIVE_OBJECTS",            godot::Performance::PHYSICS_3D_ACTIVE_OBJECTS            },
            { "PHYSICS_3D_COLLISION_PAIRS",           godot::Performance::PHYSICS_3D_COLLISION_PAIRS           },
            { "PHYSICS_3D_ISLAND_COUNT",              godot::Performance::PHYSICS_3D_ISLAND_COUNT              },
            { "AUDIO_OUTPUT_LATENCY",                 godot::Performance::AUDIO_OUTPUT_LATENCY                 },
            { "NAVIGATION_ACTIVE_MAPS",               godot::Performance::NAVIGATION_ACTIVE_MAPS               },
            { "NAVIGATION_REGION_COUNT",              godot::Performance::NAVIGATION_REGION_COUNT              },
            { "NAVIGATION_AGENT_COUNT",               godot::Performance::NAVIGATION_AGENT_COUNT               },
            { "NAVIGATION_LINK_COUNT",                godot::Performance::NAVIGATION_LINK_COUNT                },
            { "NAVIGATION_POLYGON_COUNT",             godot::Performance::NAVIGATION_POLYGON_COUNT             },
            { "NAVIGATION_EDGE_COUNT",                godot::Performance::NAVIGATION_EDGE_COUNT                },
            { "NAVIGATION_EDGE_MERGE_COUNT",          godot::Performance::NAVIGATION_EDGE_MERGE_COUNT          },
            { "NAVIGATION_EDGE_CONNECTION_COUNT",     godot::Performance::NAVIGATION_EDGE_CONNECTION_COUNT     },
            { "NAVIGATION_EDGE_FREE_COUNT",           godot::Performance::NAVIGATION_EDGE_FREE_COUNT           },
            { "NAVIGATION_OBSTACLE_COUNT",            godot::Performance::NAVIGATION_OBSTACLE_COUNT            },
            { "PIPELINE_COMPILATIONS_CANVAS",         godot::Performance::PIPELINE_COMPILATIONS_CANVAS         },
            { "PIPELINE_COMPILATIONS_MESH",           godot::Performance::PIPELINE_COMPILATIONS_MESH           },
            { "PIPELINE_COMPILATIONS_SURFACE",        godot::Performance::PIPELINE_COMPILATIONS_SURFACE        },
            { "PIPELINE_COMPILATIONS_DRAW",           godot::Performance::PIPELINE_COMPILATIONS_DRAW           },
            { "PIPELINE_COMPILATIONS_SPECIALIZATION", godot::Performance::PIPELINE_COMPILATIONS_SPECIALIZATION },
            { "NAVIGATION_2D_ACTIVE_MAPS",            godot::Performance::NAVIGATION_2D_ACTIVE_MAPS            },
            { "NAVIGATION_2D_REGION_COUNT",           godot::Performance::NAVIGATION_2D_REGION_COUNT           },
            { "NAVIGATION_2D_AGENT_COUNT",            godot::Performance::NAVIGATION_2D_AGENT_COUNT            },
            { "NAVIGATION_2D_LINK_COUNT",             godot::Performance::NAVIGATION_2D_LINK_COUNT             },
            { "NAVIGATION_2D_POLYGON_COUNT",          godot::Performance::NAVIGATION_2D_POLYGON_COUNT          },
            { "NAVIGATION_2D_EDGE_COUNT",             godot::Performance::NAVIGATION_2D_EDGE_COUNT             },
            { "NAVIGATION_2D_EDGE_MERGE_COUNT",       godot::Performance::NAVIGATION_2D_EDGE_MERGE_COUNT       },
            { "NAVIGATION_2D_EDGE_CONNECTION_COUNT",  godot::Performance::NAVIGATION_2D_EDGE_CONNECTION_COUNT  },
            { "NAVIGATION_2D_EDGE_FREE_COUNT",        godot::Performance::NAVIGATION_2D_EDGE_FREE_COUNT        },
            { "NAVIGATION_2D_OBSTACLE_COUNT",         godot::Performance::NAVIGATION_2D_OBSTACLE_COUNT         },
            { "NAVIGATION_3D_ACTIVE_MAPS",            godot::Performance::NAVIGATION_3D_ACTIVE_MAPS            },
            { "NAVIGATION_3D_REGION_COUNT",           godot::Performance::NAVIGATION_3D_REGION_COUNT           },
            { "NAVIGATION_3D_AGENT_COUNT",            godot::Performance::NAVIGATION_3D_AGENT_COUNT            },
            { "NAVIGATION_3D_LINK_COUNT",             godot::Performance::NAVIGATION_3D_LINK_COUNT             },
            { "NAVIGATION_3D_POLYGON_COUNT",          godot::Performance::NAVIGATION_3D_POLYGON_COUNT          },
            { "NAVIGATION_3D_EDGE_COUNT",             godot::Performance::NAVIGATION_3D_EDGE_COUNT             },
            { "NAVIGATION_3D_EDGE_MERGE_COUNT",       godot::Performance::NAVIGATION_3D_EDGE_MERGE_COUNT       },
            { "NAVIGATION_3D_EDGE_CONNECTION_COUNT",  godot::Performance::NAVIGATION_3D_EDGE_CONNECTION_COUNT  },
            { "NAVIGATION_3D_EDGE_FREE_COUNT",        godot::Performance::NAVIGATION_3D_EDGE_FREE_COUNT        },
            { "NAVIGATION_3D_OBSTACLE_COUNT",         godot::Performance::NAVIGATION_3D_OBSTACLE_COUNT         }
        };

        inline static const std::vector<std::pair<std::string, int>> stat_type_registry = {
            { "QUANTITY",   godot::Performance::MONITOR_TYPE_QUANTITY   },
            { "MEMORY",     godot::Performance::MONITOR_TYPE_MEMORY     },
            { "TIME",       godot::Performance::MONITOR_TYPE_TIME       },
            { "PERCENTAGE", godot::Performance::MONITOR_TYPE_PERCENTAGE }
        };

        struct Custom_Stat {
            int exec_ref = LUA_NOREF;
            std::string env;
        };

        inline static std::unordered_map<std::string, Custom_Stat> custom_stats;

        static void remove_stat_entry(Machine* vm, std::unordered_map<std::string, Custom_Stat>::iterator it) {
            godot::Performance::get_singleton() -> remove_custom_monitor(Tool::to_godot_string(it -> first));
            vm -> del_raw_reference(it -> second.exec_ref);
            custom_stats.erase(it);
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get_stat", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(stat)")
                    .require_enum(1, native_registry);

                auto monitor = static_cast<godot::Performance::Monitor>(vm -> get_int(1));
                vm -> push_value(godot::Performance::get_singleton() -> get_monitor(monitor));
                return 1;
            });

            API::bind(vm, base_scope, "get_stats", [](auto vm, auto& id) -> int {
                vm -> create_table();
                vm -> create_table();
                for (int i = 0; i < static_cast<int>(native_registry.size()); ++i) {
                    vm -> push_value(native_registry[i].first);
                    vm -> set_table_field(i + 1, -2);
                }
                vm -> set_table_field("native", -2);

                auto names = godot::Performance::get_singleton() -> get_custom_monitor_names();
                vm -> create_table();
                for (int i = 0; i < names.size(); ++i) {
                    vm -> push_value(std::string(godot::String(names[i]).utf8().get_data()));
                    vm -> set_table_field(i + 1, -2);
                }
                vm -> set_table_field("custom", -2);
                return 1;
            });

            API::bind(vm, base_scope, "has_custom_stat", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(id)")
                    .require(1, &Machine::is_string);

                vm -> push_value(godot::Performance::get_singleton() -> has_custom_monitor(Tool::to_godot_string(vm -> get_string(1))));
                return 1;
            });

            API::bind(vm, base_scope, "get_custom_stat", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(id)")
                    .require(1, &Machine::is_string);

                auto value = godot::Performance::get_singleton() -> get_custom_monitor(Tool::to_godot_string(vm -> get_string(1)));
                vm -> push_value(value);
                return 1;
            });

            API::bind(vm, base_scope, "remove_custom_stat", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(id)")
                    .require(1, &Machine::is_string);

                auto key = vm -> get_string(1);
                auto it = custom_stats.find(key);
                if (it != custom_stats.end()) remove_stat_entry(vm, it);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "add_custom_stat", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(id, exec, type)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function)
                    .require_enum(3, stat_type_registry);

                auto key = vm -> get_string(1);
                auto env = vm -> get_environment_id();
                auto type = static_cast<godot::Performance::MonitorType>(vm -> get_int(3));
                auto old = custom_stats.find(key);
                if (old != custom_stats.end()) remove_stat_entry(vm, old);
                int exec_ref = vm -> set_raw_reference(2);
                custom_stats[key] = { exec_ref, env };
                godot::Performance::get_singleton() -> add_custom_monitor(
                    Tool::to_godot_string(key),
                    Machine::make_callable(exec_ref, fmt::format("stat:{}", key)),
                    godot::Array(),
                    type
                );
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "stat_native", native_registry);
            vm -> scope_set_enum(base_scope, "stat_type", stat_type_registry);
        }

        static void clean(const std::string& env) {
            auto vm = Manager::Sandbox::get_singleton() -> get_vm();
            if (!vm) return;

            for (auto it = custom_stats.begin(); it != custom_stats.end(); ) {
                if (it -> second.env == env) remove_stat_entry(vm, it++);
                else ++it;
            }
        }
    };
}