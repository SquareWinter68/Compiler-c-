#ifndef TAC_PARSER_HEADER
#define TAC_PARSER_HEADER
#include "intermediate.hpp"
#include <any>
#include <cctype>
#include <concepts>
#include <fstream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
template<typename T>
concept string_container = requires(T a){
    {a.begin()} -> std::same_as<typename T::iterator>;
    {a.end()} -> std::same_as<typename T::iterator>;
    requires std::same_as<typename T::value_type, std::string>;
};
class Tac_parser;
void init_tac_parser(std::vector<variable_positions>& vars, std::ofstream& out, Tac_parser* pars);

void generate_asm(unsigned long long reserve);

inline std::vector<std::string> empty;
inline void parse_tac(std::vector<std::string>&, std::vector<std::string>& globals = empty){
    long long offset;
    std::regex labels{R"(^L\d{1,}:)"};
    std::regex constant_assignments{R"(\w = \d{1,})"};
    std::regex three_variable_operatoin{R"([a-zA-Z][a-zA-Z0-9_]* = [a-zA-Z][a-zA-Z0-9_]* [+\-*/] [a-zA-Z][a-zA-Z0-9_]*)"};
    if (globals.size()){

    }
}

namespace stolen{
    inline bool isalpha(char c){
        return std::isalpha(c) || c == '_';
    }
    inline bool isalnum(char c){
        return std::isalnum(c) || c == '_';
    }
}


enum class tac_tokens{
    label, id, assign, mult, div, add, sub, jump, if_, if_false, greater_than, less_than, greater_than_equal, less_than_equal, equal,not_equal, square_bracket_open, square_bracket_closed, eof_tok, const_
};

class Tac_parser{
    public:
        Tac_parser(std::string filename){
            file.open(filename);
            if (!file.is_open()) throw std::runtime_error{"Failed to open file in parse_TAC.cpp\n"};
            // initialize the peek
            do get_next_char(); while(peek == 0);
        }
        char peek{};
        std::ifstream file; 
        // template<typename T>
        // requires std::same_as<T, std::string>
        // void init(T&& file_name){
        //     file.open(file_name);
        //     if (!file.is_open()) throw std::runtime_error{"Failed to open file in parse_TAC.cpp\n"};
        //     // initialize the peek
        //     do get_next_char(); while(peek == 0);
        // }
        void get_next_char(){
            file.get(peek);
        }
        bool get_next_char(char c){
            file.get(peek);
            if (peek == c) return true;
            return false;
        }
    std::pair<tac_tokens, std::any> scan(){
        if (!peek) throw std::runtime_error{"Peek not initialized\n"};
        for (;; get_next_char()){
            if (file.eof()) return {tac_tokens::eof_tok, 0};
            // handle the first read from file
            else if (peek == '\0') continue;
            // handle whitespaces and tabs
            else if (peek == ' ' || peek == '\t') continue;
            // reset character, and increase line number
            else if (peek == '\n') continue;
            // if none are ture, let controll fall through to the rest of the lexer
            else break;
        }

        if (stolen::isalpha(peek)){
            std::string lexeme;
            while (stolen::isalnum(peek)){
                lexeme += peek;
                get_next_char();
            }
            if (peek == ':') {get_next_char(); return {tac_tokens::label, lexeme + ':'};}
            else if (lexeme == "if") return {tac_tokens::if_, 0};
            else if (lexeme == "iffalse") return {tac_tokens::if_false, 0};
            else if (lexeme == "goto") return {tac_tokens::jump, 0};
            else return {tac_tokens::id, lexeme};
        }
        else if (std::isdigit(peek)){
            long val{};
            while (std::isdigit(peek)){
                val = val *10 + (peek - '0');
                get_next_char();
            }
            return {tac_tokens::const_, val};
        }
        tac_tokens tok;
        switch (peek) {
            case '=':
                if (get_next_char('=')) {tok = tac_tokens::equal; break;} else return {tac_tokens::assign, 0};
            case '>':
                if (get_next_char('=')) {tok = tac_tokens::greater_than_equal; break;} else return {tac_tokens::greater_than, 0};
            case '<':
                if (get_next_char('=')) {tok = tac_tokens::less_than_equal; break;} else return {tac_tokens::less_than, 0};
            case '[':
                tok = tac_tokens::square_bracket_open; break;
            case ']':
                tok = tac_tokens::square_bracket_closed; break;
            case '*':
                tok = tac_tokens::mult; break;
            case '/':
                tok = tac_tokens::div; break;
            case '+':
                tok = tac_tokens::add; break;
            case '-':
                tok = tac_tokens::sub; break;
        }
        get_next_char();
        return {tok, 0};
    }

};
#endif