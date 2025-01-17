#include "parse_TAC.hpp"
#include <algorithm>
#include <any>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "generator.hpp"
#include "intermediate.hpp"
std::pair<tac_tokens, std::any> lookahaed_tac;

std::any variable_info;

std::any output_file;

void set_var_info(std::vector<variable_positions>& vars){
    std::any_cast<std::vector<variable_positions>&>(variable_info) = vars;
    // avoid copying vector i hope
}

void set_output_file(std::ofstream& out){
    output_file = &out;
}
std::ofstream* get_output_file(){
    return std::any_cast<std::ofstream*>(output_file);
}
variable_positions get_variable_pos(std::string& lexeme){
    for (auto var : std::any_cast<std::vector<variable_positions>&>(variable_info)){
        if (var.lexeme == lexeme){
            return var;
        }
    }
    throw std::runtime_error{"Parse tac, could not find the variable in variable info\n"};
}

Tac_parser parser;
void move_tac(){
    lookahaed_tac = parser.scan();
}
void match_tac(tac_tokens tok){
    if (lookahaed_tac.first == tok) move_tac(); 
    else throw std::runtime_error{"Error in parse_TAC match failed\n"};
}
std::vector<tac_tokens> operators{tac_tokens::add, tac_tokens::sub, tac_tokens::div, tac_tokens::mult};
std::vector<tac_tokens> relational_operators{tac_tokens::greater_than, tac_tokens::greater_than_equal, tac_tokens::less_than, tac_tokens::less_than_equal, tac_tokens::equal};
    void choose_command(){
        
        //auto move = [&](){lookahaed = scan();};

        // id = id copy
        // id = id op id
        // id = id [ id ]
        // id = id op const
        // id = cons op id
        // id = cont
        // label

        //auto parse_line = [&](){if ()};
    }

bool operator%(tac_tokens needle, std::vector<tac_tokens> haystack){
    if (std::find(haystack.begin(), haystack.end(), needle) != haystack.end()) return true;
    return false;
}

void init_tac_parser(){
    move_tac();
}

void parse_line(){
    std::string id_1, id2, id3, test;
    long const1, const2;
    tac_tokens operator_;
    if (lookahaed_tac.first == tac_tokens::id){
        // id
        id_1 = std::any_cast<std::string>(lookahaed_tac.second);
        move_tac();
        if (lookahaed_tac.first == tac_tokens::assign){
            // id = 
            move_tac();
            if (lookahaed_tac.first == tac_tokens::id){
                // id = id
                id2 = std::any_cast<std::string>(lookahaed_tac.second);
                move_tac();
                if (lookahaed_tac.first%operators){
                    // id = id op
                    move_tac();
                    operator_ = lookahaed_tac.first;
                    if (lookahaed_tac.first == tac_tokens::id){
                        // id = id op id
                        // call three id function
                        id3 = std::any_cast<std::string>(lookahaed_tac.second);
                        id_eq_id_op_id(operator_, get_variable_pos(id_1), get_variable_pos(id2), get_variable_pos(id3), get_output_file());
                    }
                    else if (lookahaed_tac.first == tac_tokens::const_) {
                        // id = id op const
                        // call id = id op const func
                        const1 = std::any_cast<long>(lookahaed_tac.second);
                        id_eq_id_op_const(operator_, get_variable_pos(id_1), get_variable_pos(id2), const1, get_output_file());
                    }
                }
                else if(lookahaed_tac.first == tac_tokens::square_bracket_open){
                    // always id, because the index has to be multibplied by width
                    move_tac();
                    id3 = std::any_cast<std::string>(lookahaed_tac.second);
                    move_tac();
                    match_tac(tac_tokens::square_bracket_closed);
                    id_eq_id_indexec_by_id(get_variable_pos(id_1), get_variable_pos(id2), get_variable_pos(id3), get_output_file());
                }
                // id = id
                // since above did not match, this is the assignment id = id
                // call copy function
            }
            else if (lookahaed_tac.first == tac_tokens::const_) {
                // id = const
                const1 = std::any_cast<long>(lookahaed_tac.second);
                move_tac();
                if (lookahaed_tac.first%operators){
                    // id = const op
                    operator_ = lookahaed_tac.first;
                    move_tac();
                    if (lookahaed_tac.first == tac_tokens::id){
                        // id = const op id
                        // call id = const op id function
                        id2 = std::any_cast<std::string>(lookahaed_tac.second);
                        id_eq_id_op_const(operator_, get_variable_pos(id_1), get_variable_pos(id2), const1, get_output_file());
                        move_tac();
                    }
                    else if(lookahaed_tac.first == tac_tokens::const_){
                        // id = const op const
                        // call id = const op const function
                        const2 = std::any_cast<long>(lookahaed_tac.second);
                        id_eq_const_op_const(operator_, get_variable_pos(id_1), const1, const2, get_output_file());
                        move_tac();
                    }
                }
                //id = const
                // call const assign function
                id_eq_const(get_variable_pos(id_1), const1,get_output_file());
            }
        }
    }
    else if (lookahaed_tac.first == tac_tokens::label) {
        // call emit_label_function
        emit_label(std::any_cast<std::string>(lookahaed_tac.second), get_output_file());
        move_tac();
    }
    else if (lookahaed_tac.first == tac_tokens::jump){
        // call goto fuction
        move_tac();
        std::string label = std::any_cast<std::string>(lookahaed_tac.second);
        emit_unconditional_jump(label, get_output_file());
        move_tac();
    }
    else if (lookahaed_tac.first == tac_tokens::if_){
        // if
        move_tac();
        if (lookahaed_tac.first == tac_tokens::id){
            // if id
            id_1 = std::any_cast<std::string>(lookahaed_tac.second);
            move_tac();
            if (lookahaed_tac.first%relational_operators){
                // if id op
                operator_ = lookahaed_tac.first;
                move_tac();
                if (lookahaed_tac.first == tac_tokens::id){
                    // if id op id
                    id2 = std::any_cast<std::string>(lookahaed_tac.second);
                    move_tac();
                    match_tac(tac_tokens::jump); 
                    std::string label = std::any_cast<std::string>(lookahaed_tac.second);
                    if_id_op_id_jmp(operator_, get_variable_pos(id_1), get_variable_pos(id2), label, get_output_file());
                    match_tac(tac_tokens::label);
                }
                else if (lookahaed_tac.first == tac_tokens::const_){
                    // if id op const
                    const1 = std::any_cast<long>(lookahaed_tac.second);
                    //if_id_op_const_jmp(operator_, get_variable_pos(id_1), const1, 0, get_output_file());
                    match_tac(tac_tokens::jump); match_tac(tac_tokens::label);
                }
                else throw std::runtime_error{"In parsing of if condittional jump, id not compared with anything\n"};
            }
        }
        else if (lookahaed_tac.first == tac_tokens::const_){
            // if const
            move_tac();
            if (lookahaed_tac.first%relational_operators){
                // if const op
                move_tac();
                if (lookahaed_tac.first == tac_tokens::id){
                    // if const op id
                    match_tac(tac_tokens::jump); match_tac(tac_tokens::label);
                }
                else if (lookahaed_tac.first == tac_tokens::const_) {
                    // if const op const
                    match_tac(tac_tokens::jump); match_tac(tac_tokens::label);
                }
                else throw std::runtime_error{"In parsing of if condittional jump, const not compared with anything\n"};
            }
        }
    }
    else if (lookahaed_tac.first == tac_tokens::if_false) {
        // iffalse
        move_tac();
        if (lookahaed_tac.first == tac_tokens::id){
            // iffalse id
            move_tac();
            if (lookahaed_tac.first%relational_operators){
                // iffalse id op
                move_tac();
                if(lookahaed_tac.first == tac_tokens::id){
                    // iffalse id op id
                    match_tac(tac_tokens::jump); match_tac(tac_tokens::label);
                }
                else if(lookahaed_tac.first == tac_tokens::const_){
                    // iffalse id op const
                    match_tac(tac_tokens::jump); match_tac(tac_tokens::label);
                }
            }
        }
        else if (lookahaed_tac.first == tac_tokens::const_) {
            //iffalse const
            move_tac();
            if (lookahaed_tac.first%relational_operators){
                // iffalse const op
                move_tac();
                if(lookahaed_tac.first == tac_tokens::id){
                    // iffalse const op id
                    match_tac(tac_tokens::jump); match_tac(tac_tokens::label);
                }
                else if(lookahaed_tac.first == tac_tokens::const_){
                    // iffalse const op const
                    match_tac(tac_tokens::jump); match_tac(tac_tokens::label);
                }
            }
        }
    }
}