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

namespace Vital::Type {
    class Stack {
        public:
            class Value;
        private:
            std::vector<Value> rwVector;
            std::map<std::string, Value> rwMap;
        public:
            using ValueType = std::variant<std::string, int, float, double, long, long long, long double, unsigned, unsigned long, unsigned long long>;
            class Value {
                private:
                    ValueType value;
                public:
                    // Instantiators //
                    template <typename T>
                    Value(T argument) : value(argument) {}

                    // Checkers //
                    bool isString() { return std::holds_alternative<std::string>(value); }
                    bool isNumber() {
                        return (
                            std::holds_alternative<int>(value) || 
                            std::holds_alternative<float>(value) || 
                            std::holds_alternative<double>(value) || 
                            std::holds_alternative<long>(value) || 
                            std::holds_alternative<long long>(value) || 
                            std::holds_alternative<long double>(value) || 
                            std::holds_alternative<unsigned>(value) || 
                            std::holds_alternative<unsigned long>(value) || 
                            std::holds_alternative<unsigned long long>(value)
                        );
                    }

                    // Getters //
                    template <typename T>
                    T get() const { return std::get<T>(value); }

                    // Utils
                    std::pair<std::string, std::string> serialize();
                    static Value deserialize(const std::pair<std::string, std::string>& serial);
            };

            // Checkers //
            bool isNil(int index = 0);
            bool isNil(const std::string& index);
            bool isString(int index = 0);
            bool isString(const std::string& index);
            bool isNumber(int index = 0);
            bool isNumber(const std::string& index);

            // Getters //
            template <typename T>
            T get(int index = 0);
            template <typename T>
            T get(const std::string& index);

            // Pushers //
            template <typename T>
            void push(T& value);
            template <typename T>
            void push(const std::string& index, T& value);

            // Poppers
            void pop(int index = 0);
            void pop(const std::string& index);

            // Utils //
            std::string serialize();
            static Stack deserialize(const std::string& serial);
    };

}