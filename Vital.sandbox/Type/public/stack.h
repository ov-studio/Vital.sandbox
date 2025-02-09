/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: stack.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Stack Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


/////////////////////////
// Vital: Type: Stack //
/////////////////////////

namespace Vital::Type {
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

                    // Utils
                    std::pair<std::string, std::string> serialize() {
                        if (isString()) return {"std::string", get<std::string>()};
                        if (isNumber()) {
                            std::ostringstream oss;
                            std::string type;
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
                        throw std::runtime_error("invalid-result");
                    }
                    static Value deserialize(const std::pair<std::string, std::string>& serial) {
                        const auto& rwType = serial.first;
                        const auto& rwValue = serial.second;
                        if (rwType == "std::string") {
                            return Value(rwValue);
                        } 
                        if (rwType == "int") {
                            return Value(std::stoi(rwValue));
                        } 
                        if (rwType == "float") {
                            return Value(std::stof(rwValue));
                        } 
                        if (rwType == "double") {
                            return Value(std::stod(rwValue));
                        } 
                        if (rwType == "long") {
                            return Value(std::stol(rwValue));
                        } 
                        if (rwType == "long long") {
                            return Value(std::stoll(rwValue));
                        } 
                        if (rwType == "long double") {
                            return Value(std::stold(rwValue));
                        } 
                        if (rwType == "unsigned int") {
                            return Value(static_cast<unsigned int>(std::stoul(rwValue)));
                        } 
                        if (rwType == "unsigned long") {
                            return Value(std::stoul(rwValue));
                        } 
                        if (rwType == "unsigned long long") {
                            return Value(std::stoull(rwValue));
                        } 
                        throw std::runtime_error("invalid-result");
                    }
            };

            // Checkers //
            bool isNil(int index = 0);
            bool isNil(const std::string& index);
            bool isString(int index = 0);
            bool isString(const std::string& index);
            bool isNumber(int index = 0);
            bool isNumber(const std::string& index);

            // Getters //
            template <typename T>
            T get(int index = 0) {
                if (isNil(index)) throw ErrorCode["invalid-result"];
                return vector.at(index).get<T>();
            }
            template <typename T>
            T get(const std::string& index) {
                if (isNil(index)) throw ErrorCode["invalid-result"];
                return map.at(index).get<T>();
            }

            // Pushers //
            template <typename T>
            void push(T value) { vector.push_back(Value(value)); }
            template <typename T>
            void push(const std::string& index, T value) { map.emplace(index, Value(value)); }

            // Poppers
            void pop(int index = 0);
            void pop(const std::string& index);

            // Utils //
            std::string serialize();
            static Stack deserialize(const std::string& serial);
    };

}