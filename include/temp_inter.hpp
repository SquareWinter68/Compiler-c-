// #ifndef INTERMEDIATE_TEMPORARY_HEADER
// #define INTERMEDIATE_TEMPORARY_HEADER

// #include "Tokens.hpp"
// #include "utils.hpp"
// #include <memory>
// #include <sstream>
// #include <string>

// static inline int labels{0};

// void emit_label(long i) {std::cout << "L" << i << ":";}
// void emit(std::string s) {std::cout << "\t" << s << '\n';}
// class Expression;
// using ex_shptr = std::shared_ptr<Expression>;

// class Expression{
//     public:
//         virtual ex_shptr generate() = 0;
//         virtual ex_shptr reduce() = 0;
//         virtual std::string to_string() = 0;
// };


// class constant: public Expression{
//     public:
//         double data;
//         constant(double d): data{d} {}
//         std::string to_string() override{
//             return std::to_string(data);
//         }
//         ex_shptr generate() override{
//             return std::make_shared<constant>(*this);
//         }
//         ex_shptr reduce() override{
//             return std::make_shared<constant>(*this);
//         }

// };

// class Temporary: public Expression{
//     public:
//         Type type;
//         static inline long counter;
//         long number;
//         Temporary(){number = counter++;}
//         std::string to_string() override{
//             return "T" + std::to_string(number);    
//         }
//         ex_shptr generate() override{
//             return factory<Temporary>(*this);
//         }
//         ex_shptr reduce() override{
//             return factory<Temporary>(*this);
//         }
        
// };

// class Arithmetic: public Expression{
//     public:
//         TOKENS operator_;
//         ex_shptr lhs, rhs;
//         Arithmetic(TOKENS tok, ex_shptr ex1, ex_shptr ex2): operator_{tok}, lhs{ex1}, rhs{ex2} {}
//         ex_shptr generate() override{
//             // Both lhs, and rhs, should be reduced to a single adress, if they are not already a single adress
//             // the three adress code for the temporaries should be emited, and the instance of the temporary returned.
//             // From the instance the important thing is the name e.g t1, or t5
//             return factory<Arithmetic>(Arithmetic(operator_, lhs->reduce(), rhs->reduce()));
//         }
        
//         ex_shptr reduce() override{
//             ex_shptr expr = generate();
//             Temporary temp{};
//             emit(temp.to_string() + " = " + expr->to_string());
//             return factory<Temporary>(temp);
//         }
        
//         std::string to_string() override{
//             std::ostringstream out;
//             out << lhs->to_string() << " " << find_string_repr(operator_) << " " << rhs->to_string();
//             return out.str();
//         }
        
// };
// #endif