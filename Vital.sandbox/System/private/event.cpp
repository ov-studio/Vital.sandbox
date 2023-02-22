/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: event.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/event.h>


/////////////////////////
// Vital: Type: Event //
/////////////////////////

namespace Vital::System::Event {
    // Instantiators //
    std::map<std::string, std::map<Vital::Type::Event::Handler, bool>> instance;
    bool isInstance(const std::string& name) { return instance.find(name) != instance.end(); }
    bool create(const std::string& name) {
        if (isInstance(name)) return false;
        instance.emplace(name, std::map<Vital::Type::Event::Handler, bool> {});
        return true;
    }
    bool destroy(const std::string& name) {
        if (!isInstance(name)) return false;
        instance.erase(name);
        return true;
    }

    // APIs //
    bool bind(const std::string& name, Vital::Type::Event::Handler exec) {
        if (!isInstance(name)) return false;
        instance[name].emplace(exec, true);
        return true;
    }
    bool unbind(const std::string& name, Vital::Type::Event::Handler exec) {
        if (!isInstance(name)) return false;
        instance[name].erase(exec);
        return true;
    }
    bool emit(const std::string& name, Vital::Type::Event::Arguments arguments) {
        if (!isInstance(name)) return false;
        for (auto& i : instance[name]) {
            i.first(arguments); 
        }
        return true;
    }
}