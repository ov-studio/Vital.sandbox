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
#include <Vital.sandbox/Tool/log.h>
#include <godot_cpp/variant/string.hpp>


/////////////////////////
// Vital: Tool: Stack //
/////////////////////////

namespace Vital::Tool {
    struct Stack;
    struct StackValue;
    struct StackValue {
        using stack_value = std::variant<
            std::nullptr_t,
            bool,
            int32_t,
            int64_t,
            float,
            double,
            std::string,
            std::shared_ptr<Stack>
        >;
        stack_value value{nullptr};


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
        StackValue(std::shared_ptr<Stack> v) : value(std::move(v)) {}
        explicit StackValue(Stack v);


        // Accessors //
        template<typename T>
        const T& as() const { return std::get<T>(value); }
        template<typename T>
        bool is() const { return std::holds_alternative<T>(value); }


        // Equality //
        bool operator==(const StackValue& other) const;
        bool operator!=(const StackValue& other) const { return !(*this == other); }
    };

    struct Stack {
        uint16_t version = Vital::Build.major;
        std::vector<StackValue> array;
        std::map<std::string, StackValue> object;


        // Constructors //
        Stack() = default;
        Stack(std::initializer_list<StackValue> arr) : array(arr) {}


        // Accessors //
        bool empty() const {
            return array.empty() && object.empty();
        }

        bool has(const std::string& key) const {
            return object.find(key) != object.end();
        }

        const StackValue* get(const std::string& key) const {
            auto it = object.find(key);
            return it != object.end() ? &it -> second : nullptr;
        }

        StackValue* get(const std::string& key) {
            auto it = object.find(key);
            return it != object.end() ? &it -> second : nullptr;
        }

        void clear() {
            array.clear();
            object.clear();
        }


        // Converters //
        godot::Dictionary to_dict() const {
            godot::Dictionary dict;
            godot::Array arr;
            arr.resize(static_cast<int>(array.size()));
            for (int i = 0; i < static_cast<int>(array.size()); ++i)
                arr[i] = value_to_variant(array[i]);
            dict["array"] = arr;
            godot::Dictionary obj;
            for (const auto& [key, sv] : object)
                obj[godot::String(key.c_str())] = value_to_variant(sv);
            dict["object"] = obj;
            return dict;
        }

        static Stack from_dict(const godot::Dictionary& dict) {
            Stack stack;
            if (dict.has("array")) {
                const godot::Array arr = dict["array"];
                stack.array.reserve(arr.size());
                for (int i = 0; i < arr.size(); ++i)
                    stack.array.push_back(variant_to_value(arr[i]));
            }
            if (dict.has("object")) {
                const godot::Dictionary obj = dict["object"];
                const godot::Array keys = obj.keys();
                for (int i = 0; i < keys.size(); ++i) {
                    std::string key = ((godot::String)keys[i]).utf8().get_data();
                    stack.object[key] = variant_to_value(obj[keys[i]]);
                }
            }
            return stack;
        }


        // Equality //
        bool operator==(const Stack& other) const { return (version == other.version) && (array == other.array) && (object == other.object); }
        bool operator!=(const Stack& other) const { return !(*this == other); }

        private:
            // StackValue → godot::Variant
            static godot::Variant value_to_variant(const StackValue& sv) {
                return std::visit([](auto&& val) -> godot::Variant {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, std::nullptr_t>) return godot::Variant();
                    else if constexpr (std::is_same_v<T, bool>) return val;
                    else if constexpr (std::is_same_v<T, int32_t>) return (int64_t)val;
                    else if constexpr (std::is_same_v<T, int64_t>) return val;
                    else if constexpr (std::is_same_v<T, float>) return (double)val;
                    else if constexpr (std::is_same_v<T, double>) return val;
                    else if constexpr (std::is_same_v<T, std::string>) return godot::String(val.c_str());
                    else if constexpr (std::is_same_v<T, std::shared_ptr<Stack>>) return val ? val -> to_dict() : godot::Variant();
                    return godot::Variant();
                }, sv.value);
            }

            // godot::Variant → StackValue
            static StackValue variant_to_value(const godot::Variant& v) {
                switch (v.get_type()) {
                    case godot::Variant::NIL: return StackValue(nullptr);
                    case godot::Variant::BOOL: return StackValue((bool)v);
                    case godot::Variant::INT: return StackValue((int32_t)(int64_t)v);
                    case godot::Variant::FLOAT: return StackValue((double)v);
                    case godot::Variant::STRING: return StackValue(std::string(((godot::String)v).utf8().get_data()));
                    case godot::Variant::ARRAY: {
                        const godot::Array arr = v;
                        auto nested = std::make_shared<Stack>();
                        nested -> array.reserve(arr.size());
                        for (int i = 0; i < arr.size(); ++i)
                            nested -> array.push_back(variant_to_value(arr[i]));
                        return StackValue(std::move(nested));
                    }
                    case godot::Variant::DICTIONARY: {
                        return StackValue(std::make_shared<Stack>(Stack::from_dict((godot::Dictionary)v)));
                    }
                    default: return StackValue(nullptr);
                }
            }
    };

    inline StackValue::StackValue(Stack v) : value(std::make_shared<Stack>(std::move(v))) {}
    inline bool StackValue::operator==(const StackValue& other) const {
        if (std::holds_alternative<std::shared_ptr<Stack>>(value) &&
            std::holds_alternative<std::shared_ptr<Stack>>(other.value)) {
            const auto& a = std::get<std::shared_ptr<Stack>>(value);
            const auto& b = std::get<std::shared_ptr<Stack>>(other.value);
            if (!a && !b) return true;
            if (!a || !b) return false;
            return *a == *b;
        }
        return value == other.value;
    }
}