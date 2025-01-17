
#include <algorithm>
#include <concepts>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "intermediate.hpp"
#include "parse_TAC.hpp"

std::unordered_map<tac_tokens, std::string> tok_str_mappint{{tac_tokens::add, "addq"}, {tac_tokens::sub, "subq"}, {tac_tokens::mult, "imulq"}, {tac_tokens::div, "idivq"}};
std::unordered_map<tac_tokens, std::string> jumps{{tac_tokens::greater_than, "jg"}, {tac_tokens::greater_than_equal, "jge"}, {tac_tokens::less_than, "jl"}, {tac_tokens::less_than_equal, "jle"}, {tac_tokens::equal, "je"}};

std::string get_op(tac_tokens tok){
    auto it = tok_str_mappint.find(tok);
    if (it != tok_str_mappint.end()) return it->second;
    else throw std::runtime_error{"mapping from operator to string, operator not in map\n"};
}
std::string get_jump(tac_tokens tok){
    auto it = jumps.find(tok);
    if (it != jumps.end()) return it->second;
    else throw std::runtime_error{"mapping from jump to string, no corespnoding jump in map\n"};
}
template<typename T>
concept string_container = requires(T a){
    {a.begin()} -> std::same_as<typename T::iterator>;
    {a.end()} -> std::same_as<typename T::iterator>;
    requires std::same_as<typename T::value_type, std::string>;
};

//cuz i needed to bind to both l and rvalue references
template<string_container T>
void emit_global_variables(T&& vars, std::ofstream& out){
    out << ".section .data\n";
    auto var_emitter = [&out](const std::string& var){out << ".global " << var << '\n';};
    std::for_each(vars.begin(), vars.end(), var_emitter);
}

void bootstrap_function(long offset){

}

void id_eq_id_op_id(tac_tokens operator_, variable_positions location, variable_positions operand_1, variable_positions operand_2, std::ofstream* out){
    *out << "movq " << operand_1.offset << "(%rsp), %rax\n";
    *out << get_op(operator_) << " " << operand_2.offset << "(%rsp), %rax\n";
    *out << "movq %rax, " << location.offset << "(%rsp)\n";
}

//id = id op const func
void id_eq_id_op_const(tac_tokens operator_, variable_positions location, variable_positions operand_1, long operand_2, std::ofstream* out){
    *out << "movq " << operand_1.offset << "(%rsp), %rax\n";
    *out << get_op(operator_) << " $" << operand_2 << ", %rax\n";
    *out << "movq %rax, " << location.offset << "(%rsp)\n";
}

void id_eq_id_indexec_by_id(variable_positions location, variable_positions start, variable_positions offset, std::ofstream* out){
    *out << "movq " << start.offset << "(%rsp), %rax\n";
    *out << "addq " << offset.offset << "(%rsp), %rax\n";
    *out << "movq %rax, " << location.offset << "(%rsp)\n"; 
}

void emit_unconditional_jump(const std::string& label, std::ofstream* out){
    *out << "jmp " << label << '\n';
}

void emit_label(const std::string& label, std::ofstream* out){
    *out << label << '\n';
}

void if_id_op_id_jmp(tac_tokens operator_, variable_positions operand_1, variable_positions operand_2, std::string& label, std::ofstream* out){
    // cmp src, dest = dest - src
    // cmp a, b = b-a
    // b-a = 0 => b = a
    // b-a < 0 => a > b
    // b-a > 0 => b > a
    *out << "cmpq " << operand_1.offset << "(%rsp), " << operand_2.offset << "(%rsp)\n";
    *out << get_jump(operator_) << " " << label << '\n';
}

void id_eq_const_op_const(tac_tokens operator_, variable_positions location, long operand_1, long operand_2, std::ofstream* out){
    *out << "movq $" << operand_1 << ", %rax\n";
    *out << get_op(operator_) << " $" << operand_2 << ", %rax\n";
    *out << "movq %rax, " << location.offset << "(%rsp)\n";
}

void id_eq_const(variable_positions location, long operand_1, std::ofstream* out){
    *out << "movq $" << operand_1 << ", " << location.offset << "(%rsp)\n";
}

void if_id_op_const_jmp(tac_tokens operator_, variable_positions operand_1, long operand_2, std::string& label,  std::ofstream* out){
    *out << "cmpq " << operand_1.offset << "(%rsp), $" << operand_2 << "\n";
    *out << get_jump(operator_) << " " << label << '\n';
}
// int main(){
//     std::ofstream file{"test.txt", std::ios::binary};
//     emit_global_variables(std::vector<std::string>{"a", "tis_b", "boolean"}, file);
//     std::vector<std::string> t;
//     auto test = t.begin();
// }