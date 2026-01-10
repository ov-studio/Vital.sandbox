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
#include <Vital.sandbox/System/public/vital.h>


/////////////////////////
// Vital: Tool: Event //
/////////////////////////

namespace Vital::System::Event {
    using EventID = unsigned long;
    using EventHandle = std::function<void(Vital::Tool::Stack)>;
    using EventMap = std::map<EventID, EventHandle>;
    using EventPool = std::map<std::string, EventMap>;
    inline EventID id = 0;
    inline EventPool pool;

    inline EventID bind(const std::string& identifier, EventHandle exec) {
        const EventID eid = ++id;
        pool[identifier].emplace(eid, std::move(exec));
        return eid;
    }

    inline bool unbind(const std::string& identifier, EventID eid) {
        auto it = pool.find(identifier);
        if (it == pool.end()) return false;
        it -> second.erase(eid);
        if (it -> second.empty()) pool.erase(it);
        return true;
    }

    inline bool emit(const std::string& identifier, Vital::Tool::Stack arguments = {}) {
        godot::UtilityFunctions::print("called emit -1");
        auto it = pool.find(identifier);
        if (it == pool.end()) return false;
        for (const auto& [id, handler] : it -> second) {
            handler(arguments);
        }
        return true;
    }
}