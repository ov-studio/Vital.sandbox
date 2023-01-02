/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: c++: coroutine.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Coroutine Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/thread.h>
#include <Sandbox/lua/public/api.h>
#include <Vendor/lua/lauxlib.h>
#include <Vendor/lua/lualib.h>


////////////////////////////
// Lua: Coroutine Binder //
////////////////////////////


        /*
        ** Resumes a coroutine. Returns the number of results for non-error
        ** cases or -1 for errors.
        */
#define l_likely(x)	luai_likely(x)
#define l_unlikely(x)	luai_unlikely(x)

static int auxresume(lua_State* L, lua_State* co, int narg) {
    int status, nres;
    if (l_unlikely(!lua_checkstack(co, narg))) {
        lua_pushliteral(L, "too many arguments to resume");
        return -1;  /* error flag */
    }
    lua_xmove(L, co, narg);
    status = lua_resume(co, L, narg, &nres);
    if (l_likely(status == LUA_OK || status == LUA_YIELD)) {
        if (l_unlikely(!lua_checkstack(L, nres + 1))) {
            lua_pop(co, nres);  /* remove results anyway */
            lua_pushliteral(L, "too many results to resume");
            return -1;  /* error flag */
        }
        lua_xmove(co, L, nres);  /* move yielded values */
        return nres;
    }
    else {
        lua_xmove(co, L, 1);  /* move error message */
        return -1;  /* error flag */
    }
}

static int luaB_coresume(lua_State* L) {
    lua_State* co = lua_tothread(L, 1);
    int r;
    r = auxresume(L, co, lua_gettop(L) - 1);
    if (l_unlikely(r < 0)) {
        lua_pushboolean(L, 0);
        lua_insert(L, -2);
        return 2;  /* return false + error message */
    }
    else {
        lua_pushboolean(L, 1);
        lua_insert(L, -(r + 1));
        return r + 1;  /* return true + 'resume' returns */
    }
}

namespace Vital::Sandbox::Lua::API {
    static bool isBound = false;
    void vSandbox_Coroutine() {
        if (isBound) return; isBound = true;

        bind("coroutine", "create", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isFunction(1))) throw ErrorCode["invalid-arguments"];
                auto thread = vm -> createThread();
                vm -> push(1);
                vm -> move(thread, 1);
                return 1;
            });
        });

        bind("coroutine", "customresume", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isThread(1))) throw ErrorCode["invalid-arguments"];
                auto thread = vm -> getThread(1);
                std::cout << "\nCustom Resume Invoked";
                lua_resume(thread, thread, 0, 0);
                return 1;
            });
        });

        bind("coroutine", "pause", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                //if (!isThread) throw throw ErrorCode["invalid-thread"];
                vm -> pause();
                return 1;
            });
        });

        bind("coroutine", "sleep", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isNumber(1))) throw ErrorCode["invalid-arguments"];
                auto duration = vm -> getInt(1);
                // TODO: Pause here
                auto thread = Vital::System::Thread::create([&]() -> void {
                    // TODO: Resume here
                    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
                });
                return 1;
            });
        });
    }
}
