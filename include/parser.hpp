#ifndef PARSER_HEADER
#define PARSER_HEADER
#include "Lexer.hpp"
#include "intermediate.hpp"
#include <memory>
st_shptr program(std::shared_ptr<Lexer> lx);
std::shared_ptr<Scope> get_scope();
#endif