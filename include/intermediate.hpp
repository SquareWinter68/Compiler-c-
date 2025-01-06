#ifndef INTERMEDIATE_HEADER
#define INTERMEDIATE_HEADER

#include "Lexer.hpp"
#include "Tokens.hpp"
#include "temp_inter.hpp"
#include "utils.hpp"
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

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
        void emit_lbael(long i) {cout << "L" << i << ":";}
        void emit(const std::string& str) {cout << "\t" << str << '\n';}
        long new_label(){return labels++;}
        void error(const std::string& err) {throw std::runtime_error{"Error near line " + std::to_string(lexer_line) + ": " + err};}

};
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

#endif