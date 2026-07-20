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
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/monitor.h>


//////////////////////////
// Vital: API: Monitor //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Monitor : vm_module {
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

        inline static const std::vector<std::pair<std::string, int>> format_registry = {
            { "QUANTITY",   godot::Performance::MONITOR_TYPE_QUANTITY   },
            { "MEMORY",     godot::Performance::MONITOR_TYPE_MEMORY     },
            { "TIME",       godot::Performance::MONITOR_TYPE_TIME       },
            { "PERCENTAGE", godot::Performance::MONITOR_TYPE_PERCENTAGE }
        };

        inline static const std::unordered_map<int, int> native_format = {
            { godot::Performance::TIME_FPS,                             godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::TIME_PROCESS,                         godot::Performance::MONITOR_TYPE_TIME     },
            { godot::Performance::TIME_PHYSICS_PROCESS,                 godot::Performance::MONITOR_TYPE_TIME     },
            { godot::Performance::TIME_NAVIGATION_PROCESS,              godot::Performance::MONITOR_TYPE_TIME     },
            { godot::Performance::MEMORY_STATIC,                        godot::Performance::MONITOR_TYPE_MEMORY   },
            { godot::Performance::MEMORY_STATIC_MAX,                    godot::Performance::MONITOR_TYPE_MEMORY   },
            { godot::Performance::MEMORY_MESSAGE_BUFFER_MAX,            godot::Performance::MONITOR_TYPE_MEMORY   },
            { godot::Performance::OBJECT_COUNT,                         godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::OBJECT_RESOURCE_COUNT,                godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::OBJECT_NODE_COUNT,                    godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::OBJECT_ORPHAN_NODE_COUNT,             godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::RENDER_TOTAL_OBJECTS_IN_FRAME,        godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::RENDER_TOTAL_PRIMITIVES_IN_FRAME,     godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::RENDER_TOTAL_DRAW_CALLS_IN_FRAME,     godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::RENDER_VIDEO_MEM_USED,                godot::Performance::MONITOR_TYPE_MEMORY   },
            { godot::Performance::RENDER_TEXTURE_MEM_USED,              godot::Performance::MONITOR_TYPE_MEMORY   },
            { godot::Performance::RENDER_BUFFER_MEM_USED,               godot::Performance::MONITOR_TYPE_MEMORY   },
            { godot::Performance::PHYSICS_2D_ACTIVE_OBJECTS,            godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::PHYSICS_2D_COLLISION_PAIRS,           godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::PHYSICS_2D_ISLAND_COUNT,              godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::PHYSICS_3D_ACTIVE_OBJECTS,            godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::PHYSICS_3D_COLLISION_PAIRS,           godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::PHYSICS_3D_ISLAND_COUNT,              godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::AUDIO_OUTPUT_LATENCY,                 godot::Performance::MONITOR_TYPE_TIME     },
            { godot::Performance::NAVIGATION_ACTIVE_MAPS,               godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_REGION_COUNT,              godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_AGENT_COUNT,               godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_LINK_COUNT,                godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_POLYGON_COUNT,             godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_EDGE_COUNT,                godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_EDGE_MERGE_COUNT,          godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_EDGE_CONNECTION_COUNT,     godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_EDGE_FREE_COUNT,           godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_OBSTACLE_COUNT,            godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::PIPELINE_COMPILATIONS_CANVAS,         godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::PIPELINE_COMPILATIONS_MESH,           godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::PIPELINE_COMPILATIONS_SURFACE,        godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::PIPELINE_COMPILATIONS_DRAW,           godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::PIPELINE_COMPILATIONS_SPECIALIZATION, godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_2D_ACTIVE_MAPS,            godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_2D_REGION_COUNT,           godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_2D_AGENT_COUNT,            godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_2D_LINK_COUNT,             godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_2D_POLYGON_COUNT,          godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_2D_EDGE_COUNT,             godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_2D_EDGE_MERGE_COUNT,       godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_2D_EDGE_CONNECTION_COUNT,  godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_2D_EDGE_FREE_COUNT,        godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_2D_OBSTACLE_COUNT,         godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_3D_ACTIVE_MAPS,            godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_3D_REGION_COUNT,           godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_3D_AGENT_COUNT,            godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_3D_LINK_COUNT,             godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_3D_POLYGON_COUNT,          godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_3D_EDGE_COUNT,             godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_3D_EDGE_MERGE_COUNT,       godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_3D_EDGE_CONNECTION_COUNT,  godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_3D_EDGE_FREE_COUNT,        godot::Performance::MONITOR_TYPE_QUANTITY },
            { godot::Performance::NAVIGATION_3D_OBSTACLE_COUNT,         godot::Performance::MONITOR_TYPE_QUANTITY }
        };

        struct Stat {
            int exec_ref = LUA_NOREF;
            std::string env;
            std::string name;
            int format = godot::Performance::MONITOR_TYPE_QUANTITY;
        };

        inline static std::unordered_map<std::string, Stat> buffer;

        static std::string find_format(int value) {
            for (auto& [idx, val] : format_registry) {
                if (val == value) return idx;
            }
            return "";
        }

        static void remove_stat(Machine* vm, std::unordered_map<std::string, Stat>::iterator it) {
            godot::Performance::get_singleton() -> remove_custom_monitor(Tool::to_godot_string(it -> first));
            vm -> del_raw_reference(it -> second.exec_ref);
            buffer.erase(it);
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "register", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(id, name, exec, format)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_function)
                    .require_enum(4, format_registry);

                auto key = vm -> get_string(1);
                if (buffer.find(key) != buffer.end()) vm -> push_value(false);
                else {
                    auto name = vm -> get_string(2);
                    auto env = vm -> get_environment_id();
                    auto format = static_cast<godot::Performance::MonitorType>(vm -> get_int(4));
                    int exec_ref = vm -> set_raw_reference(3);
                    buffer[key] = { exec_ref, env, name, static_cast<int>(format) };

                    godot::Array args;
                    args.push_back(Tool::to_godot_string(key));

                    godot::Performance::get_singleton() -> add_custom_monitor(
                        Tool::to_godot_string(key),
                        godot::Callable(Vital::Engine::Monitor::get_singleton(), "dispatch"),
                        args,
                        format
                    );
                    vm -> push_value(true);
                }
                return 1;
            });

            API::bind(vm, base_scope, "unregister", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(id)")
                    .require(1, &Machine::is_string);

                auto key = vm -> get_string(1);
                auto it = buffer.find(key);
                if (it == buffer.end()) vm -> push_value(false);
                else {
                    remove_stat(vm, it);
                    vm -> push_value(true);
                }
                return 1;
            });

            API::bind(vm, base_scope, "has", [](auto vm, auto& id) -> int {
                vm_args args(vm, id, "(id)");
                args.require(1, [](Machine* vm, int idx) { return vm -> is_number(idx) || vm -> is_string(idx); });

                if (vm -> is_string(1)) vm -> push_value(godot::Performance::get_singleton() -> has_custom_monitor(Tool::to_godot_string(vm -> get_string(1))));
                else {
                    auto value = vm -> get_int(1);
                    vm -> push_value(std::find_if(native_registry.begin(), native_registry.end(), [&](const auto& entry) {
                        return entry.second == value;
                    }) != native_registry.end());
                }
                return 1;
            });

            API::bind(vm, base_scope, "get", [](auto vm, auto& id) -> int {
                vm_args args(vm, id, "(id)");
                args.require(1, [](Machine* vm, int idx) { return vm -> is_number(idx) || vm -> is_string(idx); });

                if (vm -> is_string(1)) {
                    auto key = Tool::to_godot_string(vm -> get_string(1));
                    if (!godot::Performance::get_singleton() -> has_custom_monitor(key)) vm -> push_value(false);
                    else {
                        auto value = godot::Performance::get_singleton() -> get_custom_monitor(key);
                        vm -> push_value(value);
                    }
                }
                else {
                    args.validate_enum(1, native_registry);
                    auto value = static_cast<godot::Performance::Monitor>(vm -> get_int(1));
                    vm -> push_value(godot::Performance::get_singleton() -> get_monitor(value));
                }
                return 1;
            });

            API::bind(vm, base_scope, "list", [](auto vm, auto& id) -> int {
                vm -> create_table();
                {
                    vm -> create_table();
                    for (int i = 0; i < static_cast<int>(native_registry.size()); ++i) {
                        auto format_it = native_format.find(native_registry[i].second);
                        vm -> create_table();
                        vm -> push_value(static_cast<int64_t>(native_registry[i].second));
                        vm -> set_table_field("id", -2);
                        vm -> push_value(native_registry[i].first);
                        vm -> set_table_field("name", -2);
                        vm -> push_value(static_cast<int>(format_it != native_format.end() ? format_it -> second : -1));
                        vm -> set_table_field("format", -2);
                        vm -> set_table_field(i + 1, -2);
                    }
                    vm -> set_table_field("native", -2);
                }
                {
                    vm -> create_table();
                    int i = 0;
                    for (auto& [key, stat] : buffer) {
                        vm -> create_table();
                        vm -> push_value(key);
                        vm -> set_table_field("id", -2);
                        vm -> push_value(stat.name);
                        vm -> set_table_field("name", -2);
                        vm -> push_value(static_cast<int>(stat.format));
                        vm -> set_table_field("format", -2);
                        vm -> set_table_field(++i, -2);
                    }
                    vm -> set_table_field("custom", -2);
                }
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "stat_native", native_registry);
            vm -> scope_set_enum(base_scope, "stat_format", format_registry);
        }

        static void clean(const std::string& env) {
            auto vm = Manager::Sandbox::get_singleton() -> get_vm();
            if (!vm) return;

            for (auto it = buffer.begin(); it != buffer.end(); ) {
                if (it -> second.env == env) remove_stat(vm, it++);
                else ++it;
            }
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Monitor : vm_module {};
}
#endif