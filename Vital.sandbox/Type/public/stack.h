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

            // Utils //
            static bool isTypeString(const char* rwType);
            static bool isTypeNumber(const char* rwType);
            std::pair<std::string, std::string> serialize();
            //Value deserialize();
    };

    class Instance {
        private:
            std::vector<Value> rwVector;
            std::map<std::string, Value> rwMap;
        public:
            // Checkers //
            bool isNil(int index = 0);
            bool isNil(const std::string& index);
            bool isString(int index = 0);
            bool isString(const std::string& index);
            bool isNumber(int index = 0);
            bool isNumber(const std::string& index);

            // Getters //
            std::string getString(int index = 0);
            std::string getString(const std::string& index);
            int getInt(int index = 0);
            int getInt(const std::string& index);
            float getFloat(int index = 0);
            float getFloat(const std::string& index);
            double getDouble(int index = 0);
            double getDouble(const std::string& index);
            long getLong(int index = 0);
            long getLong(const std::string& index);
            long long getLongLong(int index = 0);
            long long getLongLong(const std::string& index);
            long double getLongDouble(int index = 0);
            long double getLongDouble(const std::string& index);
            unsigned getUnsigned(int index = 0);
            unsigned getUnsigned(const std::string& index);
            unsigned long getUnsignedLong(int index = 0);
            unsigned long getUnsignedLong(const std::string& index);
            unsigned long long getUnsignedLongLong(int index = 0);
            unsigned long long getUnsignedLongLong(const std::string& index);

            // Pushers //
            void push(const std::string& value);
            void push(const std::string& index, const std::string& value);
            void push(int value);
            void push(const std::string& index, int value);
            void push(float value);
            void push(const std::string& index, float value);
            void push(double value);
            void push(const std::string& index, double value);
            void push(long value);
            void push(const std::string& index, long value);
            void push(long long value);
            void push(const std::string& index, long long value);
            void push(long double value);
            void push(const std::string& index, long double value);
            void push(unsigned value);
            void push(const std::string& index, unsigned value);
            void push(unsigned long value);
            void push(const std::string& index, unsigned long value);
            void push(unsigned long long value);
            void push(const std::string& index, unsigned long long value);

            // Poppers //
            void pop(int index = 0);
            void pop(const std::string& index);

            // Utils //
            std::string serialize();
    };
}