#ifndef TOKENS_HEADER
#define TOKENS_HEADER 
#include <string>
#include <unordered_map>
enum class TOKENS{
    // keywords
    WHILE, FOR, IF, ELSE, RETURN, BREAK, DO,
    // types
    INT, FLOAT, CHAR, STRING, BOOL,
    // Literals
    INTEGRAL_LITERAL, DECIMAL_LITERAL, STRING_LITERAL, TRUE, FALSE, 
    // OPERATORS
    ADD, SUBTRACT, MULTIPLY, DIVIDE, 
    // Relational operators
    EQUAL, LESS_THAN, GREATER_THAN, LESS_THEN_EQUAL, GREATER_THAN_EQUAL, NOT_EQUAL,
    // Logical operators
    AND, OR, // not is a special case in the unary ones
    // Unary operators
    UNARY_MINUS, LOGICAL_NOT,
    // LANGUAGE SYMBOLS
    ASSIGN, PAREN_OPEN, PAREN_CLOSED, BRACKET_OPEN, BRACKET_CLOSED, SQUARE_BRACKET_OPEN, SQUARE_BRACKET_CLOSED, DOT, SEMICOLON,
    // Other
    IDENTIFIER, COMMENT, EOF_TOK

};

const std::unordered_map<TOKENS, const std::string> token_string_map{
    {TOKENS::WHILE, "while"}, {TOKENS::FOR, "for"}, {TOKENS::IF, "if"}, {TOKENS::ELSE, "else"}, {TOKENS::RETURN, "return"}, {TOKENS::BREAK, "break"}, {TOKENS::DO, "do"},
    {TOKENS::INT, "int"}, {TOKENS::FLOAT, "float"}, {TOKENS::CHAR, "char"}, {TOKENS::STRING, "string"}, {TOKENS::BOOL, "bool"},
    {TOKENS::INTEGRAL_LITERAL, "integer_literal"}, {TOKENS::DECIMAL_LITERAL, "floating_point_literal"}, {TOKENS::STRING_LITERAL, "string_literal"}, {TOKENS::TRUE, "true"}, {TOKENS::FALSE, "false"},
    {TOKENS::ADD, "+"}, {TOKENS::SUBTRACT, "-"}, {TOKENS::MULTIPLY, "*"}, {TOKENS::DIVIDE, "/"},
    {TOKENS::EQUAL, "=="}, {TOKENS::LESS_THAN, "<"}, {TOKENS::GREATER_THAN, ">"}, {TOKENS::LESS_THEN_EQUAL, "<="}, {TOKENS::GREATER_THAN_EQUAL, ">="}, {TOKENS::NOT_EQUAL, "!="},
    {TOKENS::AND, "&&"}, {TOKENS::OR, "||"},
    {TOKENS::UNARY_MINUS, "-(unray)"}, {TOKENS::LOGICAL_NOT, "!"},
    {TOKENS::ASSIGN, "="}, {TOKENS::PAREN_OPEN, "("}, {TOKENS::PAREN_CLOSED, ")"}, {TOKENS::BRACKET_OPEN, "{"}, {TOKENS::BRACKET_CLOSED, "}"}, {TOKENS::SQUARE_BRACKET_OPEN, "["}, {TOKENS::SQUARE_BRACKET_CLOSED, "]"}, {TOKENS::DOT, "."}, {TOKENS::SEMICOLON, ";"},
    {TOKENS::IDENTIFIER, "ID"}, {TOKENS::COMMENT, "//(comment)"}, {TOKENS::EOF_TOK, "End_of_file"}
};
const std::unordered_map<std::string, TOKENS> string_to_type_map{
    {"int", TOKENS::INT}, {"float", TOKENS::FLOAT}, {"char", TOKENS::CHAR}, {"bool", TOKENS::BOOL}, {"string", TOKENS::STRING}
};
inline std::string find_string_repr(const TOKENS tok){
    auto it = token_string_map.find(tok);
    if (it != token_string_map.end())
        return it->second;
    return "No match found";
}
#endif