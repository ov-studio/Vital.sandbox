/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: stack.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Stack Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


/////////////////////////
// Vital: Type: Stack //
/////////////////////////

namespace Vital::Type::Stack {
    class Value {
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
            const char* rwType;
        public:
            // Instantiators //
            Value(const std::string& argument);
            Value(int argument);
            Value(float argument);
            Value(double argument);
            Value(long argument);
            Value(long long argument);
            Value(long double argument);
            Value(unsigned argument);
            Value(unsigned long argument);
            Value(unsigned long long argument);

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


    class Ordered {
        private:
            std::vector<Value> values;
        public:
            // Checkers //
            bool isNil(int index = 1);
            bool isString(int index = 1);
            bool isNumber(int index = 1);

            // Getters //
            std::string getString(int index = 1);
            int getInt(int index = 1);
            float getFloat(int index = 1);
            double getDouble(int index = 1);
            long getLong(int index = 1);
            long long getLongLong(int index = 1);
            long double getLongDouble(int index = 1);
            unsigned getUnsigned(int index = 1);
            unsigned long getUnsignedLong(int index = 1);
            unsigned long long getUnsignedLongLong(int index = 1);

            // Pushers //
            void push(int index = 1, const std::string& value);
            void push(int index = 1, int value);
            void push(int index = 1, float value);
            void push(int index = 1, double value);
            void push(int index = 1, long value);
            void push(int index = 1, long long value);
            void push(int index = 1, long double value);
            void push(int index = 1, unsigned value);
            void push(int index = 1, unsigned long value);
            void push(int index = 1, unsigned long long value);
    };

    class Unordered {
        private:
            std::map<std::string, Value> values;
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
}