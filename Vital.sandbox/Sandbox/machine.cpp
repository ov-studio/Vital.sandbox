// New file: machine.cpp
#pragma once
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.sandbox/API/utility/coroutine.h>
#include <Vital.sandbox/API/utility/timer.h>
#include <Vital.sandbox/API/utility/thread.h>

// TODO: Improve later // Anisa

namespace Vital::Sandbox {
    void Machine::clear_environment_id(const std::string& id) {
        if (!is_reference(id)) return;
        API::Coroutine::cancel_env(id);
        API::Timer::cancel_env(id);
        API::Thread::cancel_env(id);
        get_reference(id, true);
        lua_pushnil(state);
        lua_rawset(state, LUA_REGISTRYINDEX);
        del_reference(id);
    }
}