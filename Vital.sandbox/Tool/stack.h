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


/////////////////////////
// Vital: Tool: Stack //
/////////////////////////

namespace Vital::Tool {
    class Stack {
        public:
            class Value;
        private:
            std::vector<Value> vector;
            std::map<std::string, Value> map;
        public:
            class Value {
                private:
                    std::variant<std::string, int, float, double, long, long long, long double, unsigned, unsigned long, unsigned long long> value;
                public:
                    // Instantiators //
                    template <typename T>
                    Value(T argument) : value(argument) {}

                    // Checkers //
                    bool isString() { return std::holds_alternative<std::string>(value); }
                    bool isNumber() {
                        return (
                            std::holds_alternative<int>(value) || 
                            std::holds_alternative<float>(value) || 
                            std::holds_alternative<double>(value) || 
                            std::holds_alternative<long>(value) || 
                            std::holds_alternative<long long>(value) || 
                            std::holds_alternative<long double>(value) || 
                            std::holds_alternative<unsigned>(value) || 
                            std::holds_alternative<unsigned long>(value) || 
                            std::holds_alternative<unsigned long long>(value)
                        );
                    }

                    // Getters //
                    template <typename T>
                    T get() const { return std::get<T>(value); }

                    // Utils //
                    std::pair<std::string, std::string> serialize() {
                        if (isString()) return {"std::string", get<std::string>()};
                        if (isNumber()) {
                            std::string type;
                            std::ostringstream oss;
                            if (std::holds_alternative<int>(value)) {
                                type = "int";
                                oss << get<int>();
                            } else if (std::holds_alternative<float>(value)) {
                                type = "float";
                                oss << get<float>();
                            } else if (std::holds_alternative<double>(value)) {
                                type = "double";
                                oss << get<double>();
                            } else if (std::holds_alternative<long>(value)) {
                                type = "long";
                                oss << get<long>();
                            } else if (std::holds_alternative<long long>(value)) {
                                type = "long long";
                                oss << get<long long>();
                            } else if (std::holds_alternative<long double>(value)) {
                                type = "long double";
                                oss << get<long double>();
                            } else if (std::holds_alternative<unsigned int>(value)) {
                                type = "unsigned int";
                                oss << get<unsigned int>();
                            } else if (std::holds_alternative<unsigned long>(value)) {
                                type = "unsigned long";
                                oss << get<unsigned long>();
                            } else if (std::holds_alternative<unsigned long long>(value)) {
                                type = "unsigned long long";
                                oss << get<unsigned long long>();
                            } 
                            return {type, oss.str()};
                        }
                        throw Vital::Error::fetch("invalid-result");
                    }
                    static Value deserialize(const std::pair<std::string, std::string>& serial) {
                        const auto& type = serial.first;
                        const auto& value = serial.second;
                        if (type == "std::string") return Value(value);
                        if (type == "int") return Value(std::stoi(value));
                        if (type == "float") return Value(std::stof(value));
                        if (type == "double") return Value(std::stod(value));
                        if (type == "long") return Value(std::stol(value));
                        if (type == "long long") return Value(std::stoll(value));
                        if (type == "long double") return Value(std::stold(value));
                        if (type == "unsigned int") return Value(static_cast<unsigned int>(std::stoul(value)));
                        if (type == "unsigned long") return Value(std::stoul(value));
                        if (type == "unsigned long long") return Value(std::stoull(value));
                        throw Vital::Error::fetch("invalid-result");
                    }
            };

            // Checkers //
            bool isNil(int index) { return ((index < 0) || vector.empty() || (index >= vector.size()) && true) || false; };
            bool isNil(const std::string& index) { return map.find(index) == map.end(); };
            bool isString(int index) { return (!isNil(index) && vector.at(index).isString() && true) || false; }
            bool isString(const std::string& index) { return (!isNil(index) && map.at(index).isString() && true) || false; }
            bool isNumber(int index) { return (!isNil(index) && vector.at(index).isNumber() && true) || false; }
            bool isNumber(const std::string& index) { return (!isNil(index) && map.at(index).isNumber() && true) || false; }
        
            // Getters //
            template <typename T>
            T get(int index = 0) {
                if (isNil(index)) throw Vital::Error::fetch("invalid-result");
                return vector.at(index).get<T>();
            }
            template <typename T>
            T get(const std::string& index) {
                if (isNil(index)) throw Vital::Error::fetch("invalid-result");
                return map.at(index).get<T>();
            }

            // Pushers //
            template <typename T>
            void push(T value) { vector.push_back(Value(value)); }
            template <typename T>
            void push(const std::string& index, T value) { map.emplace(index, Value(value)); }

            // Poppers
            void pop(int index) { vector.erase(vector.begin() + index - 1); }
            void pop(const std::string& index) { map.erase(index); }

            // Utils //
            std::string serialize() {
                std::ostringstream stream;
                const auto size = sizeof(size_t);
                const size_t size_vector = vector.size(), size_map = map.size();
                stream.write(reinterpret_cast<const char*>(&size_vector), size);
                stream.write(reinterpret_cast<const char*>(&size_map), size);
                for (int i = 0; i < size_vector; i++) {
                    auto value = vector.at(i).serialize();
                    const size_t size_value_type = value.first.size(), size_value = value.second.size();
                    stream.write(reinterpret_cast<const char*>(&size_value_type), size);
                    stream.write(reinterpret_cast<const char*>(&size_value), size);
                    stream.write(value.first.c_str(), size_value_type);
                    stream.write(value.second.c_str(), size_value);
                }
                for (auto& i : map) {
                    auto value = i.second.serialize();
                    const size_t size_index = i.first.size(), size_value_type = value.first.size(), size_value = value.second.size();
                    stream.write(reinterpret_cast<const char*>(&size_index), size);
                    stream.write(reinterpret_cast<const char*>(&size_value_type), size);
                    stream.write(reinterpret_cast<const char*>(&size_value), size);
                    stream.write(i.first.c_str(), size_index);
                    stream.write(value.first.c_str(), size_value_type);
                    stream.write(value.second.c_str(), size_value);
                }
                return stream.str();
            }
            static Stack deserialize(const std::string& serial) {
                std::istringstream stream(serial);
                Stack stack;
                const auto size = sizeof(size_t);
                size_t size_vector, size_map;
                stream.read(reinterpret_cast<char*>(&size_vector), size);
                stream.read(reinterpret_cast<char*>(&size_map), size);
                for (int i = 0; i < size_vector; i++) {
                    size_t size_value_type, size_value;
                    stream.read(reinterpret_cast<char*>(&size_value_type), size);
                    stream.read(reinterpret_cast<char*>(&size_value), size);
                    std::vector<char> value_type(size_value_type);
                    std::vector<char> value(size_value);
                    stream.read(value_type.data(), size_value_type);
                    stream.read(value.data(), size_value);
                    stack.push(Value::deserialize({std::string(value_type.data(), size_value_type), std::string(value.data(), size_value)}));
                }
                for (int i = 0; i < size_map; i++) {
                    size_t size_index, size_value_type, size_value;
                    stream.read(reinterpret_cast<char*>(&size_index), size);
                    stream.read(reinterpret_cast<char*>(&size_value_type), size);
                    stream.read(reinterpret_cast<char*>(&size_value), size);
                    std::vector<char> index(size_index);
                    std::vector<char> value_type(size_value_type);
                    std::vector<char> value(size_value);
                    stream.read(index.data(), size_index);
                    stream.read(value_type.data(), size_value_type);
                    stream.read(value.data(), size_value);
                    stack.push(std::string(index.data(), size_index), Value::deserialize({std::string(value_type.data(), size_value_type), std::string(value.data(), size_value)}));
                }
                return stack;
            }
    };

}