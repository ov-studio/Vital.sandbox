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
    std::string Stack::getString(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getString();
    }
    std::string Stack::getString(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getString();
    }
    int Stack::getInt(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getInt();
    }
    int Stack::getInt(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getInt();
    }
    float Stack::getFloat(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getFloat();
    }
    float Stack::getFloat(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getFloat();
    }
    double Stack::getDouble(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getDouble();
    }
    double Stack::getDouble(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getDouble();
    }
    long Stack::getLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getLong();
    }
    long Stack::getLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getLong();
    }
    long long Stack::getLongLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getLongLong();
    }
    long long Stack::getLongLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getLongLong();
    }
    long double Stack::getLongDouble(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getLongDouble();
    }
    long double Stack::getLongDouble(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getLongDouble();
    }
    unsigned Stack::getUnsigned(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getUnsigned();
    }
    unsigned Stack::getUnsigned(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getUnsigned();
    }
    unsigned long Stack::getUnsignedLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getUnsignedLong();
    }
    unsigned long Stack::getUnsignedLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getUnsignedLong();
    }
    unsigned long long Stack::getUnsignedLongLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getUnsignedLongLong();
    }
    unsigned long long Stack::getUnsignedLongLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getUnsignedLongLong();
    }

    // Pushers //
    void Stack::push(const std::string& value) { pushValue(Value(value)); }
    void Stack::push(const std::string& index, const std::string& value) { pushValue(index, Value(value)); }
    void Stack::push(int value) { pushValue(Value(value)); }
    void Stack::push(const std::string& index, int value) { pushValue(index, Value(value)); }
    void Stack::push(float value) { pushValue(Value(value)); }
    void Stack::push(const std::string& index, float value) { pushValue(index, Value(value)); }
    void Stack::push(double value) { pushValue(Value(value)); }
    void Stack::push(const std::string& index, double value) { pushValue(index, Value(value)); }
    void Stack::push(long value) { pushValue(Value(value)); }
    void Stack::push(const std::string& index, long value) { pushValue(index, Value(value)); }
    void Stack::push(long long value) { pushValue(Value(value)); }
    void Stack::push(const std::string& index, long long value) { pushValue(index, Value(value)); }
    void Stack::push(long double value) { pushValue(Value(value)); }
    void Stack::push(const std::string& index, long double value) { pushValue(index, Value(value)); }
    void Stack::push(unsigned value) { pushValue(Value(value)); }
    void Stack::push(const std::string& index, unsigned value) { pushValue(index, Value(value)); }
    void Stack::push(unsigned long value) { pushValue(Value(value)); }
    void Stack::push(const std::string& index, unsigned long value) { pushValue(index, Value(value)); }
    void Stack::push(unsigned long long value) { pushValue(Value(value)); }
    void Stack::push(const std::string& index, unsigned long long value) { pushValue(index, Value(value)); }

    // Poppers //
    void Stack::pop(int index) { rwVector.erase(rwVector.begin() + index - 1); }
    void Stack::pop(const std::string& index) { rwMap.erase(index); }

    // Utils //
    void Stack::pushValue(Value value) { rwVector.push_back(value); }
    void Stack::pushValue(const std::string& index, Value value) { rwMap.emplace(index, value); }
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
            stack.pushValue(Value::deserialize({std::string(valueType, valueTypeSize), std::string(value, valueSize)}));
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
            stack.pushValue(std::string(index, indexSize), Value::deserialize({std::string(valueType, valueTypeSize), std::string(value, valueSize)}));
            delete[] valueType;
            delete[] value;
        }
        return stack;
    }
}

namespace Vital::Type {
    // Instantiators //
    Stack::Value::Value(const std::string& argument) { rwString = argument, rwType = typeid(argument).name(); }
    Stack::Value::Value(int argument) { rwInt = argument, rwType = typeid(argument).name(); }
    Stack::Value::Value(float argument) { rwFloat = argument, rwType = typeid(argument).name(); }
    Stack::Value::Value(double argument) { rwDouble = argument, rwType = typeid(argument).name(); }
    Stack::Value::Value(long argument) { rwLong = argument, rwType = typeid(argument).name(); }
    Stack::Value::Value(long long argument) { rwLongLong = argument, rwType = typeid(argument).name(); }
    Stack::Value::Value(long double argument) { rwLongDouble = argument, rwType = typeid(argument).name(); }
    Stack::Value::Value(unsigned argument) { rwUnsigned = argument, rwType = typeid(argument).name(); }
    Stack::Value::Value(unsigned long argument) { rwUnsignedLong = argument, rwType = typeid(argument).name(); }
    Stack::Value::Value(unsigned long long argument) { rwUnsignedLongLong = argument, rwType = typeid(argument).name(); }

    // Checkers //
    bool Stack::Value::isString() { return isTypeString(rwType); }
    bool Stack::Value::isNumber() { return isTypeNumber(rwType); }

    // Getters //
    std::string Stack::Value::getString() {
        if (!isString()) throw ErrorCode["invalid-result"];
        return rwString;
    }
    int Stack::Value::getInt() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (!strcmp(rwType, typeid(int).name())) return static_cast<int>(rwInt);
        else if (!strcmp(rwType, typeid(float).name())) return static_cast<int>(rwFloat);
        else if (!strcmp(rwType, typeid(double).name())) return static_cast<int>(rwDouble);
        else if (!strcmp(rwType, typeid(long).name())) return static_cast<int>(rwLong);
        else if (!strcmp(rwType, typeid(long long).name())) return static_cast<int>(rwLongLong);
        else if (!strcmp(rwType, typeid(long double).name())) return static_cast<int>(rwLongDouble);
        else if (!strcmp(rwType, typeid(unsigned).name())) return static_cast<int>(rwUnsigned);
        else if (!strcmp(rwType, typeid(unsigned long).name())) return static_cast<int>(rwUnsignedLong);
        else if (!strcmp(rwType, typeid(unsigned long long).name())) return static_cast<int>(rwUnsignedLongLong);
        throw ErrorCode["invalid-result"];
    }
    float Stack::Value::getFloat() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (!strcmp(rwType, typeid(int).name())) return static_cast<float>(rwInt);
        else if (!strcmp(rwType, typeid(float).name())) return static_cast<float>(rwFloat);
        else if (!strcmp(rwType, typeid(double).name())) return static_cast<float>(rwDouble);
        else if (!strcmp(rwType, typeid(long).name())) return static_cast<float>(rwLong);
        else if (!strcmp(rwType, typeid(long long).name())) return static_cast<float>(rwLongLong);
        else if (!strcmp(rwType, typeid(long double).name())) return static_cast<float>(rwLongDouble);
        else if (!strcmp(rwType, typeid(unsigned).name())) return static_cast<float>(rwUnsigned);
        else if (!strcmp(rwType, typeid(unsigned long).name())) return static_cast<float>(rwUnsignedLong);
        else if (!strcmp(rwType, typeid(unsigned long long).name())) return static_cast<float>(rwUnsignedLongLong);
        throw ErrorCode["invalid-result"];
    }
    double Stack::Value::getDouble() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (!strcmp(rwType, typeid(int).name())) return static_cast<double>(rwInt);
        else if (!strcmp(rwType, typeid(float).name())) return static_cast<double>(rwFloat);
        else if (!strcmp(rwType, typeid(double).name())) return static_cast<double>(rwDouble);
        else if (!strcmp(rwType, typeid(long).name())) return static_cast<double>(rwLong);
        else if (!strcmp(rwType, typeid(long long).name())) return static_cast<double>(rwLongLong);
        else if (!strcmp(rwType, typeid(long double).name())) return static_cast<double>(rwLongDouble);
        else if (!strcmp(rwType, typeid(unsigned).name())) return static_cast<double>(rwUnsigned);
        else if (!strcmp(rwType, typeid(unsigned long).name())) return static_cast<double>(rwUnsignedLong);
        else if (!strcmp(rwType, typeid(unsigned long long).name())) return static_cast<double>(rwUnsignedLongLong);
        throw ErrorCode["invalid-result"];
    }
    long Stack::Value::getLong() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (!strcmp(rwType, typeid(int).name())) return static_cast<long>(rwInt);
        else if (!strcmp(rwType, typeid(float).name())) return static_cast<long>(rwFloat);
        else if (!strcmp(rwType, typeid(double).name())) return static_cast<long>(rwDouble);
        else if (!strcmp(rwType, typeid(long).name())) return static_cast<long>(rwLong);
        else if (!strcmp(rwType, typeid(long long).name())) return static_cast<long>(rwLongLong);
        else if (!strcmp(rwType, typeid(long double).name())) return static_cast<long>(rwLongDouble);
        else if (!strcmp(rwType, typeid(unsigned).name())) return static_cast<long>(rwUnsigned);
        else if (!strcmp(rwType, typeid(unsigned long).name())) return static_cast<long>(rwUnsignedLong);
        else if (!strcmp(rwType, typeid(unsigned long long).name())) return static_cast<long>(rwUnsignedLongLong);
        throw ErrorCode["invalid-result"];
    }
    long long Stack::Value::getLongLong() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (!strcmp(rwType, typeid(int).name())) return static_cast<long long>(rwInt);
        else if (!strcmp(rwType, typeid(float).name())) return static_cast<long long>(rwFloat);
        else if (!strcmp(rwType, typeid(double).name())) return static_cast<long long>(rwDouble);
        else if (!strcmp(rwType, typeid(long).name())) return static_cast<long long>(rwLong);
        else if (!strcmp(rwType, typeid(long long).name())) return static_cast<long long>(rwLongLong);
        else if (!strcmp(rwType, typeid(long double).name())) return static_cast<long long>(rwLongDouble);
        else if (!strcmp(rwType, typeid(unsigned).name())) return static_cast<long long>(rwUnsigned);
        else if (!strcmp(rwType, typeid(unsigned long).name())) return static_cast<long long>(rwUnsignedLong);
        else if (!strcmp(rwType, typeid(unsigned long long).name())) return static_cast<long long>(rwUnsignedLongLong);
        throw ErrorCode["invalid-result"];
    }
    long double Stack::Value::getLongDouble() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (!strcmp(rwType, typeid(int).name())) return static_cast<long double>(rwInt);
        else if (!strcmp(rwType, typeid(float).name())) return static_cast<long double>(rwFloat);
        else if (!strcmp(rwType, typeid(double).name())) return static_cast<long double>(rwDouble);
        else if (!strcmp(rwType, typeid(long).name())) return static_cast<long double>(rwLong);
        else if (!strcmp(rwType, typeid(long long).name())) return static_cast<long double>(rwLongLong);
        else if (!strcmp(rwType, typeid(long double).name())) return static_cast<long double>(rwLongDouble);
        else if (!strcmp(rwType, typeid(unsigned).name())) return static_cast<long double>(rwUnsigned);
        else if (!strcmp(rwType, typeid(unsigned long).name())) return static_cast<long double>(rwUnsignedLong);
        else if (!strcmp(rwType, typeid(unsigned long long).name())) return static_cast<long double>(rwUnsignedLongLong);
        throw ErrorCode["invalid-result"];
    }
    unsigned Stack::Value::getUnsigned() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (!strcmp(rwType, typeid(int).name())) return static_cast<unsigned>(rwInt);
        else if (!strcmp(rwType, typeid(float).name())) return static_cast<unsigned>(rwFloat);
        else if (!strcmp(rwType, typeid(double).name())) return static_cast<unsigned>(rwDouble);
        else if (!strcmp(rwType, typeid(long).name())) return static_cast<unsigned>(rwLong);
        else if (!strcmp(rwType, typeid(long long).name())) return static_cast<unsigned>(rwLongLong);
        else if (!strcmp(rwType, typeid(long double).name())) return static_cast<unsigned>(rwLongDouble);
        else if (!strcmp(rwType, typeid(unsigned).name())) return static_cast<unsigned>(rwUnsigned);
        else if (!strcmp(rwType, typeid(unsigned long).name())) return static_cast<unsigned>(rwUnsignedLong);
        else if (!strcmp(rwType, typeid(unsigned long long).name())) return static_cast<unsigned>(rwUnsignedLongLong);
        throw ErrorCode["invalid-result"];
    }
    unsigned long Stack::Value::getUnsignedLong() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (!strcmp(rwType, typeid(int).name())) return static_cast<unsigned long>(rwInt);
        else if (!strcmp(rwType, typeid(float).name())) return static_cast<unsigned long>(rwFloat);
        else if (!strcmp(rwType, typeid(double).name())) return static_cast<unsigned long>(rwDouble);
        else if (!strcmp(rwType, typeid(long).name())) return static_cast<unsigned long>(rwLong);
        else if (!strcmp(rwType, typeid(long long).name())) return static_cast<unsigned long>(rwLongLong);
        else if (!strcmp(rwType, typeid(long double).name())) return static_cast<unsigned long>(rwLongDouble);
        else if (!strcmp(rwType, typeid(unsigned).name())) return static_cast<unsigned long>(rwUnsigned);
        else if (!strcmp(rwType, typeid(unsigned long).name())) return static_cast<unsigned long>(rwUnsignedLong);
        else if (!strcmp(rwType, typeid(unsigned long long).name())) return static_cast<unsigned long>(rwUnsignedLongLong);
        throw ErrorCode["invalid-result"];
    }
    unsigned long long Stack::Value::getUnsignedLongLong() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (!strcmp(rwType, typeid(int).name())) return static_cast<unsigned long long>(rwInt);
        else if (!strcmp(rwType, typeid(float).name())) return static_cast<unsigned long long>(rwFloat);
        else if (!strcmp(rwType, typeid(double).name())) return static_cast<unsigned long long>(rwDouble);
        else if (!strcmp(rwType, typeid(long).name())) return static_cast<unsigned long long>(rwLong);
        else if (!strcmp(rwType, typeid(long long).name())) return static_cast<unsigned long long>(rwLongLong);
        else if (!strcmp(rwType, typeid(long double).name())) return static_cast<unsigned long long>(rwLongDouble);
        else if (!strcmp(rwType, typeid(unsigned).name())) return static_cast<unsigned long long>(rwUnsigned);
        else if (!strcmp(rwType, typeid(unsigned long).name())) return static_cast<unsigned long long>(rwUnsignedLong);
        else if (!strcmp(rwType, typeid(unsigned long long).name())) return static_cast<unsigned long long>(rwUnsignedLongLong);
        throw ErrorCode["invalid-result"];
    }

    // Utils //
    bool Stack::Value::isTypeString(const char* rwType) { return !strcmp(rwType, typeid(const std::string&).name()); }
    bool Stack::Value::isTypeNumber(const char* rwType) {
        return ((
            (!strcmp(rwType, typeid(int).name())) ||
            (!strcmp(rwType, typeid(float).name())) ||
            (!strcmp(rwType, typeid(double).name())) ||
            (!strcmp(rwType, typeid(long).name())) ||
            (!strcmp(rwType, typeid(long long).name())) ||
            (!strcmp(rwType, typeid(long double).name())) ||
            (!strcmp(rwType, typeid(unsigned).name())) ||
            (!strcmp(rwType, typeid(unsigned long).name())) ||
            (!strcmp(rwType, typeid(unsigned long long).name()))
            ) && true) || false;
    }

    std::pair<std::string, std::string> Stack::Value::serialize() {
        if (isString()) return { rwType, rwString };
        else if (isNumber()) {
            std::string rwValue;
            if (!strcmp(rwType, typeid(float).name())) rwValue = std::to_string(rwFloat);
            else if (!strcmp(rwType, typeid(double).name())) rwValue = std::to_string(rwDouble);
            else if (!strcmp(rwType, typeid(long).name())) rwValue = std::to_string(rwLong);
            else if (!strcmp(rwType, typeid(long long).name())) rwValue = std::to_string(rwLongLong);
            else if (!strcmp(rwType, typeid(long double).name())) rwValue = std::to_string(rwLongDouble);
            else if (!strcmp(rwType, typeid(unsigned).name())) rwValue = std::to_string(rwUnsigned);
            else if (!strcmp(rwType, typeid(unsigned long).name())) rwValue = std::to_string(rwUnsignedLong);
            else if (!strcmp(rwType, typeid(unsigned long long).name())) rwValue = std::to_string(rwUnsignedLongLong);
            return { rwType, rwValue };
        }
        throw ErrorCode["invalid-result"];
    }
    Stack::Value Stack::Value::deserialize(std::pair<std::string, std::string> serial) {
        auto rwType = serial.first.c_str();
        auto rwValue = serial.second;
        if (isTypeString(rwType)) return Value(rwValue);
        else if (isTypeNumber(rwType)) {
            if (!strcmp(rwType, typeid(int).name())) return Value(std::stoi(rwValue));
            else if (!strcmp(rwType, typeid(float).name())) return Value(std::stof(rwValue));
            else if (!strcmp(rwType, typeid(double).name())) return Value(std::stod(rwValue));
            else if (!strcmp(rwType, typeid(long).name())) return Value(std::stol(rwValue));
            else if (!strcmp(rwType, typeid(long long).name())) return Value(std::stoll(rwValue));
            else if (!strcmp(rwType, typeid(long double).name())) return Value(std::stold(rwValue));
            else if (!strcmp(rwType, typeid(unsigned).name())) return Value(static_cast<unsigned>(std::stoul(rwValue)));
            else if (!strcmp(rwType, typeid(unsigned long).name())) return Value(std::stoul(rwValue));
            else if (!strcmp(rwType, typeid(unsigned long long).name())) return Value(std::stoull(rwValue));
        }
        throw ErrorCode["invalid-result"];
    }
}