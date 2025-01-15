#ifndef LEXER_HEADER
#define LEXER_HEADER
#include "Lexical_anal.hpp"
#include "Tokens.hpp"
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include "utils.hpp"


namespace my_locale{
    inline bool isalpha(char c){
        return std::isalpha(c) || c == '_';
    }
    inline bool isalnum(char c){
        return std::isalnum(c) || c == '_';
    }
}

namespace loc = my_locale;

class Lexer{
    private:
        char peek{0};
        std::unordered_map<std::string, Word> seen_words;
        std::shared_ptr<Token> eof_token = std::make_shared<Token>(Basic_token{TOKENS::EOF_TOK});
        std::shared_ptr<Token> comment_token = std::make_shared<Token>(Basic_token{TOKENS::COMMENT});
        std::filesystem::path file_path;
        std::ifstream file;
        void get_next_char(){
            file.get(peek);
            ++character;
        }
        bool get_next_char(char c){
            file.get(peek);
            ++character;
            if (peek == c) return true;
            return false;
        }
        void add_to_seen_words(Word w){
            seen_words.insert({w.lexeme, w});
        }
        void add_to_seen_words(std::initializer_list<Word> words){
            //seen_words.insert({w.lexeme, std::make_shared<Word>(w)});
            for(Word word:words){
                seen_words.insert({word.lexeme, std::move(word)});
            }            
        }

     public:
        int line{1};
        int character{};
        Lexer(){
            // add keywords
            add_to_seen_words({{TOKENS::TRUE, "true"}, {TOKENS::FALSE, "false"}, {TOKENS::WHILE, "while"}, {TOKENS::FOR, "for"}, {TOKENS::IF, "if"}, {TOKENS::ELSE, "else"}, {TOKENS::RETURN, "return"}, {TOKENS::DO, "do"}, {TOKENS::BREAK, "break"}});
            // add types
            add_to_seen_words({{TOKENS::INT, "int"}, {TOKENS::FLOAT, "float"}, {TOKENS::CHAR, "char"}, {TOKENS::BOOL, "bool"}, {TOKENS::STRING, "string"}});
        }
        Lexer(std::string path):Lexer(){
            // because the delegating consturcor, must stand alone
            file_path = path;
            file.open(file_path);
            if (!file.is_open()){ throw std::runtime_error{"Failed to open file"};}
            //initialize the peek
            file.get(peek);
        }
        std::shared_ptr<Token> get_word(std::string lexeme){
            auto it = seen_words.find(lexeme);
            auto tok_it = string_to_type_map.find(lexeme);
            if (it != seen_words.end() && tok_it != string_to_type_map.end()) return factory<Token>(Basic_token(tok_it->second));
            else if (it != seen_words.end()) return factory<Token>(it->second);
            return nullptr;
        }
        std::shared_ptr<Token>scan();
            
};



inline std::shared_ptr<Token> Lexer::scan(){
    if (!peek) throw std::runtime_error{"Peek not initialized\n"};
    for (;; get_next_char()){
        if (file.eof()) return eof_token;
        // handle the first read from file
        else if (peek == '\0') continue;
        // handle whitespaces and tabs
        else if (peek == ' ' || peek == '\t') continue;
        // reset character, and increase line number
        else if (peek == '\n') {character = 0; ++line;}
        // if none are ture, let controll fall through to the rest of the lexer
        else break;
    }
    if (loc::isalpha(peek)){
        std::string lexeme;
        while (loc::isalnum(peek)){
            lexeme += peek;
            get_next_char();
        }
        //auto it = seen_words.find(lexeme);
        if (auto token = get_word(lexeme)){
            return token;
        }
        // make new word, and return
        Word word{TOKENS::IDENTIFIER, std::move(lexeme)};
        add_to_seen_words(word);
        return factory<Token>(word);
    }
    else if (std::isdigit(peek)){
        int val{};
        while (std::isdigit(peek)){
            val = val *10 + (peek - '0');
            get_next_char();
        }
        if (peek == '.'){
            float decimal_part{};
            int devisor{10};
            get_next_char();
            if (!std::isdigit(peek)){
                std::ostringstream error;
                error << "Error on line: " << line << "float must contain at least one digit after '.'\n";
            }
            while(std::isdigit(peek)){
                decimal_part += ((float)(peek - '0'))/devisor;
                devisor *= 10;
                get_next_char();
            }
            return factory<Token>(Number_float(val+decimal_part));
        }
        else return factory<Token>(Number_integer(val));
    }
    else if (peek == '\'' || peek == '\"'){
        const char symbol{peek};
        std::string literal{""};
        while(!get_next_char(symbol)){
            if (file.eof()){
                std::ostringstream error;
                error << "the string literral on line " << apply_err_fmt(std::to_string(line)) << "character " << apply_err_fmt(std::to_string(character)) << "was not closed before EOF wa reached\n";
                throw std::runtime_error{error.str()};
            }
            literal += peek;
        }
        get_next_char();
        return factory<Token>(String_literal(std::move(literal)));
    }
    
    Token tok;
    switch(peek){
        case '=':
            // if the next character happens to be =, then the stream needs to be moved one character forward
            // if however the get_nex_char('=') returns false, this means the desired token was ASSIGN, and the current character
            // pointed to by the peek, will be handled in the next call
            if (get_next_char('=')) {tok = Basic_token(TOKENS::EQUAL); break;} else return factory<Token>(Basic_token(TOKENS::ASSIGN));
        
        case '>':
            // no need to design a move constructor for Basic token, yes this is an r-value assignment, but the move token class
            // ony contains one member, which is a class enum
            if (get_next_char('=')) {tok = Basic_token(TOKENS::GREATER_THAN_EQUAL); break;} else return factory<Token>(Basic_token(TOKENS::GREATER_THAN));
        
        case '<':
            if (get_next_char('=')) {tok = Basic_token(TOKENS::LESS_THEN_EQUAL); break;} else return factory<Token>(Basic_token(TOKENS::LESS_THAN));
        
        case '!':
            if (get_next_char('=')) {tok = Basic_token(TOKENS::NOT_EQUAL); break;} else return factory<Token>(Basic_token(TOKENS::LOGICAL_NOT));

        case '/':
            {   // scoped because of tidineds.
                get_next_char();
                if (peek == '/') {while (!get_next_char('\n')); return comment_token;}
                else if (peek == '*'){
                    char prev{'*'};
                    while (1){
                        if (file.eof()){
                            throw std::runtime_error{"Ran out of characters before the multiline comment ended\n"};
                        }
                        get_next_char();
                                                        // this primes peek for the next function call
                        if (prev == '*' && peek == '/') { get_next_char(); return comment_token;}
                        prev = peek;
                        if (peek == '\n') {character = 0; ++line;}
                    }
                }
                else return factory<Token>(Basic_token(TOKENS::DIVIDE));
            }
        
        case '&':
        case '|':
            {   // scoped because of the variable declaration
                char prev = peek;
                get_next_char();
                if (prev == peek){
                    if (peek == '&') {tok = Basic_token(TOKENS::AND); break;} else {tok = Basic_token(TOKENS::OR); break;}
                }
                else{
                    std::ostringstream error;
                    error << "Line: " << apply_err_fmt(std::to_string(line)) << " character: " << apply_err_fmt(std::to_string(character)) << ": Language does not support bitwise operations\n";
                    throw std::runtime_error{error.str()};
                }
            }
        
        case '(':
            tok = Basic_token(TOKENS::PAREN_OPEN); break;
        case  ')':
            tok = Basic_token(TOKENS::PAREN_CLOSED); break;
        case '{':
            tok = Basic_token(TOKENS::BRACKET_OPEN); break;
        case '}':
            tok = Basic_token(TOKENS::BRACKET_CLOSED); break;
        case '[':
            tok = Basic_token(TOKENS::SQUARE_BRACKET_OPEN); break;
        case ']':
            tok = Basic_token(TOKENS::SQUARE_BRACKET_CLOSED); break;
        case '.':
            tok = Basic_token(TOKENS::DOT); break;
        case '+':
            tok = Basic_token(TOKENS::ADD); break;
        case '-':
            tok = Basic_token(TOKENS::SUBTRACT); break;
        case '*':
            tok = Basic_token(TOKENS::MULTIPLY); break;
        case ';':
            tok = Basic_token(TOKENS::SEMICOLON); break;
        
        default:
        {
            std::ostringstream out;
            out << "Illegal character/token encountered on line: " << apply_err_fmt(std::to_string(line)) << " character: " << apply_err_fmt(std::to_string(character)) << '\n';
            throw std::runtime_error{out.str()};
        }
    }
    // handle all cases which break
    get_next_char();
    return factory<Token>(tok);
}
#endif