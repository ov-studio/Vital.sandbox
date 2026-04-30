/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: machine.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Machine
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#include <Vital.sandbox/Sandbox/machine.h>

namespace Vital::Sandbox {
    vm_env_cancellers Machine::env_cancellers = {};

    void Machine::clear_environment_id(const std::string& id) {
        if (!is_reference(id)) return;
        for (auto& cancel : env_cancellers) cancel(id);
        get_reference(id, true);
        lua_pushnil(state);
        lua_rawset(state, LUA_REGISTRYINDEX);
        del_reference(id);
    }
}