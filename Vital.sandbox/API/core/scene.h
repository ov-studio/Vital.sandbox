/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: scene.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Scene APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/API/core/model.h>


////////////////////////
// Vital: API: Scene //
////////////////////////

// TODO: Improve
namespace Vital::Sandbox::API {
    struct Scene : vm_module {
        inline static const std::string base_name = "engine";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "get_entities", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(type)")
                    .require(1, &Machine::is_string);

                const std::string type = vm -> get_string(1);
                vm -> create_table();
                int count = 0;

                if (type == "model") {
                    std::lock_guard<std::mutex> lock(Model::mutex);
                    for (auto& [instance_id, instance] : Model::buffer) {
                        if (!instance -> model) continue;
                        if (instance -> userdata) instance -> vm -> get_reference(instance -> self_reference(), true);
                        else {
                            vm -> create_object(Model::base_name, instance.get());
                            instance -> userdata = vm_module::get_userdata_ptr(vm, -1);
                            instance -> set_ref(instance -> self_reference(), -1);
                        }
                        vm -> set_table_field(++count, -2);
                    }
                }
                else throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: unknown element type '{}'", type));
                return 1;
            });
        }
    };
}