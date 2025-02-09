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
    bool Stack::isNil(int index) { return ((index < 0) || vector.empty() || (index >= vector.size()) && true) || false; };
    bool Stack::isNil(const std::string& index) { return map.find(index) == map.end(); };
    bool Stack::isString(int index) { return (!isNil(index) && vector.at(index).isString() && true) || false; }
    bool Stack::isString(const std::string& index) { return (!isNil(index) && map.at(index).isString() && true) || false; }
    bool Stack::isNumber(int index) { return (!isNil(index) && vector.at(index).isNumber() && true) || false; }
    bool Stack::isNumber(const std::string& index) { return (!isNil(index) && map.at(index).isNumber() && true) || false; }

    // Poppers //
    void Stack::pop(int index) { vector.erase(vector.begin() + index - 1); }
    void Stack::pop(const std::string& index) { map.erase(index); }

    // Utils //
    std::string Stack::serialize() {
        std::ostringstream stream;
        const auto typeSize = sizeof(size_t);
        const size_t rwVectorSize = vector.size(), rwMapSize = map.size();
        stream.write(reinterpret_cast<const char*>(&rwVectorSize), typeSize);
        stream.write(reinterpret_cast<const char*>(&rwMapSize), typeSize);
        for (int i = 0; i < rwVectorSize; i++) {
            auto value = vector.at(i).serialize();
            const size_t valueTypeSize = value.first.size(), valueSize = value.second.size();
            stream.write(reinterpret_cast<const char*>(&valueTypeSize), typeSize);
            stream.write(reinterpret_cast<const char*>(&valueSize), typeSize);
            stream.write(value.first.c_str(), valueTypeSize);
            stream.write(value.second.c_str(), valueSize);
        }
        for (auto& i : map) {
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