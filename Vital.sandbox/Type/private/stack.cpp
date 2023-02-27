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
    bool Ordered::isNil(int index) { return values.find(index) == values.end(); };
    bool Ordered::isString(int index) { return (!isNil(index) && values.at(index).isString() && true) || false; }
    bool Ordered::isNumber(int index) { return (!isNil(index) && values.at(index).isNumber() && true) || false; }

    // Getters //
    std::string Ordered::getString(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getString();
    }
    int Ordered::getInt(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getInt();
    }
    float Ordered::getFloat(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getFloat();
    }
    double Ordered::getDouble(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getDouble();
    }
    long Ordered::getLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getLong();
    }
    long long Ordered::getLongLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getLongLong();
    }
    long double Ordered::getLongDouble(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getLongDouble();
    }
    unsigned Ordered::getUnsigned(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getUnsigned();
    }
    unsigned long Ordered::getUnsignedLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getUnsignedLong();
    }
    unsigned long long Ordered::getUnsignedLongLong(int index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getUnsignedLongLong();
    }

    // Pushers //
    void Ordered::push(int index, const std::string& value) { values.emplace(index, Value(value)); }
    void Ordered::push(int index, int value) { values.emplace(index, Value(value)); }
    void Ordered::push(int index, float value) { values.emplace(index, Value(value)); }
    void Ordered::push(int index, double value) { values.emplace(index, Value(value)); }
    void Ordered::push(int index, long value) { values.emplace(index, Value(value)); }
    void Ordered::push(int index, long long value) { values.emplace(index, Value(value)); }
    void Ordered::push(int index, long double value) { values.emplace(index, Value(value)); }
    void Ordered::push(int index, unsigned value) { values.emplace(index, Value(value)); }
    void Ordered::push(int index, unsigned long value) { values.emplace(index, Value(value)); }
    void Ordered::push(int index, unsigned long long value) { values.emplace(index, Value(value)); }
}

namespace Vital::Type::Stack {
    // Checkers //
    bool Unordered::isNil(const std::string& index) { return values.find(index) == values.end(); };
    bool Unordered::isString(const std::string& index) { return (!isNil(index) && values.at(index).isString() && true) || false; }
    bool Unordered::isNumber(const std::string& index) { return (!isNil(index) && values.at(index).isNumber() && true) || false; }

    // Getters //
    std::string Unordered::getString(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getString();
    }
    int Unordered::getInt(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getInt();
    }
    float Unordered::getFloat(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getFloat();
    }
    double Unordered::getDouble(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getDouble();
    }
    long Unordered::getLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getLong();
    }
    long long Unordered::getLongLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getLongLong();
    }
    long double Unordered::getLongDouble(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getLongDouble();
    }
    unsigned Unordered::getUnsigned(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getUnsigned();
    }
    unsigned long Unordered::getUnsignedLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getUnsignedLong();
    }
    unsigned long long Unordered::getUnsignedLongLong(const std::string& index) {
        if (isNil(index)) throw ErrorCode["invalid-result"];
        return values.at(index).getUnsignedLongLong();
    }

    // Pushers //
    void Unordered::push(const std::string& index, const std::string& value) { values.emplace(index, Value(value)); }
    void Unordered::push(const std::string& index, int value) { values.emplace(index, Value(value)); }
    void Unordered::push(const std::string& index, float value) { values.emplace(index, Value(value)); }
    void Unordered::push(const std::string& index, double value) { values.emplace(index, Value(value)); }
    void Unordered::push(const std::string& index, long value) { values.emplace(index, Value(value)); }
    void Unordered::push(const std::string& index, long long value) { values.emplace(index, Value(value)); }
    void Unordered::push(const std::string& index, long double value) { values.emplace(index, Value(value)); }
    void Unordered::push(const std::string& index, unsigned value) { values.emplace(index, Value(value)); }
    void Unordered::push(const std::string& index, unsigned long value) { values.emplace(index, Value(value)); }
    void Unordered::push(const std::string& index, unsigned long long value) { values.emplace(index, Value(value)); }
}