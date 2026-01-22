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
    struct nil_t {
        constexpr bool operator == (nil_t) const noexcept { return true; }
    };
    struct Stack;
    struct Value {
        using stack_ptr = std::shared_ptr<Stack>;
        using array_t   = std::vector<Value>;

        using variant_t = std::variant<
            nil_t,
            bool,
            int64_t,
            double,
            std::string,
            array_t,
            stack_ptr
        >;

        variant_t data;

        /* Constructors */
        Value() noexcept : data(nil_t{}) {}
        Value(nil_t) noexcept : data(nil_t{}) {}
        Value(bool v) noexcept : data(v) {}
        Value(int v) noexcept : data(int64_t(v)) {}
        Value(int64_t v) noexcept : data(v) {}
        Value(double v) noexcept : data(v) {}
        Value(const char* v) : data(std::string(v)) {}
        Value(std::string v) noexcept : data(std::move(v)) {}
        Value(array_t v) noexcept : data(std::move(v)) {}
        Value(const Stack& v);
        Value(Stack&& v);

        /* Type checks */
        bool is_nil()    const noexcept { return std::holds_alternative<nil_t>(data); }
        bool is_bool()   const noexcept { return std::holds_alternative<bool>(data); }
        bool is_int()    const noexcept { return std::holds_alternative<int64_t>(data); }
        bool is_double() const noexcept { return std::holds_alternative<double>(data); }
        bool is_string() const noexcept { return std::holds_alternative<std::string>(data); }
        bool is_array()  const noexcept { return std::holds_alternative<array_t>(data); }
        bool is_stack()  const noexcept { return std::holds_alternative<stack_ptr>(data); }

        /* Accessors (unchecked, fast) */
        Stack& as_stack() const { return *std::get<stack_ptr>(data); }
    };

    struct Stack {
        using map_t = std::unordered_map<std::string, Value>;
        map_t values;

        Stack() = default;

        Stack(std::initializer_list<std::pair<std::string, Value>> init) {
            values.reserve(init.size());
            for (auto& it : init)
                values.emplace(it.first, it.second);
        }

        /* Key API */
        bool has(std::string_view key) const {
            return values.find(std::string(key)) != values.end();
        }

        Value& operator[](std::string key) {
            return values[std::move(key)];
        }

        const Value* get(std::string_view key) const {
            auto it = values.find(std::string(key));
            return it != values.end() ? &it->second : nullptr;
        }

        void erase(std::string_view key) {
            values.erase(std::string(key));
        }

        void clear() noexcept {
            values.clear();
        }

        size_t size() const noexcept {
            return values.size();
        }

        /* ============================
           Compatibility API (REQUIRED)
        ============================ */

        // Used by network.cpp
        void push(Stack&& other) {
            for (auto& [k, v] : other.values)
                values.emplace(std::move(k), std::move(v));
        }

        // Stub serialization (replace later with binary/JSON)
        std::string serialize() const {
            return "{}"; // safe stub
        }

        static Stack deserialize(const void*) {
            return {};
        }
    };

    /* ============================
       Value(Stack) ctors
    ============================ */
    inline Value::Value(const Stack& v)
        : data(std::make_shared<Stack>(v)) {}

    inline Value::Value(Stack&& v)
        : data(std::make_shared<Stack>(std::move(v))) {}

} // namespace Vital::Tool
