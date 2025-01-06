#include <concepts>
#include<iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include "Lexer.hpp"
#include "Tokens.hpp"
#include "Lexical_anal.hpp"
struct Value{
    int int_member;
    std::string string_member;
};

template<typename T>
concept printable = requires(T a){
    {a.to_string()} -> std::same_as<std::string>;
};


template<printable... Args>
void print(Args&&... args){
    (std::cout << ... << args.to_string()) << '\n';
}

int was_main(){
    using std::cout;
    auto token_test = TOKENS::WHILE;
    cout << (int)token_test << "The value of the token test, tada! \n";
    auto ans = token_string_map.find(token_test);
    if (ans != token_string_map.end()){
        cout << "The value of the token is " << ans->second << "\n";
    }

    Basic_token bt{TOKENS::WHILE};
    Token basic_tok{bt};
    Value basic_tok_val{42, "Hello there"};
    
    Word wt{TOKENS::IDENTIFIER, "naacho"};
    Token word_token{wt};
    Value word_token_value{11, "General kenobi"};

    Number_integer nit{42, TOKENS::INT};
    Token number_integer_token{nit};
    Value number_integer_token_value{14, "Heart of gold"};

    Number_float nft{1.0/137, TOKENS::FLOAT};
    Token number_float_token{nft};
    Value number_float_token_value{15, "Infinite improbability drive"};

    String_literal slt{"Pandimensional mice", TOKENS::STRING_LITERAL};
    Token string_literal_token{slt};
    Value string_literal_token_value{92, "Arthur Dent"};

    std::unordered_map<Token, Value, token_hash> map;
    map.insert({basic_tok, basic_tok_val});
    map.insert({word_token, word_token_value});
    map.insert({number_integer_token, number_integer_token_value});
    map.insert({number_float_token, number_float_token_value});
    map.insert({string_literal_token, string_literal_token_value});
    
    print(bt, wt, nit, nft, slt);

    Lexer lex{};
    cout << "\nThis is cool\n";
    if (auto word = lex.get_word("true")){
        cout << "Correction, mega cool\n";
        //print(*word);
    }

}