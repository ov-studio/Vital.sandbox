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
            std::string rwString;
            int rwInt;
            float rwFloat;
            double rwDouble;
            long rwLong;
            long long rwLongLong;
            long double rwLongDouble;
            unsigned rwUnsigned;
            unsigned long rwUnsignedLong;
            unsigned long long rwUnsignedLongLong;
            std::string rwType;
        public:
            // Instantiators //
            Argument(const std::string& argument);
            Argument(int argument);
            Argument(float argument);
            Argument(double argument);
            Argument(long argument);
            Argument(long long argument);
            Argument(long double argument);
            Argument(unsigned argument);
            Argument(unsigned long argument);
            Argument(unsigned long long argument);

            // Checkers //
            bool isString();
            bool isNumber();

            // Getters //
            std::string getString();
            int getInt();
            float getFloat();
            double getDouble();
            long getLong();
            long long getLongLong();
            long double getLongDouble();
            unsigned getUnsigned();
            unsigned long getUnsignedLong();
            unsigned long long getUnsignedLongLong();
    };

    class Arguments {
        private:
            std::map<std::string, Argument> arguments;
        public:
            // Checkers //
            bool isNil(const std::string& index);
            bool isString(const std::string& index);
            bool isNumber(const std::string& index);

            // Getters //
            std::string getString(const std::string& index);
            int getInt(const std::string& index);
            float getFloat(const std::string& index);
            double getDouble(const std::string& index);
            long getLong(const std::string& index);
            long long getLongLong(const std::string& index);
            long double getLongDouble(const std::string& index);
            unsigned getUnsigned(const std::string& index);
            unsigned long getUnsignedLong(const std::string& index);
            unsigned long long getUnsignedLongLong(const std::string& index);

            // Pushers //
            void push(const std::string& index, const std::string& value);
            void push(const std::string& index, int value);
            void push(const std::string& index, float value);
            void push(const std::string& index, double value);
            void push(const std::string& index, long value);
            void push(const std::string& index, long long value);
            void push(const std::string& index, long double value);
            void push(const std::string& index, unsigned value);
            void push(const std::string& index, unsigned long value);
            void push(const std::string& index, unsigned long long value);
    };
    typedef unsigned long HandleID;
    typedef struct {
        const std::function<void()> unbind;
    } Handle;
    typedef std::function<void(Arguments)> Handler;
}