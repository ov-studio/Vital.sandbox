/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: public: event.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/stack.h>


/////////////////////////
// Vital: Tool: Event //
/////////////////////////

namespace Vital::Tool::Event {
    typedef unsigned long ID;

    typedef struct {
        const std::function<void()> unbind;
    } Handle;

    typedef std::function<void(Vital::Tool::Stack)> Handler;
}