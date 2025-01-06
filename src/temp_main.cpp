#include "Lexer.hpp"
#include "Lexical_anal.hpp"
#include "Tokens.hpp"
#include "intermediate.hpp"
#include "temp_inter.hpp"
#include "utils.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <variant>
int main(){
    using std::cout;
    Lexer lex{"/home/squarewinter/temp/temp_compiler/test.し++"};
    TOKENS return_value;
    auto printer = [&return_value](auto& T) {return_value = T.token;};
    std::shared_ptr<Token> scan;
    cout << "Encountered the following tokens\n";
    do {
    scan = lex.scan();
    std::visit(printer, *scan);
    cout << find_string_repr(return_value) << '\n';
    }while (return_value != TOKENS::EOF_TOK);

    // Test Basic types
    Type integer = types::INTEGER;
    Type floating_point = types::FLOAT;

    // test nested array
    Array int_arr{integer, 5};
    std::shared_ptr<Array> shptr_arr = std::make_shared<Array>(Array(integer, 5));
    Array wrapper_arr{shptr_arr, 10};
    cout << wrapper_arr.to_string() << '\n';
    cout << "seems that everything worked seemlessly.\n";

    // test expressoins
    Arithmetic plus{TOKENS::ADD, factory<constant_temp_use>(constant_temp_use(10, types::FLOAT)), factory<constant_temp_use>(constant_temp_use(5, types::INTEGER))};
    Arithmetic minus{TOKENS::SUBTRACT, factory<constant_temp_use>(constant_temp_use(5, types::INTEGER)),factory<constant_temp_use>(constant_temp_use(1, types::INTEGER))};
    Arithmetic complex{TOKENS::MULTIPLY, factory<Arithmetic>(plus), factory<Arithmetic>(minus)};
    //auto test = plus.generate();
    plus.emit_lbael(plus.new_label());
    auto test = complex.generate();
    test->emit(test->to_string());
    //cout << "Huh, let's see " << test->to_string() << '\n';

    // Test expressions with Identifiers
    Arithmetic times{TOKENS::MULTIPLY, factory<Identifier>(Identifier(0, types::INTEGER, "a")), factory<Identifier>(Identifier(1, types::INTEGER, "b"))};
    Arithmetic divide{TOKENS::DIVIDE, factory<Identifier>(Identifier(2, types::INTEGER, "divident")), factory<Identifier>(Identifier(3, types::INTEGER, "devisor"))};
    Arithmetic complex2{TOKENS::SUBTRACT, factory<Arithmetic>(times), factory<Arithmetic>(divide)};

    auto test2 = complex2.generate();
    test2->emit(test2->to_string());

    // Test expressions with unary operators

    Unary_operations unary_minus{TOKENS::UNARY_MINUS, factory<Identifier>(Identifier(5, types::INTEGER, "hello_there"))};
    Arithmetic complex3{TOKENS::DIVIDE, factory<Arithmetic>(times), factory<Unary_operations>(unary_minus)};

    auto test3 = complex3.generate();
    test3->emit(test3->to_string());

}