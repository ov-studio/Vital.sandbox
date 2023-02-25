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
    class Argument {
        private:
            std::string buffer, type;
        public:
            inline Argument(const std::string& arg) { buffer = arg, type = std::string(typeid(arg).name()); }
            inline Argument(int arg) { buffer = arg, type = std::string(typeid(arg).name()); }
            inline Argument(long arg) { buffer = arg, type = std::string(typeid(arg).name()); }
            inline Argument(long long arg) { buffer = arg, type = std::string(typeid(arg).name()); }
            inline Argument(unsigned arg) { buffer = arg, type = std::string(typeid(arg).name()); }
            inline Argument(unsigned long arg) { buffer = arg, type = std::string(typeid(arg).name()); }
            inline Argument(unsigned long long arg) { buffer = arg, type = std::string(typeid(arg).name()); }
            inline Argument(float arg) { buffer = arg, type = std::string(typeid(arg).name()); }
            inline Argument(double arg) { buffer = arg, type = std::string(typeid(arg).name()); }
            inline Argument(long double arg) { buffer = arg, type = std::string(typeid(arg).name()); }

            template<typename T>
            T value() {
                switch(type) {
                    typeid(const std::string&).name():
                        return buffer;
                    typeid(int).name():
                        return reinterpret_cast<int>(buffer);
                    typeid(long).name():
                        return reinterpret_cast<long>(buffer);
                    typeid(long long).name():
                        return reinterpret_cast<long long>(buffer);
                    typeid(unsigned).name():
                        return reinterpret_cast<unsigned>(buffer);
                    typeid(unsigned long).name():
                        return reinterpret_cast<unsigned long>(buffer);
                    typeid(unsigned long long).name():
                        return reinterpret_cast<unsigned long long>(buffer);
                    typeid(float).name():
                        return reinterpret_cast<float>(buffer);
                    typeid(double).name():
                        return reinterpret_cast<double>(buffer);
                    typeid(long double).name():
                        return reinterpret_cast<long double>(buffer);
                }
            }
    };
    typedef std::map<std::string, Argument> Arguments;
    typedef std::function<void(Arguments)> Handler;
}