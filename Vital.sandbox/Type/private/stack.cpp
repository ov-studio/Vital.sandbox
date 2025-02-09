/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: stack.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Stack Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/stack.h>


/////////////////////////
// Vital: Type: Stack //
/////////////////////////

namespace Vital::Type {
    // Checkers //
    bool Stack::isNil(int index) { return ((index < 0) || rwVector.empty() || (index >= rwVector.size()) && true) || false; };
    bool Stack::isNil(const std::string& index) { return rwMap.find(index) == rwMap.end(); };
    bool Stack::isString(int index) { return (!isNil(index) && rwVector.at(index).isString() && true) || false; }
    bool Stack::isString(const std::string& index) { return (!isNil(index) && rwMap.at(index).isString() && true) || false; }
    bool Stack::isNumber(int index) { return (!isNil(index) && rwVector.at(index).isNumber() && true) || false; }
    bool Stack::isNumber(const std::string& index) { return (!isNil(index) && rwMap.at(index).isNumber() && true) || false; }

    // Getters //
    template <typename T>
    T Stack::get(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).get<T>();
    }
    template <typename T>
    T Stack::get(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).get<T>();
    }

    // Pushers //
    template <typename T>
    void Stack::push(T& value) { rwVector.push_back(Value(value)); };
    template <typename T>
    void Stack::push(const std::string& index, T& value) { rwMap.emplace(index, Value(value)); };

    // Poppers //
    void Stack::pop(int index) { rwVector.erase(rwVector.begin() + index - 1); }
    void Stack::pop(const std::string& index) { rwMap.erase(index); }

    // Utils //
    std::string Stack::serialize() {
        std::ostringstream stream;
        const auto typeSize = sizeof(size_t);
        const size_t rwVectorSize = rwVector.size(), rwMapSize = rwMap.size();
        stream.write(reinterpret_cast<const char*>(&rwVectorSize), typeSize);
        stream.write(reinterpret_cast<const char*>(&rwMapSize), typeSize);
        for (int i = 0; i < rwVectorSize; i++) {
            auto value = rwVector.at(i).serialize();
            const size_t valueTypeSize = value.first.size(), valueSize = value.second.size();
            stream.write(reinterpret_cast<const char*>(&valueTypeSize), typeSize);
            stream.write(reinterpret_cast<const char*>(&valueSize), typeSize);
            stream.write(value.first.c_str(), valueTypeSize);
            stream.write(value.second.c_str(), valueSize);
        }
        for (auto& i : rwMap) {
            auto value = i.second.serialize();
            const size_t indexSize = i.first.size(), valueTypeSize = value.first.size(), valueSize = value.second.size();
            stream.write(reinterpret_cast<const char*>(&indexSize), typeSize);
            stream.write(reinterpret_cast<const char*>(&valueTypeSize), typeSize);
            stream.write(reinterpret_cast<const char*>(&valueSize), typeSize);
            stream.write(i.first.c_str(), indexSize);
            stream.write(value.first.c_str(), valueTypeSize);
            stream.write(value.second.c_str(), valueSize);
        }
        return stream.str();
    }
    Stack Stack::deserialize(const std::string& serial) {
        std::istringstream stream(serial);
        Stack stack;
        const auto typeSize = sizeof(size_t);
        size_t rwVectorSize, rwMapSize;
        stream.read(reinterpret_cast<char*>(&rwVectorSize), typeSize);
        stream.read(reinterpret_cast<char*>(&rwMapSize), typeSize);
        for (int i = 0; i < rwVectorSize; i++) {
            size_t valueTypeSize, valueSize;
            stream.read(reinterpret_cast<char*>(&valueTypeSize), typeSize);
            stream.read(reinterpret_cast<char*>(&valueSize), typeSize);
            char* valueType = new char[valueTypeSize];
            char* value = new char[valueSize];
            stream.read(valueType, valueTypeSize);
            stream.read(value, valueSize);
            stack.push(Value::deserialize({std::string(valueType, valueTypeSize), std::string(value, valueSize)}));
            delete[] valueType;
            delete[] value;
        }
        for (int i = 0; i < rwMapSize; i++) {
            size_t indexSize, valueTypeSize, valueSize;
            stream.read(reinterpret_cast<char*>(&indexSize), typeSize);
            stream.read(reinterpret_cast<char*>(&valueTypeSize), typeSize);
            stream.read(reinterpret_cast<char*>(&valueSize), typeSize);
            char* index = new char[indexSize];
            char* valueType = new char[valueTypeSize];
            char* value = new char[valueSize];
            stream.read(index, indexSize);
            stream.read(valueType, valueTypeSize);
            stream.read(value, valueSize);
            stack.push(std::string(index, indexSize), Value::deserialize({std::string(valueType, valueTypeSize), std::string(value, valueSize)}));
            delete[] valueType;
            delete[] value;
        }
        return stack;
    }
}

namespace Vital::Type {
    std::pair<std::string, std::string> Stack::Value::serialize() {
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
    Stack::Value Stack::Value::deserialize(const std::pair<std::string, std::string>& serial) {
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
}