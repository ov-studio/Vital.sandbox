/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: event.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/event.h>


/////////////////////////
// Vital: Type: Event //
/////////////////////////

namespace Vital::Type::Event {
    // Instantiators //
    Argument::Argument(const std::string& argument) { rwString = argument, rwType = typeid(argument).name(); }
    Argument::Argument(int argument) { rwInt = argument, rwType = typeid(argument).name(); }
    Argument::Argument(float argument) { rwFloat = argument, rwType = typeid(argument).name(); }
    Argument::Argument(double argument) { rwDouble = argument, rwType = typeid(argument).name(); }
    Argument::Argument(long argument) { rwLong = argument, rwType = typeid(argument).name(); }
    Argument::Argument(long long argument) { rwLongLong = argument, rwType = typeid(argument).name(); }
    Argument::Argument(long double argument) { rwLongDouble = argument, rwType = typeid(argument).name(); }
    Argument::Argument(unsigned argument) { rwUnsigned = argument, rwType = typeid(argument).name(); }
    Argument::Argument(unsigned long argument) { rwUnsignedLong = argument, rwType = typeid(argument).name(); }
    Argument::Argument(unsigned long long argument) { rwUnsignedLongLong = argument, rwType = typeid(argument).name(); }

    // Checkers //
    bool Argument::isString() { return (rwType == typeid(const std::string&).name()); }
    bool Argument::isNumber() {
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
    std::string Argument::getString() {
        if (!isString()) return 0;
        return rwString;
    }
    int Argument::getInt() {
        if (!isNumber()) return 0;
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
    float Argument::getFloat() {
        if (!isNumber()) return 0;
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
    double Argument::getDouble() {
        if (!isNumber()) return 0;
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
    long Argument::getLong() {
        if (!isNumber()) return 0;
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
    long long Argument::getLongLong() {
        if (!isNumber()) return 0;
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
    long double Argument::getLongDouble() {
        if (!isNumber()) return 0;
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
    unsigned Argument::getUnsigned() {
        if (!isNumber()) return 0;
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
    unsigned long Argument::getUnsignedLong() {
        if (!isNumber()) return 0;
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
    unsigned long long Argument::getUnsignedLongLong() {
        if (!isNumber()) return 0;
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

namespace Vital::Type::Event {
    // Checkers //
    bool Arguments::isNil(const std::string& index) { return arguments.find(index) == arguments.end(); };
    bool Arguments::isString(const std::string& index) { return (!isNil(index) && arguments.at(index).isString() && true) || false; }
    bool Arguments::isNumber(const std::string& index) { return (!isNil(index) && arguments.at(index).isNumber() && true) || false; }

    // Getters //
    std::string Arguments::getString(const std::string& index) {
        if (isNil(index)) throw;
        return arguments.at(index).getString();
    }
    int Arguments::getInt(const std::string& index) {
        if (isNil(index)) throw;
        return arguments.at(index).getInt();
    }
    float Arguments::getFloat(const std::string& index) {
        if (isNil(index)) throw;
        return arguments.at(index).getFloat();
    }
    double Arguments::getDouble(const std::string& index) {
        if (isNil(index)) throw;
        return arguments.at(index).getDouble();
    }
    long Arguments::getLong(const std::string& index) {
        if (isNil(index)) throw;
        return arguments.at(index).getLong();
    }
    long long Arguments::getLongLong(const std::string& index) {
        if (isNil(index)) throw;
        return arguments.at(index).getLongLong();
    }
    long double Arguments::getLongDouble(const std::string& index) {
        if (isNil(index)) throw;
        return arguments.at(index).getLongDouble();
    }
    unsigned Arguments::getUnsigned(const std::string& index) {
        if (isNil(index)) throw;
        return arguments.at(index).getUnsigned();
    }
    unsigned long Arguments::getUnsignedLong(const std::string& index) {
        if (isNil(index)) throw;
        return arguments.at(index).getUnsignedLong();
    }
    unsigned long long Arguments::getUnsignedLongLong(const std::string& index) {
        if (isNil(index)) throw;
        return arguments.at(index).getUnsignedLongLong();
    }

    // Pushers //
    void Arguments::push(const std::string& index, const std::string& value) { arguments.emplace(index, Argument(value)); }
    void Arguments::push(const std::string& index, int value) { arguments.emplace(index, Argument(value)); }
    void Arguments::push(const std::string& index, float value) { arguments.emplace(index, Argument(value)); }
    void Arguments::push(const std::string& index, double value) { arguments.emplace(index, Argument(value)); }
    void Arguments::push(const std::string& index, long value) { arguments.emplace(index, Argument(value)); }
    void Arguments::push(const std::string& index, long long value) { arguments.emplace(index, Argument(value)); }
    void Arguments::push(const std::string& index, long double value) { arguments.emplace(index, Argument(value)); }
    void Arguments::push(const std::string& index, unsigned value) { arguments.emplace(index, Argument(value)); }
    void Arguments::push(const std::string& index, unsigned long value) { arguments.emplace(index, Argument(value)); }
    void Arguments::push(const std::string& index, unsigned long long value) { arguments.emplace(index, Argument(value)); }
}