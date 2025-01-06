#ifndef UTILS_HEADER
#define UTILS_HEADER
#include "Tokens.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
using std::string_literals::operator""s;

template<typename T, typename T1>
std::shared_ptr<T> factory(T1&& arg){
    // wrap a pointer to the generic class t with the smart pointer
    // new T (std::forward<T1>(arg)) is almost like new T(arg) but it conserves the rvalue/lvalue-ness of the argument
    // it operates not much unlike std::move
    return std::shared_ptr<T>(new T(std::forward<T1>(arg)));
    // Thre reason for this is so that both const, non const and r-value parameters can bind to teh parameters
}

inline std::string apply_err_fmt(std::string opt){
    std::string before{"\033[1;4;31m"};
    std::string after{"\033[0m"};
    return before + opt + after;
}

using Basic_type = std::pair<TOKENS, int>;

struct types{
    constexpr static Basic_type INTEGER{TOKENS::INT, 4};
    constexpr static Basic_type FLOAT{TOKENS::FLOAT, 8};
    constexpr static Basic_type CHAR{TOKENS::CHAR, 1};
    constexpr static Basic_type BOOL{TOKENS::BOOL, 1};
};

class Array;

using Type = std::variant<Basic_type, std::shared_ptr<Array>>;


class Array{
    public:
        Type type;
        int size;
        std::string to_string(){
            if (std::holds_alternative<Basic_type>(type)){
                return "["s + std::to_string(size) + "]"  + find_string_repr(std::get<Basic_type>(type).first);
            }
            else{
                auto arr = std::get<std::shared_ptr<Array>>(type);
                return "["s + std::to_string(size) + "]" + arr->to_string();
            }
        }
};

inline bool is_numeric_type(Type p){
    auto type = std::get_if<Basic_type>(&p);
    if (type)
        if (type->first == TOKENS::INT || type->first == TOKENS::FLOAT || type->first == TOKENS::CHAR) return true;
    return false;
}

inline std::optional<Basic_type> max_type(Type t1, Type t2){
    if (!(is_numeric_type(t1) && is_numeric_type(t2))) return {};
    else{
        auto type_1 = std::get<Basic_type>(t1), type_2 = std::get<Basic_type>(t2);
        if (type_1.first == TOKENS::FLOAT || type_2.first == TOKENS::FLOAT) return types::FLOAT;
        if (type_1.first == TOKENS::INT || type_2.first == TOKENS::INT) return types::INTEGER;
        return types::CHAR;
    }
}

inline std::string type_string_repr(Type type){
    if (std::holds_alternative<Basic_type>(type)){
        return find_string_repr(std::get<Basic_type>(type).first);
    }
    else {
        // should return to_string of an array, will handle at later time
        return "Todo, implement array.to_string(), for this case implies giving expression as argument";
    }
}

#endif