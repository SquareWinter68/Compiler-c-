#ifndef GENERATOR_HEADER
#define GENERATOR_HEADER
#include "intermediate.hpp"
#include "parse_TAC.hpp"

void id_eq_id_op_id(tac_tokens operator_, variable_positions location, variable_positions operand_1, variable_positions operand_2, std::ofstream* out);
void id_eq_id_op_const(tac_tokens operator_, variable_positions location, variable_positions operand_1, long operand_2, std::ofstream* out);
void id_eq_id_indexec_by_id(variable_positions location, variable_positions start, variable_positions offset, std::ofstream* out);
void if_id_op_id_jmp(tac_tokens operator_, variable_positions operand_1, variable_positions operand_2, std::string& label, std::ofstream* out);
void emit_unconditional_jump(const std::string& label, std::ofstream* out);
void emit_label(const std::string& label, std::ofstream* out);
void id_eq_const_op_const(tac_tokens operator_, variable_positions location, long operand_1, long operand_2, std::ofstream* out);
void id_eq_const(variable_positions location, long operand_1, std::ofstream* out);
void if_id_op_const_jmp(tac_tokens operator_, variable_positions operand_1, long operand_2, std::string& label,  std::ofstream* out);
#endif