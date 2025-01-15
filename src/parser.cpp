
#include "Lexer.hpp"
#include "Lexical_anal.hpp"
#include "Tokens.hpp"
#include "intermediate.hpp"
#include "utils.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
std::shared_ptr<Lexer> lex;              // lexer object
long current_line;      // current line being processed
std::shared_ptr<Token> lookahead;        // lookahead token
std::shared_ptr<Scope> top_level_scope = factory<Scope>(Scope{});  // the widest scope
long variable_offset{}; // the accumulated offset of all variables seen so far
TOKENS get_TOKENS(){
    TOKENS current;
    auto check_value = [&current](auto& T){current = T.token;};
    std::visit(check_value, *lookahead);
    return current;
}
bool is_comment(Token tok){
    if (std::holds_alternative<Basic_token>(tok) && std::get<Basic_token>(tok).token == TOKENS::COMMENT){
        return true;
    }
    else return false;
}
void error(const std::string& err) {throw std::runtime_error{"Error near line " + std::to_string(current_line) + ": " + err + '\n'};}
void move(){
    do {
    lookahead = lex->scan();
    current_line = lex->line;
    }while (is_comment(*lookahead));
    //lookahead = lex->scan();
    //cout << "Lexer moved the character is " << lex->get_character() << "now\n";
}
void
match(TOKENS tok){
    if (get_TOKENS() == tok) move();
    else error("Token mismatch in match, parser.cpp\n");//throw std::runtime_error{"Token mismatch in match, parser.cpp\n"};
}

void declarations();
Type type();
Type dimensions(Type tp);
st_shptr block();
st_shptr statements();
st_shptr statement();
st_shptr assign();
ex_shptr log_or();
ex_shptr log_and();
ex_shptr equality();
ex_shptr relational();
ex_shptr expression();
ex_shptr term();
ex_shptr unary();
ex_shptr factor();
ex_shptr array_access(Identifier id);

st_shptr program(std::shared_ptr<Lexer> lx){
    lex = lx;
    move();
    return block();
}

st_shptr block(){
    match(TOKENS::BRACKET_OPEN);
    //Scope saved_scope = top_level_scope; top_level_scope = Scope()
    auto saved_scope = top_level_scope; top_level_scope = factory<Scope>(Scope(top_level_scope));
    declarations();
    st_shptr stmts = statements();
    match(TOKENS::BRACKET_CLOSED);
    top_level_scope = saved_scope;
    return stmts;
}

void declarations(){
    while (get_TOKENS()%std::vector<TOKENS>{TOKENS::INT, TOKENS::FLOAT, TOKENS::CHAR, TOKENS::BOOL}){
        // int a; float b; char c; bool d;  //Obviously there is no assigning in declarations, hence no call to match(TOKENS::EUQUAL) between identifier, and semicolon
        Type tp = type(); auto temp_id = lookahead; match(TOKENS::IDENTIFIER); match(TOKENS::SEMICOLON);
        Identifier id{variable_offset, tp, std::get<Word>(*temp_id).lexeme};
        top_level_scope->put(std::get<Word>(*temp_id), id);
        top_level_scope->get(Word(std::get<Word>(*temp_id).token, std::get<Word>(*temp_id).lexeme));
        variable_offset += get_type_width(tp);
    }
}

Type type(){
    /* this expects to first encounter a basic type, needs to be expanded for function declarations
    this should not be too hard*/
    auto tok = get_TOKENS();
    Type tp;
    if (std::holds_alternative<Basic_token>(*lookahead)){
        //auto word = std::get<Word>(*lookahead);
        switch (tok) {
            case TOKENS::INT:
                tp = types::INTEGER;
                break;
            case TOKENS::FLOAT:
                tp = types::FLOAT;
                break;
            case TOKENS::CHAR:
                tp = types::CHAR;
                break;
            case TOKENS::BOOL:
                tp = types::BOOL;
                break;
            default:
                error("In type: Encountered non basic type from class Word, should not be possible\n");
        }
    }
    else{
        error("In type: Token is not Word, non-Word tokens cannot contain basic types\n");
        //return nullptr;
    }
    move();
    if (get_TOKENS() != TOKENS::SQUARE_BRACKET_OPEN) return tp;
    else return dimensions(tp);
}

Type dimensions(Type tp){
    match(TOKENS::SQUARE_BRACKET_OPEN); auto element_count = lookahead; match(TOKENS::INTEGRAL_LITERAL); match(TOKENS::SQUARE_BRACKET_CLOSED);
    if (get_TOKENS() == TOKENS::SQUARE_BRACKET_OPEN) tp = dimensions(tp);
    return factory<Array>(Array{tp, std::get<Number_integer>(*element_count).value});
}

// st_shptr statements(std::shared_ptr<Statements> parent = nullptr){
//     if (get_TOKENS() == TOKENS::SEMICOLON) return nullptr;
//     // recursively build right heavy ast
//     Statements(statement(), nullptr);
//     if (parent) 
//     // infinite recursion, try to coble another way
//     //else return factory<Statements>(Statements(statement(), statements()));
// }

st_shptr statements(){
    if (get_TOKENS() == TOKENS::BRACKET_CLOSED) return nullptr;
    else{
        auto temp = statement();
        auto temp2 = statements();
        return factory<Statements>(Statements(temp, temp2));
    }
}

st_shptr statement(){
    ex_shptr expression;
    st_shptr stmt, stmt2;
    // save enclosing statement for breaks
    st_shptr saved_statement;
    switch (get_TOKENS()) {
        case TOKENS::SEMICOLON:
            move();
            return nullptr;

        case TOKENS::IF:
            // if (expression)
            match(TOKENS::IF); match(TOKENS::PAREN_OPEN); expression = log_or(); match(TOKENS::PAREN_CLOSED);
            stmt = statement();
            if (get_TOKENS() != TOKENS::ELSE) return factory<If>(If(expression, stmt));
            match(TOKENS::ELSE);
            stmt2 = statement();
            return factory<Else>(Else(expression, stmt, stmt2));
        
        case TOKENS::WHILE:
            {
                st_shptr while_statemnt = factory<While>(While{});
                saved_statement = Statement::Enclosing_block;
                Statement::Enclosing_block = while_statemnt;
                match(TOKENS::WHILE); match(TOKENS::PAREN_OPEN); expression = log_or(); match(TOKENS::PAREN_CLOSED);
                stmt = statement();
                std::dynamic_pointer_cast<While>(while_statemnt)->setup(expression, stmt);
                Statement::Enclosing_block = saved_statement; // reset enclosing block
                return while_statemnt;
            }
        case TOKENS::DO:
            {
                st_shptr do_while_statement = factory<Do_while>(Do_while{});
                saved_statement = Statement::Enclosing_block;
                Statement::Enclosing_block = do_while_statement;
                match(TOKENS::DO);
                stmt = statement();
                match(TOKENS::WHILE); match(TOKENS::PAREN_OPEN); expression = log_or(); match(TOKENS::PAREN_CLOSED); match(TOKENS::SEMICOLON);
                std::dynamic_pointer_cast<Do_while>(do_while_statement)->setup(expression, stmt);
                Statement::Enclosing_block = saved_statement;
                return do_while_statement;
            }
        case TOKENS::BREAK:
            // break;
            match(TOKENS::BREAK); match(TOKENS::SEMICOLON);
            /* break can optionaly take the enoclosing block argument, but it implicitly
            does so in it's constructor function by acessing the static class member
            Statement::Enclosing_block from it's constructor*/
            return factory<Break>(Break());
        
        case TOKENS::BRACKET_OPEN:
            return block();
        default:
            // continue parsing down the hierarchy if nothing matches
            return assign();
    }
}

st_shptr assign(){
    st_shptr stmt;
    auto temp = lookahead;
    match(TOKENS::IDENTIFIER);
    auto id = top_level_scope->get(std::get<Word>(*temp));
    if (id){
        if (get_TOKENS() == TOKENS::ASSIGN){
            move(); stmt = factory<Assign>(Assign(*id, log_or()));
        }
        else {
            auto array_ac = *std::dynamic_pointer_cast<Array_acess>(array_access(*id));
            match(TOKENS::ASSIGN); stmt = factory<Assign_array>(Assign_array(array_ac, log_or()));
        }
        match(TOKENS::SEMICOLON);
        return  stmt;
    }
    error(std::get<Word>(*temp).lexeme + " is not declared before use\n");
    return nullptr;
}

ex_shptr log_or(){
    ex_shptr expr = log_and();
    while (get_TOKENS() == TOKENS::OR) {
        auto temp = get_TOKENS(); move(); expr = factory<Or>(Or(temp, expr, log_and()));
    }
    return expr;
}

ex_shptr log_and(){
    ex_shptr expr = equality();
    while (get_TOKENS() == TOKENS::AND){
        auto temp = get_TOKENS(); move(); expr = factory<And>(And(temp, expr, equality()));
    }
    return expr;
}

ex_shptr equality(){
    ex_shptr expr = relational();
    while(get_TOKENS() == TOKENS::EQUAL || get_TOKENS() == TOKENS::NOT_EQUAL){
        auto temp = get_TOKENS(); move(); expr = factory<Relational>(Relational(temp, expr, relational()));
    }
    return expr;
}

ex_shptr relational(){
    ex_shptr expr = expression();

    switch (get_TOKENS()) {
        case TOKENS::LESS_THAN:
        case TOKENS::LESS_THEN_EQUAL:
        case TOKENS::GREATER_THAN:
        case TOKENS::GREATER_THAN_EQUAL:
            {auto temp = get_TOKENS(); move(); return factory<Relational>(Relational(temp, expr, expression()));}
        default:
            return expr;
    }
}

ex_shptr expression(){
    ex_shptr expr = term();
    while (get_TOKENS() == TOKENS::ADD || get_TOKENS() == TOKENS::SUBTRACT){
        /* same as below, the difference being, this has a lower level of precedence, so the term is called first, since adition and subtraction is of smaller
        precedence than multiplication and division*/
        auto temp = get_TOKENS(); move(); expr = factory<Arithmetic>(Arithmetic(temp, expr, term()));
    }
    return expr;
}

ex_shptr term(){
    ex_shptr expr = unary();
    while (get_TOKENS() == TOKENS::MULTIPLY || get_TOKENS() == TOKENS::DIVIDE){
        /* recursively build the ast with the above expressoins as root, this function will return in a general case either a node
        single node, or a subtree*/
        auto temp = get_TOKENS(); move(); expr = factory<Arithmetic>(Arithmetic(temp, expr, unary()));
    }
    return expr;
}

ex_shptr unary(){
    if (get_TOKENS() == TOKENS::UNARY_MINUS){
        /* unary is passed as expression, since you can by definition (source grammar), only go down the hierachy, paranthesis surpass this,
        they jump back up the hierarchy, but unary operations cannot, hence unary is called as second argument to the constructor*/
        move(); return factory<Unary_operations>(Unary_operations{TOKENS::UNARY_MINUS, unary()});
    }
    else if (get_TOKENS() == TOKENS::LOGICAL_NOT){
        auto temp = get_TOKENS(); move(); return factory<Not>(Not(temp, unary()));
    }
    else return factor();
}

ex_shptr factor(){
    ex_shptr temp;
    TOKENS current{get_TOKENS()};
    switch(current){
        case TOKENS::PAREN_OPEN:
            move(); temp = log_or(); match(TOKENS::PAREN_CLOSED);
            return temp;
        case TOKENS::INTEGRAL_LITERAL:
            // make an integer constant,
            temp = factory<Constant<int>>(Constant<int>(std::get<Number_integer>(*lookahead).value));
            move(); return temp;
        case TOKENS::DECIMAL_LITERAL:
            // this should also work since constructor takes only one argument
            temp = factory<Constant<float>>(std::get<Number_float>(*lookahead).value); move(); return temp;
        case TOKENS::TRUE:
            temp = factory<Constant<bool>>(true); move(); return temp;
        case TOKENS::FALSE:
            temp = factory<Constant<bool>>(false); move(); return temp;
        case TOKENS::IDENTIFIER:
            if (auto id = top_level_scope->get(std::get<Word>(*lookahead))){
                move();
                current = get_TOKENS();
                if (current != TOKENS::SQUARE_BRACKET_OPEN) return factory<Identifier>(*id);
                else return array_access(*id);
            }
            else throw std::runtime_error{std::get<Word>(*lookahead).lexeme + " is undeclared\n"};
        default:
            error("Syntax error");
            return nullptr;
    }
}

ex_shptr array_access(Identifier id){
    ex_shptr index, type_width, temp1, temp2, location;
    Type type = id.type;
    match(TOKENS::SQUARE_BRACKET_OPEN); index = log_or(); match(TOKENS::SQUARE_BRACKET_CLOSED);
    type = std::get<Array_type>(type)->type;
    // get the underlying type, so we can do offsets;
    // constant/number of bytes of type, used to multiply by index to get offset of array
    int const_mult = std::get<Basic_type>(type).second;
    // this is the width of the type by which to multiply index, it is a constant 
    type_width = factory<Constant<int>>(Constant<int>(const_mult));
    // This temporary should be synthesized in the three adress code, used to calculate the aforementioned offset
    temp1 = factory<Arithmetic>(Arithmetic(TOKENS::MULTIPLY, index, type_width));

    Array_acess ac{factory<Identifier>(id), temp1, type};
    return factory<Array_acess>(ac);
}