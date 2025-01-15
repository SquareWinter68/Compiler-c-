#ifndef LEXICAL_ANAL_HEADER
#define LEXICAL_ANAL_HEADER
#include "Tokens.hpp"
#include <cstddef>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
struct Basic_token{
    TOKENS token;
    std::string
    to_string()
    {   auto pair = token_string_map.find(token); 
        if (pair != token_string_map.end()) return pair->second; 
        else throw std::runtime_error{"ERROR in Lexical_anal.hpp\ncalled to_string on an unexistent token"};

    }
};
inline bool operator==(const Basic_token& lhs, const Basic_token& rhs){
    return lhs.token == rhs.token;
}
struct Word{
    
    TOKENS token;
    std::string lexeme;
    Word(TOKENS tok, std::string lex): token{tok}, lexeme{lex}{}
    std::string to_string(){return lexeme;}
};
inline bool operator==(const Word& lhs, const Word& rhs){
    return (lhs.lexeme == rhs.lexeme) && (lhs.token == rhs.token);
}
struct Word_equal{
    bool operator()(const Word& lhs, const Word& rhs)const {
        return true;//(lhs.lexeme == rhs.lexeme) && (lhs.token == rhs.token);
    }
};

struct Number_integer{
    TOKENS token;
    int value;
    Number_integer(int val, TOKENS tok = TOKENS::INTEGRAL_LITERAL): token{tok}, value{val} {}
    std::string to_string(){return std::to_string(value);}
};
inline bool operator==(const Number_integer& lhs, const Number_integer& rhs){
    return (lhs.token == rhs.token) && (lhs.value == rhs.value);
}

struct Number_float{
    TOKENS token;
    float value;
    Number_float(float val, TOKENS tok = TOKENS::DECIMAL_LITERAL): token{tok}, value{val}{}
    std::string to_string(){return std::to_string(value);}
};
inline bool operator==(const Number_float& lhs, const Number_float& rhs){
    return (lhs.token == rhs.token) && (lhs.value == rhs.value);
}

struct String_literal{
    TOKENS token;
    std::string value;
    String_literal(std::string val, TOKENS tok = TOKENS::STRING_LITERAL): token{tok}, value{val}{}
    std::string to_string(){return value;}
};
inline bool operator==(const String_literal& lhs, const String_literal& rhs){
    return (lhs.token == rhs.token) && (lhs.value == rhs.value);
}

using Token = std::variant<Basic_token, Word, Number_integer, Number_float, String_literal>;

struct token_hash{
    size_t operator()(const Token& token) const{
        if (std::holds_alternative<Basic_token>(token)){
            auto alternative = std::get<Basic_token>(token);
            return std::hash<int>()((int)alternative.token);
        }
        else if (std::holds_alternative<Word>(token)){
            auto alternative = std::get<Word>(token);
            return std::hash<int>()((int)alternative.token) ^ std::hash<std::string>()(alternative.lexeme);
        }
        else if (std::holds_alternative<Number_integer>(token)) {
            auto alternative = std::get<Number_integer>(token);
            return std::hash<int>()((int)alternative.token) ^ std::hash<int>()(alternative.value);
        }
        else if (std::holds_alternative<Number_float>(token)){
            auto alternative = std::get<Number_float>(token);
            return std::hash<int>()((int)alternative.token) ^ std::hash<float>()((float)alternative.value);
        }
        else {
            auto alternative = std::get<String_literal>(token);
            return std::hash<int>()((int)alternative.token) ^ std::hash<std::string>()(alternative.value);
        }
    }
};


#endif