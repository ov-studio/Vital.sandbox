/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: event.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>


/////////////////////////
// Vital: Tool: Event //
/////////////////////////

namespace Vital::System::Event {
    using event_id = unsigned long;
    using event_handle = std::function<void(Vital::Tool::Stack)>;
    using event_map = std::map<event_id, event_handle>;
    using event_pool = std::map<std::string, event_map>;
    inline event_id id = 0;
    inline event_pool pool;

    inline event_id bind(const std::string& identifier, event_handle exec) {
        const event_id eid = ++id;
        pool[identifier].emplace(eid, std::move(exec));
        return eid;
    }

    inline bool unbind(const std::string& identifier, event_id eid) {
        auto it = pool.find(identifier);
        if (it == pool.end()) return false;
        it -> second.erase(eid);
        if (it -> second.empty()) pool.erase(it);
        return true;
    }

    inline bool emit(const std::string& identifier, Vital::Tool::Stack arguments = {}) {
        auto it = pool.find(identifier);
        if (it == pool.end()) return false;
        for (const auto& [id, handler] : it -> second) {
            handler(arguments);
        }
        return true;
    }
}