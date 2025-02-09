/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: event.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>
#include <Type/public/stack.h>


/////////////////////////
// Vital: Type: Event //
/////////////////////////

namespace Vital::Type::Event {
    typedef unsigned long ID;

    typedef struct {
        const std::function<void()> unbind;
    } Handle;

    typedef std::function<void(Vital::Type::Stack::Instance)> Handler;
}