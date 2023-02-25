/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: event.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


/////////////////////////
// Vital: Type: Event //
/////////////////////////

namespace Vital::Type::Event {
    typedef std::map<std::string, std::pair<std::string, std::string>> Arguments;
    typedef std::function<void(Arguments)> Handler;

    inline std::pair<std::string, std::string> Argument(const std::string& arg) { return {arg, std::string(typeid(arg).name())}; }
    inline std::pair<std::string, std::string> Argument(int arg) { return {std::to_string(arg), std::string(typeid(arg).name())}; }
    inline std::pair<std::string, std::string> Argument(long arg) { return {std::to_string(arg), std::string(typeid(arg).name())}; }
    inline std::pair<std::string, std::string> Argument(long long arg) { return {std::to_string(arg), std::string(typeid(arg).name())}; }
    inline std::pair<std::string, std::string> Argument(unsigned arg) { return {std::to_string(arg), std::string(typeid(arg).name())}; }
    inline std::pair<std::string, std::string> Argument(unsigned long arg) { return {std::to_string(arg), std::string(typeid(arg).name())}; }
    inline std::pair<std::string, std::string> Argument(unsigned long long arg) { return {std::to_string(arg), std::string(typeid(arg).name())}; }
    inline std::pair<std::string, std::string> Argument(float arg) { return {std::to_string(arg), std::string(typeid(arg).name())}; }
    inline std::pair<std::string, std::string> Argument(double arg) { return {std::to_string(arg), std::string(typeid(arg).name())}; }
    inline std::pair<std::string, std::string> Argument(long double arg) { return {std::to_string(arg), std::string(typeid(arg).name())}; }

    template<typename T>
    T ArgumentValue(std::pair<std::string, std::string> arg) {
        switch(arg.second) {
            typeid(const std::string&).name():
                return arg.first;
            typeid(int).name():
                return reinterpret_cast<int>(arg.first);
            typeid(long).name():
                return reinterpret_cast<long>(arg.first);
            typeid(long long).name():
                return reinterpret_cast<long long>(arg.first);
            typeid(unsigned).name():
                return reinterpret_cast<unsigned>(arg.first);
            typeid(unsigned long).name():
                return reinterpret_cast<unsigned long>(arg.first);
            typeid(unsigned long long).name():
                return reinterpret_cast<unsigned long long>(arg.first);
            typeid(float).name():
                return reinterpret_cast<float>(arg.first);
            typeid(double).name():
                return reinterpret_cast<double>(arg.first);
            typeid(long double).name():
                return reinterpret_cast<long double>(arg.first);
        }
        throw ErrorCode["invalid-arguments"];
    }
}