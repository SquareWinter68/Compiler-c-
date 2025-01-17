
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
std::unordered_map<tac_tokens, std::string> jumps{{tac_tokens::greater_than, "jg"}, {tac_tokens::greater_than_equal, "jge"}, {tac_tokens::less_than, "jl"}, {tac_tokens::less_than_equal, "jle"}, {tac_tokens::equal, "je"},{tac_tokens::not_equal,"jne"}};
std::unordered_map<tac_tokens, std::string> opposite_jumps{{tac_tokens::greater_than, "jle"}, {tac_tokens::greater_than_equal, "jl"}, {tac_tokens::less_than, "jge"}, {tac_tokens::less_than_equal, "jg"}, {tac_tokens::equal, "jne"},{tac_tokens::not_equal,"je"}};

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
std::string get_opp_jump(tac_tokens tok){
    auto it = opposite_jumps.find(tok);
    if (it != opposite_jumps.end()) return it->second;
    else throw std::runtime_error{"mapping from jump to string, no corespnoding jump in map\n"};
}

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
    *out << "\tmovq " << operand_1.offset << "(%rsp), %rax\n";
    *out << "\t"<<get_op(operator_) << " " << operand_2.offset << "(%rsp), %rax\n";
    *out << "\tmovq %rax, " << location.offset << "(%rsp)\n";
}

//id = id op const func
void id_eq_id_op_const(tac_tokens operator_, variable_positions location, variable_positions operand_1, long operand_2, std::ofstream* out){
    *out << "\tmovq " << operand_1.offset << "(%rsp), %rax\n";
    *out << "\t"<<get_op(operator_) << " $" << operand_2 << ", %rax\n";
    *out << "\tmovq %rax, " << location.offset << "(%rsp)\n";
}

void id_eq_id_indexec_by_id(variable_positions location, variable_positions start, variable_positions offset, std::ofstream* out){
    *out << "\tmovq " << start.offset << "(%rsp), %rdi\n";
    *out << "\tmovq " << offset.offset << "(%rsp), %rsi\n";
    *out << "\tmovq (%rdi,%rsi), %rax\n"; 
    *out << "\tmovq %rax " << location.offset << "(%rsp)\n";
}

void id_index_by_id_eq_id(variable_positions start, variable_positions offset, variable_positions location, std::ofstream* out){
    *out << "\tmovq " << location.offset << "(%rsp), %rax\n";
    *out << "\tmovq " << start.offset << "(%rsp), %rdi\n";
    *out << "\taddq " << offset.offset << "(%rsp), %rsi\n";
    *out << "\tmovq %rax, " << "(%rdi,%rsi)\n"; 
}

void emit_unconditional_jump(const std::string& label, std::ofstream* out){
    *out << "\tjmp " << label << '\n';
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

    //  last operand needs to be a register
    // cmp operand_1, operand_2
    //jl operand_2<operand_1
    //jg operand_2>operand_1
    //jle operand_2<=operand_1
    //jge operand_2>=operand_1
    *out << "\tmovq " << operand_2.offset << "(%rsp), %rax\n"; 
    *out << "\tcmp " << operand_1.offset << "(%rsp), %rax\n";
    *out <<"\t" <<get_jump(operator_) << " " << label << '\n';
}

void id_eq_const_op_const(tac_tokens operator_, variable_positions location, long operand_1, long operand_2, std::ofstream* out){
    *out << "\tmovq $" << operand_1 << ", %rax\n";
    *out << "\t"<<get_op(operator_) << " $" << operand_2 << ", %rax\n";
    *out << "\tmovq %rax, " << location.offset << "(%rsp)\n";
}

void id_eq_const(variable_positions location, long operand_1, std::ofstream* out){
    *out << "\tmovq $" << operand_1 << ", " << location.offset << "(%rsp)\n";
}

void if_id_op_const_jmp(tac_tokens operator_, variable_positions operand_1, long operand_2, std::string& label,  std::ofstream* out){
    *out << "\tcmp " << operand_1.offset << "(%rsp), $" << operand_2 << "\n";
    *out << "\t"<<get_jump(operator_) << " " << label << "\n";
    // do the jump
}

void if_const_op_id(tac_tokens operator_, long operand_1, variable_positions operand_2, std::string& label, std::ofstream* out){
    if_id_op_const_jmp(operator_, operand_2, operand_1, label, out);
}

void if_const_op_const(tac_tokens operator_, long operand_1, long operand_2, std::string& label, std::ofstream* out){
    *out << "\tmovq " << operand_1 << "(%rsp), %rax\n";
    *out << "\tcmp " << operand_2 << "(%rsp), " << "%rax\n" ;
    *out << "\t"<<get_jump(operator_) << " " << label << "\n";
    // do the jump
}

void iffalse_id_op_id(tac_tokens operator_, variable_positions operand_1, variable_positions operand2, std::string& label, std::ofstream* out){
    *out << "\tmovq " << operand_1.offset << "(%rsp), %rax\n";
    //*out << "cmp %rax, " << operand2.offset << "(%rsp)\n";
    *out << "\tcmp " << operand2.offset << "(%rsp), " << "%rax\n";
    *out << "\t"<<get_opp_jump(operator_) << " " << label << "\n";
    // do the jump 
}

void iffalse_id_op_const(tac_tokens operator_, variable_positions operand_1, long operand_2, std::string& label, std::ofstream* out){
    *out << "\tmovq " << operand_1.offset << "(%rsp), %rax\n";
    //*out << "cmp %rax, $" << operand_2 << '\n';
    *out << "\tcmp $" << operand_2 << ", %rax\n";
    *out << "\t"<<get_opp_jump(operator_) << " " << label << "\n";
    // do the jump
}

void iffalse_const_op_id(tac_tokens operator_, long operand_1, variable_positions operand_2, std::string& label, std::ofstream* out){
    iffalse_id_op_const(operator_, operand_2, operand_1, label, out);
}

void iffalse_const_op_const(tac_tokens operator_, long operand_1, long operand_2, std::string& label, std::ofstream* out){
    *out << "\tmovq " << operand_1 << "(%rsp), %rax\n";
    //*out << "cmp %rax, " << operand_2 << "(%rsp)\n";
    *out << "\t" << operand_2 << "(%rsp), %rax\n";
    *out << "\t"<<get_opp_jump(operator_) << " " << label << "\n";
    // do the jump
}

void id_eq_id(variable_positions operand_1, variable_positions operand_2, std::ofstream* out){
    // operand 1= operand2
    *out << "\tmovq " << operand_2.offset << "(%rsp), %rax\n";
    *out << "\tmovq %rax, " << operand_1.offset << "(%rsp)\n"; 
}
// int main(){
//     std::ofstream file{"test.txt", std::ios::binary};
//     emit_global_variables(std::vector<std::string>{"a", "tis_b", "boolean"}, file);
//     std::vector<std::string> t;
//     auto test = t.begin();
// }