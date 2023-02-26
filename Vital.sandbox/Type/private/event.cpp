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
    Argument::Argument(const std::string& argument) { rwString = argument, rwType = std::string(typeid(argument).name()); }
    Argument::Argument(int argument) { rwInt = argument, rwType = std::string(typeid(argument).name()); }
    Argument::Argument(float argument) { rwFloat = argument, rwType = std::string(typeid(argument).name()); }
    Argument::Argument(double argument) { rwDouble = argument, rwType = std::string(typeid(argument).name()); }
    Argument::Argument(long argument) { rwLong = argument, rwType = std::string(typeid(argument).name()); }
    Argument::Argument(long long argument) { rwLongLong = argument, rwType = std::string(typeid(argument).name()); }
    Argument::Argument(long double argument) { rwLongDouble = argument, rwType = std::string(typeid(argument).name()); }
    Argument::Argument(unsigned argument) { rwUnsigned = argument, rwType = std::string(typeid(argument).name()); }
    Argument::Argument(unsigned long argument) { rwUnsignedLong = argument, rwType = std::string(typeid(argument).name()); }
    Argument::Argument(unsigned long long argument) { rwUnsignedLongLong = argument, rwType = std::string(typeid(argument).name()); }

    // Checkers //
    bool Argument::isString() { return (rwType == typeid(const std::string&).name()); }
    bool Argument::isNumber() {
        switch(rwType) {
            typeid(int).name():
            typeid(float).name():
            typeid(double).name():
            typeid(long).name():
            typeid(long long).name():
            typeid(long double).name():
            typeid(unsigned).name():
            typeid(unsigned long).name():
            typeid(unsigned long long).name():
            default:
                return false;
        }
        return true;
    }

    // Getters //
    std::string Argument::getString() {
        if (!isString(argument)) return 0;
        return rwString;
    }
    int Argument::getInt() {
        if (!isNumber(argument)) return 0;
        switch(rwType) {
            typeid(int).name():
                return static_cast<int>(rwInt);
            typeid(float).name():
                return static_cast<int>(rwFloat);
            typeid(double).name():
                return static_cast<int>(rwDouble);
            typeid(long).name():
                return static_cast<int>(rwLong);
            typeid(long long).name():
                return static_cast<int>(rwLongLong);
            typeid(long double).name():
                return static_cast<int>(rwLongDouble);
            typeid(unsigned).name():
                return static_cast<int>(rwUnsigned);
            typeid(unsigned long).name():
                return static_cast<int>(rwUnsignedLong);
            typeid(unsigned long long).name():
                return static_cast<int>(rwUnsignedLongLong);
        }
    }
    float Arguments::getFloat() {
        if (!isNumber(argument)) return 0;
        switch(rwType) {
            typeid(int).name():
                return static_cast<float>(rwInt);
            typeid(float).name():
                return static_cast<float>(rwFloat);
            typeid(double).name():
                return static_cast<float>(rwDouble);
            typeid(long).name():
                return static_cast<float>(rwLong);
            typeid(long long).name():
                return static_cast<float>(rwLongLong);
            typeid(long double).name():
                return static_cast<float>(rwLongDouble);
            typeid(unsigned).name():
                return static_cast<float>(rwUnsigned);
            typeid(unsigned long).name():
                return static_cast<float>(rwUnsignedLong);
            typeid(unsigned long long).name():
                return static_cast<float>(rwUnsignedLongLong);
        }
    }
    double Argument::getDouble() {
        if (!isNumber(argument)) return 0;
        switch(rwType) {
            typeid(int).name():
                return static_cast<double>(rwInt);
            typeid(float).name():
                return static_cast<double>(rwFloat);
            typeid(double).name():
                return static_cast<double>(rwDouble);
            typeid(long).name():
                return static_cast<double>(rwLong);
            typeid(long long).name():
                return static_cast<double>(rwLongLong);
            typeid(long double).name():
                return static_cast<double>(rwLongDouble);
            typeid(unsigned).name():
                return static_cast<double>(rwUnsigned);
            typeid(unsigned long).name():
                return static_cast<double>(rwUnsignedLong);
            typeid(unsigned long long).name():
                return static_cast<double>(rwUnsignedLongLong);
        }
    }
    long Argument::getLong() {
        if (!isNumber(argument)) return 0;
        switch(rwType) {
            typeid(int).name():
                return static_cast<long>(rwInt);
            typeid(float).name():
                return static_cast<long>(rwFloat);
            typeid(double).name():
                return static_cast<long>(rwDouble);
            typeid(long).name():
                return static_cast<long>(rwLong);
            typeid(long long).name():
                return static_cast<long>(rwLongLong);
            typeid(long double).name():
                return static_cast<long>(rwLongDouble);
            typeid(unsigned).name():
                return static_cast<long>(rwUnsigned);
            typeid(unsigned long).name():
                return static_cast<long>(rwUnsignedLong);
            typeid(unsigned long long).name():
                return static_cast<long>(rwUnsignedLongLong);
        }
    }
    long long Argument::getLongLong() {
        if (!isNumber(argument)) return 0;
        switch(rwType) {
            typeid(int).name():
                return static_cast<long long>(rwInt);
            typeid(float).name():
                return static_cast<long long>(rwFloat);
            typeid(double).name():
                return static_cast<long long>(rwDouble);
            typeid(long).name():
                return static_cast<long long>(rwLong);
            typeid(long long).name():
                return static_cast<long long>(rwLongLong);
            typeid(long double).name():
                return static_cast<long long>(rwLongDouble);
            typeid(unsigned).name():
                return static_cast<long long>(rwUnsigned);
            typeid(unsigned long).name():
                return static_cast<long long>(rwUnsignedLong);
            typeid(unsigned long long).name():
                return static_cast<long long>(rwUnsignedLongLong);
        }
    }
    long double Argument::getLongDouble() {
        if (!isNumber(argument)) return 0;
        switch(rwType) {
            typeid(int).name():
                return static_cast<long double>(rwInt);
            typeid(float).name():
                return static_cast<long double>(rwFloat);
            typeid(double).name():
                return static_cast<long double>(rwDouble);
            typeid(long).name():
                return static_cast<long double>(rwLong);
            typeid(long long).name():
                return static_cast<long double>(rwLongLong);
            typeid(long double).name():
                return static_cast<long double>(rwLongDouble);
            typeid(unsigned).name():
                return static_cast<long double>(rwUnsigned);
            typeid(unsigned long).name():
                return static_cast<long double>(rwUnsignedLong);
            typeid(unsigned long long).name():
                return static_cast<long double>(rwUnsignedLongLong);
        }
    }
    unsigned Argument::getUnsigned() {
        if (!isNumber(argument)) return 0;
        switch(rwType) {
            typeid(int).name():
                return static_cast<unsigned>(rwInt);
            typeid(float).name():
                return static_cast<unsigned>(rwFloat);
            typeid(double).name():
                return static_cast<unsigned>(rwDouble);
            typeid(long).name():
                return static_cast<unsigned>(rwLong);
            typeid(long long).name():
                return static_cast<unsigned>(rwLongLong);
            typeid(long double).name():
                return static_cast<unsigned>(rwLongDouble);
            typeid(unsigned).name():
                return static_cast<unsigned>(rwUnsigned);
            typeid(unsigned long).name():
                return static_cast<unsigned>(rwUnsignedLong);
            typeid(unsigned long long).name():
                return static_cast<unsigned>(rwUnsignedLongLong);
        }
    }
    unsigned long Argument::getUnsignedLong() {
        if (!isNumber(argument)) return 0;
        switch(rwType) {
            typeid(int).name():
                return static_cast<unsigned long>(rwInt);
            typeid(float).name():
                return static_cast<unsigned long>(rwFloat);
            typeid(double).name():
                return static_cast<unsigned long>(rwDouble);
            typeid(long).name():
                return static_cast<unsigned long>(rwLong);
            typeid(long long).name():
                return static_cast<unsigned long>(rwLongLong);
            typeid(long double).name():
                return static_cast<unsigned long>(rwLongDouble);
            typeid(unsigned).name():
                return static_cast<unsigned long>(rwUnsigned);
            typeid(unsigned long).name():
                return static_cast<unsigned long>(rwUnsignedLong);
            typeid(unsigned long long).name():
                return static_cast<unsigned long>(rwUnsignedLongLong);
        }
    }
    unsigned long long Argument::getUnsignedLongLong() {
        if (!isNumber(argument)) return 0;
        switch(rwType) {
            typeid(int).name():
                return static_cast<unsigned long long>(rwInt);
            typeid(float).name():
                return static_cast<unsigned long long>(rwFloat);
            typeid(double).name():
                return static_cast<unsigned long long>(rwDouble);
            typeid(long).name():
                return static_cast<unsigned long long>(rwLong);
            typeid(long long).name():
                return static_cast<unsigned long long>(rwLongLong);
            typeid(long double).name():
                return static_cast<unsigned long long>(rwLongDouble);
            typeid(unsigned).name():
                return static_cast<unsigned long long>(rwUnsigned);
            typeid(unsigned long).name():
                return static_cast<unsigned long long>(rwUnsignedLong);
            typeid(unsigned long long).name():
                return static_cast<unsigned long long>(rwUnsignedLongLong);
        }
    }
}

namespace Vital::Type::Event {
    // Checkers //
    bool Arguments::isNil(const std::string& index) { return arguments.find(index) == arguments.end() };
    bool Arguments::isString(const std::string& index) { return (!isNil(index) && arguments.at(index).isString() && true) || false }
    bool Arguments::isNumber(const std::string& index) { return (!isNil(index) && arguments.at(index).isNumber() && true) || false }

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