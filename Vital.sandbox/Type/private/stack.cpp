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

namespace Vital::Type::Stack {
    // Instantiators //
    Value::Value(const std::string& argument) { rwString = argument, rwType = typeid(argument).name(); }
    Value::Value(int argument) { rwInt = argument, rwType = typeid(argument).name(); }
    Value::Value(float argument) { rwFloat = argument, rwType = typeid(argument).name(); }
    Value::Value(double argument) { rwDouble = argument, rwType = typeid(argument).name(); }
    Value::Value(long argument) { rwLong = argument, rwType = typeid(argument).name(); }
    Value::Value(long long argument) { rwLongLong = argument, rwType = typeid(argument).name(); }
    Value::Value(long double argument) { rwLongDouble = argument, rwType = typeid(argument).name(); }
    Value::Value(unsigned argument) { rwUnsigned = argument, rwType = typeid(argument).name(); }
    Value::Value(unsigned long argument) { rwUnsignedLong = argument, rwType = typeid(argument).name(); }
    Value::Value(unsigned long long argument) { rwUnsignedLongLong = argument, rwType = typeid(argument).name(); }

    // Checkers //
    bool Value::isString() { return (rwType == typeid(const std::string&).name()); }
    bool Value::isNumber() {
        return ((
            (rwType == typeid(int).name()) ||
            (rwType == typeid(float).name()) ||
            (rwType == typeid(double).name()) ||
            (rwType == typeid(long).name()) ||
            (rwType == typeid(long long).name()) ||
            (rwType == typeid(long double).name()) ||
            (rwType == typeid(unsigned).name()) ||
            (rwType == typeid(unsigned long).name()) ||
            (rwType == typeid(unsigned long long).name())
        ) && true) || false;
    }

    // Getters //
    std::string Value::getString() {
        if (!isString()) throw ErrorCode["invalid-result"];
        return rwString;
    }
    int Value::getInt() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (rwType == typeid(int).name()) return static_cast<int>(rwInt);
        else if (rwType == typeid(float).name()) return static_cast<int>(rwFloat);
        else if (rwType == typeid(double).name()) return static_cast<int>(rwDouble);
        else if (rwType == typeid(long).name()) return static_cast<int>(rwLong);
        else if (rwType == typeid(long long).name()) return static_cast<int>(rwLongLong);
        else if (rwType == typeid(long double).name()) return static_cast<int>(rwLongDouble);
        else if (rwType == typeid(unsigned).name()) return static_cast<int>(rwUnsigned);
        else if (rwType == typeid(unsigned long).name()) return static_cast<int>(rwUnsignedLong);
        else if (rwType == typeid(unsigned long long).name()) return static_cast<int>(rwUnsignedLongLong);
    }
    float Value::getFloat() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (rwType == typeid(int).name()) return static_cast<float>(rwInt);
        else if (rwType == typeid(float).name()) return static_cast<float>(rwFloat);
        else if (rwType == typeid(double).name()) return static_cast<float>(rwDouble);
        else if (rwType == typeid(long).name()) return static_cast<float>(rwLong);
        else if (rwType == typeid(long long).name()) return static_cast<float>(rwLongLong);
        else if (rwType == typeid(long double).name()) return static_cast<float>(rwLongDouble);
        else if (rwType == typeid(unsigned).name()) return static_cast<float>(rwUnsigned);
        else if (rwType == typeid(unsigned long).name()) return static_cast<float>(rwUnsignedLong);
        else if (rwType == typeid(unsigned long long).name()) return static_cast<float>(rwUnsignedLongLong);
    }
    double Value::getDouble() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (rwType == typeid(int).name()) return static_cast<double>(rwInt);
        else if (rwType == typeid(float).name()) return static_cast<double>(rwFloat);
        else if (rwType == typeid(double).name()) return static_cast<double>(rwDouble);
        else if (rwType == typeid(long).name()) return static_cast<double>(rwLong);
        else if (rwType == typeid(long long).name()) return static_cast<double>(rwLongLong);
        else if (rwType == typeid(long double).name()) return static_cast<double>(rwLongDouble);
        else if (rwType == typeid(unsigned).name()) return static_cast<double>(rwUnsigned);
        else if (rwType == typeid(unsigned long).name()) return static_cast<double>(rwUnsignedLong);
        else if (rwType == typeid(unsigned long long).name()) return static_cast<double>(rwUnsignedLongLong);
    }
    long Value::getLong() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (rwType == typeid(int).name()) return static_cast<long>(rwInt);
        else if (rwType == typeid(float).name()) return static_cast<long>(rwFloat);
        else if (rwType == typeid(double).name()) return static_cast<long>(rwDouble);
        else if (rwType == typeid(long).name()) return static_cast<long>(rwLong);
        else if (rwType == typeid(long long).name()) return static_cast<long>(rwLongLong);
        else if (rwType == typeid(long double).name()) return static_cast<long>(rwLongDouble);
        else if (rwType == typeid(unsigned).name()) return static_cast<long>(rwUnsigned);
        else if (rwType == typeid(unsigned long).name()) return static_cast<long>(rwUnsignedLong);
        else if (rwType == typeid(unsigned long long).name()) return static_cast<long>(rwUnsignedLongLong);
    }
    long long Value::getLongLong() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (rwType == typeid(int).name()) return static_cast<long long>(rwInt);
        else if (rwType == typeid(float).name()) return static_cast<long long>(rwFloat);
        else if (rwType == typeid(double).name()) return static_cast<long long>(rwDouble);
        else if (rwType == typeid(long).name()) return static_cast<long long>(rwLong);
        else if (rwType == typeid(long long).name()) return static_cast<long long>(rwLongLong);
        else if (rwType == typeid(long double).name()) return static_cast<long long>(rwLongDouble);
        else if (rwType == typeid(unsigned).name()) return static_cast<long long>(rwUnsigned);
        else if (rwType == typeid(unsigned long).name()) return static_cast<long long>(rwUnsignedLong);
        else if (rwType == typeid(unsigned long long).name()) return static_cast<long long>(rwUnsignedLongLong);
    }
    long double Value::getLongDouble() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (rwType == typeid(int).name()) return static_cast<long double>(rwInt);
        else if (rwType == typeid(float).name()) return static_cast<long double>(rwFloat);
        else if (rwType == typeid(double).name()) return static_cast<long double>(rwDouble);
        else if (rwType == typeid(long).name()) return static_cast<long double>(rwLong);
        else if (rwType == typeid(long long).name()) return static_cast<long double>(rwLongLong);
        else if (rwType == typeid(long double).name()) return static_cast<long double>(rwLongDouble);
        else if (rwType == typeid(unsigned).name()) return static_cast<long double>(rwUnsigned);
        else if (rwType == typeid(unsigned long).name()) return static_cast<long double>(rwUnsignedLong);
        else if (rwType == typeid(unsigned long long).name()) return static_cast<long double>(rwUnsignedLongLong);
    }
    unsigned Value::getUnsigned() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (rwType == typeid(int).name()) return static_cast<unsigned>(rwInt);
        else if (rwType == typeid(float).name()) return static_cast<unsigned>(rwFloat);
        else if (rwType == typeid(double).name()) return static_cast<unsigned>(rwDouble);
        else if (rwType == typeid(long).name()) return static_cast<unsigned>(rwLong);
        else if (rwType == typeid(long long).name()) return static_cast<unsigned>(rwLongLong);
        else if (rwType == typeid(long double).name()) return static_cast<unsigned>(rwLongDouble);
        else if (rwType == typeid(unsigned).name()) return static_cast<unsigned>(rwUnsigned);
        else if (rwType == typeid(unsigned long).name()) return static_cast<unsigned>(rwUnsignedLong);
        else if (rwType == typeid(unsigned long long).name()) return static_cast<unsigned>(rwUnsignedLongLong);
    }
    unsigned long Value::getUnsignedLong() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (rwType == typeid(int).name()) return static_cast<unsigned long>(rwInt);
        else if (rwType == typeid(float).name()) return static_cast<unsigned long>(rwFloat);
        else if (rwType == typeid(double).name()) return static_cast<unsigned long>(rwDouble);
        else if (rwType == typeid(long).name()) return static_cast<unsigned long>(rwLong);
        else if (rwType == typeid(long long).name()) return static_cast<unsigned long>(rwLongLong);
        else if (rwType == typeid(long double).name()) return static_cast<unsigned long>(rwLongDouble);
        else if (rwType == typeid(unsigned).name()) return static_cast<unsigned long>(rwUnsigned);
        else if (rwType == typeid(unsigned long).name()) return static_cast<unsigned long>(rwUnsignedLong);
        else if (rwType == typeid(unsigned long long).name()) return static_cast<unsigned long>(rwUnsignedLongLong);
    }
    unsigned long long Value::getUnsignedLongLong() {
        if (!isNumber()) throw ErrorCode["invalid-result"];
        if (rwType == typeid(int).name()) return static_cast<unsigned long long>(rwInt);
        else if (rwType == typeid(float).name()) return static_cast<unsigned long long>(rwFloat);
        else if (rwType == typeid(double).name()) return static_cast<unsigned long long>(rwDouble);
        else if (rwType == typeid(long).name()) return static_cast<unsigned long long>(rwLong);
        else if (rwType == typeid(long long).name()) return static_cast<unsigned long long>(rwLongLong);
        else if (rwType == typeid(long double).name()) return static_cast<unsigned long long>(rwLongDouble);
        else if (rwType == typeid(unsigned).name()) return static_cast<unsigned long long>(rwUnsigned);
        else if (rwType == typeid(unsigned long).name()) return static_cast<unsigned long long>(rwUnsignedLong);
        else if (rwType == typeid(unsigned long long).name()) return static_cast<unsigned long long>(rwUnsignedLongLong);
    }
}

namespace Vital::Type::Stack {
    // Checkers //
    bool Instance::isNil(int index) { return ((index < 0) || rwVector.empty() || (index >= rwVector.size()) && true) || false; };
    bool Instance::isNil(const std::string& index) { return rwMap.find(index) == rwMap.end(); };
    bool Instance::isString(int index) { return (!isNil(index) && rwVector.at(index).isString() && true) || false; }
    bool Instance::isString(const std::string& index) { return (!isNil(index) && rwMap.at(index).isString() && true) || false; }
    bool Instance::isNumber(int index) { return (!isNil(index) && rwVector.at(index).isNumber() && true) || false; }
    bool Instance::isNumber(const std::string& index) { return (!isNil(index) && rwMap.at(index).isNumber() && true) || false; }

    // Getters //
    std::string Instance::getString(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getString();
    }
    std::string Instance::getString(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getString();
    }
    int Instance::getInt(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getInt();
    }
    int Instance::getInt(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getInt();
    }
    float Instance::getFloat(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getFloat();
    }
    float Instance::getFloat(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getFloat();
    }
    double Instance::getDouble(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getDouble();
    }
    double Instance::getDouble(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getDouble();
    }
    long Instance::getLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getLong();
    }
    long Instance::getLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getLong();
    }
    long long Instance::getLongLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getLongLong();
    }
    long long Instance::getLongLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getLongLong();
    }
    long double Instance::getLongDouble(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getLongDouble();
    }
    long double Instance::getLongDouble(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getLongDouble();
    }
    unsigned Instance::getUnsigned(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getUnsigned();
    }
    unsigned Instance::getUnsigned(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getUnsigned();
    }
    unsigned long Instance::getUnsignedLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getUnsignedLong();
    }
    unsigned long Instance::getUnsignedLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getUnsignedLong();
    }
    unsigned long long Instance::getUnsignedLongLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwVector.at(index).getUnsignedLongLong();
    }
    unsigned long long Instance::getUnsignedLongLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return rwMap.at(index).getUnsignedLongLong();
    }

    // Pushers //
    void Instance::push(const std::string& value) { rwVector.push_back(Value(value)); }
    void Instance::push(const std::string& index, const std::string& value) { rwMap.emplace(index, Value(value)); }
    void Instance::push(int value) { rwVector.push_back(Value(value)); }
    void Instance::push(const std::string& index, int value) { rwMap.emplace(index, Value(value)); }
    void Instance::push(float value) { rwVector.push_back(Value(value)); }
    void Instance::push(const std::string& index, float value) { rwMap.emplace(index, Value(value)); }
    void Instance::push(double value) { rwVector.push_back(Value(value)); }
    void Instance::push(const std::string& index, double value) { rwMap.emplace(index, Value(value)); }
    void Instance::push(long value) { rwVector.push_back(Value(value)); }
    void Instance::push(const std::string& index, long value) { rwMap.emplace(index, Value(value)); }
    void Instance::push(long long value) { rwVector.push_back(Value(value)); }
    void Instance::push(const std::string& index, long long value) { rwMap.emplace(index, Value(value)); }
    void Instance::push(long double value) { rwVector.push_back(Value(value)); }
    void Instance::push(const std::string& index, long double value) { rwMap.emplace(index, Value(value)); }
    void Instance::push(unsigned value) { rwVector.push_back(Value(value)); }
    void Instance::push(const std::string& index, unsigned value) { rwMap.emplace(index, Value(value)); }
    void Instance::push(unsigned long value) { rwVector.push_back(Value(value)); }
    void Instance::push(const std::string& index, unsigned long value) { rwMap.emplace(index, Value(value)); }
    void Instance::push(unsigned long long value) { rwVector.push_back(Value(value)); }
    void Instance::push(const std::string& index, unsigned long long value) { rwMap.emplace(index, Value(value)); }

    // Poppers //
    void Instance::pop(int index) { rwVector.erase(rwVector.begin() + index - 1); }
    void Instance::pop(const std::string& index) { rwMap.erase(index); }
}