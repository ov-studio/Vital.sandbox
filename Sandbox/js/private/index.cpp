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
    void test() {
        duk_context* ctx = duk_create_heap_default();
        if (ctx) {
            duk_eval_string(ctx, "print('Hello world from Javascript!');");
            duk_destroy_heap(ctx);
        }
    }
}