#ifndef INTERMEDIATE_HEADER
#define INTERMEDIATE_HEADER

#include "Lexer.hpp"
#include "Tokens.hpp"
#include "jumping.hpp"
#include "temp_inter.hpp"
#include "utils.hpp"
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

using std::cout;

class Node{
    /* this class is the most generic version of node, and mainly contains stuff which all nodes share
    This includes emit_label which just emits another label, and emit, which emits code coresponding to 
    the closest above label, this is the reason that it starts with a tab
    */
    static inline long labels{};
    long lexer_line;
    public:
        Node() {}
        Node(const Lexer& lex): lexer_line{lex.line} {}
        static void emit_lbael(long i) {cout << "L" << i << ":";}
        static void emit(const std::string& str) {cout << "\t" << str << '\n';}
        static long new_label(){return labels++;}
        void error(const std::string& err) {throw std::runtime_error{"Error near line " + std::to_string(lexer_line) + ": " + err};}

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
using ex_shptr = std::shared_ptr<Expression>;

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
        int offset;
        std::string lexeme;
        Identifier(int ofst, Type type, std::string lex): Expression{type}, offset{ofst}, lexeme{lex} {}
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
        static inline long counter;
        long number;
        Temporary(Type tp): Expression{tp} {number = counter++;}
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
            emit(type_string_repr(type) + " " + temp.to_string() + " = " + expr->to_string());
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
                emit(type_string_repr(type) + " " + temp.to_string() + " = " + expr->to_string());
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
            emit(type_string_repr(type) + ' ' + temp.to_string());
            jumping(0, false_exit);
            emit(temp.to_string() + " = true");
            emit("goto L" + std::to_string(after));
            Node::emit_lbael(false_exit);
            emit(temp.to_string() + " = false");
            Node::emit_lbael(after);
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
            if (true_exit == 0) Node::emit_lbael(true_exit_or_after);
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
            if (false_exit == 0) Node::emit_lbael(false_exit_of_after);
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

#endif