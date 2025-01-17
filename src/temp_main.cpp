#include "Lexer.hpp"
#include "Lexical_anal.hpp"
#include "Tokens.hpp"
#include "intermediate.hpp"
#include "parse_TAC.hpp"
#include "utils.hpp"
#include <algorithm>
#include <bits/std_thread.h>
#include <cassert>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include "parser.hpp"
#include <cstdlib>
// int main(){
//     using std::cout;
//     Lexer lex{"/home/squarewinter/temp/temp_compiler/test.し++"};
//     TOKENS return_value;
//     auto printer = [&return_value](auto& T) {return_value = T.token;};
//     std::shared_ptr<Token> scan;
//     cout << "Encountered the following tokens\n";
//     do {
//     scan = lex.scan();
//     std::visit(printer, *scan);
//     cout << find_string_repr(return_value) << '\n';
//     }while (return_value != TOKENS::EOF_TOK);

//     // Test Basic types
//     Type integer = types::INTEGER;
//     Type floating_point = types::FLOAT;

//     // test nested array
//     Array int_arr{integer, 5};
//     std::shared_ptr<Array> shptr_arr = std::make_shared<Array>(Array(integer, 5));
//     Array wrapper_arr{shptr_arr, 10};
//     cout << wrapper_arr.to_string() << '\n';
//     cout << "seems that everything worked seemlessly.\n";

//     // test expressoins
//     Arithmetic plus{TOKENS::ADD, factory<constant_temp_use>(constant_temp_use(10, types::FLOAT)), factory<constant_temp_use>(constant_temp_use(5, types::INTEGER))};
//     Arithmetic minus{TOKENS::SUBTRACT, factory<constant_temp_use>(constant_temp_use(5, types::INTEGER)),factory<constant_temp_use>(constant_temp_use(1, types::INTEGER))};
//     Arithmetic complex{TOKENS::MULTIPLY, factory<Arithmetic>(plus), factory<Arithmetic>(minus)};
//     //auto test = plus.generate();
//     plus.emit_lbael(plus.new_label());
//     auto test = complex.generate();
//     Node::emit(test->to_string());
//     //cout << "Huh, let's see " << test->to_string() << '\n';

//     // Test expressions with Identifiers
//     Arithmetic times{TOKENS::MULTIPLY, factory<Identifier>(Identifier(0, types::INTEGER, "a")), factory<Identifier>(Identifier(1, types::INTEGER, "b"))};
//     Arithmetic divide{TOKENS::DIVIDE, factory<Identifier>(Identifier(2, types::INTEGER, "divident")), factory<Identifier>(Identifier(3, types::INTEGER, "devisor"))};
//     Arithmetic complex2{TOKENS::SUBTRACT, factory<Arithmetic>(times), factory<Arithmetic>(divide)};

//     auto test2 = complex2.generate();
//     Node::emit(test2->to_string());

//     // Test expressions with unary operators

//     Unary_operations unary_minus{TOKENS::UNARY_MINUS, factory<Identifier>(Identifier(5, types::INTEGER, "hello_there"))};
//     Arithmetic complex3{TOKENS::DIVIDE, factory<Arithmetic>(times), factory<Unary_operations>(unary_minus)};

//     auto test3 = complex3.generate();
//     Node::emit(test3->to_string());

//     // test expressions with jumping code

//     And log_and{TOKENS::AND, factory<Constant<bool>>(Constant<bool>(true)), factory<Constant<bool>>(Constant<bool>(true))};

//     cout << "\n\n\n";
//     auto test_4 = log_and.generate();
//     //Node::emit(test_4->to_string());

//     Relational rel{TOKENS::GREATER_THAN, factory<Constant<int>>(42), factory<Constant<int>>(11)};
//     auto test_5 = rel.generate();
//     //Node::emit(test_5->to_string());

//     cout << "Testing array\n\n";
//     // test array acess 
//     Array arr{types::INTEGER, 10};
//     Identifier id{0, factory<Array>(arr), "array_varaible"};
//     Array_acess aa{factory<Identifier>(id), factory<Arithmetic>(complex), types::INTEGER};
//     auto test_6 = aa.generate();
//     Node::emit(test_6->to_string());

// }

using std::string_literals::operator""s;

int main(){
    //std::ofstream out_file{"out.inter", std::ios::binary};
    auto lexer =  std::make_shared<Lexer>(Lexer("/home/squarewinter/temp/temp_compiler/tests/loops_test.し++"));
        // TOKENS return_value;
        // auto printer = [&return_value](auto& T) {return_value = T.token;};
        // std::shared_ptr<Token> scan;
        // cout << "Encountered the following tokens\n";
        // do {
        // scan = lexer->scan();
        // std::visit(printer, *scan);
        // cout << find_string_repr(return_value) << '\n';
        // }while (return_value != TOKENS::EOF_TOK);
    lexer = std::make_shared<Lexer>(Lexer("/home/squarewinter/temp/temp_compiler/tests/loops_test.し++"));
    auto var = program(lexer);
    auto begin = Statement::new_label(), after = Statement::new_label();
    Statement::emit_label(begin); 
    
    if (var) var->generate(begin, after); 
    else cout << "program contains zero statements\n";
    Statement::emit_label(after);
    auto instruction_vector = Node::vector;
    auto printer_lambda = [](std::string t){cout << t;};
    cout << "Did we successfully steal the vector from Node\n";
    //std::for_each(vec.begin(), vec.end(), printer_lambda);
    auto t = get_scope();
    std::vector<variable_positions> vars;
    t->get_identifiers(vars);
    auto temps = Node::temps;
    cout << "Printing variables\n";
    auto var_printer = [](const variable_positions& v){cout << v.lexeme  << " starts at: " << v.offset << " bytes" << '\n';};
    auto temp_printer = [](std::shared_ptr<Temporary> temp){cout << temp->to_string() << "\n";};
    unsigned long long offset{};
    for (auto var : vars) {
        var.offset = offset;
        offset += var.byte_width;
    }
    for(auto temp: temps){
        vars.push_back(variable_positions{temp->to_string(), (long)offset, 0});
        offset += get_type_width(temp->type);
    }
    std::for_each(vars.begin(), vars.end(), var_printer);
    
    std::ofstream output_file{"し++.inter", std::ios::binary};
    auto file_writer = [&output_file](std::string& elem){output_file << elem;};
    std::for_each(instruction_vector.begin(), instruction_vector.end(), file_writer);
    output_file.flush();
    Tac_parser tac_parser{"し++.inter"};
    //tac_parser.init("し++.inter"s);
    //tac_parser.scan();
    std::vector<variable_positions> varibales_;
    std::ofstream asm_output_file{"し++.s", std::ios::binary};
    init_tac_parser(vars, asm_output_file, &tac_parser);
   // tac_parser.scan();
    generate_asm(offset);
    asm_output_file.flush();
    std::string command{"gcc -g -o し++.out し++.s"};
    system(command.c_str());
}