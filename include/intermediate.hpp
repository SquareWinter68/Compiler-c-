#ifndef INTERMEDIATE_HEADER
#define INTERMEDIATE_HEADER

#include "Lexer.hpp"
#include "Lexical_anal.hpp"
#include "Tokens.hpp"
#include "utils.hpp"
#include <concepts>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <iostream>
#include <vector>
#include "code_gen_structures.hpp"

using std::cout;
class Temporary;
class Node{
    /* this class is the most generic version of node, and mainly contains stuff which all nodes share
    This includes emit_label which just emits another label, and emit, which emits code coresponding to 
    the closest above label, this is the reason that it starts with a tab
    */
    static inline long labels{1};
    long lexer_line;
    public:
        Node() {}
        Node(const Lexer& lex): lexer_line{lex.line} {}
        inline static std::vector<std::string> vector{};
        inline static std::vector<std::shared_ptr<Temporary>> temps;
        static void emit_label(long i) {vector.emplace_back("L" + std::to_string(i) + ":");cout << "L" << i << ":";}
        static void emit(const std::string& str) {vector.emplace_back("\t" + str + '\n');cout << "\t" << str << '\n';}
        static long new_label(){return labels++;}
        void error(const std::string& err) {throw std::runtime_error{"Error near line " + std::to_string(lexer_line) + ": " + err + '\n'};}

};

inline void jump_if_test(const std::string& test, long true_exit, long false_exit){
    // if false is fallthrough, aka in case of if without else
    /*
        if (test){
            true_exit code
        }
        false_exit code

        if (test){
            true_exit code
        }
        else {
            flase_exit code
        }

        if (!(test)){
            false_exit code
        }
        true_exit code
    */
    if (true_exit && false_exit){
        Node::emit("if " + test + " goto L" + std::to_string(true_exit));
        Node::emit("goto L" + std::to_string(false_exit));
    }
    else if (true_exit){
        // This means the false exit is fallthrough
        Node::emit("if " + test + " goto L" + std::to_string(true_exit));
    }
    else if (false_exit) {
        // this means that true exit if fallthrough, aka the first if above
        // Jump to false exit if the expression evaluates to false
        Node::emit("iffalse " + test + " goto L" + std::to_string(false_exit));
    }
    else throw std::runtime_error{"Error in jump_if_test, both ture and false exit are fallthrough\n"};
}

class Expression;
class Statement;
using ex_shptr = std::shared_ptr<Expression>;
using st_shptr = std::shared_ptr<Statement>;

class Expression: public Node{
    public:
        /* every expression must have a type
        coercions happen later down the line, the larger type always gets chosen this is
        for the obvious reason of avoiding narrowing type conversions*/
        Type type;
        Expression(Type tp): type{tp} {}
        /* default constructor, since the type needs to be calculated in the Arithmetic subtype
        and the type calculating function retruns std::optional hence the argument cannot
        be provided at instantiation time without the fear of unhandled exceptions
        */
        Expression() {}
        virtual ex_shptr generate() = 0;
        virtual ex_shptr reduce() = 0;
        virtual std::string to_string() = 0;
        virtual void jumping(long true_exit, long false_exit) {/*Delibaratly left blank, as a defalut implementation*/}
        virtual ~Expression(){}
};

class constant_temp_use: public Expression{
    public:
        double data;
        constant_temp_use(double d, Type tp): Expression{tp}, data{d} {}
        std::string to_string() override{
            return std::to_string(data);
        }
        ex_shptr generate() override{
            return factory<constant_temp_use>(*this);
        }
        ex_shptr reduce() override{
            return factory<constant_temp_use>(*this);
        }
};

class Identifier: public Expression{
    public:
        long offset;
        std::string lexeme;
        Identifier(long ofst, Type type, std::string lex): Expression{type}, offset{ofst}, lexeme{lex} {}
        std::string to_string() override{
            return lexeme;
        }
        ex_shptr generate() override{
            return factory<Identifier>(*this);
        }
        ex_shptr reduce() override{
            return factory<Identifier>(*this);
        }
};

class Temporary: public Expression{
    public:
        static inline long counter{1};
        long number;
        Temporary(Type tp): Expression{tp} {number = counter++; temps.push_back(factory<Temporary>(*this));}
        std::string to_string() override{
            return "T" + std::to_string(number);    
        }
        ex_shptr generate() override{
            return factory<Temporary>(*this);
        }
        ex_shptr reduce() override{
            return factory<Temporary>(*this);
        }
        
};

class Arithmetic: public Expression{
    public:
        TOKENS operator_;
        ex_shptr lhs, rhs;
        Arithmetic(TOKENS tok, ex_shptr ex1, ex_shptr ex2): operator_{tok}, lhs{ex1}, rhs{ex2} {
            // handle the type checking
            if (auto res = max_type(lhs->type, rhs->type)){
                type = *res;
            }
            else
                throw std::runtime_error{"In Arithmetic expression: \nBad type, function max_type returned empty object, all arithmetic expressoins must be typed\n"};
        }
        ex_shptr generate() override{
            // Both lhs, and rhs, should be reduced to a single adress, if they are not already a single adress
            // the three adress code for the temporaries should be emited, and the instance of the temporary returned.
            // From the instance the important thing is the name e.g t1, or t5
            return factory<Arithmetic>(Arithmetic(operator_, lhs->reduce(), rhs->reduce()));
        }
        
        ex_shptr reduce() override{
            ex_shptr expr = generate();
            Temporary temp{type};
            Node::emit(temp.to_string() + " = " + expr->to_string());
            return factory<Temporary>(temp);
        }
        
        std::string to_string() override{
            std::ostringstream out;
            out << lhs->to_string() << " " << find_string_repr(operator_) << " " << rhs->to_string();
            return out.str();
        }
        
};

class Unary_operations: public Expression{
    public:
        TOKENS operator_;
            // no left and right hand side, since this is a unary operator, which by definition
            // applies to only one operand, hence expression
            ex_shptr expression;
            Unary_operations(TOKENS tok, ex_shptr expr): operator_{tok}, expression{expr} {
                if (auto res = max_type(expression->type, types::FLOAT)){
                    type = *res;
                }
                else
                    throw std::runtime_error{"In Unary_operaions expression, bad type\n"};
            }
            ex_shptr generate() override{
                return factory<Unary_operations>(Unary_operations(operator_, expression->reduce()));
            }
            ex_shptr reduce() override{
                ex_shptr expr = generate();
                Temporary temp{type};
                emit(temp.to_string() + " = " + expr->to_string());
                return factory<Temporary>(temp);
            }
            std::string to_string() override{
                return find_string_repr(operator_) + " " + expression->to_string();
            }
};

// this class should handle constants, maybe even string literals, well see
template<Constant_types T>
class Constant: public Expression{
    public:
        T value;
        Constant(T val, Type type): Expression{type}, value{val} {}
        Constant(int val): Expression{types::INTEGER}, value{val} {}
        Constant(float val): Expression{types::FLOAT}, value{val} {}
        Constant(bool val): Expression{types::BOOL}, value{val} {}
        std::string to_string() override{
            return type_to_string(value);
        }
        ex_shptr generate() override{
            return factory<Constant<T>>(*this);
        }
        ex_shptr reduce() override{
            return factory<Constant<T>>(*this);
        }
        void jumping(long true_exit, long false_exit) override{
            if (std::is_same_v<T, bool>){
                if (value == true && true_exit != 0) emit("goto L" + std::to_string(true_exit));
                else if (value == false && false_exit != 0) emit("goto L" + std::to_string(false_exit));
            }
            else throw std::runtime_error{"jumping called on non-boolean, constant Constant class\n"};
        }
};

class Logical: public Expression{
    public:
        TOKENS operator_;
        ex_shptr lhs, rhs;
        Logical(TOKENS tok, ex_shptr ex1, ex_shptr ex2): operator_{tok}, lhs{ex1}, rhs{ex2} {
            auto res = is_bool(ex1->type, ex2->type);
            if (res && *res == types::BOOL){
                type = types::BOOL;
            }
            else throw std::runtime_error{"Error in Logical, types are not boolean\n"};
        }
        // Constructor overload for not
        Logical(TOKENS tok, ex_shptr expression): operator_(tok), lhs{expression}{
            auto res = is_bool(expression->type, expression->type);
            if (res && *res == types::BOOL){
                type = types::BOOL;
            }
            else throw std::runtime_error{"Error in Logical, (Constructor overload for not), type is not boolean\n"};
        }
        Logical(TOKENS tok, ex_shptr ex1, ex_shptr ex2, std::function<std::optional<Basic_type> (Type, Type)> predicate): operator_(tok), lhs{ex1}, rhs{ex2}{
            // This overload is used for derived classes which want to supply their own check operation above
            auto result = predicate(lhs->type, rhs->type);
            if (result){
                type = *result;
            }
            else throw std::runtime_error{"Error, in lambda overload of predicate, gdb is your best option\n"};

        }
        ex_shptr generate() override{
            /* after is emited after the end of the false exit, so at the end of the true
             exit one must jump to after to avoid having the controll fall through to the false exit
            Without this jump code would go through both paths of the branch regardles the result of the test 
            */
            long false_exit = Node::new_label(), after = Node::new_label();
            Temporary temp{type};
            emit(temp.to_string());
            jumping(0, false_exit);
            emit(temp.to_string() + " = true");
            emit("goto L" + std::to_string(after));
            Node::emit_label(false_exit);
            emit(temp.to_string() + " = false");
            Node::emit_label(after);
            return factory<Temporary>(temp);
        }
        std::string to_string() override{
            std::ostringstream out;
            out << lhs->to_string() << " " << find_string_repr(operator_) << " " << rhs->to_string();
            return out.str();
        }
};

class Or: public Logical{
    public:
        Or(TOKENS tok, ex_shptr ex1, ex_shptr ex2): Logical(tok, ex1, ex2) {}
        void jumping(long true_exit, long false_exit) override{
            /* call jumping on first expression, with fallthrough on false
            This is because either one of the arguments has to evalueate as true for
            the expression to be true
            
            However if the first expression evaluates to true, the latter part should be skiped
            The problem arises if this function was called with true exit as 0, which is the fallthrough
            value, therfore this case must be handled by generating a labe to which the code should jump
            should the first expression evalueate to false.

            If it was called with non-zero true and false exits however, there is no need to generate
            and emit this synthetic variable after.
            */
            long true_exit_or_after = true_exit != 0? true_exit:Node::new_label();
            lhs->jumping(true_exit_or_after, 0); // If true, jump to end, else test other argument
            /* this works if a true exit is provided for obvious reasons, but it also works if the ture_exit is 0/fallthrough
            because in case of true_exit being 0 the below function falls through to the after label, which gets emited
            to signal the end of branching instructions.
            */
            rhs->jumping(true_exit, false_exit); 
            if (true_exit == 0) Node::emit_label(true_exit_or_after);
        }
        ex_shptr reduce() override{
            return factory<Or>(*this);
        }
};

class And: public Logical{
    public:
        And(TOKENS tok, ex_shptr ex1, ex_shptr ex2): Logical(tok, ex1, ex2) {}
        void jumping(long true_exit, long false_exit) override{
            /*The first expression needs a fallthrough case on the true exit, this is because
            both the subexpression tests must evaluate to true for the expression to evalueate to true.
            In case the fall-through parameter 0 is passed for the false exit, the synthetic label after
            must be provided by this function, this serves the purpose of early termination, in case the first
            operand/expression evalueates as false;
            */
            long false_exit_of_after = false_exit != 0? false_exit:Node::new_label();
            // falls through to check second operand, both must evalueate to true
            lhs->jumping(0, false_exit_of_after);
            // as mentioned above this works both when false_exit is 0 and non-zero
            rhs->jumping(true_exit, false_exit);
            if (false_exit == 0) Node::emit_label(false_exit_of_after);
        }
        ex_shptr reduce() override{
            return factory<And>(*this);
        }
};

class Not: public Logical{
    public:
        Not(TOKENS tok, ex_shptr expression): Logical(tok, expression) {}
        void jumping(long true_exit, long false_exit) override{
            // this should reverse the true and false exits
            lhs->jumping(false_exit, true_exit);
        }
        std::string to_string() override{
            return find_string_repr(operator_) + " " + lhs->to_string();
        }
        ex_shptr reduce() override{
            return factory<Not>(*this);
        }
};

class Relational: public Logical{
    public:
        Relational(TOKENS tok, ex_shptr ex1, ex_shptr ex2): Logical{tok, ex1, ex2, [](Type t1, Type t2)->std::optional<Basic_type>{
            if (std::holds_alternative<Array_type>(t1) || std::holds_alternative<Array_type>(t2)) return {};
            if (std::get<Basic_type>(t1) == std::get<Basic_type>(t2)) return types::BOOL;
            // leaving room for type coersions later down the line
        }} {}

        void jumping(long true_exit, long false_exit) override{
            ex_shptr left = lhs->reduce(), right = rhs->reduce();
            std::string test{left->to_string() + ' ' + find_string_repr(operator_) + ' ' + right->to_string()};
            jump_if_test(test, true_exit, false_exit);
        }
        ex_shptr reduce() override{
            return factory<Relational>(*this);
        }
};

class Array_acess: public Expression{
    public:
        /* The array_identifier is an identifier representing an array, and the index expression is either a constant, or an expression
         which gets reduced, so that the array can be accessed.*/
        ex_shptr array_identifier;
        ex_shptr index_expression;
        /* type is the underlying type of the array, that is to say, if the array is nested arbitraraly deeply, type refers to the 
        most deeply nested array (in other word type refers to the array at which the nesting ends, the one which contains a basic type)*/
        Array_acess(ex_shptr array_id, ex_shptr index, Type type): Expression{type}, array_identifier{array_id}, index_expression{index} {} 
        ex_shptr generate() override{
            return factory<Array_acess>(Array_acess(array_identifier, index_expression->reduce(), type));
        }
        ex_shptr reduce() override{
            ex_shptr expr = generate();
            Temporary temp{type};
            emit(temp.to_string() + " = " + expr->to_string());
            return factory<Temporary>(temp);
        }
        void jumping(long true_exit, long false_exit) override{
            jump_if_test(reduce()->to_string(), true_exit, false_exit);
        }
        std::string to_string() override{
            return array_identifier->to_string() + " [ " + index_expression->to_string() + " ] ";
        }

};

// Intermediate code for statements, resides under

class Statement: public Node{
    public:
        /* holds the value of the parent block, useful for break the parser sets this variable */
        inline static st_shptr Enclosing_block{nullptr};
        // optional does probably not need to be excplicitly initialized to be empty
        Statement() {}
        // begin obviously points to begin, after points to the label after the statement has finished
        virtual void generate(long begin_stmt, long after_stmt){}
        long after = 0;
        // loops save their after_stmt variable in the above after

        virtual ~Statement() {}
};

class If: public Statement{
    public:
        // what the if tests for
        ex_shptr test_expression;
        // what happens should the test result in ture
        st_shptr statement;
        If(ex_shptr test, st_shptr stmt): test_expression{test}, statement{stmt} {
            if (!is_bool(test->type)) throw std::runtime_error{"Cannot have a test expression which is not of type bool in if statement\n"};
        }
        void generate(long begin_stmt, long after_stmt) override{
            long stmt_label = Node::new_label();
            // if ture, fall through to statement, else jump to first insturction after statement
            test_expression->jumping(0, after_stmt);
            // start of statement
            Node::emit_label(stmt_label);
            statement->generate(stmt_label, after_stmt);
        }
};

class Else: public Statement{
    public:
        ex_shptr test_expression;
        st_shptr if_stmt, else_stmt;
        Else(ex_shptr test, st_shptr if_, st_shptr else_): test_expression{test}, if_stmt{if_}, else_stmt{else_} {
            if (!is_bool(test->type)) throw std::runtime_error{"Cannot have a test expression which is not of type bool in if-else statement\n"};
        }
        void generate(long begin_stmt, long after_stmt) override{
            long if_stmt_label = Node::new_label();
            long else_stmt_label = Node::new_label();
            // fall through on ture, jump to else on false
            test_expression->jumping(0, else_stmt_label);
            Node::emit_label(if_stmt_label); if_stmt->generate(if_stmt_label, after_stmt);
            // since we at this point fell through to the we should jump to after, once the if has evaluated
            Node::emit("goto L" + std::to_string(after_stmt));
            
            // Code for else, starting with label for else statement
            Node::emit_label(else_stmt_label); else_stmt->generate(else_stmt_label, after_stmt);
        }
};

class While: public Statement{
    public:
        // loop test
        ex_shptr test_expression{nullptr};
        // statement which gets executed in case the loop test evaluates to true
        st_shptr statement{nullptr};
        void check_type(ex_shptr test){if (!is_bool(test->type)) throw std::runtime_error{"Cannot have a test expression which is not of type bool in a while statement statement\n"};}
        void setup(ex_shptr test, st_shptr stmt){check_type(test); test_expression = test; statement = stmt;}
        While(){}
        While(ex_shptr test, st_shptr stmt): test_expression{test}, statement{stmt} {check_type(test);}
        void generate(long begin_stmt, long after_stmt) override{
            // saved so that break can jump to this
            after = after_stmt;
            long statement_label = Node::new_label();
            // if loop fails, jump to end of statement, else go through statement
            test_expression->jumping(0, after_stmt);
            Node::emit_label(statement_label);
            statement->generate(statement_label, begin_stmt);
            // loop, go back to top
            Node::emit("goto L" + std::to_string(begin_stmt));
        }
};

class Do_while: public Statement{
    public:
        ex_shptr test_expression{nullptr};
        st_shptr statement{nullptr};
        void check_type(ex_shptr test){if (!is_bool(test->type)) throw std::runtime_error{"Cannot have a test expression which is not of type bool in a do-while statement statement\n"};}
        void setup(ex_shptr test, st_shptr stmt){check_type(test); test_expression = test; statement = stmt;}
        Do_while() {}
        Do_while(ex_shptr test, st_shptr stmt): test_expression{test},  statement{stmt} {check_type(test);}
        void generate(long begin_stmt, long after_stmt) override{
            after = after_stmt;
            long expression_label = Node::new_label();
            statement->generate(begin_stmt, expression_label);
            Node::emit_label(expression_label);
            // if false, fall through, else loop/ jump back to start
            test_expression->jumping(begin_stmt, 0);
        }

};

class For: public Statement{
    public:

};

class Assign: public Statement{
    public:
        // id is what the expression is assignet to
        Identifier identifier; ex_shptr value;
        
        Assign(Identifier id, ex_shptr val): identifier{id}, value{val} {
            if (is_numeric_type(id.type) && is_numeric_type(val->type)) {}
            else if (is_bool(id.type) && is_bool(val->type)) {}
            else throw std::runtime_error{"Error Assign: The identifier, and type do not match\n"};
        }
    void generate(long begin_stmt, long after_stmt) override{
        Node::emit(identifier.to_string() + " = " + value->generate()->to_string());
    }
};

class Assign_array: public Statement{
    public:
        // Array is an identifier to which we assign, index is where in the array we assign, and the value is what we assign
        ex_shptr array_id, index, value;
        Assign_array(Array_acess acess, ex_shptr val): array_id{acess.array_identifier}, index{acess.index_expression}, value{val} {
            auto type1 = std::get_if<Array_type>(&acess.type), type2 = std::get_if<Array_type>(&val->type);
            if (type1 || type2) throw std::runtime_error{"Error Assign_array: One of the types, is an Array_type, cannot assign an array, and cannot assign to an arr[array]\n"};
            //else if (std::get<Basic_type>(acess->type) == std::get<Basic_type>(val->type)) {}
            else if (is_numeric_type(acess.type) && is_numeric_type(val->type)) {}
            else throw std::runtime_error{"Error Assign_array: The identifier, and type do not match\n"};
        }
        void generate(long begin_stmt, long after_stmt) override{
            // handles assignment to an array, should be obvious
            emit(array_id->to_string() + " [ " + index->reduce()->to_string() + " ] = " + value->reduce()->to_string());
        }
};

/* this class is represents a subtree of the tree holding arbitraraly many statements, i would have called it block, but a block can contain expressions and
declarations as well, but it is pretty much like a block, but for statements only, hence the name statements */

class Statements: public Statement{
    public:
        st_shptr statement1, statement2;
        Statements(){}
        Statements(st_shptr stmt1, st_shptr stmt2): statement1{stmt1}, statement2{stmt2} {}
        void generate(long begin_stmt, long after_stmt) override{
            if (statement1 && statement2){
                long label = Node::new_label();
                statement1->generate(begin_stmt, label);
                Node::emit_label(label);
                statement2->generate(label, after_stmt);
            }
            else if (statement1) statement1->generate(begin_stmt, after_stmt);
            else if (statement2) statement2->generate(begin_stmt, after_stmt);
        }
};

class Break: public Statement{
    public:
        // most likely a loop
        st_shptr parent_statement;
        Break(): parent_statement{Statement::Enclosing_block} {
            if (!(Statement::Enclosing_block)) Node::error("Break without loop enclosing it");
        }
        Break(st_shptr enclosing_block): parent_statement{enclosing_block} {}
        void generate(long begin_stmt, long after_stmt) override{
            Node::emit("goto L" + std::to_string(parent_statement->after));
        }
};

// scopes
class Scope{
    public:
        std::shared_ptr<Scope> parent;
        std::vector<std::shared_ptr<Scope>> children;
        std::unordered_map<Word, Identifier, token_hash, Word_equal> lexeme_to_id;
        Scope(): parent{nullptr} {}
        Scope(std::shared_ptr<Scope> scope): parent{scope} {}
        void put(Word word, Identifier id) {lexeme_to_id.insert({word, id});}
        // std::optional<Identifier> get(Word word) {
        //     std::shared_ptr<Scope> current = factory<Scope>(*this);
        //     if (current){
        //         do {
        //         auto it = lexeme_to_id.find(word);
        //         if (it != current->lexeme_to_id.end()) return it->second;
        //         current = current->parent;

        //         }while (current);
        //         return {};
        //     }
        //     return {};
        // }
        std::optional<Identifier> get(Word word) {
            Scope* current = this; // Start with the current scope
            while (current) {
                auto it = current->lexeme_to_id.find(word); // Use the current scope's map
                if (it != current->lexeme_to_id.end()) {
                    return it->second; // Found the identifier
                }
                current = current->parent.get(); // Move to the parent scope
            }
            return {}; // Not found
        }
        void printIdentifiers() {
            // Print identifiers in the current scope
            for (const auto& pair : lexeme_to_id) {
                // Assuming Identifier has a way to be printed, e.g., an overloaded operator<<
                std::cout << type_string_repr(pair.second.type) << " " <<  pair.second.lexeme  << '\n'; // Replace with actual printing logic
            }   
            // Recursively print identifiers of children
            for (const auto& child : children) {
                child->printIdentifiers();
            }
        }
        void get_identifiers(std::vector<variable_positions>& out){
            for (const auto& pair: lexeme_to_id){
                out.push_back({pair.second.lexeme, pair.second.offset, get_type_width(pair.second.type)});
            }
            for (const auto& child: children){
                child->get_identifiers(out);
            }
        }

};

#endif