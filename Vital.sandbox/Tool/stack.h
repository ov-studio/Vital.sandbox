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

#pragma once

#define MSGPACK_NO_BOOST 1

#include <msgpack.hpp>
#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <sstream>
#include <stdexcept>

namespace Vital::Tool {

// =====================================
// StackValue
// =====================================
struct StackValue {
    enum class Type : uint8_t {
        Int,
        Float,
        Double,
        Long,
        LongLong,
        LongDouble
    };

    Type type{Type::Int};

    union {
        int         i;
        float       f;
        double      d;
        long        l;
        long long   ll;
        long double ld;
    };

    StackValue() : i(0) {}
    StackValue(int v)         : type(Type::Int), i(v) {}
    StackValue(float v)       : type(Type::Float), f(v) {}
    StackValue(double v)      : type(Type::Double), d(v) {}
    StackValue(long v)        : type(Type::Long), l(v) {}
    StackValue(long long v)   : type(Type::LongLong), ll(v) {}
    StackValue(long double v) : type(Type::LongDouble), ld(v) {}
};

// =====================================
// Stack
// =====================================
struct Stack {
    std::vector<StackValue> values;
    std::map<std::string, StackValue> named;

    // ---------- Serialization ----------
    std::string serialize() const {
        std::stringstream ss;
        msgpack::pack(ss, *this);
        return ss.str();
    }

    static Stack deserialize(const void* data, size_t size) {
        msgpack::object_handle oh =
            msgpack::unpack(static_cast<const char*>(data), size);

        Stack s;
        oh.get().convert(s);
        return s;
    }

    static Stack deserialize(const std::string& bytes) {
        return deserialize(bytes.data(), bytes.size());
    }
};

} // namespace Vital::Tool

// =====================================
// MsgPack Adaptors
// =====================================
namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

// ---------- StackValue ----------

template<>
struct pack<Vital::Tool::StackValue> {
    template<typename Stream>
    packer<Stream>& operator()(packer<Stream>& pk,
                               Vital::Tool::StackValue const& v) const {
        pk.pack_array(2);
        pk.pack(static_cast<uint8_t>(v.type));

        switch (v.type) {
            case Vital::Tool::StackValue::Type::Int:        pk.pack(v.i); break;
            case Vital::Tool::StackValue::Type::Float:      pk.pack(v.f); break;
            case Vital::Tool::StackValue::Type::Double:     pk.pack(v.d); break;
            case Vital::Tool::StackValue::Type::Long:       pk.pack(v.l); break;
            case Vital::Tool::StackValue::Type::LongLong:   pk.pack(v.ll); break;
            case Vital::Tool::StackValue::Type::LongDouble:
                pk.pack(static_cast<double>(v.ld)); // msgpack-safe
                break;
        }
        return pk;
    }
};

template<>
struct convert<Vital::Tool::StackValue> {
    msgpack::object const& operator()(msgpack::object const& obj,
                                      Vital::Tool::StackValue& v) const {
        if (obj.type != msgpack::type::ARRAY || obj.via.array.size != 2)
            throw msgpack::type_error();

        v.type = static_cast<Vital::Tool::StackValue::Type>(
            obj.via.array.ptr[0].as<uint8_t>()
        );

        const msgpack::object& val = obj.via.array.ptr[1];

        switch (v.type) {
            case Vital::Tool::StackValue::Type::Int:        v.i  = val.as<int>(); break;
            case Vital::Tool::StackValue::Type::Float:      v.f  = val.as<float>(); break;
            case Vital::Tool::StackValue::Type::Double:     v.d  = val.as<double>(); break;
            case Vital::Tool::StackValue::Type::Long:       v.l  = val.as<long>(); break;
            case Vital::Tool::StackValue::Type::LongLong:   v.ll = val.as<long long>(); break;
            case Vital::Tool::StackValue::Type::LongDouble:
                v.ld = static_cast<long double>(val.as<double>());
                break;
        }
        return obj;
    }
};

// ---------- Stack ----------

template<>
struct pack<Vital::Tool::Stack> {
    template<typename Stream>
    packer<Stream>& operator()(packer<Stream>& pk,
                               Vital::Tool::Stack const& s) const {
        pk.pack_map(2);
        pk.pack("values");
        pk.pack(s.values);
        pk.pack("named");
        pk.pack(s.named);
        return pk;
    }
};

template<>
struct convert<Vital::Tool::Stack> {
    msgpack::object const& operator()(msgpack::object const& obj,
                                      Vital::Tool::Stack& s) const {
        if (obj.type != msgpack::type::MAP)
            throw msgpack::type_error();

        for (uint32_t i = 0; i < obj.via.map.size; ++i) {
            auto& k = obj.via.map.ptr[i].key;
            auto& v = obj.via.map.ptr[i].val;

            if (k.as<std::string>() == "values")
                v.convert(s.values);
            else if (k.as<std::string>() == "named")
                v.convert(s.named);
        }
        return obj;
    }
};

} // adaptor
} // api
} // msgpack
