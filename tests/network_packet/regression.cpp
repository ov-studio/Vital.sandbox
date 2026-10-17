#include <cstdlib>
#include <iostream>
#include <godot_cpp/godot.hpp>
#include <Vital.sandbox/Tool/network_packet.h>
#include <Vital.sandbox/Tool/stack.h>

namespace {
    using Vital::Tool::Stack;
    using Vital::Tool::StackValue;
    using Vital::Tool::NetworkPacket::Kind;
    constexpr auto internal_name = "Network.manager";
    int checks = 0;

    void check(bool passed, const char* label) {
        ++checks;
        if (!passed) {
            std::cerr << "FAIL: " << label << std::endl;
            std::exit(1);
        }
    }

    godot::Dictionary packet(godot::Array args = {}, godot::Variant type = "Network.manager") {
        godot::Dictionary object;
        object["packet"] = type;
        godot::Dictionary data;
        data["array"] = args;
        data["object"] = object;
        return data;
    }

    Kind classify(const godot::Dictionary& data, bool admitted = false) {
        return Vital::Tool::NetworkPacket::classify(data, internal_name, admitted);
    }

    void run() {
        godot::Array ping;
        ping.push_back("ping");
        check(classify(packet(ping)) == Kind::Ping, "legacy ping without serial");
        ping.push_back("fingerprint");
        check(classify(packet(ping)) == Kind::Ping, "ping with serial");
        check(classify(packet(ping), true) == Kind::Ping, "repeat ping");

        godot::Array quit;
        quit.push_back("quit");
        check(classify(packet(quit)) == Kind::Quit, "quit before admission");
        check(classify(packet(quit), true) == Kind::Quit, "quit after admission");
        quit.push_back("extra");
        check(classify(packet(quit)) == Kind::Reject, "quit rejects extra arguments");
        check(classify(packet()) == Kind::Reject, "empty internal command array");

        const std::vector<godot::Variant> wrong_types = {
            godot::Variant(), true, 7, 3.5, godot::Array(), godot::Dictionary()
        };
        for (const auto& value : wrong_types) {
            check(classify(packet(ping, value)) == Kind::Reject, "non-string discriminator before admission");
            check(classify(packet(ping, value), true) == Kind::Reject, "non-string discriminator after admission");
            godot::Array command;
            command.push_back(value);
            check(classify(packet(command)) == Kind::Reject, "non-string command");
            auto bad_serial = ping.duplicate();
            bad_serial[1] = value;
            check(classify(packet(bad_serial)) == Kind::Reject, "non-string serial");
        }

        godot::Array unknown;
        unknown.push_back("unknown");
        check(classify(packet(unknown)) == Kind::Reject, "unknown command");
        auto extra_args = ping.duplicate();
        extra_args.push_back("extra");
        check(classify(packet(extra_args)) == Kind::Reject, "ping rejects extra arguments");
        auto extra_field = packet(ping);
        extra_field["extra"] = godot::Array();
        check(classify(extra_field) == Kind::Reject, "internal envelope rejects extra fields");
        extra_field = packet(ping);
        godot::Dictionary object = extra_field["object"];
        object["extra"] = true;
        check(classify(extra_field) == Kind::Reject, "internal object rejects extra fields");

        for (const auto& value : wrong_types) {
            if (value.get_type() != godot::Variant::ARRAY) {
                auto data = packet(ping);
                data["array"] = value;
                check(classify(data) == Kind::Reject, "wrong array envelope type");
            }
            if (value.get_type() != godot::Variant::DICTIONARY) {
                auto data = packet(ping);
                data["object"] = value;
                check(classify(data) == Kind::Reject, "wrong object envelope type");
            }
        }
        auto missing = packet(ping);
        missing.erase("array");
        check(classify(missing) == Kind::Reject, "internal packet needs array");
        missing = packet(ping);
        missing.erase("object");
        check(classify(missing) == Kind::Reject, "pre-handshake packet needs discriminator");

        auto serial = ping.duplicate();
        serial[1] = godot::String("a").repeat(256);
        check(classify(packet(serial)) == Kind::Ping, "bounded serial accepted");
        serial[1] = godot::String("a").repeat(257);
        check(classify(packet(serial)) == Kind::Reject, "oversized serial rejected");

        auto application = packet({}, "game");
        check(classify(application) == Kind::Reject, "application before handshake");
        check(classify(application, true) == Kind::Application, "application after handshake");
        for (const char* field : {"__event", "__reply_serial"}) {
            godot::Dictionary event;
            event[field] = "value";
            application["object"] = event;
            check(classify(application) == Kind::Reject, "script dispatch before handshake");
            check(classify(application, true) == Kind::Application, "script dispatch after handshake");
        }

        // Deep and cyclic values must never reach recursive Stack conversion before admission.
        godot::Array nested;
        for (int i = 0; i < 512; ++i) {
            godot::Array parent;
            parent.push_back(nested);
            nested = parent;
        }
        check(classify(packet(nested, "game")) == Kind::Reject, "deep application before handshake");
        check(classify(packet(nested)) == Kind::Reject, "deep internal command");
        auto nested_serial = ping.duplicate();
        nested_serial[1] = nested;
        check(classify(packet(nested_serial)) == Kind::Reject, "deep ping serial");
        godot::Array cyclic;
        cyclic.push_back(cyclic);
        check(classify(packet(cyclic, "game")) == Kind::Reject, "cyclic application before handshake");
        cyclic.clear();

        Stack stack;
        check(!stack.is_packet(internal_name), "missing Stack discriminator");
        const std::vector<StackValue> invalid_discriminators = {
            nullptr, true, int32_t(7), int64_t(8), 1.5f, 3.5,
            StackValue(std::make_shared<Stack>())
        };
        for (const auto& value : invalid_discriminators) {
            stack.object["packet"] = value;
            check(!stack.is_packet(internal_name), "Stack discriminator type guard");
        }
        stack = Stack::make_packet(internal_name, {"ping", "fingerprint"});
        check(stack.is_packet(internal_name), "matching Stack discriminator");
        check(!stack.is_packet("other"), "mismatched Stack discriminator");
        check(classify(stack.to_dict()) == Kind::Ping, "production ping serialization");
        check(Stack::from_dict(stack.to_dict()) == stack, "valid Stack roundtrip");
        const auto numeric = Stack::from_dict(packet({}, 7));
        check(!numeric.is_packet(internal_name), "numeric wire discriminator converted without throwing");
        std::cout << "PASS: " << checks << " network packet regression checks" << std::endl;
    }

    void initialize(godot::ModuleInitializationLevel level) {
        if (level == godot::MODULE_INITIALIZATION_LEVEL_SCENE) run();
    }
}

extern "C" GDExtensionBool GDE_EXPORT network_packet_test_init(
    GDExtensionInterfaceGetProcAddress get_proc_address,
    GDExtensionClassLibraryPtr library,
    GDExtensionInitialization* initialization) {
    godot::GDExtensionBinding::InitObject init(get_proc_address, library, initialization);
    init.register_initializer(initialize);
    init.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
    return init.init();
}
