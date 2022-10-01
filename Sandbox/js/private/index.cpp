/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: js: public: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/js/public/index.h>


////////////////////
// Namespace: JS //
////////////////////

namespace Vital::JS {
    std::vector<std::string> vBlacklist = {};
    std::vector<std::string> vModules = {};

    void test() {
        duk_context* ctx = duk_create_heap_default();
        duk_eval_string(ctx, "1+2");
        printf("1+2=%d\n", duk_get_int(ctx, -1));
        duk_destroy_heap(ctx);
    }
}