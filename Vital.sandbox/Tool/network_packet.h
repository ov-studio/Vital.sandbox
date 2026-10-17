#pragma once
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace Vital::Tool::NetworkPacket {
    enum class Kind { Reject, Ping, Quit, Application };

    // Inspect only the shallow wire envelope before any recursive Stack conversion.
    // Unadmitted peers may send ping [serial?] or quit, with no extra fields.
    inline Kind classify(const godot::Dictionary& data, const char* internal_name, bool admitted) {
        if (data.has("array") && data["array"].get_type() != godot::Variant::ARRAY) return Kind::Reject;
        if (data.has("object") && data["object"].get_type() != godot::Variant::DICTIONARY) return Kind::Reject;

        const godot::Dictionary object = data.has("object") ? (godot::Dictionary)data["object"] : godot::Dictionary();
        if (object.has("packet")) {
            const godot::Variant packet = object["packet"];
            if (packet.get_type() != godot::Variant::STRING) return Kind::Reject;
            if ((godot::String)packet == internal_name) {
                if (data.size() != 2 || object.size() != 1 || !data.has("array")) return Kind::Reject;
                const godot::Array args = data["array"];
                if (args.size() < 1 || args.size() > 2 || args[0].get_type() != godot::Variant::STRING) return Kind::Reject;
                const godot::String command = args[0];
                if (command == "ping") {
                    if (args.size() == 2) {
                        if (args[1].get_type() != godot::Variant::STRING) return Kind::Reject;
                        // Fingerprints are short strings; keep pre-handshake allocation bounded.
                        if (((godot::String)args[1]).length() > 256) return Kind::Reject;
                    }
                    return Kind::Ping;
                }
                if (command == "quit" && args.size() == 1) return Kind::Quit;
                return Kind::Reject;
            }
        }
        return admitted ? Kind::Application : Kind::Reject;
    }
}
