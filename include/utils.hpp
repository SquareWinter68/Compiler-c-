#ifndef UTILS_HEADER
#define UTILS_HEADER
#include "Lexical_anal.hpp"
#include "Tokens.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>
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

using acceptable_types = std::variant<int, float, bool, unsigned, long, short, std::string>;
template<typename T>
concept Constant_types = requires(T arg){
    {arg} -> std::convertible_to<acceptable_types>;
};

template<Constant_types T>
std::string type_to_string(T argument){
    return std::to_string(argument);
}

template<>
inline std::string type_to_string(std::string argument){
    return argument;
}

class Array;

using Basic_type = std::pair<TOKENS, int>;
using Array_type = std::shared_ptr<Array>;

struct types{
    constexpr static Basic_type INTEGER{TOKENS::INT, 4};
    constexpr static Basic_type FLOAT{TOKENS::FLOAT, 8};
    constexpr static Basic_type CHAR{TOKENS::CHAR, 1};
    constexpr static Basic_type BOOL{TOKENS::BOOL, 1};
};


using Type = std::variant<Basic_type, Array_type>;


class Array{
    public:
        Type type;
        int size;
        std::string to_string(){
            if (std::holds_alternative<Basic_type>(type)){
                return "["s + std::to_string(size) + "]"  + find_string_repr(std::get<Basic_type>(type).first);
            }
            else{
                auto arr = std::get<Array_type>(type);
                return "["s + std::to_string(size) + "]" + arr->to_string();
            }
        }
        TOKENS get_basic_type(){
            if (std::holds_alternative<Basic_type>(type)) return std::get<Basic_type>(type).first;
            else{
                auto arr = std::get<Array_type>(type);
                return arr->get_basic_type();
            }
        }
        int get_basic_type_width(){
            if (std::holds_alternative<Basic_type>(type)) return std::get<Basic_type>(type).second;
            else{
                auto arr = std::get<Array_type>(type);
                return arr->get_basic_type_width();
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
        if (type_1 == types::FLOAT || type_2 == types::FLOAT) return types::FLOAT;
        if (type_1 == types::INTEGER || type_2 == types::INTEGER) return types::INTEGER;
        return types::CHAR;
    }
}

inline std::optional<Basic_type> is_bool(Type t1, Type t2){
    auto type1 = std::get_if<Basic_type>(&t1), type2 = std::get_if<Basic_type>(&t2);
    if (type1 && type2){
        if (*type1 == types::BOOL && *type2 == types::BOOL) return types::BOOL;
    }
    return {};
}
inline bool is_bool(Type t1){
    auto type = std::get_if<Basic_type>(&t1);
    if (type && *type == types::BOOL) return true;
    return false;
}

inline std::string type_string_repr(Type type, long size = 0){
    if (std::holds_alternative<Basic_type>(type)){
        return find_string_repr(std::get<Basic_type>(type).first);
    }
    else {
        // should return to_string of an array, will handle at later time
        //return "Todo, implement array.to_string(), for this case implies giving expression as argument";
        auto temp = std::get<Array_type>(type);
        return find_string_repr(temp->get_basic_type()) + " [" + std::to_string(temp->size) + "]";
    }
}
// checks if type is in allowed types.
inline bool operator%(const Basic_type& needle, std::vector<Basic_type> haystack){
    if(std::find(haystack.begin(), haystack.end(), needle) != haystack.end()) return true;
    return false;
}
inline bool operator%(const TOKENS& needle, std::vector<TOKENS> haystack){
    if (std::find(haystack.begin(), haystack.end(), needle) != haystack.end()) return true;
    else return false;
}

inline long get_type_width(const Type& type){
    if (std::holds_alternative<Basic_type>(type)){
        return std::get<Basic_type>(type).second;
    }
    else{
        auto arr = std::get<Array_type>(type);
        return arr->size * arr->get_basic_type_width();
    }
}

#endif