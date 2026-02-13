/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: stack.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Stack Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/error.h>
#include <Vital.sandbox/Vendor/msgpack/msgpack.hpp>


/////////////////////////
// Vital: Tool: Stack //
/////////////////////////

namespace Vital::Tool {
    struct StackValue {
        using Value = std::variant<
            std::nullptr_t,
            bool,
            int32_t,
            int64_t,
            float,
            double,
            std::string,
            std::vector<std::string>
        >;

        Value value{nullptr};

        // Constructors //
        StackValue() = default;
        StackValue(std::nullptr_t) : value(nullptr) {}
        StackValue(bool v) : value(v) {}
        StackValue(int32_t v) : value(v) {}
        StackValue(int64_t v) : value(v) {}
        StackValue(float v) : value(v) {}
        StackValue(double v) : value(v) {}
        StackValue(const char* v) : value(std::string(v)) {}
        StackValue(std::string v) : value(std::move(v)) {}
        StackValue(std::vector<std::string> v) : value(std::move(v)) {}

        // Accessors //
        template<typename T>
        const T& as() const {
            return std::get<T>(value);
        }

        template<typename T>
        bool is() const {
            return std::holds_alternative<T>(value);
        }
    };

    struct Stack {
        // TODO: USE SANDBOX VERSIONING
        static constexpr uint16_t ProtocolVersion = 1;
        uint16_t version{ProtocolVersion};
        std::vector<StackValue> array;
        std::map<std::string, StackValue> object;

        std::string serialize() const {
            msgpack::sbuffer buffer;
            msgpack::pack(buffer, *this);
            return { buffer.data(), buffer.size() };
        }

        static Stack deserialize(const void* data, size_t size) {
            msgpack::object_handle oh = msgpack::unpack(static_cast<const char*>(data), size);
            Stack s;
            oh.get().convert(s);
            if (s.version != ProtocolVersion) throw std::runtime_error("Stack protocol version mismatch");
            return s;
        }

        static Stack deserialize(const std::string& bytes) {
            return deserialize(bytes.data(), bytes.size());
        }
    };
}

namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {
            // StackValue Adapter //
            template<>
            struct pack<Vital::Tool::StackValue> {
                template<typename Stream>
                msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& pk, Vital::Tool::StackValue const& v) const {
                    pk.pack_array(2);
                    std::visit([&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, std::nullptr_t>) pk.pack(0), pk.pack_nil();
                        else if constexpr (std::is_same_v<T, bool>) pk.pack(1), pk.pack(arg);
                        else if constexpr (std::is_same_v<T, int32_t>) pk.pack(2), pk.pack(arg);
                        else if constexpr (std::is_same_v<T, int64_t>) pk.pack(3), pk.pack(arg);
                        else if constexpr (std::is_same_v<T, float>) pk.pack(4), pk.pack(arg);
                        else if constexpr (std::is_same_v<T, double>) pk.pack(5), pk.pack(arg);
                        else if constexpr (std::is_same_v<T, std::string>) pk.pack(6), pk.pack(arg);
                        else if constexpr (std::is_same_v<T, std::vector<std::string>>) pk.pack(7), pk.pack(arg);
                    }, v.value);
                    return pk;
                }
            };

            template<>
            struct convert<Vital::Tool::StackValue> {
                msgpack::object const& operator()(msgpack::object const& o, Vital::Tool::StackValue& v) const {
                    if (o.type != msgpack::type::ARRAY || o.via.array.size != 2) throw msgpack::type_error();
                    int type = o.via.array.ptr[0].as<int>();
                    const msgpack::object& val = o.via.array.ptr[1];
                    switch (type) {
                        case 0: v.value = nullptr; break;
                        case 1: v.value = val.as<bool>(); break;
                        case 2: v.value = val.as<int32_t>(); break;
                        case 3: v.value = val.as<int64_t>(); break;
                        case 4: v.value = val.as<float>(); break;
                        case 5: v.value = val.as<double>(); break;
                        case 6: v.value = val.as<std::string>(); break;
                        case 7: v.value = val.as<std::vector<std::string>>(); break;
                        default: throw msgpack::type_error();
                    }
                    return o;
                }
            };


            // Stack Adapter //
            template<>
            struct pack<Vital::Tool::Stack> {
                template<typename Stream>
                msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& pk, Vital::Tool::Stack const& s) const {
                    pk.pack_map(3);
                    pk.pack("version"); pk.pack(s.version);
                    pk.pack("array"); pk.pack(s.array);
                    pk.pack("object"); pk.pack(s.object);
                    return pk;
                }
            };

            template<>
            struct convert<Vital::Tool::Stack> {
                msgpack::object const& operator()(msgpack::object const& obj, Vital::Tool::Stack& s) const {
                    if (obj.type != msgpack::type::MAP) throw msgpack::type_error();
                    for (uint32_t i = 0; i < obj.via.map.size; ++i) {
                        auto& k = obj.via.map.ptr[i].key;
                        auto& v = obj.via.map.ptr[i].val;
                        const std::string key = k.as<std::string>();
                        if (key == "version") v.convert(s.version);
                        else if (key == "array") v.convert(s.array);
                        else if (key == "object") v.convert(s.object);
                    }
                    return obj;
                }
            };
        }
    }
}
